#include "CharacterComponents/IKBodyComponent.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogIKBodyComponent);

// Sets default values for this component's properties
UIKBodyComponent::UIKBodyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = .01f;
	bAutoActivate = true;
}

void UIKBodyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Body != nullptr && Camera != nullptr)
	{
		// Detach the body from it's parent so it doesn't automatically move with the players head movement.
		static FDetachmentTransformRules DetachRules 
			= FDetachmentTransformRules(EDetachmentRule::KeepWorld, false);
		Body->DetachFromComponent(DetachRules);

		// Set body at camera position + offsets
		this->BodyTargetLocation = Camera->GetComponentLocation()
			+ (UKismetMathLibrary::GetForwardVector(Camera->GetComponentRotation()) * BodyOffset); // 20 units back from cam to avoid clipping
		this->BodyTargetLocation.Z -= this->PlayerHeight;
		this->Body->SetWorldLocation(this->BodyTargetLocation);

		// Rotate Body to match
		this->BodyTargetRotation.Yaw = Camera->GetComponentRotation().Yaw + this->BodyRotationOffset;
		this->Body->SetWorldRotation(this->BodyTargetRotation);

		// Set current position to match target
		this->BodyCurrentLocation = this->BodyTargetLocation;
		this->BodyCurrentRotation = this->BodyTargetRotation;
		
		UE_LOG(LogIKBodyComponent, Log, TEXT("Succesfully initialized with body and camera!"));
	}
	else
	{
		// Body should be assigned in component owner's construction script!
		UE_LOG(LogIKBodyComponent, Warning, TEXT("Please ensure a body and camera reference are set before BeginPlay by setting it in the owner's construction script."));
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
		CameraCurrentPosition.GetRotation().Rotator().Yaw - LastCameraPosition.GetRotation().Rotator().Yaw
	);

	// Update the camera position and movement if the player moved further away than threshold (enables leaning/ head tilt without moving the body)
	if (DistanceMoved > this->MovementThreshold)
	{
		// Set new body target location
		this->BodyTargetLocation = CameraCurrentPosition.GetLocation() 
			+ (UKismetMathLibrary::GetForwardVector(CameraCurrentPosition.GetRotation().Rotator()) * BodyOffset); // 20 units back from cam to avoid clipping

		// Update movement speed and direction
		this->MovementDirection = GetMovementDirection(&LastCameraPosition, &CameraCurrentPosition);
		this->MovementSpeed = DistanceMoved / DeltaTime;

		// Save new position
		this->LastCameraPosition = CameraCurrentPosition;
	}

	// Apply new rotation to the body if turned far enough (allows head turning without rotating the whole body)
	if (YawDifference > this->RotationThreshold)
	{
		this->BodyTargetRotation.Yaw = Camera->GetComponentRotation().Yaw + this->BodyRotationOffset;
	}

	// If the body hasn't reached it's target location yet we move it towards it.
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentLocation.X, BodyTargetLocation.X, 9.99997f) 
		&& UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentLocation.Y, BodyTargetLocation.Y, 9.99997f))
	{
		this->MovementSpeed = 0;
		this->MovementDirection = 0;
	}
	else
	{
		// Tick towards location based on movement speed
		FVector MovementVector = UKismetMathLibrary::GetDirectionUnitVector(this->BodyCurrentLocation, this->BodyTargetLocation) * MovementSpeed * DeltaTime;
		this->BodyCurrentLocation = this->BodyCurrentLocation + MovementVector;
		this->Body->SetWorldLocation(BodyCurrentLocation);
	}

	// If the body hasn't reached it's target rotation yet we interp towards it.
	if (!UKismetMathLibrary::NearlyEqual_FloatFloat(BodyCurrentRotation.Yaw, BodyTargetRotation.Yaw, 9.99997f))
	{
		BodyCurrentRotation.Yaw = UKismetMathLibrary::FInterpTo(BodyCurrentRotation.Yaw, BodyTargetRotation.Yaw, DeltaTime, UKismetMathLibrary::Max(5.0f, MovementSpeed));
		this->Body->SetWorldRotation(BodyCurrentRotation);
	}

	// Always set Z to enable seamless crouching
	FVector BodyLocation = FVector(BodyCurrentLocation.X, BodyCurrentLocation.Y, CameraCurrentPosition.GetLocation().Z - this->PlayerHeight);
	this->Body->SetWorldLocation(BodyLocation);
}

/*
 * Sets the body target position to the camera position, rotating and moving it down so that the character eyes match the HMD position
*/
void UIKBodyComponent::SetBodyTargetPosition(FTransform* CameraTransform)
{
	this->BodyTargetRotation.Yaw = CameraTransform->GetRotation().Z + this->BodyRotationOffset;
	this->BodyTargetLocation = CameraTransform->GetLocation() + (UKismetMathLibrary::GetRightVector(this->BodyTargetRotation) * -20);
	this->BodyTargetLocation.Z = BodyTargetLocation.Z - this->PlayerHeight;
}

/*
 * This function isn't fully correct, expected result is direction in degrees between -180 and 180
*/
float UIKBodyComponent::GetMovementDirection(FTransform* First, FTransform* Second)
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