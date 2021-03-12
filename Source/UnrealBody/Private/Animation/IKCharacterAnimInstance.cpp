// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2020 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Haziq Fadhil, Jens Bjarne Myhre


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
		UpdateMovementValues(DeltaSeconds);
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

	// Get offsets between hand root bone and target socket location
	const FTransform LeftHandOffset = OwnerComp->GetSocketTransform("hand_lSocket", ERelativeTransformSpace::RTS_ParentBoneSpace);
	const FTransform RightHandOffset = OwnerComp->GetSocketTransform("hand_rSocket", ERelativeTransformSpace::RTS_ParentBoneSpace);

	// Get controller transform
	ArmIKValues.LeftTargetTransform = this->BodyComponent->LeftController->GetComponentTransform();
	ArmIKValues.RightTargetTransform = this->BodyComponent->RightController->GetComponentTransform();

	// Calculate hand bone location/rotation by controler transform and bone offset
	ArmIKValues.LeftHandLocation = ArmIKValues.LeftTargetTransform.GetLocation()
		+ (ArmIKValues.LeftTargetTransform.GetRotation().GetForwardVector() * LeftHandOffset.GetLocation().X)
		+ (ArmIKValues.LeftTargetTransform.GetRotation().GetRightVector() * LeftHandOffset.GetLocation().Y)
		+ (ArmIKValues.LeftTargetTransform.GetRotation().GetUpVector() * LeftHandOffset.GetLocation().Z);

	ArmIKValues.RightHandLocation = ArmIKValues.RightTargetTransform.GetLocation()
		+ (ArmIKValues.RightTargetTransform.GetRotation().GetForwardVector() * RightHandOffset.GetLocation().X)
		+ (ArmIKValues.RightTargetTransform.GetRotation().GetRightVector() * RightHandOffset.GetLocation().Y)
		+ (ArmIKValues.RightTargetTransform.GetRotation().GetUpVector() * RightHandOffset.GetLocation().Z);

	// Hand Rotations are controller rotations + any offset
	ArmIKValues.LeftHandRotation = ArmIKValues.LeftTargetTransform.GetRotation().Rotator() + LeftHandOffset.GetRotation().Rotator();
	ArmIKValues.RightHandRotation = ArmIKValues.RightTargetTransform.GetRotation().Rotator() + LeftHandOffset.GetRotation().Rotator();
}

void UIKCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	
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