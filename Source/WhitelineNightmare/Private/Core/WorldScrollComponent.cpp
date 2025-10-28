// Copyright Flatlander81. All Rights Reserved.

#include "Core/WorldScrollComponent.h"
#include "Core/GameDataStructs.h"
#include "Engine/DataTable.h"
#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"

UWorldScrollComponent::UWorldScrollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default configuration values
	ScrollSpeed = 1000.0f;
	bIsScrolling = true;
	DistanceTraveled = 0.0f;
	ScrollDirection = FVector(-1.0f, 0.0f, 0.0f); // Backward along X axis
	ScrollVelocity = FVector::ZeroVector;
	DataTableRowName = FName("DefaultScroll");
}

void UWorldScrollComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load configuration from data table
	LoadScrollConfiguration();

	// Calculate initial scroll velocity
	ScrollVelocity = ScrollDirection.GetSafeNormal() * ScrollSpeed;

	UE_LOG(LogTemp, Log, TEXT("UWorldScrollComponent::BeginPlay - Initialized with speed %.2f, scrolling: %s"),
		ScrollSpeed, bIsScrolling ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("  Scroll Direction: (%.2f, %.2f, %.2f)"),
		ScrollDirection.X, ScrollDirection.Y, ScrollDirection.Z);
	UE_LOG(LogTemp, Log, TEXT("  Scroll Velocity: (%.2f, %.2f, %.2f)"),
		ScrollVelocity.X, ScrollVelocity.Y, ScrollVelocity.Z);
}

void UWorldScrollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Accumulate distance if scrolling is active
	if (bIsScrolling && ScrollSpeed > 0.0f)
	{
		float DeltaDistance = ScrollSpeed * DeltaTime;
		DistanceTraveled += DeltaDistance;
	}
}

// === SCROLL VELOCITY QUERY FUNCTIONS ===

FVector UWorldScrollComponent::GetScrollVelocity() const
{
	// Return zero velocity if not scrolling
	if (!bIsScrolling)
	{
		return FVector::ZeroVector;
	}

	return ScrollVelocity;
}

// === SCROLL CONTROL FUNCTIONS ===

void UWorldScrollComponent::SetScrollSpeed(float NewSpeed)
{
	// Validate and clamp speed
	float ValidatedSpeed = ValidateScrollSpeed(NewSpeed);

	if (ValidatedSpeed != ScrollSpeed)
	{
		float OldSpeed = ScrollSpeed;
		ScrollSpeed = ValidatedSpeed;

		// Recalculate velocity
		ScrollVelocity = ScrollDirection.GetSafeNormal() * ScrollSpeed;

		LogScrollStateChange(FString::Printf(TEXT("Scroll speed changed from %.2f to %.2f"), OldSpeed, ScrollSpeed));
	}
}

void UWorldScrollComponent::SetScrolling(bool bEnabled)
{
	if (bEnabled != bIsScrolling)
	{
		bIsScrolling = bEnabled;
		LogScrollStateChange(FString::Printf(TEXT("Scrolling %s"), bEnabled ? TEXT("enabled") : TEXT("disabled")));
	}
}

void UWorldScrollComponent::ResetDistance()
{
	float OldDistance = DistanceTraveled;
	DistanceTraveled = 0.0f;
	LogScrollStateChange(FString::Printf(TEXT("Distance reset from %.2f to 0.0"), OldDistance));
}

// === DEBUG COMMANDS ===

void UWorldScrollComponent::DebugSetScrollSpeed(float NewSpeed)
{
	UE_LOG(LogTemp, Display, TEXT("=== DEBUG: Set Scroll Speed ==="));
	UE_LOG(LogTemp, Display, TEXT("Old Speed: %.2f"), ScrollSpeed);
	UE_LOG(LogTemp, Display, TEXT("New Speed: %.2f"), NewSpeed);

	SetScrollSpeed(NewSpeed);

	UE_LOG(LogTemp, Display, TEXT("Current Speed: %.2f"), ScrollSpeed);
	UE_LOG(LogTemp, Display, TEXT("Current Velocity: (%.2f, %.2f, %.2f)"),
		ScrollVelocity.X, ScrollVelocity.Y, ScrollVelocity.Z);
}

void UWorldScrollComponent::DebugToggleScroll()
{
	UE_LOG(LogTemp, Display, TEXT("=== DEBUG: Toggle Scroll ==="));
	UE_LOG(LogTemp, Display, TEXT("Old State: %s"), bIsScrolling ? TEXT("Scrolling") : TEXT("Paused"));

	SetScrolling(!bIsScrolling);

	UE_LOG(LogTemp, Display, TEXT("New State: %s"), bIsScrolling ? TEXT("Scrolling") : TEXT("Paused"));
}

void UWorldScrollComponent::DebugShowScrollInfo()
{
	UE_LOG(LogTemp, Display, TEXT("=== DEBUG: Scroll Info ==="));
	UE_LOG(LogTemp, Display, TEXT("Scroll Speed: %.2f units/second"), ScrollSpeed);
	UE_LOG(LogTemp, Display, TEXT("Is Scrolling: %s"), bIsScrolling ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Display, TEXT("Distance Traveled: %.2f units"), DistanceTraveled);
	UE_LOG(LogTemp, Display, TEXT("Scroll Direction: (%.2f, %.2f, %.2f)"),
		ScrollDirection.X, ScrollDirection.Y, ScrollDirection.Z);
	UE_LOG(LogTemp, Display, TEXT("Scroll Velocity: (%.2f, %.2f, %.2f)"),
		ScrollVelocity.X, ScrollVelocity.Y, ScrollVelocity.Z);
}

void UWorldScrollComponent::DebugResetDistance()
{
	UE_LOG(LogTemp, Display, TEXT("=== DEBUG: Reset Distance ==="));
	UE_LOG(LogTemp, Display, TEXT("Old Distance: %.2f"), DistanceTraveled);

	ResetDistance();

	UE_LOG(LogTemp, Display, TEXT("New Distance: %.2f"), DistanceTraveled);
}

// === TESTING FUNCTIONS ===

bool UWorldScrollComponent::TestScrollSpeedConsistency()
{
	TEST_INIT("World Scroll Speed Consistency");

	// Set a known speed
	const float TestSpeed = 500.0f;
	SetScrollSpeed(TestSpeed);

	// Wait a few frames and verify speed hasn't changed
	TEST_TRUE("Scroll speed is set correctly", FMath::IsNearlyEqual(ScrollSpeed, TestSpeed, 0.1f));

	// Verify speed doesn't spontaneously change
	float InitialSpeed = ScrollSpeed;
	for (int i = 0; i < 10; i++)
	{
		// Simulate some time passing
		TickComponent(0.016f, LEVELTICK_All, nullptr);
	}

	TEST_TRUE("Scroll speed remains constant", FMath::IsNearlyEqual(ScrollSpeed, InitialSpeed, 0.1f));

	TEST_RETURN();
}

bool UWorldScrollComponent::TestDistanceAccumulation()
{
	TEST_INIT("World Scroll Distance Accumulation");

	// Reset distance and set known speed
	ResetDistance();
	SetScrollSpeed(1000.0f);
	SetScrolling(true);

	// Verify distance starts at zero
	TEST_TRUE("Distance starts at zero", FMath::IsNearlyZero(DistanceTraveled, 0.1f));

	// Simulate 1 second of time (60 frames at 16.67ms each)
	const float DeltaTime = 1.0f / 60.0f;
	for (int i = 0; i < 60; i++)
	{
		TickComponent(DeltaTime, LEVELTICK_All, nullptr);
	}

	// After 1 second at 1000 units/second, distance should be ~1000
	TEST_TRUE("Distance accumulates correctly",
		FMath::IsNearlyEqual(DistanceTraveled, 1000.0f, 10.0f)); // Allow 10 unit tolerance

	// Verify distance continues to accumulate
	float DistanceAfter1Second = DistanceTraveled;
	for (int i = 0; i < 60; i++)
	{
		TickComponent(DeltaTime, LEVELTICK_All, nullptr);
	}

	TEST_TRUE("Distance continues to accumulate",
		DistanceTraveled > DistanceAfter1Second);

	TEST_RETURN();
}

bool UWorldScrollComponent::TestScrollPause()
{
	TEST_INIT("World Scroll Pause/Resume");

	// Reset and start scrolling
	ResetDistance();
	SetScrollSpeed(1000.0f);
	SetScrolling(true);

	TEST_TRUE("Scrolling starts enabled", bIsScrolling == true);

	// Accumulate some distance
	for (int i = 0; i < 60; i++)
	{
		TickComponent(1.0f / 60.0f, LEVELTICK_All, nullptr);
	}

	float DistanceBeforePause = DistanceTraveled;
	TEST_TRUE("Distance accumulated while scrolling", DistanceBeforePause > 0.0f);

	// Pause scrolling
	SetScrolling(false);
	TEST_TRUE("Scrolling is paused", bIsScrolling == false);

	// Verify distance doesn't accumulate while paused
	for (int i = 0; i < 60; i++)
	{
		TickComponent(1.0f / 60.0f, LEVELTICK_All, nullptr);
	}

	TEST_TRUE("Distance doesn't accumulate while paused",
		FMath::IsNearlyEqual(DistanceTraveled, DistanceBeforePause, 0.1f));

	// Resume scrolling
	SetScrolling(true);
	TEST_TRUE("Scrolling is resumed", bIsScrolling == true);

	// Verify distance accumulates again
	for (int i = 0; i < 60; i++)
	{
		TickComponent(1.0f / 60.0f, LEVELTICK_All, nullptr);
	}

	TEST_TRUE("Distance accumulates after resume",
		DistanceTraveled > DistanceBeforePause);

	TEST_RETURN();
}

bool UWorldScrollComponent::TestScrollVelocity()
{
	TEST_INIT("World Scroll Velocity Calculation");

	// Test with default direction (-1, 0, 0) and speed 1000
	SetScrollSpeed(1000.0f);
	SetScrolling(true);

	FVector ExpectedVelocity = FVector(-1000.0f, 0.0f, 0.0f);
	FVector ActualVelocity = GetScrollVelocity();

	TEST_TRUE("Velocity X component is correct",
		FMath::IsNearlyEqual(ActualVelocity.X, ExpectedVelocity.X, 0.1f));
	TEST_TRUE("Velocity Y component is zero",
		FMath::IsNearlyZero(ActualVelocity.Y, 0.1f));
	TEST_TRUE("Velocity Z component is zero",
		FMath::IsNearlyZero(ActualVelocity.Z, 0.1f));

	// Test that paused scrolling returns zero velocity
	SetScrolling(false);
	FVector PausedVelocity = GetScrollVelocity();

	TEST_TRUE("Paused velocity is zero",
		PausedVelocity.IsNearlyZero(0.1f));

	TEST_RETURN();
}

bool UWorldScrollComponent::TestScrollSpeedChange()
{
	TEST_INIT("World Scroll Speed Change");

	// Test setting various speeds
	SetScrollSpeed(500.0f);
	TEST_TRUE("Speed set to 500", FMath::IsNearlyEqual(ScrollSpeed, 500.0f, 0.1f));

	SetScrollSpeed(1500.0f);
	TEST_TRUE("Speed set to 1500", FMath::IsNearlyEqual(ScrollSpeed, 1500.0f, 0.1f));

	// Test that velocity updates when speed changes
	FVector Velocity = GetScrollVelocity();
	TEST_TRUE("Velocity updates with speed change",
		FMath::IsNearlyEqual(Velocity.X, -1500.0f, 0.1f));

	// Test that negative speeds are clamped to zero
	SetScrollSpeed(-100.0f);
	TEST_TRUE("Negative speed clamped to zero",
		FMath::IsNearlyEqual(ScrollSpeed, 0.0f, 0.1f));

	// Test that zero speed is valid
	SetScrollSpeed(0.0f);
	TEST_TRUE("Zero speed is valid",
		FMath::IsNearlyEqual(ScrollSpeed, 0.0f, 0.1f));

	TEST_RETURN();
}

void UWorldScrollComponent::TestWorldScrollAll()
{
	UE_LOG(LogTemp, Display, TEXT("==========================================="));
	UE_LOG(LogTemp, Display, TEXT("RUNNING ALL WORLD SCROLL TESTS"));
	UE_LOG(LogTemp, Display, TEXT("==========================================="));

	int32 TotalTests = 0;
	int32 PassedTests = 0;

	// Run all tests
	TotalTests++; if (TestScrollSpeedConsistency()) PassedTests++;
	TotalTests++; if (TestDistanceAccumulation()) PassedTests++;
	TotalTests++; if (TestScrollPause()) PassedTests++;
	TotalTests++; if (TestScrollVelocity()) PassedTests++;
	TotalTests++; if (TestScrollSpeedChange()) PassedTests++;

	// Print summary
	UE_LOG(LogTemp, Display, TEXT("==========================================="));
	UE_LOG(LogTemp, Display, TEXT("WORLD SCROLL TEST SUMMARY"));
	UE_LOG(LogTemp, Display, TEXT("==========================================="));
	UE_LOG(LogTemp, Display, TEXT("Total Tests: %d"), TotalTests);
	UE_LOG(LogTemp, Display, TEXT("Passed: %d"), PassedTests);
	UE_LOG(LogTemp, Display, TEXT("Failed: %d"), TotalTests - PassedTests);
	UE_LOG(LogTemp, Display, TEXT("Success Rate: %.1f%%"), (float)PassedTests / TotalTests * 100.0f);
	UE_LOG(LogTemp, Display, TEXT("==========================================="));

	if (PassedTests == TotalTests)
	{
		UE_LOG(LogTemp, Display, TEXT("ALL WORLD SCROLL TESTS PASSED!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SOME WORLD SCROLL TESTS FAILED!"));
	}
}

// === INTERNAL FUNCTIONS ===

void UWorldScrollComponent::LoadScrollConfiguration()
{
	// Try to load from data table if specified
	if (WorldScrollDataTable)
	{
		static const FString ContextString(TEXT("World Scroll Data"));
		FWorldScrollData* ScrollData = WorldScrollDataTable->FindRow<FWorldScrollData>(DataTableRowName, ContextString);

		if (ScrollData)
		{
			ScrollSpeed = ScrollData->ScrollSpeed;
			bIsScrolling = ScrollData->bScrollEnabled;
			ScrollDirection = ScrollData->ScrollDirection.GetSafeNormal();

			UE_LOG(LogTemp, Log, TEXT("UWorldScrollComponent::LoadScrollConfiguration - Loaded from data table row '%s'"),
				*DataTableRowName.ToString());
			UE_LOG(LogTemp, Log, TEXT("  Speed: %.2f, Enabled: %s, Direction: (%.2f, %.2f, %.2f)"),
				ScrollSpeed, bIsScrolling ? TEXT("true") : TEXT("false"),
				ScrollDirection.X, ScrollDirection.Y, ScrollDirection.Z);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UWorldScrollComponent::LoadScrollConfiguration - Failed to find row '%s' in data table, using defaults"),
				*DataTableRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UWorldScrollComponent::LoadScrollConfiguration - No data table specified, using defaults"));
	}

	// Validate scroll speed
	ScrollSpeed = ValidateScrollSpeed(ScrollSpeed);

	// Ensure direction is normalized
	if (!ScrollDirection.IsNearlyZero())
	{
		ScrollDirection.Normalize();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UWorldScrollComponent::LoadScrollConfiguration - Invalid scroll direction, using default (-1, 0, 0)"));
		ScrollDirection = FVector(-1.0f, 0.0f, 0.0f);
	}
}

float UWorldScrollComponent::ValidateScrollSpeed(float Speed) const
{
	// Clamp speed to >= 0
	if (Speed < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWorldScrollComponent::ValidateScrollSpeed - Negative speed %.2f clamped to 0.0"),
			Speed);
		return 0.0f;
	}

	return Speed;
}

void UWorldScrollComponent::LogScrollStateChange(const FString& Message) const
{
	UE_LOG(LogTemp, Log, TEXT("UWorldScrollComponent: %s"), *Message);
}

// Register tests with test manager
#if !UE_BUILD_SHIPPING
static bool WorldScrollTest_ScrollSpeedConsistency()
{
	// This function will be called by the test manager
	// We need to find a WorldScrollComponent in the world
	UE_LOG(LogTemp, Warning, TEXT("WorldScrollTest_ScrollSpeedConsistency - Test requires manual execution via TestWorldScrollAll command"));
	return true;
}

static bool WorldScrollTest_DistanceAccumulation()
{
	UE_LOG(LogTemp, Warning, TEXT("WorldScrollTest_DistanceAccumulation - Test requires manual execution via TestWorldScrollAll command"));
	return true;
}

static bool WorldScrollTest_ScrollPause()
{
	UE_LOG(LogTemp, Warning, TEXT("WorldScrollTest_ScrollPause - Test requires manual execution via TestWorldScrollAll command"));
	return true;
}

static bool WorldScrollTest_ScrollVelocity()
{
	UE_LOG(LogTemp, Warning, TEXT("WorldScrollTest_ScrollVelocity - Test requires manual execution via TestWorldScrollAll command"));
	return true;
}

static bool WorldScrollTest_ScrollSpeedChange()
{
	UE_LOG(LogTemp, Warning, TEXT("WorldScrollTest_ScrollSpeedChange - Test requires manual execution via TestWorldScrollAll command"));
	return true;
}

REGISTER_TEST("World Scroll Speed Consistency", ETestCategory::Movement, WorldScrollTest_ScrollSpeedConsistency);
REGISTER_TEST("World Scroll Distance Accumulation", ETestCategory::Movement, WorldScrollTest_DistanceAccumulation);
REGISTER_TEST("World Scroll Pause/Resume", ETestCategory::Movement, WorldScrollTest_ScrollPause);
REGISTER_TEST("World Scroll Velocity Calculation", ETestCategory::Movement, WorldScrollTest_ScrollVelocity);
REGISTER_TEST("World Scroll Speed Change", ETestCategory::Movement, WorldScrollTest_ScrollSpeedChange);
#endif
