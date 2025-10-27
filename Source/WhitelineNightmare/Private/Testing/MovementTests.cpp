// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "WarRig/LaneSystemComponent.h"
#include "WarRig/WarRigPawn.h"
#include "World/WorldScrollComponent.h"
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

// Helper function to create a lane system component for testing
static ULaneSystemComponent* CreateTestLaneSystemComponent()
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

	// Create and attach the lane system component
	ULaneSystemComponent* LaneSystem = NewObject<ULaneSystemComponent>(DummyActor);
	if (LaneSystem)
	{
		LaneSystem->RegisterComponent();
	}

	return LaneSystem;
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
	UWorldScrollComponent* WorldScroll = NewObject<UWorldScrollComponent>(DummyActor);
	if (WorldScroll)
	{
		WorldScroll->RegisterComponent();
	}

	return WorldScroll;
}

/**
 * Test: Lane System Bounds
 * Verify that the lane system prevents movement beyond lane limits
 */
static bool TestLaneSystemBounds()
{
	ULaneSystemComponent* LaneSystem = CreateTestLaneSystemComponent();
	TEST_NOT_NULL(LaneSystem, "Lane system component should be created");

	// Initialize with 5 lanes
	LaneSystem->Initialize(400.0f, 5);

	// Start in center lane (index 2 for 5 lanes)
	int32 CurrentLane = LaneSystem->GetCurrentLane();
	TEST_EQUAL(CurrentLane, 2, "Should start in center lane (index 2)");

	// Try to move left beyond bounds (from lane 2 to lane -1)
	for (int32 i = 0; i < 3; ++i)
	{
		LaneSystem->RequestLaneChange(-1);
		// Wait for lane change to complete (simulate multiple ticks)
		for (int32 tick = 0; tick < 10; ++tick)
		{
			LaneSystem->SimulateTick(0.1f);
		}
	}

	CurrentLane = LaneSystem->GetCurrentLane();
	TEST_EQUAL(CurrentLane, 0, "Should be at leftmost lane (index 0)");

	// Try to move left one more time - should be rejected
	bool bChangeAccepted = LaneSystem->RequestLaneChange(-1);
	TEST_FALSE(bChangeAccepted, "Lane change beyond left boundary should be rejected");
	TEST_EQUAL(LaneSystem->GetCurrentLane(), 0, "Should still be at leftmost lane");

	// Try to move right beyond bounds (from lane 0 to lane 5)
	for (int32 i = 0; i < 5; ++i)
	{
		LaneSystem->RequestLaneChange(1);
		// Wait for lane change to complete
		for (int32 tick = 0; tick < 10; ++tick)
		{
			LaneSystem->SimulateTick(0.1f);
		}
	}

	CurrentLane = LaneSystem->GetCurrentLane();
	TEST_EQUAL(CurrentLane, 4, "Should be at rightmost lane (index 4)");

	// Try to move right one more time - should be rejected
	bChangeAccepted = LaneSystem->RequestLaneChange(1);
	TEST_FALSE(bChangeAccepted, "Lane change beyond right boundary should be rejected");
	TEST_EQUAL(LaneSystem->GetCurrentLane(), 4, "Should still be at rightmost lane");

	// Cleanup
	if (LaneSystem->GetOwner())
	{
		LaneSystem->GetOwner()->Destroy();
	}

	TEST_SUCCESS("TestLaneSystemBounds");
}

/**
 * Test: Lane Transition Speed
 * Verify that lane transitions interpolate smoothly at the specified speed
 */
static bool TestLaneTransitionSpeed()
{
	ULaneSystemComponent* LaneSystem = CreateTestLaneSystemComponent();
	TEST_NOT_NULL(LaneSystem, "Lane system component should be created");

	// Initialize with known parameters
	float LaneWidth = 400.0f;
	float LaneChangeSpeed = 400.0f; // Should take 1 second to change lanes
	LaneSystem->Initialize(LaneWidth, 5);
	LaneSystem->SetLaneChangeSpeed(LaneChangeSpeed);

	// Start in center lane
	int32 StartLane = LaneSystem->GetCurrentLane();
	float StartY = LaneSystem->GetCurrentYPosition();

	// Request lane change to the right
	bool bChangeAccepted = LaneSystem->RequestLaneChange(1);
	TEST_TRUE(bChangeAccepted, "Lane change should be accepted");
	TEST_TRUE(LaneSystem->IsChangingLanes(), "Should be changing lanes");

	// Tick for 1 second (10 ticks of 0.1s each)
	float TotalTime = 0.0f;
	float DeltaTime = 0.1f;
	int32 TickCount = 0;

	while (LaneSystem->IsChangingLanes() && TickCount < 20) // Max 2 seconds to prevent infinite loop
	{
		LaneSystem->SimulateTick(DeltaTime);
		TotalTime += DeltaTime;
		TickCount++;
	}

	// Verify lane change completed
	TEST_FALSE(LaneSystem->IsChangingLanes(), "Lane change should be complete");
	TEST_EQUAL(LaneSystem->GetCurrentLane(), StartLane + 1, "Should be in next lane");

	// Verify position changed by one lane width
	float EndY = LaneSystem->GetCurrentYPosition();
	float ExpectedDeltaY = LaneWidth;
	float ActualDeltaY = FMath::Abs(EndY - StartY);
	TEST_NEAR(ActualDeltaY, ExpectedDeltaY, 1.0f, "Position should have changed by one lane width");

	// Verify time taken is approximately correct (should be ~1 second)
	TEST_NEAR(TotalTime, 1.0f, 0.2f, "Lane change should take approximately 1 second");

	// Cleanup
	if (LaneSystem->GetOwner())
	{
		LaneSystem->GetOwner()->Destroy();
	}

	TEST_SUCCESS("TestLaneTransitionSpeed");
}

/**
 * Test: Tile Pool Recycling
 * Verify that ground tiles are reused from the pool, not destroyed and recreated
 */
static bool TestTilePoolRecycling()
{
	// TODO: Implement when GroundTilePoolComponent is integrated with the game
	// For now, this is a placeholder
	UE_LOG(LogTemp, Warning, TEXT("TestTilePoolRecycling: Not yet implemented - requires integration with game mode"));
	TEST_SUCCESS("TestTilePoolRecycling (placeholder)");
}

/**
 * Test: Scroll Speed Consistency
 * Verify that world scroll speed remains constant during gameplay
 */
static bool TestScrollSpeedConsistency()
{
	UWorldScrollComponent* WorldScroll = CreateTestWorldScrollComponent();
	TEST_NOT_NULL(WorldScroll, "World scroll component should be created");

	// Initialize with known scroll speed
	float ExpectedSpeed = 1000.0f;
	WorldScroll->InitializeWithSpeed(ExpectedSpeed);

	// Verify initial speed
	TEST_EQUAL(WorldScroll->GetScrollSpeed(), ExpectedSpeed, "Scroll speed should match initialization value");

	// Tick several times and verify speed remains constant
	for (int32 i = 0; i < 10; ++i)
	{
		WorldScroll->SimulateTick(0.1f);
		TEST_EQUAL(WorldScroll->GetScrollSpeed(), ExpectedSpeed, "Scroll speed should remain constant");
	}

	// Verify distance traveled is accumulating correctly
	float ExpectedDistance = ExpectedSpeed * 1.0f; // 10 ticks * 0.1s = 1.0s
	float ActualDistance = WorldScroll->GetDistanceTraveled();
	TEST_NEAR(ActualDistance, ExpectedDistance, 10.0f, "Distance traveled should accumulate correctly");

	// Change speed and verify it updates
	float NewSpeed = 500.0f;
	WorldScroll->SetScrollSpeed(NewSpeed);
	TEST_EQUAL(WorldScroll->GetScrollSpeed(), NewSpeed, "Scroll speed should update when changed");

	// Cleanup
	if (WorldScroll->GetOwner())
	{
		WorldScroll->GetOwner()->Destroy();
	}

	TEST_SUCCESS("TestScrollSpeedConsistency");
}

/**
 * Test: War Rig Data Loading
 * Verify that war rig data loads correctly from data table
 */
static bool TestWarRigDataLoading()
{
	// TODO: Implement when data table is created
	// For now, this is a placeholder
	UE_LOG(LogTemp, Warning, TEXT("TestWarRigDataLoading: Not yet implemented - requires data table asset"));
	TEST_SUCCESS("TestWarRigDataLoading (placeholder)");
}

// Register tests with the test manager
REGISTER_TEST("Lane System Bounds", ETestCategory::Movement, TestLaneSystemBounds);
REGISTER_TEST("Lane Transition Speed", ETestCategory::Movement, TestLaneTransitionSpeed);
REGISTER_TEST("Tile Pool Recycling", ETestCategory::Movement, TestTilePoolRecycling);
REGISTER_TEST("Scroll Speed Consistency", ETestCategory::Movement, TestScrollSpeedConsistency);
REGISTER_TEST("War Rig Data Loading", ETestCategory::Movement, TestWarRigDataLoading);

#endif // !UE_BUILD_SHIPPING
