/*
*   Copyright 2022 Kaz Voeten
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Library/CharacterStateLibrary.h"
#include "Library/AnimationStructLibrary.h"

#include "IKBodyComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogIKBodyComponent, Log, All);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNREALBODY_API UIKBodyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UIKBodyComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		USkeletalMeshComponent* Body = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		UPrimitiveComponent* RightController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		UPrimitiveComponent* LeftController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		UCameraComponent* Camera = nullptr;

	/*
	 * Non-Replicated Setting variables
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
		float MovementSpeedMultiplier = 1.0f
		UMETA(Tooltip = "Increase or decrease the speed of the character during movement. Use this to avoid the character lagging behind over longer distances.");


	/*
	 * Movement Values, replicated (server changes are sent to clients)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "IKBody")
		float MovementThreshold = 60.0f 
		UMETA(Tooltip = "Amount of units a player's head has to move to consider it a step instead of head movement.");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "IKBody")
		float RotationThreshold = 25.0f 
		UMETA(Tooltip = "Amount of degrees player's head has to turn to consider it more than just head movement.");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "IKBody")
		float PlayerHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "IKBody")
		float BodyOffset = -20.0f
		UMETA(Tooltip = "Units to move the body from the camera looking direction to avoid clipping.");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float BodyRotationOffset = -90.0f 
		UMETA(Tooltip = "Corrective rotation to align the body with the camera direction.");

	/*
		Replicated movement value changes, run on server
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable)
		void UpdateMovementThreshold(float Value);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void UpdateRotationThreshold(float Value);

	UFUNCTION(Server, Reliable, BlueprintCallable)
		void UpdatePlayerHeight(float Value);

	UFUNCTION(Server, Unreliable, BlueprintCallable)
		void UpdateBodyOffset(float Value);

	/*
		System Ticks
	*/
	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void TickBodyMovement(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void TickFingerIK(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void StartFingerIK(AActor* Target, ECharacterIKHand Hand);

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void StopFingerIK(ECharacterIKHand Hand);

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void BeginTeleport() { this->IsTeleporting = true; };

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void EndTeleport() { this->IsTeleporting = false; };

	/*
		Finger IK maps
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody | Fingers")
		TMap<EFingerBone, UCapsuleComponent*> FingerHitboxes;

	UFUNCTION(BlueprintCallable, Category = "IKBody | Fingers")
		void SetAllHitBoxes(
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
		);

	FAnimGraphFingerIK FingerIKValues = FAnimGraphFingerIK();

	// Movement variables
	float MovementDirection = 0.0f;
	float MovementSpeed = 0.0f;
	float FInterpSpeed = 0.0f;

private:
	FTransform LastCameraPosition = FTransform();

	// Body Position (XY only!)
	FVector BodyCurrentLocation = FVector();
	FVector BodyTargetLocation = FVector();

	// Body Rotation (Yaw only!)
	FRotator BodyTargetRotation = FRotator();
	FRotator BodyCurrentRotation = FRotator();

	// Finger States
	FFingerStateMap FingerStates = FFingerStateMap();
	
	// Grip States
	AActor* LeftGrip = nullptr;
	AActor* RightGrip = nullptr;

	// Teleport
	bool IsTeleporting = false;

	// Movement Direction Util
	float GetMovementDirection(FTransform* First, FTransform* Second);

	// Body Offset Util
	void SetBodyTargetPosition(FTransform* CameraTransform);

	// Finger reset
	void ResetHandFingers(ECharacterIKHand Hand);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
