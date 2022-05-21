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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "IKBody")
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
