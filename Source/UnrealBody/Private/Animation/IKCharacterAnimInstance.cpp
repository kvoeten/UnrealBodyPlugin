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

#include "Animation/IKCharacterAnimInstance.h"
#include "Library/AnimationStructLibrary.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogIKBodyAnimation);

void UIKCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* PawnOwner = TryGetPawnOwner();
	if (PawnOwner != nullptr)
	{
		// Set Character Reference
		this->Character = PawnOwner;
	}

	else UE_LOG(LogIKBodyAnimation, Warning, TEXT("Unable to get pawn owner!"));
}

void UIKCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!Character || DeltaSeconds == 0.0f)
	{
		return;
	}

	this->BodyComponent = (UIKBodyComponent*)Character->GetComponentByClass(UIKBodyComponent::StaticClass());
	if (this->BodyComponent != nullptr)
	{
		UpdateHandValues();
		UpdateHeadValues();
		UpdateMovementValues();
		UpdateFingerIKValues();
	}
	else UE_LOG(LogIKBodyAnimation, Warning, TEXT("Pawn owner has no IKBodyComponent"));

	// Feet IK doesn't need any component references
	UpdateFootIK();
}

void UIKCharacterAnimInstance::UpdateFootIK()
{
	// Get actor socket locations
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	FVector LeftFoot = OwnerComp->GetSocketLocation("foot_l");
	FVector RightFoot = OwnerComp->GetSocketLocation("foot_r");
	
	// Establish trace context
	UWorld* World = GetWorld();
	check(World);

	// Filter out player from hits
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// Debug Draw
	const FName TraceTag("FeetTraces");
	World->DebugDrawTraceTag = TraceTag;
	Params.TraceTag = TraceTag;

	// Trace both feet and set result in AnimGraph
	TraceFoot(LeftFoot, &FootIKValues.LeftFootLocation, 
		&FootIKValues.LeftFootRotation, World, &Params);
	TraceFoot(RightFoot, &FootIKValues.RightFootLocation,
		&FootIKValues.RightFootRotation, World, &Params);
}

void UIKCharacterAnimInstance::TraceFoot(FVector Foot, FVector* ResultLocation, 
	FRotator* ResultRotation, UWorld* World, FCollisionQueryParams* Params)
{
	// Establish trace start & end point
	const float ZRoot = GetOwningComponent()->GetComponentLocation().Z;
	const FVector Start = FVector(Foot.X, Foot.Y, ZRoot + 60);
	const FVector End = FVector(Foot.X, Foot.Y, ZRoot);

	// Trace
	FHitResult HitResult; // Establish Hit Result
	World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, *Params);

	// Check for hit
	if (HitResult.bBlockingHit)
	{
		const FVector ImpactLocation = HitResult.Location;
		const FVector ImpactNormal = HitResult.ImpactNormal;

		if (ImpactLocation.Z > ZRoot)
		{
			// Create additive vector for Z position
			ResultLocation->X = UKismetMathLibrary::Abs(ImpactLocation.Z - ZRoot);

			// Calculate and set new rotation
			ResultRotation->Roll = UKismetMathLibrary::Atan2(ImpactNormal.Y, ImpactNormal.Z);
			ResultRotation->Pitch = UKismetMathLibrary::Atan2(ImpactNormal.X, ImpactNormal.Z);
			
			// Return to confirm settings
			return;
		}

		// Maintain the settings from the previous tick
		return;
	}

	// Set defaults if no trace success
	ResultLocation->X = 0;
	ResultRotation->Roll = 0;
	ResultRotation->Pitch = 0;
}

void UIKCharacterAnimInstance::UpdateHeadValues()
{
	// Simply set head values to match camera at all times.
	HeadIKValues.HeadRotation = this->BodyComponent->Camera->GetComponentRotation();
	HeadIKValues.HeadLocation = this->BodyComponent->Camera->GetComponentLocation();

	// Apply the same offset as the component does
	HeadIKValues.HeadLocation += (UKismetMathLibrary::GetForwardVector(HeadIKValues.HeadRotation) * this->BodyComponent->BodyOffset);
}

void UIKCharacterAnimInstance::UpdateHandValues()
{
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();

	// Get controller transform * offset
	ArmIKValues.LeftTargetTransform = this->BodyComponent->LeftController->GetComponentTransform();
	ArmIKValues.RightTargetTransform = this->BodyComponent->RightController->GetComponentTransform();
}

void UIKCharacterAnimInstance::UpdateMovementValues()
{
	MovementValues.Speed = this->BodyComponent->MovementSpeed;
	MovementValues.Direction = this->BodyComponent->MovementDirection;
}

void UIKCharacterAnimInstance::UpdateFingerIKValues()
{
	// This is probably kinda inefficient, gotta find a way to directly access the FingerIKBlendmap with a pointer to replace it's value. 
	// Direct ref to anim? (cross dep tho)
	for (const TPair<EFingerBone, float>& pair : this->BodyComponent->FingerIKValues.BlendMap)
	{
		const EFingerBone Bone = pair.Key;
		const float Alpha = pair.Value;
		*this->FingerIKValues.BlendMap.Find(Bone) = Alpha;
	}
}