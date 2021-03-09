#include "CharacterComponents/IKBodyComponent.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogIKBodyComponent);

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
		// Detach the body from it's parent so it doesn't automatically move with the players head movement.
		static FDetachmentTransformRules DetachRules 
			= FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
		Body->DetachFromComponent(DetachRules);
	}
	else
	{
		// Body should be assigned in component owner's construction script!
		UE_LOG(LogIKBodyComponent, Warning, TEXT("Please ensure a body reference is set before BeginPlay by setting it in the owner's construction script.)"));
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
	FTransform CameraCurrentPosition = this->Camera->GetComponentTransform();

	// Calculate the XY distance moved
	float DistanceMoved = FVector::Distance(
		FVector(CameraCurrentPosition.GetLocation().X, CameraCurrentPosition.GetLocation().X, 0),
		FVector(LastCameraPosition.GetLocation().X, LastCameraPosition.GetLocation().X, 0)
	);

	// Calculate Yaw difference
	float YawDifference = UKismetMathLibrary::Abs(
		CameraCurrentPosition.GetRotation().Z - LastCameraPosition.GetRotation().Z
	);

	// Update the camera position and movement if the player moved further away than threshold (enables leaning/ head tilt without moving the body)
	if (DistanceMoved > this->MovementThreshold)
	{
		this->MovementDirection = GetMovementDirection(&LastCameraPosition, &CameraCurrentPosition);
		this->MovementSpeed = (DistanceMoved / 1000) / DeltaTime; // Division by 1000 to make it meters
		this->LastCameraPosition = CameraCurrentPosition;
		SetBodyTargetPosition(&CameraCurrentPosition);
	}

	// Apply new rotation to the body if turned far enough (allows head turning without rotating the whole body)
	if (YawDifference > this->RotationThreshold)
	{

	}


}

void UIKBodyComponent::SetBodyTargetPosition(FTransform* CameraTransform)
{
	this->BodyTargetRotation.Yaw = CameraTransform->GetRotation().Z + this->BodyRotationOffset;
	this->BodyTargetLocation = CameraTransform->GetLocation() + UKismetMathLibrary::GetRightVector(this->BodyTargetRotation) * -20;
	this->BodyTargetLocation.Z = BodyTargetLocation.Z - this->PlayerHeight;
}

/*
 * This function isn't fully correct, expected result is direction in degrees between -180 and 180
*/
float GetMovementDirection(FTransform* First, FTransform* Second)
{
	float Rotation = 0;

	FVector FirstForward = UKismetMathLibrary::GetForwardVector(First->GetRotation().Rotator());
	FVector SecondForward = UKismetMathLibrary::GetForwardVector(Second->GetRotation().Rotator());

	if (FirstForward.Normalize() && SecondForward.Normalize())
	{
		// Should return 0~180
		Rotation = UKismetMathLibrary::DegAcos(UKismetMathLibrary::Dot_VectorVector(FirstForward, SecondForward));
		
		// Check if it's to the front or to the back
		FVector Difference = Second->GetLocation() - First->GetLocation();
		if (Difference.Normalize())
		{
			float Dot = UKismetMathLibrary::Dot_VectorVector(Difference, FirstForward);
			// TODO test, otherwise have to reverse cos this
			if (Dot < 0) Rotation = -1 * Rotation;
		}
	}

	return Rotation;
}