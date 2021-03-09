#include "IKCharacter.h"

AIKCharacter::AIKCharacter(const FObjectInitializer& ObjectInitializer) : AVRCharacter(ObjectInitializer)
{
	IKBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("IKBodyMesh"));
	IKBodyMesh->SetupAttachment(VRRootReference);
}