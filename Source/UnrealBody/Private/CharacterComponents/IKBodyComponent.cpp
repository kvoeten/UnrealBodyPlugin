/*
*   This file is part of the Unreal Body Plugin by Kaz Voeten.
*   Copyright (C) 2021 Kaz Voeten
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CharacterComponents/IKBodyComponent.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogIKBodyComponent);

// Sets default values for this component's properties
UIKBodyComponent::UIKBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = .01f;
	SetNetAddressable();
	SetIsReplicated(true);
}

void UIKBodyComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UIKBodyComponent, MovementThreshold);
	DOREPLIFETIME(UIKBodyComponent, RotationThreshold);
	DOREPLIFETIME(UIKBodyComponent, BodyRotationOffset);
	DOREPLIFETIME(UIKBodyComponent, PlayerHeight);
	DOREPLIFETIME(UIKBodyComponent, BodyOffset);
}

void UIKBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Body != nullptr && Camera != nullptr)
	{
		// Detach the body from it's parent so it doesn't automatically move with the players head movement.
		static FDetachmentTransformRules DetachRules 
			= FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
		Body->DetachFromComponent(DetachRules);

		// Set body at camera position + offsets
		this->BodyTargetLocation = Camera->GetComponentLocation()
			+ (UKismetMathLibrary::GetForwardVector(Camera->GetComponentRotation()) * BodyOffset); // 20 units back from cam to avoid clipping
		this->BodyTargetLocation.Z -= this->PlayerHeight;
		this->Body->SetWorldLocation(this->BodyTargetLocation);

		// Rotate Body to match
		this->BodyTargetRotation.Yaw = Camera->GetComponentRotation().Yaw + this->BodyRotationOffset;
		this->Body->SetWorldRotation(this->BodyTargetRotation);

		// Set current position to match target
		this->BodyCurrentLocation = this->BodyTargetLocation;
		this->BodyCurrentRotation = this->BodyTargetRotation; 
				
		UE_LOG(LogIKBodyComponent, Log, TEXT("Succesfully initialized with body and camera!"));
	}
	else
	{
		// Body should be assigned in component owner's construction script!
		UE_LOG(LogIKBodyComponent, Warning, TEXT("Please ensure a body and camera reference are set before BeginPlay by setting it in the owner's construction script."));
	}
}

void UIKBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Body != nullptr && Camera != nullptr)
	{
		this->TickBodyMovement(DeltaTime);
		this->TickFingerIK(DeltaTime);
	}
}

void UIKBodyComponent::TickBodyMovement(float DeltaTime)
{
	FTransform CameraCurrentPosition = this->Camera->GetComponentTransform();

	// Calculate the XY distance moved
	float DistanceMoved = FVector::Distance(
		FVector(CameraCurrentPosition.GetLocation().X, CameraCurrentPosition.GetLocation().Y, 0),
		FVector(LastCameraPosition.GetLocation().X, LastCameraPosition.GetLocation().Y, 0)
	);

	// Calculate Yaw difference
	float YawDifference = UKismetMathLibrary::Abs(
		CameraCurrentPosition.GetRotation().Rotator().Yaw - LastCameraPosition.GetRotation().Rotator().Yaw
	);

	// Update the camera position and movement if the player moved further away than threshold (enables leaning/ head tilt without moving the body)
	if (DistanceMoved > this->MovementThreshold)
	{
		// Set new body target location
		this->BodyTargetLocation = CameraCurrentPosition.GetLocation() 
			+ (UKismetMathLibrary::GetForwardVector(CameraCurrentPosition.GetRotation().Rotator()) * BodyOffset); // 20 units back from cam to avoid clipping

		// Update movement speed and direction
		this->MovementDirection = GetMovementDirection(&LastCameraPosition, &CameraCurrentPosition);
		this->MovementSpeed = UKismetMathLibrary::FFloor((DistanceMoved / DeltaTime) / 1000);

		// Save new position
		this->LastCameraPosition = CameraCurrentPosition;
	}

	// Apply new rotation to the body if turned far enough (allows head turning without rotating the whole body)
	if (YawDifference > this->RotationThreshold)
	{
		this->BodyTargetRotation.Yaw = Camera->GetComponentRotation().Yaw + this->BodyRotationOffset;
		this->MovementDirection = YawDifference;
	}

	// If the body hasn't reached it's target location yet we move it towards it.
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentLocation.X, BodyTargetLocation.X, 9.99997f) 
		&& UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentLocation.Y, BodyTargetLocation.Y, 9.99997f))
	{
		this->MovementSpeed = 0;
		this->MovementDirection = 0;
	}
	else
	{
		// Tick towards location based on movement speed
		this->BodyCurrentLocation = UKismetMathLibrary::VInterpTo(this->BodyCurrentLocation, this->BodyTargetLocation, DeltaTime, MovementSpeed);
		this->Body->SetWorldLocation(BodyCurrentLocation);
	}

	// If the body hasn't reached it's target rotation yet we interp towards it.
	if (!UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentRotation.Yaw, BodyTargetRotation.Yaw, 9.99997f))
	{
		BodyCurrentRotation.Yaw = UKismetMathLibrary::FInterpTo(BodyCurrentRotation.Yaw, BodyTargetRotation.Yaw, DeltaTime, UKismetMathLibrary::Max(2.0f, MovementSpeed));
		this->Body->SetWorldRotation(BodyCurrentRotation);
	}
	else this->MovementDirection = 0;

	// Always set Z to enable seamless crouching
	FVector BodyLocation = FVector(BodyCurrentLocation.X, BodyCurrentLocation.Y, CameraCurrentPosition.GetLocation().Z - this->PlayerHeight);
	this->Body->SetWorldLocation(BodyLocation);

	UE_LOG(LogIKBodyComponent, Warning, TEXT("MovementSpeed: %f"), this->MovementSpeed);
}

/*
 * Sets the body target position to the camera position, rotating and moving it down so that the character eyes match the HMD position
*/
void UIKBodyComponent::SetBodyTargetPosition(FTransform* CameraTransform)
{
	this->BodyTargetRotation.Yaw = CameraTransform->GetRotation().Z + this->BodyRotationOffset;
	this->BodyTargetLocation = CameraTransform->GetLocation() + (UKismetMathLibrary::GetRightVector(this->BodyTargetRotation) * -20);
	this->BodyTargetLocation.Z = BodyTargetLocation.Z - this->PlayerHeight;
}

/*
 * Find the (shortest) angle in degrees between two transforms on the XY axis
 * Huge thanks to Eprim at https://answers.unrealengine.com/ for showing a neat trick to resolve quaternion results
*/
float UIKBodyComponent::GetMovementDirection(FTransform* First, FTransform* Second)
{
	float Rotation = 0.0f;
	FVector Axis;

	// Get rotational vecotors
	FVector FirstRotVec = First->GetRotation().Vector();
	FVector SecondRotVec = Second->GetRotation().Vector();

	// Find quat and angle
	const auto Quaternion = FQuat::FindBetweenNormals(FirstRotVec, SecondRotVec);
	Quaternion.ToAxisAndAngle(Axis, Rotation);

	// Convert to degrees and use axis.Z as left-right direction control to constrain angle between -180 and 180
	return Axis.Z * FMath::RadiansToDegrees(Rotation);
}

// Helper function that resets the Finger states of the given hand.
void UIKBodyComponent::ResetHandFingers(ECharacterIKHand Hand)
{
	switch (Hand)
	{
	case ECharacterIKHand::Left:
		*FingerStates.StateMap.Find(EFingerBone::index_01_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::index_02_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::index_03_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_01_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_02_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_03_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_01_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_02_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_03_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_01_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_02_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_03_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_01_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_02_l) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_03_l) = false;
		break;
	case ECharacterIKHand::Right:
		*FingerStates.StateMap.Find(EFingerBone::index_01_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::index_02_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::index_03_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_01_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_02_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::middle_03_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_01_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_02_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::ring_03_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_01_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_02_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::pinky_03_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_01_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_02_r) = false;
		*FingerStates.StateMap.Find(EFingerBone::thumb_03_r) = false;
		break;
	}
}

void UIKBodyComponent::StartFingerIK(AActor* Target, ECharacterIKHand Hand)
{
	if (Target == nullptr)
		return;

	switch (Hand)
	{
	case ECharacterIKHand::Left:
		this->LeftGrip = Target;
		this->ResetHandFingers(Hand);
		break;
	case ECharacterIKHand::Right:
		this->RightGrip = Target;
		this->ResetHandFingers(Hand);
		break;
	}
}

void UIKBodyComponent::StopFingerIK(ECharacterIKHand Hand)
{
	switch (Hand)
	{
	case ECharacterIKHand::Left:
		this->LeftGrip = nullptr;
		this->ResetHandFingers(Hand);
		break;
	case ECharacterIKHand::Right:
		this->RightGrip = nullptr;
		this->ResetHandFingers(Hand);
		break;
	}
}

void UIKBodyComponent::TickFingerIK(float DeltaTime)
{
	for (auto& Element : FingerStates.StateMap)
	{
		const EFingerBone Bone = (EFingerBone) Element.Key;
		const bool Finished = (bool) Element.Value;

		if (!Finished)
		{
			AActor* GripTarget = nullptr;
			switch (Bone)
			{
			case EFingerBone::index_01_l:
			case EFingerBone::index_02_l:
			case EFingerBone::index_03_l:
			case EFingerBone::middle_01_l:
			case EFingerBone::middle_02_l:
			case EFingerBone::middle_03_l:
			case EFingerBone::ring_01_l:
			case EFingerBone::ring_02_l:
			case EFingerBone::ring_03_l:
			case EFingerBone::pinky_01_l:
			case EFingerBone::pinky_02_l:
			case EFingerBone::pinky_03_l:
			case EFingerBone::thumb_01_l:
			case EFingerBone::thumb_02_l:
			case EFingerBone::thumb_03_l:
				GripTarget = LeftGrip;
				break;
			case EFingerBone::index_01_r:
			case EFingerBone::index_02_r:
			case EFingerBone::index_03_r:
			case EFingerBone::middle_01_r:
			case EFingerBone::middle_02_r:
			case EFingerBone::middle_03_r:
			case EFingerBone::ring_01_r:
			case EFingerBone::ring_02_r:
			case EFingerBone::ring_03_r:
			case EFingerBone::pinky_01_r:
			case EFingerBone::pinky_02_r:
			case EFingerBone::pinky_03_r:
			case EFingerBone::thumb_01_r:
			case EFingerBone::thumb_02_r:
			case EFingerBone::thumb_03_r:
				GripTarget = RightGrip;
			}

			float TargetAlpha = GripTarget == nullptr ? 0 : 1.0f;
			float* CurrentAlpha = this->FingerIKValues.BlendMap.Find(Bone);
			
			// Check if ptr to capsule ptr is valid, and then get capsule ptr
			UCapsuleComponent** CapsulePtrPtr = this->FingerHitboxes.Find(Bone);
			UCapsuleComponent* Capsule = CapsulePtrPtr == nullptr ? nullptr : *CapsulePtrPtr;
			
			if (*CurrentAlpha == TargetAlpha)
			{
				// Target is already reached since moving previous tick
				*this->FingerStates.StateMap.Find(Bone) = true;
				continue; // Skip to next bone
			}
			
			else if (Capsule != nullptr && GripTarget != nullptr)
			{
				// Check if capsule is colliding with target actor since being moved previous tick
				TArray <FOverlapInfo> OverlapInfo;
				if (Capsule->GetOverlapsWithActor(GripTarget, OverlapInfo))
				{
					*this->FingerStates.StateMap.Find(Bone) = true;
					continue; // Skip to next bone
				}
			}
			
			// Finterp to target
			*CurrentAlpha = UKismetMathLibrary::FInterpTo(*CurrentAlpha, TargetAlpha, DeltaTime, 4.0f);
		}
	}
}

void UIKBodyComponent::UpdateMovementThreshold_Implementation(float Value) { this->MovementThreshold = Value; }

void UIKBodyComponent::UpdateRotationThreshold_Implementation(float Value) { this->RotationThreshold = Value; }

void UIKBodyComponent::UpdatePlayerHeight_Implementation(float Value) { this->PlayerHeight = Value; }

void UIKBodyComponent::UpdateBodyOffset_Implementation(float Value) { this->BodyOffset = Value; }

void UIKBodyComponent::SetAllHitBoxes(
	UCapsuleComponent* index_01_l,
	UCapsuleComponent* index_02_l,
	UCapsuleComponent* index_03_l,
	UCapsuleComponent* middle_01_l,
	UCapsuleComponent* middle_02_l,
	UCapsuleComponent* middle_03_l,
	UCapsuleComponent* ring_01_l,
	UCapsuleComponent* ring_02_l,
	UCapsuleComponent* ring_03_l,
	UCapsuleComponent* pinky_01_l,
	UCapsuleComponent* pinky_02_l,
	UCapsuleComponent* pinky_03_l,
	UCapsuleComponent* thumb_01_l,
	UCapsuleComponent* thumb_02_l,
	UCapsuleComponent* thumb_03_l,
	UCapsuleComponent* index_01_r,
	UCapsuleComponent* index_02_r,
	UCapsuleComponent* index_03_r,
	UCapsuleComponent* middle_01_r,
	UCapsuleComponent* middle_02_r,
	UCapsuleComponent* middle_03_r,
	UCapsuleComponent* ring_01_r,
	UCapsuleComponent* ring_02_r,
	UCapsuleComponent* ring_03_r,
	UCapsuleComponent* pinky_01_r,
	UCapsuleComponent* pinky_02_r,
	UCapsuleComponent* pinky_03_r,
	UCapsuleComponent* thumb_01_r,
	UCapsuleComponent* thumb_02_r,
	UCapsuleComponent* thumb_03_r
)
{
	this->FingerHitboxes.Empty();
	this->FingerHitboxes.Add(EFingerBone::index_01_l, index_01_l);
	this->FingerHitboxes.Add(EFingerBone::index_02_l, index_02_l);
	this->FingerHitboxes.Add(EFingerBone::index_03_l, index_03_l);
	this->FingerHitboxes.Add(EFingerBone::middle_01_l, middle_01_l);
	this->FingerHitboxes.Add(EFingerBone::middle_02_l, middle_02_l);
	this->FingerHitboxes.Add(EFingerBone::middle_03_l, middle_03_l);
	this->FingerHitboxes.Add(EFingerBone::ring_01_l, ring_01_l);
	this->FingerHitboxes.Add(EFingerBone::ring_02_l, ring_02_l);
	this->FingerHitboxes.Add(EFingerBone::ring_03_l, ring_03_l);
	this->FingerHitboxes.Add(EFingerBone::pinky_01_l, pinky_01_l);
	this->FingerHitboxes.Add(EFingerBone::pinky_02_l, pinky_02_l);
	this->FingerHitboxes.Add(EFingerBone::pinky_03_l, pinky_03_l);
	this->FingerHitboxes.Add(EFingerBone::thumb_01_l, thumb_01_l);
	this->FingerHitboxes.Add(EFingerBone::thumb_02_l, thumb_02_l);
	this->FingerHitboxes.Add(EFingerBone::thumb_03_l, thumb_03_l);
	this->FingerHitboxes.Add(EFingerBone::index_01_r, index_01_r);
	this->FingerHitboxes.Add(EFingerBone::index_02_r, index_02_r);
	this->FingerHitboxes.Add(EFingerBone::index_03_r, index_03_r);
	this->FingerHitboxes.Add(EFingerBone::middle_01_r, middle_01_r);
	this->FingerHitboxes.Add(EFingerBone::middle_02_r, middle_02_r);
	this->FingerHitboxes.Add(EFingerBone::middle_03_r, middle_03_r);
	this->FingerHitboxes.Add(EFingerBone::ring_01_r, ring_01_r);
	this->FingerHitboxes.Add(EFingerBone::ring_02_r, ring_02_r);
	this->FingerHitboxes.Add(EFingerBone::ring_03_r, ring_03_r);
	this->FingerHitboxes.Add(EFingerBone::pinky_01_r, pinky_01_r);
	this->FingerHitboxes.Add(EFingerBone::pinky_02_r, pinky_02_r);
	this->FingerHitboxes.Add(EFingerBone::pinky_03_r, pinky_03_r);
	this->FingerHitboxes.Add(EFingerBone::thumb_01_r, thumb_01_r);
	this->FingerHitboxes.Add(EFingerBone::thumb_02_r, thumb_02_r);
	this->FingerHitboxes.Add(EFingerBone::thumb_03_r, thumb_03_r);
}