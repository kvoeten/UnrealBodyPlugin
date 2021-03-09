#include "CharacterComponents/IKBodyComponent.h"

// Sets default values for this component's properties
UIKBodyComponent::UIKBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = .01f;
}

void UIKBodyComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Body != nullptr)
	{
		static FDetachmentTransformRules DetachRules 
			= FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
		Body->DetachFromComponent(DetachRules);
	}
}

void UIKBodyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (Body != nullptr && Camera != nullptr)
	{
		this->TickBodyMovement(DeltaTime);
	}
}

void UIKBodyComponent::TickBodyMovement(float DeltaTime)
{

}