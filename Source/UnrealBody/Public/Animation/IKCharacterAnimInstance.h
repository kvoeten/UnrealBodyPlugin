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

	void UpdateMovementValues(float DeltaSeconds);

	void UpdateHeadValues();

	void UpdateHandValues();

	/** Helper function that performs foot trace and sets Anim Graph values */
	void TraceFoot(FVector* Foot, FVector* ResultLocation,
		FRotator* ResultRotation, float* Effector, UWorld* World, FCollisionQueryParams* Params);

protected:
	/** References */
	UPROPERTY(BlueprintReadOnly)
	UIKBodyComponent* BodyComponent = nullptr;

	/** References */
	UPROPERTY(BlueprintReadOnly)
	APawn* Character = nullptr;

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
