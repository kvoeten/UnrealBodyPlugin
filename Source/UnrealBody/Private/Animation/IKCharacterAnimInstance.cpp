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

		// Try find IKBodyComponent
		TArray<UIKBodyComponent*> Comps;
		PawnOwner->GetComponents(Comps);
		if (Comps.Num() > 0) this->BodyComponent = Comps[0];
		else UE_LOG(LogIKBodyAnimation, Warning, TEXT("Pawn owner has no IKBodyComponent"));
	}

	else UE_LOG(LogIKBodyAnimation, Warning, TEXT("Unable to get pawn owner!"));
}

void UIKCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateFootIK();
	UpdateHeadValues();
	UpdateMovementValues(DeltaSeconds);
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

	// Trace both feet and set result in AnimGraph
	TraceFoot(&LeftFoot, &FootIKValues.LeftFootLocation, 
		&FootIKValues.LeftFootRotation, &FootIKValues.LeftEffector, World, &Params);
	TraceFoot(&RightFoot, &FootIKValues.RightFootLocation,
		&FootIKValues.RightFootRotation, &FootIKValues.RightEffector, World, &Params);
}

void UIKCharacterAnimInstance::TraceFoot(FVector* Foot, FVector* ResultLocation, 
	FRotator* ResultRotation, float* Effector, UWorld* World, FCollisionQueryParams* Params)
{
	// Start location is half player height, End location is current foot location
	FVector Start = FVector(Foot->X, Foot->Y, Foot->Z + (this->BodyComponent->PlayerHeight / 2));

	// Trace
	FHitResult HitResult; // Establish Hit Result
	World->LineTraceSingleByChannel(HitResult, Start, *Foot, ECC_Visibility, *Params);

	// Check for hit
	if (HitResult.bBlockingHit)
	{
		const FVector ImpactPoint = HitResult.ImpactPoint;
		const FVector ImpactNormal = HitResult.ImpactNormal;

		if (ImpactPoint.Z > Foot->Z)
		{
			// Set new location
			*ResultLocation = HitResult.ImpactPoint;

			// Set effector
			*Effector = UKismetMathLibrary::Abs(ResultLocation->Z - Foot->Z);

			// Calculate and set new rotation
			(*ResultRotation).Roll = UKismetMathLibrary::Atan2(ImpactNormal.Y, ImpactNormal.Z);
			(*ResultRotation).Pitch = UKismetMathLibrary::Atan2(ImpactNormal.X, ImpactNormal.Z);
			(*ResultRotation).Yaw = 0;
		}

	}
}

void UIKCharacterAnimInstance::UpdateHeadValues()
{
	//Simply set head values to match camera at all times.
	HeadIKValues.HeadRotation = this->BodyComponent->Camera->GetComponentRotation();
	HeadIKValues.HeadLocation = this->BodyComponent->Camera->GetComponentLocation();

	// Apply the same offset as the component does
	HeadIKValues.HeadLocation += (UKismetMathLibrary::GetForwardVector(HeadIKValues.HeadRotation) * this->BodyComponent->BodyOffset);
}

void UIKCharacterAnimInstance::UpdateHandValues()
{
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();

	// Get bone locations
	FVector LeftHand = OwnerComp->GetSocketLocation("hand_l");
	FVector RightHand = OwnerComp->GetSocketLocation("hand_r");

	// Get desired socket locations
	FVector LeftSocket = OwnerComp->GetSocketLocation("hand_lSocket");
	FVector RightSocket = OwnerComp->GetSocketLocation("hand_rSocket");

	// Hand locations are controller locations + corrective offset defined by socket difference
	ArmIKValues.LeftHandLocation = this->BodyComponent->LeftController->GetActorLocation() + (LeftSocket - LeftHand);
	ArmIKValues.RightHandLocation = this->BodyComponent->RightController->GetActorLocation() + (RightSocket - RightHand);
}

void UIKCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	
}