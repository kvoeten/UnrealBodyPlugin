// Project:         Advanced Locomotion System V4 on C++
// Copyright:       Copyright (C) 2020 Doğa Can Yanıkoğlu
// License:         MIT License (http://www.opensource.org/licenses/mit-license.php)
// Source Code:     https://github.com/dyanikoglu/ALSV4_CPP
// Original Author: Doğa Can Yanıkoğlu
// Contributors:    Haziq Fadhil, Jens Bjarne Myhre


#include "Animation/IKCharacterAnimInstance.h"
#include "Library/AnimationStructLibrary.h"
#include "Kismet/KismetMathLibrary.h"

void UIKCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UIKCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	UpdateFootIK(DeltaSeconds); 
	UpdateMovementValues(DeltaSeconds);
	UpdateRotationValues();
}

void UIKCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	
}

void UIKCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	
}

void UIKCharacterAnimInstance::UpdateRotationValues()
{
	
}