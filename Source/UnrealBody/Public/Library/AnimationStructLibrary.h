/*
*   Copyright 2022 Kaz Voeten
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "CoreMinimal.h"

#include "AnimationStructLibrary.generated.h"

/** Anim Graph - Movement */
USTRUCT(BlueprintType)
struct FAnimGraphMovement
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		float Speed = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		float Direction = 0.0f;
};

/** Anim Graph - Head IK */
USTRUCT(BlueprintType)
struct FAnimGraphHeadIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FRotator HeadRotation = FRotator();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FVector HeadLocation = FVector();
};

/** Anim Graph - Foot IK */
USTRUCT(BlueprintType)
struct FAnimGraphFootIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FVector LeftFootLocation = FVector();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FVector RightFootLocation = FVector();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FRotator LeftFootRotation = FRotator();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FRotator RightFootRotation = FRotator();
};

/** Anim Graph - Arm IK */
USTRUCT(BlueprintType)
struct FAnimGraphArmIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FTransform LeftTargetTransform = FTransform();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		FTransform RightTargetTransform = FTransform();
};

UENUM(BlueprintType)
enum class EFingerBone : uint8
{
	index_01_l,
	index_02_l,
	index_03_l,
	middle_01_l,
	middle_02_l,
	middle_03_l,
	ring_01_l,
	ring_02_l,
	ring_03_l,
	pinky_01_l,
	pinky_02_l,
	pinky_03_l,
	thumb_01_l,
	thumb_02_l,
	thumb_03_l,
	index_01_r,
	index_02_r,
	index_03_r,
	middle_01_r,
	middle_02_r,
	middle_03_r,
	ring_01_r,
	ring_02_r,
	ring_03_r,
	pinky_01_r,
	pinky_02_r,
	pinky_03_r,
	thumb_01_r,
	thumb_02_r,
	thumb_03_r
};

/** Anim Graph - Finger IK */
USTRUCT(BlueprintType)
struct FAnimGraphFingerIK
{
	GENERATED_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		TMap<EFingerBone, float> BlendMap;

	FAnimGraphFingerIK()
	{
		// Populate the MAP on construction
		BlendMap.Add(EFingerBone::index_01_l, 0.0f);
		BlendMap.Add(EFingerBone::index_02_l, 0.0f);
		BlendMap.Add(EFingerBone::index_03_l, 0.0f);
		BlendMap.Add(EFingerBone::middle_01_l, 0.0f);
		BlendMap.Add(EFingerBone::middle_02_l, 0.0f);
		BlendMap.Add(EFingerBone::middle_03_l, 0.0f);
		BlendMap.Add(EFingerBone::ring_01_l, 0.0f);
		BlendMap.Add(EFingerBone::ring_02_l, 0.0f);
		BlendMap.Add(EFingerBone::ring_03_l, 0.0f);
		BlendMap.Add(EFingerBone::pinky_01_l, 0.0f);
		BlendMap.Add(EFingerBone::pinky_02_l, 0.0f);
		BlendMap.Add(EFingerBone::pinky_03_l, 0.0f);
		BlendMap.Add(EFingerBone::thumb_01_l, 0.0f);
		BlendMap.Add(EFingerBone::thumb_02_l, 0.0f);
		BlendMap.Add(EFingerBone::thumb_03_l, 0.0f);
		BlendMap.Add(EFingerBone::index_01_r, 0.0f);
		BlendMap.Add(EFingerBone::index_02_r, 0.0f);
		BlendMap.Add(EFingerBone::index_03_r, 0.0f);
		BlendMap.Add(EFingerBone::middle_01_r, 0.0f);
		BlendMap.Add(EFingerBone::middle_02_r, 0.0f);
		BlendMap.Add(EFingerBone::middle_03_r, 0.0f);
		BlendMap.Add(EFingerBone::ring_01_r, 0.0f);
		BlendMap.Add(EFingerBone::ring_02_r, 0.0f);
		BlendMap.Add(EFingerBone::ring_03_r, 0.0f);
		BlendMap.Add(EFingerBone::pinky_01_r, 0.0f);
		BlendMap.Add(EFingerBone::pinky_02_r, 0.0f);
		BlendMap.Add(EFingerBone::pinky_03_r, 0.0f);
		BlendMap.Add(EFingerBone::thumb_01_r, 0.0f);
		BlendMap.Add(EFingerBone::thumb_02_r, 0.0f);
		BlendMap.Add(EFingerBone::thumb_03_r, 0.0f);
	}
};

/** Anim Graph - Finger IK */
USTRUCT(BlueprintType)
struct FFingerStateMap
{
	GENERATED_BODY()

		UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite)
		TMap<EFingerBone, bool> StateMap;

	FFingerStateMap()
	{
		// Populate the MAP on construction
		StateMap.Add(EFingerBone::index_01_l, false);
		StateMap.Add(EFingerBone::index_02_l, false);
		StateMap.Add(EFingerBone::index_03_l, false);
		StateMap.Add(EFingerBone::middle_01_l, false);
		StateMap.Add(EFingerBone::middle_02_l, false);
		StateMap.Add(EFingerBone::middle_03_l, false);
		StateMap.Add(EFingerBone::ring_01_l, false);
		StateMap.Add(EFingerBone::ring_02_l, false);
		StateMap.Add(EFingerBone::ring_03_l, false);
		StateMap.Add(EFingerBone::pinky_01_l, false);
		StateMap.Add(EFingerBone::pinky_02_l, false);
		StateMap.Add(EFingerBone::pinky_03_l, false);
		StateMap.Add(EFingerBone::thumb_01_l, false);
		StateMap.Add(EFingerBone::thumb_02_l, false);
		StateMap.Add(EFingerBone::thumb_03_l, false);
		StateMap.Add(EFingerBone::index_01_r, false);
		StateMap.Add(EFingerBone::index_02_r, false);
		StateMap.Add(EFingerBone::index_03_r, false);
		StateMap.Add(EFingerBone::middle_01_r, false);
		StateMap.Add(EFingerBone::middle_02_r, false);
		StateMap.Add(EFingerBone::middle_03_r, false);
		StateMap.Add(EFingerBone::ring_01_r, false);
		StateMap.Add(EFingerBone::ring_02_r, false);
		StateMap.Add(EFingerBone::ring_03_r, false);
		StateMap.Add(EFingerBone::pinky_01_r, false);
		StateMap.Add(EFingerBone::pinky_02_r, false);
		StateMap.Add(EFingerBone::pinky_03_r, false);
		StateMap.Add(EFingerBone::thumb_01_r, false);
		StateMap.Add(EFingerBone::thumb_02_r, false);
		StateMap.Add(EFingerBone::thumb_03_r, false);
	}
};
