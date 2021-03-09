#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterComponents/IKBodyComponent.h"
#include "Library/AnimationStructLibrary.h"

#include "IKCharacterAnimInstance.generated.h"

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

	void UpdateFootIK(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

protected:
	/** References */
	UPROPERTY(BlueprintReadOnly)
	UIKBodyComponent* BodyComponent = nullptr;

protected:
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
