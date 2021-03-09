#pragma once

#include "CoreMinimal.h"
#include "CharacterStateLibrary.generated.h"

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle,
	Teleporting,
	Climbing
};

UENUM(BlueprintType)
enum class ECharacterIKHand : uint8
{
	Left,
	Right
};