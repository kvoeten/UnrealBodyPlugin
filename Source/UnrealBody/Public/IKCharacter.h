// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VRCharacter.h"
#include "IKCharacter.generated.h"

UCLASS()
class UNREALBODY_API AIKCharacter : public AVRCharacter
{
	GENERATED_BODY()

public:
	AIKCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		USkeletalMeshComponent* IKBodyMesh = nullptr;
};