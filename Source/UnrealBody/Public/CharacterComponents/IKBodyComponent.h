// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Camera/CameraComponent.h"
#include "Components/ActorComponent.h"

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
		AActor* RightController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		AActor* LeftController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		UCameraComponent* Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float MovementThreshold = 15.0f 
		UMETA(Tooltip = "Amount of units a player's head has to move to consider it a step instead of head movement.");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float RotationThreshold = 15.0f 
		UMETA(Tooltip = "Amount of degrees player's head has to turn to consider it more than just head movement.");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float PlayerHeight = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float BodyRotationOffset = -90.0f 
		UMETA(Tooltip = "Corrective rotation to align the body with the camera direction.");

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void TickBodyMovement(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void BeginTeleport() { this->IsTeleporting = true; };

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void EndTeleport() { this->IsTeleporting = false; };

private:
	FTransform LastCameraPosition = FTransform();

	// Body Position (XY only!)
	FVector BodyCurrentLocation = FVector();
	FVector BodyTargetLocation = FVector();

	// Body Rotation (Yaw only!)
	FRotator BodyTargetRotation = FRotator();
	FRotator BodyCurrentRotation = FRotator();

	// Movement variables
	float MovementDirection = 0.0f;
	float MovementSpeed = 0.0f;
	float FInterpSpeed = 0.0f;

	// Teleport
	bool IsTeleporting = false;

	// Movement Direction Util
	float GetMovementDirection(FTransform* First, FTransform* Second);

	// Body Offset Util
	void SetBodyTargetPosition(FTransform* CameraTransform);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
