// Copyright Flatlander81. All Rights Reserved.

#include "Core/LaneSystemComponent.h"
#include "Core/GameDataStructs.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Testing/TestMacros.h"

ULaneSystemComponent::ULaneSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default configuration values
	NumLanes = 5;
	LaneSpacing = 200.0f;
	CenterLaneIndex = 2;
	LaneChangeSpeed = 500.0f;

	// Default state
	CurrentLaneIndex = 2; // Start at center
	TargetLaneIndex = 2;
	TransitionState = ELaneTransitionState::Idle;
	CurrentYPosition = 0.0f;

	// Debug
	bShowLaneDebug = false;
}

void ULaneSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize lane configuration
	InitializeLaneConfiguration();

	// Start in the center lane
	CurrentLaneIndex = CenterLaneIndex;
	TargetLaneIndex = CenterLaneIndex;
	CurrentYPosition = GetLaneYPosition(CenterLaneIndex);
	TransitionState = ELaneTransitionState::Idle;

	// Set initial position
	AActor* Owner = GetOwner();
	if (Owner)
	{
		FVector Location = Owner->GetActorLocation();
		Location.Y = CurrentYPosition;
		Owner->SetActorLocation(Location);
	}

	UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::BeginPlay - Initialized at center lane %d (Y: %.2f)"),
		CurrentLaneIndex, CurrentYPosition);
}

void ULaneSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update lane transition if currently transitioning
	if (TransitionState == ELaneTransitionState::Transitioning)
	{
		UpdateLaneTransition(DeltaTime);
	}

	// Draw debug visualization if enabled
	if (bShowLaneDebug)
	{
		DrawDebugLanes();
	}
}

// === LANE CHANGE FUNCTIONS ===

bool ULaneSystemComponent::ChangeLane(int32 Direction)
{
	// Validate direction
	if (Direction != -1 && Direction != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLane - Invalid direction %d (must be -1 or 1)"), Direction);
		return false;
	}

	// Check if already transitioning
	if (TransitionState == ELaneTransitionState::Transitioning)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLane - Already transitioning between lanes"));
		return false;
	}

	// Calculate target lane
	int32 NewTargetLane = CurrentLaneIndex + Direction;

	// Validate target lane is within bounds
	if (!IsValidLaneIndex(NewTargetLane))
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLane - Cannot change to lane %d (out of bounds)"),
			NewTargetLane);
		return false;
	}

	// Validate lane change speed
	if (LaneChangeSpeed <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("ULaneSystemComponent::ChangeLane - Invalid lane change speed %.2f (must be > 0)"),
			LaneChangeSpeed);
		return false;
	}

	// Begin lane change
	TargetLaneIndex = NewTargetLane;
	TransitionState = ELaneTransitionState::Transitioning;

	UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::ChangeLane - Starting transition from lane %d to lane %d"),
		CurrentLaneIndex, TargetLaneIndex);

	return true;
}

void ULaneSystemComponent::ChangeLaneLeft()
{
	if (CanChangeLaneLeft())
	{
		ChangeLane(-1);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLaneLeft - Cannot change lane left"));
	}
}

void ULaneSystemComponent::ChangeLaneRight()
{
	if (CanChangeLaneRight())
	{
		ChangeLane(1);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLaneRight - Cannot change lane right"));
	}
}

// === LANE QUERY FUNCTIONS ===

bool ULaneSystemComponent::CanChangeLaneLeft() const
{
	// Can't change left if at leftmost lane (index 0)
	if (CurrentLaneIndex <= 0)
	{
		return false;
	}

	// Can't change if already transitioning
	if (TransitionState == ELaneTransitionState::Transitioning)
	{
		return false;
	}

	return true;
}

bool ULaneSystemComponent::CanChangeLaneRight() const
{
	// Can't change right if at rightmost lane
	if (CurrentLaneIndex >= NumLanes - 1)
	{
		return false;
	}

	// Can't change if already transitioning
	if (TransitionState == ELaneTransitionState::Transitioning)
	{
		return false;
	}

	return true;
}

float ULaneSystemComponent::GetLaneYPosition(int32 LaneIndex) const
{
	// Validate index
	if (!IsValidLaneIndex(LaneIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::GetLaneYPosition - Invalid lane index %d"), LaneIndex);
		return 0.0f;
	}

	// Return precalculated position if available
	if (LaneYPositions.IsValidIndex(LaneIndex))
	{
		return LaneYPositions[LaneIndex];
	}

	// Fallback: Calculate position (shouldn't happen if properly initialized)
	return (LaneIndex - CenterLaneIndex) * LaneSpacing;
}

// === INTERNAL FUNCTIONS ===

void ULaneSystemComponent::InitializeLaneConfiguration()
{
	// Try to load from data table if specified
	if (LaneSystemDataTable)
	{
		static const FString ContextString(TEXT("Lane System Data"));
		FLaneSystemData* LaneData = LaneSystemDataTable->FindRow<FLaneSystemData>(FName("Default"), ContextString);

		if (LaneData)
		{
			NumLanes = LaneData->NumLanes;
			LaneSpacing = LaneData->LaneSpacing;
			CenterLaneIndex = LaneData->CenterLaneIndex;

			// Use explicit positions if provided, otherwise calculate
			if (LaneData->LaneYPositions.Num() > 0)
			{
				LaneYPositions = LaneData->LaneYPositions;
				NumLanes = LaneYPositions.Num(); // Update num lanes to match
			}

			UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::InitializeLaneConfiguration - Loaded from data table: %d lanes, spacing %.2f"),
				NumLanes, LaneSpacing);
		}
	}

	// Calculate lane positions if not explicitly specified
	if (LaneYPositions.Num() == 0)
	{
		LaneYPositions.SetNum(NumLanes);
		for (int32 i = 0; i < NumLanes; i++)
		{
			// Calculate Y offset from center
			// Lane 0 = leftmost (negative Y), Lane 2 = center (0), Lane 4 = rightmost (positive Y)
			LaneYPositions[i] = (i - CenterLaneIndex) * LaneSpacing;
		}

		UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::InitializeLaneConfiguration - Auto-calculated %d lane positions"),
			NumLanes);
	}

	// Validate configuration
	if (NumLanes < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("ULaneSystemComponent::InitializeLaneConfiguration - Invalid NumLanes %d (must be >= 1)"),
			NumLanes);
		NumLanes = 5; // Fallback to default
	}

	if (CenterLaneIndex < 0 || CenterLaneIndex >= NumLanes)
	{
		UE_LOG(LogTemp, Error, TEXT("ULaneSystemComponent::InitializeLaneConfiguration - Invalid CenterLaneIndex %d (must be 0-%d)"),
			CenterLaneIndex, NumLanes - 1);
		CenterLaneIndex = NumLanes / 2; // Fallback to middle
	}

	// Log lane positions for debugging
	for (int32 i = 0; i < LaneYPositions.Num(); i++)
	{
		UE_LOG(LogTemp, Log, TEXT("  Lane %d: Y = %.2f"), i, LaneYPositions[i]);
	}
}

bool ULaneSystemComponent::IsValidLaneIndex(int32 LaneIndex) const
{
	return LaneIndex >= 0 && LaneIndex < NumLanes;
}

void ULaneSystemComponent::UpdateLaneTransition(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("ULaneSystemComponent::UpdateLaneTransition - No owner actor"));
		TransitionState = ELaneTransitionState::Idle;
		return;
	}

	// Get current and target Y positions
	float TargetYPosition = GetLaneYPosition(TargetLaneIndex);

	// Interpolate current Y position toward target
	// Use FInterpTo for smooth, frame-rate independent interpolation
	CurrentYPosition = FMath::FInterpTo(CurrentYPosition, TargetYPosition, DeltaTime, LaneChangeSpeed / LaneSpacing);

	// Update actor position (ONLY Y-axis, X and Z remain unchanged)
	FVector CurrentLocation = Owner->GetActorLocation();
	CurrentLocation.Y = CurrentYPosition;
	Owner->SetActorLocation(CurrentLocation);

	// Check if we've reached the target
	const float Tolerance = 1.0f; // 1 unit tolerance
	if (FMath::Abs(CurrentYPosition - TargetYPosition) < Tolerance)
	{
		// Snap to exact target position
		CurrentYPosition = TargetYPosition;
		CurrentLocation.Y = CurrentYPosition;
		Owner->SetActorLocation(CurrentLocation);

		// Update current lane index
		CurrentLaneIndex = TargetLaneIndex;

		// Transition complete
		TransitionState = ELaneTransitionState::Idle;

		UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::UpdateLaneTransition - Completed transition to lane %d (Y: %.2f)"),
			CurrentLaneIndex, CurrentYPosition);
	}
}

void ULaneSystemComponent::DrawDebugLanes() const
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();
	const float LineLength = 2000.0f; // Extend forward
	const float LineThickness = 3.0f;

	// Draw all lane lines
	for (int32 i = 0; i < LaneYPositions.Num(); i++)
	{
		float LaneY = LaneYPositions[i];
		FVector StartPoint(OwnerLocation.X - LineLength / 2, LaneY, OwnerLocation.Z);
		FVector EndPoint(OwnerLocation.X + LineLength / 2, LaneY, OwnerLocation.Z);

		// Color coding
		FColor LineColor;
		if (i == CurrentLaneIndex && TransitionState == ELaneTransitionState::Idle)
		{
			LineColor = FColor::Green; // Current lane (not transitioning)
		}
		else if (i == CurrentLaneIndex && TransitionState == ELaneTransitionState::Transitioning)
		{
			LineColor = FColor::Yellow; // Current lane (transitioning away)
		}
		else if (i == TargetLaneIndex && TransitionState == ELaneTransitionState::Transitioning)
		{
			LineColor = FColor::Cyan; // Target lane (transitioning toward)
		}
		else
		{
			LineColor = FColor::White; // Other lanes
		}

		DrawDebugLine(GetWorld(), StartPoint, EndPoint, LineColor, false, -1.0f, 0, LineThickness);
	}

	// Draw current position marker
	FVector CurrentPosMarker(OwnerLocation.X, CurrentYPosition, OwnerLocation.Z + 50.0f);
	DrawDebugSphere(GetWorld(), CurrentPosMarker, 20.0f, 8, FColor::Orange, false, -1.0f, 0, 2.0f);
}

// === DEBUG FUNCTIONS ===

void ULaneSystemComponent::DebugShowLanes()
{
	bShowLaneDebug = !bShowLaneDebug;
	UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::DebugShowLanes - Debug visualization %s"),
		bShowLaneDebug ? TEXT("ENABLED") : TEXT("DISABLED"));
}

// === TESTING FUNCTIONS ===

#if !UE_BUILD_SHIPPING

bool ULaneSystemComponent::TestLaneSystemBounds()
{
	UE_LOG(LogTemp, Log, TEXT("=== TestLaneSystemBounds START ==="));

	// Reset to center lane
	CurrentLaneIndex = CenterLaneIndex;
	TargetLaneIndex = CenterLaneIndex;
	TransitionState = ELaneTransitionState::Idle;

	// Test left boundary - move to leftmost lane
	for (int32 i = CenterLaneIndex; i > 0; i--)
	{
		TEST_TRUE(CanChangeLaneLeft(), "Should be able to move left when not at boundary");
	}

	CurrentLaneIndex = 0; // Set to leftmost
	TEST_FALSE(CanChangeLaneLeft(), "Should NOT be able to move left from leftmost lane");

	// Test right boundary - move to rightmost lane
	CurrentLaneIndex = CenterLaneIndex;
	for (int32 i = CenterLaneIndex; i < NumLanes - 1; i++)
	{
		TEST_TRUE(CanChangeLaneRight(), "Should be able to move right when not at boundary");
		CurrentLaneIndex++; // Simulate move
	}

	CurrentLaneIndex = NumLanes - 1; // Set to rightmost
	TEST_FALSE(CanChangeLaneRight(), "Should NOT be able to move right from rightmost lane");

	// Test invalid lane change attempt
	TEST_FALSE(ChangeLane(0), "Should reject lane change with invalid direction 0");
	TEST_FALSE(ChangeLane(2), "Should reject lane change with invalid direction 2");

	UE_LOG(LogTemp, Log, TEXT("=== TestLaneSystemBounds PASSED ==="));
	TEST_SUCCESS("TestLaneSystemBounds");
}

bool ULaneSystemComponent::TestLaneTransitionSpeed()
{
	UE_LOG(LogTemp, Log, TEXT("=== TestLaneTransitionSpeed START ==="));

	// Validate lane change speed is positive
	TEST_TRUE(LaneChangeSpeed > 0.0f, "Lane change speed must be positive");

	// Test that speed affects transition time
	// Distance between adjacent lanes is LaneSpacing
	// Expected time = LaneSpacing / LaneChangeSpeed (approximate)
	float ExpectedTimeApprox = LaneSpacing / LaneChangeSpeed;

	UE_LOG(LogTemp, Log, TEXT("Lane spacing: %.2f, Speed: %.2f, Expected transition time: ~%.2f seconds"),
		LaneSpacing, LaneChangeSpeed, ExpectedTimeApprox);

	// Speed should be reasonable (not too slow or too fast)
	TEST_TRUE(LaneChangeSpeed >= 100.0f, "Lane change speed should be at least 100 units/sec");
	TEST_TRUE(LaneChangeSpeed <= 5000.0f, "Lane change speed should be at most 5000 units/sec");

	UE_LOG(LogTemp, Log, TEXT("=== TestLaneTransitionSpeed PASSED ==="));
	TEST_SUCCESS("TestLaneTransitionSpeed");
}

bool ULaneSystemComponent::TestLaneChangeValidation()
{
	UE_LOG(LogTemp, Log, TEXT("=== TestLaneChangeValidation START ==="));

	// Reset to center lane
	CurrentLaneIndex = CenterLaneIndex;
	TargetLaneIndex = CenterLaneIndex;
	TransitionState = ELaneTransitionState::Idle;

	// Test that we can start a valid lane change
	TEST_TRUE(ChangeLane(1), "Should be able to change lane right from center");

	// Test that we can't change lane while already transitioning
	TEST_FALSE(ChangeLane(-1), "Should NOT be able to change lane while transitioning");
	TEST_FALSE(CanChangeLaneLeft(), "CanChangeLaneLeft should return false while transitioning");
	TEST_FALSE(CanChangeLaneRight(), "CanChangeLaneRight should return false while transitioning");

	// Complete the transition
	TransitionState = ELaneTransitionState::Idle;
	CurrentLaneIndex = TargetLaneIndex;

	// Test invalid direction values
	TEST_FALSE(ChangeLane(0), "Should reject direction 0");
	TEST_FALSE(ChangeLane(3), "Should reject direction 3");
	TEST_FALSE(ChangeLane(-5), "Should reject direction -5");

	UE_LOG(LogTemp, Log, TEXT("=== TestLaneChangeValidation PASSED ==="));
	TEST_SUCCESS("TestLaneChangeValidation");
}

bool ULaneSystemComponent::TestCurrentLaneTracking()
{
	UE_LOG(LogTemp, Log, TEXT("=== TestCurrentLaneTracking START ==="));

	// Reset to center lane
	CurrentLaneIndex = CenterLaneIndex;
	TargetLaneIndex = CenterLaneIndex;
	TransitionState = ELaneTransitionState::Idle;

	// Test initial state
	TEST_EQUAL(GetCurrentLane(), CenterLaneIndex, "Should start at center lane");
	TEST_FALSE(IsTransitioning(), "Should not be transitioning initially");

	// Start a lane change
	TEST_TRUE(ChangeLane(1), "Should be able to change lane");
	TEST_TRUE(IsTransitioning(), "Should be transitioning after ChangeLane");
	TEST_EQUAL(GetCurrentLane(), CenterLaneIndex, "Current lane should not change until transition completes");

	// Simulate completion of transition
	TransitionState = ELaneTransitionState::Idle;
	CurrentLaneIndex = TargetLaneIndex;

	TEST_EQUAL(GetCurrentLane(), CenterLaneIndex + 1, "Current lane should update after transition completes");
	TEST_FALSE(IsTransitioning(), "Should not be transitioning after completion");

	UE_LOG(LogTemp, Log, TEXT("=== TestCurrentLaneTracking PASSED ==="));
	TEST_SUCCESS("TestCurrentLaneTracking");
}

bool ULaneSystemComponent::TestStationaryInOtherAxes()
{
	UE_LOG(LogTemp, Log, TEXT("=== TestStationaryInOtherAxes START ==="));

	AActor* Owner = GetOwner();
	TEST_NOT_NULL(Owner, "Owner must exist for position test");

	// Store initial X and Z positions
	FVector InitialLocation = Owner->GetActorLocation();
	float InitialX = InitialLocation.X;
	float InitialZ = InitialLocation.Z;

	UE_LOG(LogTemp, Log, TEXT("Initial position - X: %.2f, Y: %.2f, Z: %.2f"),
		InitialLocation.X, InitialLocation.Y, InitialLocation.Z);

	// Reset to center and simulate a lane change
	CurrentLaneIndex = CenterLaneIndex;
	TargetLaneIndex = CenterLaneIndex;
	TransitionState = ELaneTransitionState::Idle;
	CurrentYPosition = GetLaneYPosition(CenterLaneIndex);

	// Start lane change
	TEST_TRUE(ChangeLane(1), "Should be able to start lane change");

	// Simulate several ticks of lane transition
	float DeltaTime = 0.016f; // ~60 FPS
	for (int32 i = 0; i < 10; i++)
	{
		UpdateLaneTransition(DeltaTime);

		FVector CurrentLocation = Owner->GetActorLocation();

		// X and Z should remain unchanged
		TEST_NEARLY_EQUAL(CurrentLocation.X, InitialX, 0.01f, "X position should not change during lane transition");
		TEST_NEARLY_EQUAL(CurrentLocation.Z, InitialZ, 0.01f, "Z position should not change during lane transition");
	}

	UE_LOG(LogTemp, Log, TEXT("=== TestStationaryInOtherAxes PASSED ==="));
	TEST_SUCCESS("TestStationaryInOtherAxes");
}

#else

// Stub implementations for shipping builds
bool ULaneSystemComponent::TestLaneSystemBounds() { return true; }
bool ULaneSystemComponent::TestLaneTransitionSpeed() { return true; }
bool ULaneSystemComponent::TestLaneChangeValidation() { return true; }
bool ULaneSystemComponent::TestCurrentLaneTracking() { return true; }
bool ULaneSystemComponent::TestStationaryInOtherAxes() { return true; }

#endif // !UE_BUILD_SHIPPING
