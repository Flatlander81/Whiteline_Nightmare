// Copyright Flatlander81. All Rights Reserved.

#include "WarRig/LaneSystemComponent.h"
#include "DrawDebugHelpers.h"

ULaneSystemComponent::ULaneSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Default configuration
	NumLanes = 5;
	LaneWidth = 400.0f;
	CurrentLaneIndex = 2; // Start in center lane
	TargetLaneIndex = 2;
	CurrentYPosition = 0.0f;
	LaneChangeSpeed = 500.0f;
	bIsChangingLanes = false;
	bIsInitialized = false;
	bShowDebugVisualization = false;
	DebugLineLength = 3000.0f;
}

void ULaneSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-initialize with defaults if not already initialized
	if (!bIsInitialized)
	{
		Initialize(LaneWidth, NumLanes);
	}
}

void ULaneSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized)
	{
		return;
	}

	// Update lane transition
	if (bIsChangingLanes)
	{
		UpdateLaneTransition(DeltaTime);
	}

	// Draw debug visualization if enabled
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
}

void ULaneSystemComponent::Initialize(float InLaneWidth, int32 InNumLanes)
{
	// Validate parameters
	if (InNumLanes < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("LaneSystemComponent: Invalid number of lanes (%d). Must be at least 1."), InNumLanes);
		return;
	}

	if (InLaneWidth <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("LaneSystemComponent: Invalid lane width (%.2f). Must be greater than 0."), InLaneWidth);
		return;
	}

	LaneWidth = InLaneWidth;
	NumLanes = InNumLanes;

	// Start in center lane
	CurrentLaneIndex = NumLanes / 2;
	TargetLaneIndex = CurrentLaneIndex;
	CurrentYPosition = GetLaneYPosition(CurrentLaneIndex);

	bIsChangingLanes = false;
	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("LaneSystemComponent: Initialized with %d lanes, width %.2f. Starting in lane %d."),
		NumLanes, LaneWidth, CurrentLaneIndex);
}

bool ULaneSystemComponent::RequestLaneChange(int32 Direction)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("LaneSystemComponent: Cannot change lanes - not initialized."));
		return false;
	}

	// Can't change lanes while already changing
	if (bIsChangingLanes)
	{
		UE_LOG(LogTemp, Verbose, TEXT("LaneSystemComponent: Cannot change lanes - already in transition."));
		return false;
	}

	// Calculate target lane
	int32 NewLaneIndex = CurrentLaneIndex + Direction;

	// Validate target lane
	if (!IsValidLane(NewLaneIndex))
	{
		UE_LOG(LogTemp, Verbose, TEXT("LaneSystemComponent: Cannot change to lane %d - out of bounds (0-%d)."),
			NewLaneIndex, NumLanes - 1);
		return false;
	}

	// Initiate lane change
	TargetLaneIndex = NewLaneIndex;
	bIsChangingLanes = true;

	UE_LOG(LogTemp, Log, TEXT("LaneSystemComponent: Changing from lane %d to lane %d."),
		CurrentLaneIndex, TargetLaneIndex);

	return true;
}

void ULaneSystemComponent::SetLaneChangeSpeed(float Speed)
{
	if (Speed <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("LaneSystemComponent: Invalid lane change speed (%.2f). Must be greater than 0."), Speed);
		return;
	}

	LaneChangeSpeed = Speed;
	UE_LOG(LogTemp, Log, TEXT("LaneSystemComponent: Lane change speed set to %.2f."), LaneChangeSpeed);
}

float ULaneSystemComponent::GetLaneYPosition(int32 LaneIndex) const
{
	if (!IsValidLane(LaneIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("LaneSystemComponent: Invalid lane index %d. Returning 0."), LaneIndex);
		return 0.0f;
	}

	// Calculate Y position: center lane is at 0, others offset by lane width
	// For 5 lanes: indices 0,1,2,3,4 -> positions -2,-1,0,1,2 * LaneWidth
	int32 CenterLane = NumLanes / 2;
	int32 OffsetFromCenter = LaneIndex - CenterLane;
	return OffsetFromCenter * LaneWidth;
}

void ULaneSystemComponent::UpdateLaneTransition(float DeltaTime)
{
	if (!bIsChangingLanes)
	{
		return;
	}

	float TargetYPosition = GetLaneYPosition(TargetLaneIndex);
	float Direction = (TargetYPosition > CurrentYPosition) ? 1.0f : -1.0f;
	float DeltaY = LaneChangeSpeed * DeltaTime * Direction;

	// Check if we've reached or passed the target
	if (FMath::Abs(TargetYPosition - CurrentYPosition) <= FMath::Abs(DeltaY))
	{
		// Snap to target and finish transition
		CurrentYPosition = TargetYPosition;
		CurrentLaneIndex = TargetLaneIndex;
		bIsChangingLanes = false;

		UE_LOG(LogTemp, Log, TEXT("LaneSystemComponent: Lane change complete. Now in lane %d at Y=%.2f."),
			CurrentLaneIndex, CurrentYPosition);

		// Update owner's location
		if (AActor* Owner = GetOwner())
		{
			FVector NewLocation = Owner->GetActorLocation();
			NewLocation.Y = CurrentYPosition;
			Owner->SetActorLocation(NewLocation);
		}
	}
	else
	{
		// Continue interpolation
		CurrentYPosition += DeltaY;

		// Update owner's location
		if (AActor* Owner = GetOwner())
		{
			FVector NewLocation = Owner->GetActorLocation();
			NewLocation.Y = CurrentYPosition;
			Owner->SetActorLocation(NewLocation);
		}
	}
}

bool ULaneSystemComponent::IsValidLane(int32 LaneIndex) const
{
	return LaneIndex >= 0 && LaneIndex < NumLanes;
}

void ULaneSystemComponent::DrawDebugVisualization()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();

	// Draw lane lines extending forward
	for (int32 i = 0; i < NumLanes; ++i)
	{
		float YPos = GetLaneYPosition(i);
		FVector Start = FVector(OwnerLocation.X, YPos, OwnerLocation.Z);
		FVector End = FVector(OwnerLocation.X + DebugLineLength, YPos, OwnerLocation.Z);

		// Highlight current lane in green, target lane in yellow, others in white
		FColor LineColor = FColor::White;
		float Thickness = 1.0f;

		if (i == CurrentLaneIndex)
		{
			LineColor = FColor::Green;
			Thickness = 3.0f;
		}
		else if (i == TargetLaneIndex && bIsChangingLanes)
		{
			LineColor = FColor::Yellow;
			Thickness = 2.0f;
		}

		DrawDebugLine(GetWorld(), Start, End, LineColor, false, -1.0f, 0, Thickness);
	}

	// Draw current position marker
	FVector CurrentPos = FVector(OwnerLocation.X, CurrentYPosition, OwnerLocation.Z + 50.0f);
	DrawDebugSphere(GetWorld(), CurrentPos, 30.0f, 8, FColor::Cyan, false, -1.0f, 0, 2.0f);

	// Draw lane boundary markers
	float LeftBoundary = GetLaneYPosition(0);
	float RightBoundary = GetLaneYPosition(NumLanes - 1);

	FVector LeftStart = FVector(OwnerLocation.X, LeftBoundary, OwnerLocation.Z);
	FVector LeftEnd = FVector(OwnerLocation.X + DebugLineLength, LeftBoundary, OwnerLocation.Z);
	DrawDebugLine(GetWorld(), LeftStart, LeftEnd, FColor::Red, false, -1.0f, 0, 4.0f);

	FVector RightStart = FVector(OwnerLocation.X, RightBoundary, OwnerLocation.Z);
	FVector RightEnd = FVector(OwnerLocation.X + DebugLineLength, RightBoundary, OwnerLocation.Z);
	DrawDebugLine(GetWorld(), RightStart, RightEnd, FColor::Red, false, -1.0f, 0, 4.0f);
}
