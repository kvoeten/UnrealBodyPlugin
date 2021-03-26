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

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterComponents/IKBodyComponent.h"
#include "Library/AnimationStructLibrary.h"

#include "IKCharacterAnimInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIKBodyAnimation, Log, All);

class IKBodyComponent;

/**
 * Main anim instance class for character
 */
UCLASS(Blueprintable, BlueprintType)
class UNREALBODY_API UIKCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

	void UpdateFootIK();

	void UpdateMovementValues();

	void UpdateHeadValues();

	void UpdateHandValues();

	void UpdateFingerIKValues();

	/** Helper function that performs foot trace and sets Anim Graph values */
	void TraceFoot(FVector Foot, FVector* ResultLocation,
		FRotator* ResultRotation, UWorld* World, FCollisionQueryParams* Params);

protected:
	/** References */
	UPROPERTY(BlueprintReadOnly)
	UIKBodyComponent* BodyComponent = nullptr;

	/** References */
	UPROPERTY(BlueprintReadOnly)
	APawn* Character = nullptr;

protected:
	/** Anim Graph - Movement */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Movement", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphMovement MovementValues;

	/** Anim Graph - Foot IK */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Head IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphHeadIK HeadIKValues;

	/** Anim Graph - Foot IK */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphFootIK FootIKValues;

	/** Anim Graph - Context Awareness */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Arm IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphArmIK ArmIKValues;

		/** Anim Graph - Context Awareness */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Finger IK", Meta = (
		ShowOnlyInnerProperties))
	FAnimGraphFingerIK FingerIKValues;
};
