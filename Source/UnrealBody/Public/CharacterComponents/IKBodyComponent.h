// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "Components/ActorComponent.h"

#include "IKBodyComponent.generated.h"


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
		AActor* Camera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float MovementThreshold = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float RotationThreshold = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IKBody")
		float PlayerHeight = 180.0f;

	UFUNCTION(BlueprintCallable, Category = "IKBody")
		void TickBodyMovement(float DeltaTime);

private:
	FTransform LastCameraPosition = FTransform();
	FTransform BodyTargetPosition = FTransform();
	FTransform BodyCurrentPosition = FTransform();
	float MovementDirection = 0.0f;
	float MovementSpeed = 0.0f;
	float BodyMovementAlpha = 0.0f;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
