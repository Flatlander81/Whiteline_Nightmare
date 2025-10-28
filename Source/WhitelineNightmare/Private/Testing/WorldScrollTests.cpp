// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "Core/WorldScrollComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING

// Helper function to get a valid world for testing
static UWorld* GetTestWorld()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
		{
			return Context.World();
		}
	}
	return nullptr;
}

// Helper function to create a world scroll component for testing
static UWorldScrollComponent* CreateTestWorldScrollComponent()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	// Create a dummy actor to hold the component
	AActor* DummyActor = World->SpawnActor<AActor>();
	if (!DummyActor)
	{
		return nullptr;
	}

	// Create and attach the world scroll component
	UWorldScrollComponent* ScrollComponent = NewObject<UWorldScrollComponent>(DummyActor);
	if (ScrollComponent)
	{
		ScrollComponent->RegisterComponent();
	}

	return ScrollComponent;
}

/**
 * Test: Scroll Speed Consistency
 * Verify that scroll speed remains constant over multiple frames
 */
static bool WorldScrollTest_ScrollSpeedConsistency()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set a specific scroll speed
	const float TestSpeed = 500.0f;
	ScrollComponent->SetScrollSpeed(TestSpeed);

	// Verify speed is set correctly
	TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), TestSpeed, 0.01f, "Scroll speed should be set correctly");

	// Tick multiple times and verify speed remains consistent
	for (int32 i = 0; i < 10; ++i)
	{
		ScrollComponent->TickComponent(0.016f, ELevelTick::LEVELTICK_All, nullptr);
		TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), TestSpeed, 0.01f, "Scroll speed should remain consistent");
	}

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_ScrollSpeedConsistency");
}

/**
 * Test: Distance Accumulation
 * Verify that distance increases correctly over time
 */
static bool WorldScrollTest_DistanceAccumulation()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set scroll speed and reset distance
	const float TestSpeed = 1000.0f;
	ScrollComponent->SetScrollSpeed(TestSpeed);
	ScrollComponent->ResetDistance();
	ScrollComponent->SetScrolling(true);

	// Initial distance should be zero
	TEST_NEARLY_EQUAL(ScrollComponent->GetDistanceTraveled(), 0.0f, 0.01f, "Initial distance should be zero");

	// Simulate 1 second of scrolling (60 frames at 16.67ms each)
	const float DeltaTime = 0.016667f;
	for (int32 i = 0; i < 60; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}

	// Distance should be approximately speed * time (1000 * 1.0 = 1000)
	const float ExpectedDistance = TestSpeed * (DeltaTime * 60.0f);
	TEST_NEARLY_EQUAL(ScrollComponent->GetDistanceTraveled(), ExpectedDistance, 1.0f, "Distance should accumulate correctly");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_DistanceAccumulation");
}

/**
 * Test: Scroll Pause
 * Verify that scrolling can be paused and resumed
 */
static bool WorldScrollTest_ScrollPause()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set scroll speed and reset distance
	const float TestSpeed = 1000.0f;
	ScrollComponent->SetScrollSpeed(TestSpeed);
	ScrollComponent->ResetDistance();
	ScrollComponent->SetScrolling(true);

	// Scroll for a bit
	const float DeltaTime = 0.016667f;
	for (int32 i = 0; i < 30; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}

	const float DistanceAfterScrolling = ScrollComponent->GetDistanceTraveled();
	TEST_TRUE(DistanceAfterScrolling > 0.0f, "Distance should increase while scrolling");

	// Pause scrolling
	ScrollComponent->SetScrolling(false);
	TEST_FALSE(ScrollComponent->IsScrolling(), "Scrolling should be paused");

	// Tick while paused
	for (int32 i = 0; i < 30; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}

	// Distance should not change while paused
	TEST_NEARLY_EQUAL(ScrollComponent->GetDistanceTraveled(), DistanceAfterScrolling, 0.01f, "Distance should not change while paused");

	// Resume scrolling
	ScrollComponent->SetScrolling(true);
	TEST_TRUE(ScrollComponent->IsScrolling(), "Scrolling should be resumed");

	// Tick while scrolling again
	for (int32 i = 0; i < 30; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}

	// Distance should increase again
	TEST_TRUE(ScrollComponent->GetDistanceTraveled() > DistanceAfterScrolling, "Distance should increase after resuming");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_ScrollPause");
}

/**
 * Test: Scroll Velocity
 * Verify that velocity calculation is correct
 */
static bool WorldScrollTest_ScrollVelocity()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set scroll speed and direction
	const float TestSpeed = 1000.0f;
	const FVector TestDirection(-1.0f, 0.0f, 0.0f);
	ScrollComponent->SetScrollSpeed(TestSpeed);
	ScrollComponent->SetScrollDirection(TestDirection);
	ScrollComponent->SetScrolling(true);

	// Calculate expected velocity
	const FVector ExpectedVelocity = TestDirection.GetSafeNormal() * TestSpeed;
	const FVector ActualVelocity = ScrollComponent->GetScrollVelocity();

	// Verify velocity components
	TEST_NEARLY_EQUAL(ActualVelocity.X, ExpectedVelocity.X, 0.01f, "Velocity X should be correct");
	TEST_NEARLY_EQUAL(ActualVelocity.Y, ExpectedVelocity.Y, 0.01f, "Velocity Y should be correct");
	TEST_NEARLY_EQUAL(ActualVelocity.Z, ExpectedVelocity.Z, 0.01f, "Velocity Z should be correct");

	// Verify velocity magnitude
	TEST_NEARLY_EQUAL(ActualVelocity.Size(), TestSpeed, 0.01f, "Velocity magnitude should equal scroll speed");

	// When paused, velocity should be zero
	ScrollComponent->SetScrolling(false);
	const FVector ZeroVelocity = ScrollComponent->GetScrollVelocity();
	TEST_NEARLY_EQUAL(ZeroVelocity.X, 0.0f, 0.01f, "Velocity X should be zero when paused");
	TEST_NEARLY_EQUAL(ZeroVelocity.Y, 0.0f, 0.01f, "Velocity Y should be zero when paused");
	TEST_NEARLY_EQUAL(ZeroVelocity.Z, 0.0f, 0.01f, "Velocity Z should be zero when paused");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_ScrollVelocity");
}

/**
 * Test: Scroll Speed Change
 * Verify that runtime speed changes work correctly
 */
static bool WorldScrollTest_ScrollSpeedChange()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set initial speed
	const float InitialSpeed = 500.0f;
	ScrollComponent->SetScrollSpeed(InitialSpeed);
	TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), InitialSpeed, 0.01f, "Initial speed should be set");

	// Change speed to a higher value
	const float NewSpeed = 1500.0f;
	ScrollComponent->SetScrollSpeed(NewSpeed);
	TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), NewSpeed, 0.01f, "Speed should change to new value");

	// Change speed to zero (paused state)
	ScrollComponent->SetScrollSpeed(0.0f);
	TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), 0.0f, 0.01f, "Speed should be zero");

	// Try to set negative speed (should be clamped to zero)
	ScrollComponent->SetScrollSpeed(-100.0f);
	TEST_NEARLY_EQUAL(ScrollComponent->GetScrollSpeed(), 0.0f, 0.01f, "Negative speed should be clamped to zero");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_ScrollSpeedChange");
}

/**
 * Test: Direction Normalization
 * Verify that scroll direction is always normalized
 */
static bool WorldScrollTest_DirectionNormalization()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Set an unnormalized direction
	const FVector UnnormalizedDirection(3.0f, 4.0f, 0.0f); // Length = 5
	ScrollComponent->SetScrollDirection(UnnormalizedDirection);

	// Verify direction is normalized
	const FVector Direction = ScrollComponent->GetScrollDirection();
	const float Length = Direction.Size();
	TEST_NEARLY_EQUAL(Length, 1.0f, 0.01f, "Direction should be normalized to length 1");

	// Verify direction components are correct (3/5, 4/5, 0)
	TEST_NEARLY_EQUAL(Direction.X, 0.6f, 0.01f, "Direction X should be normalized");
	TEST_NEARLY_EQUAL(Direction.Y, 0.8f, 0.01f, "Direction Y should be normalized");
	TEST_NEARLY_EQUAL(Direction.Z, 0.0f, 0.01f, "Direction Z should be normalized");

	// Try to set zero direction (should use default)
	ScrollComponent->SetScrollDirection(FVector::ZeroVector);
	const FVector DefaultDirection = ScrollComponent->GetScrollDirection();
	TEST_NEARLY_EQUAL(DefaultDirection.X, -1.0f, 0.01f, "Default direction X should be -1");
	TEST_NEARLY_EQUAL(DefaultDirection.Y, 0.0f, 0.01f, "Default direction Y should be 0");
	TEST_NEARLY_EQUAL(DefaultDirection.Z, 0.0f, 0.01f, "Default direction Z should be 0");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_DirectionNormalization");
}

/**
 * Test: Distance Reset
 * Verify that distance counter can be reset
 */
static bool WorldScrollTest_DistanceReset()
{
	UWorldScrollComponent* ScrollComponent = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(ScrollComponent, "Scroll component should be created");

	// Scroll for a bit to accumulate distance
	ScrollComponent->SetScrollSpeed(1000.0f);
	ScrollComponent->SetScrolling(true);

	const float DeltaTime = 0.016667f;
	for (int32 i = 0; i < 60; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}

	// Verify distance is non-zero
	TEST_TRUE(ScrollComponent->GetDistanceTraveled() > 0.0f, "Distance should be non-zero after scrolling");

	// Reset distance
	ScrollComponent->ResetDistance();
	TEST_NEARLY_EQUAL(ScrollComponent->GetDistanceTraveled(), 0.0f, 0.01f, "Distance should be zero after reset");

	// Verify scrolling still works after reset
	for (int32 i = 0; i < 30; ++i)
	{
		ScrollComponent->TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr);
	}
	TEST_TRUE(ScrollComponent->GetDistanceTraveled() > 0.0f, "Distance should accumulate again after reset");

	// Cleanup
	if (ScrollComponent->GetOwner())
	{
		ScrollComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("WorldScrollTest_DistanceReset");
}

/**
 * Test: Run All World Scroll Tests
 * Comprehensive test that runs all world scroll tests and provides a detailed summary
 */
static bool WorldScrollTest_TestAll()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  WORLD SCROLL SYSTEM - ALL TESTS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));

	int32 PassedTests = 0;
	int32 TotalTests = 0;

	// Define test structure
	struct FTestInfo
	{
		FString Name;
		bool (*Function)();
		bool bPassed;
	};

	TArray<FTestInfo> Tests = {
		{ TEXT("Scroll Speed Consistency"), &WorldScrollTest_ScrollSpeedConsistency, false },
		{ TEXT("Distance Accumulation"), &WorldScrollTest_DistanceAccumulation, false },
		{ TEXT("Scroll Pause/Resume"), &WorldScrollTest_ScrollPause, false },
		{ TEXT("Scroll Velocity Calculation"), &WorldScrollTest_ScrollVelocity, false },
		{ TEXT("Runtime Speed Changes"), &WorldScrollTest_ScrollSpeedChange, false },
		{ TEXT("Direction Normalization"), &WorldScrollTest_DirectionNormalization, false },
		{ TEXT("Distance Counter Reset"), &WorldScrollTest_DistanceReset, false }
	};

	TotalTests = Tests.Num();

	// Run each test
	for (int32 i = 0; i < Tests.Num(); ++i)
	{
		FTestInfo& Test = Tests[i];

		UE_LOG(LogTemp, Log, TEXT("  [%d/%d] Running: %s..."), i + 1, TotalTests, *Test.Name);

		// Run the test
		Test.bPassed = Test.Function();

		if (Test.bPassed)
		{
			PassedTests++;
			UE_LOG(LogTemp, Log, TEXT("        [PASS] %s"), *Test.Name);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("        [FAIL] %s"), *Test.Name);
		}

		UE_LOG(LogTemp, Log, TEXT(""));
	}

	// Print summary
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  TEST SUMMARY"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));

	// Individual test results
	for (const FTestInfo& Test : Tests)
	{
		FString Status = Test.bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
		FColor Color = Test.bPassed ? FColor::Green : FColor::Red;

		UE_LOG(LogTemp, Log, TEXT("  %s %s"), *Status, *Test.Name);
	}

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("  Total Tests: %d"), TotalTests);
	UE_LOG(LogTemp, Log, TEXT("  Passed: %d"), PassedTests);
	UE_LOG(LogTemp, Log, TEXT("  Failed: %d"), TotalTests - PassedTests);

	const float PassPercentage = (float)PassedTests / (float)TotalTests * 100.0f;
	UE_LOG(LogTemp, Log, TEXT("  Pass Rate: %.1f%%"), PassPercentage);

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	if (PassedTests == TotalTests)
	{
		UE_LOG(LogTemp, Log, TEXT("  ALL TESTS PASSED!"));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("  SOME TESTS FAILED!"));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return false;
	}
}

/**
 * Register all world scroll tests with the test manager
 * This function should be called from WhitelineNightmareGameMode::BeginPlay()
 */
void RegisterWorldScrollTests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	TestManager->RegisterTest(TEXT("WorldScroll_ScrollSpeedConsistency"), ETestCategory::Movement, &WorldScrollTest_ScrollSpeedConsistency);
	TestManager->RegisterTest(TEXT("WorldScroll_DistanceAccumulation"), ETestCategory::Movement, &WorldScrollTest_DistanceAccumulation);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollPause"), ETestCategory::Movement, &WorldScrollTest_ScrollPause);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollVelocity"), ETestCategory::Movement, &WorldScrollTest_ScrollVelocity);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollSpeedChange"), ETestCategory::Movement, &WorldScrollTest_ScrollSpeedChange);
	TestManager->RegisterTest(TEXT("WorldScroll_DirectionNormalization"), ETestCategory::Movement, &WorldScrollTest_DirectionNormalization);
	TestManager->RegisterTest(TEXT("WorldScroll_DistanceReset"), ETestCategory::Movement, &WorldScrollTest_DistanceReset);

	// Register comprehensive test-all function
	TestManager->RegisterTest(TEXT("WorldScroll_TestAll"), ETestCategory::Movement, &WorldScrollTest_TestAll);
}

#endif // !UE_BUILD_SHIPPING
