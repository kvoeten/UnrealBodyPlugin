/*
*   Copyright 2022 Kaz Voeten
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

	this->BodyComponent = Cast<UIKBodyComponent>(Character->GetComponentByClass(UIKBodyComponent::StaticClass()));
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
	if(!OwnerComp) return;
	
	FVector LeftFoot = OwnerComp->GetSocketLocation("foot_l");
	FVector RightFoot = OwnerComp->GetSocketLocation("foot_r");
	
	// Establish trace context
	UWorld* World = GetWorld();
	check(World);

	// Filter out player from hits
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// Trace both feet and set result in AnimGraph
	TraceFoot(LeftFoot, &FootIKValues.LeftFootLocation, 
		&FootIKValues.LeftFootRotation, World, &Params);
	TraceFoot(RightFoot, &FootIKValues.RightFootLocation,
		&FootIKValues.RightFootRotation, World, &Params);
}

void UIKCharacterAnimInstance::TraceFoot(FVector Foot, FVector* ResultLocation, 
	FRotator* ResultRotation, UWorld* World, FCollisionQueryParams* Params)
{
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	if(!OwnerComp) return;
	
	// Establish trace start & end point
	const float ZRoot = OwnerComp->GetComponentLocation().Z;
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
	if (!this->BodyComponent->Camera) return;
	
	// Simply set head values to match camera at all times.
	HeadIKValues.HeadRotation = this->BodyComponent->Camera->GetComponentRotation();
	HeadIKValues.HeadLocation = this->BodyComponent->Camera->GetComponentLocation();

	// Apply the same offset as the component does
	HeadIKValues.HeadLocation += (UKismetMathLibrary::GetForwardVector(HeadIKValues.HeadRotation) * this->BodyComponent->BodyOffset);
}

void UIKCharacterAnimInstance::UpdateHandValues()
{
	const USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	if (!OwnerComp) return;

	if (!this->BodyComponent->LeftController || !this->BodyComponent->RightController)
	{
		UE_LOG(LogIKBodyAnimation, Warning, TEXT("Unable to get controller transforms. This is normal in animation preview, but a setup issue in game."))
		return;
	}

	// Get offsets
	FTransform LeftOffset = OwnerComp->GetSocketTransform("hand_lSocket", ERelativeTransformSpace::RTS_ParentBoneSpace);
	FTransform RightOffset = OwnerComp->GetSocketTransform("hand_rSocket", ERelativeTransformSpace::RTS_ParentBoneSpace);

	// Fix left offset
	LeftOffset.ScaleTranslation(-1);

	// Get controller transform * offset
	ArmIKValues.LeftTargetTransform = this->BodyComponent->LeftController->GetComponentTransform() * LeftOffset;
	ArmIKValues.RightTargetTransform = this->BodyComponent->RightController->GetComponentTransform() * RightOffset;
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