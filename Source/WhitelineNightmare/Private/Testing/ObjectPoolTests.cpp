// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "Testing/ObjectPoolTestHelpers.h"
#include "Core/ObjectPoolComponent.h"
#include "Core/ObjectPoolTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "World/GroundTile.h"
#include "World/GroundTileManager.h"

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

// Helper function to create a pool component for testing
static UObjectPoolComponent* CreateTestPoolComponent()
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

	// Create and attach the pool component
	UObjectPoolComponent* PoolComponent = NewObject<UObjectPoolComponent>(DummyActor);
	if (PoolComponent)
	{
		PoolComponent->RegisterComponent();
	}

	return PoolComponent;
}

/**
 * Test: Pool Initialization
 * Verify that the pool creates the correct number of actors on initialization
 */
static bool ObjectPoolTest_Initialization()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	// Create a pool configuration
	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;

	// Initialize the pool
	bool bInitSuccess = PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	// Verify pool size
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Pool should have 5 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Pool should have 0 active objects");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 5, "Pool total size should be 5");
	TEST_TRUE(PoolComponent->HasAvailable(), "Pool should have available objects");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_Initialization");
}

/**
 * Test: Get From Pool
 * Verify that GetFromPool returns a valid actor and updates counts correctly
 */
static bool ObjectPoolTest_GetFromPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	FVector SpawnLocation(100.0f, 200.0f, 300.0f);
	FRotator SpawnRotation(0.0f, 90.0f, 0.0f);
	AActor* Actor = PoolComponent->GetFromPool(SpawnLocation, SpawnRotation);

	TEST_NOT_NULL(Actor, "GetFromPool should return a valid actor");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 2, "Pool should have 2 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 1, "Pool should have 1 active object");

	// Verify actor location and rotation
	FVector ActorLocation = Actor->GetActorLocation();
	TEST_NEARLY_EQUAL(ActorLocation.X, SpawnLocation.X, 0.1f, "Actor X location should match spawn location");
	TEST_NEARLY_EQUAL(ActorLocation.Y, SpawnLocation.Y, 0.1f, "Actor Y location should match spawn location");
	TEST_NEARLY_EQUAL(ActorLocation.Z, SpawnLocation.Z, 0.1f, "Actor Z location should match spawn location");

	// Verify actor is visible and has collision enabled
	TEST_FALSE(Actor->IsHidden(), "Actor should be visible");

	// Verify OnActivated was called
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->ActivationCount, 1, "OnActivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_GetFromPool");
}

/**
 * Test: Return To Pool
 * Verify that ReturnToPool deactivates the actor and returns it to the available pool
 */
static bool ObjectPoolTest_ReturnToPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	AActor* Actor = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor, "GetFromPool should return a valid actor");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 1, "Pool should have 1 active object");

	// Return the actor to the pool
	bool bReturnSuccess = PoolComponent->ReturnToPool(Actor);
	TEST_TRUE(bReturnSuccess, "ReturnToPool should succeed");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Pool should have 3 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Pool should have 0 active objects");

	// Verify actor is hidden
	TEST_TRUE(Actor->IsHidden(), "Actor should be hidden");

	// Verify OnDeactivated was called
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->DeactivationCount, 1, "OnDeactivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ReturnToPool");
}

/**
 * Test: Pool Exhaustion
 * Verify behavior when the pool is exhausted (no auto-expand)
 */
static bool ObjectPoolTest_PoolExhaustion()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 2;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get all actors from the pool
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_NOT_NULL(Actor1, "First actor should be retrieved");
	TEST_NOT_NULL(Actor2, "Second actor should be retrieved");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 0, "Pool should be exhausted");
	TEST_FALSE(PoolComponent->HasAvailable(), "Pool should not have available objects");

	// Try to get another actor (should fail)
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NULL(Actor3, "GetFromPool should return null when pool is exhausted");

	// Return one actor and try again
	PoolComponent->ReturnToPool(Actor1);
	AActor* Actor4 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor4, "GetFromPool should succeed after returning an actor");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_PoolExhaustion");
}

/**
 * Test: Pool Reuse
 * Verify that the same actor instance is reused when returned to the pool
 */
static bool ObjectPoolTest_PoolReuse()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 1;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor1, "First GetFromPool should return a valid actor");

	// Return the actor to the pool
	PoolComponent->ReturnToPool(Actor1);

	// Get an actor from the pool again
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor2, "Second GetFromPool should return a valid actor");

	// Verify it's the same instance
	TEST_EQUAL(Actor1, Actor2, "Pool should reuse the same actor instance");

	// Verify activation/deactivation counts
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor2);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->ActivationCount, 2, "OnActivated should have been called twice");
		TEST_EQUAL(TestActor->DeactivationCount, 1, "OnDeactivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_PoolReuse");
}

/**
 * Test: Active Count Tracking
 * Verify that active/available counts are accurate as objects move between pools
 */
static bool ObjectPoolTest_ActiveCount()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Initial state
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Initial active count should be 0");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Initial available count should be 5");

	// Get 3 actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_EQUAL(PoolComponent->GetActiveCount(), 3, "Active count should be 3");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 2, "Available count should be 2");

	// Return 1 actor
	PoolComponent->ReturnToPool(Actor2);
	TEST_EQUAL(PoolComponent->GetActiveCount(), 2, "Active count should be 2 after returning one");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Available count should be 3 after returning one");

	// Clear pool (return all active)
	PoolComponent->ClearPool();
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Active count should be 0 after clearing");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Available count should be 5 after clearing");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ActiveCount");
}

/**
 * Test: Auto-Expand
 * Verify that the pool can auto-expand when exhausted
 */
static bool ObjectPoolTest_AutoExpand()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 2;
	Config.bAutoExpand = true;
	Config.MaxPoolSize = 4;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get all initial actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 2, "Initial pool size should be 2");

	// Get another actor (should auto-expand)
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor3, "GetFromPool should succeed with auto-expand");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 3, "Pool should have expanded to 3");

	// Get one more (should expand again)
	AActor* Actor4 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor4, "GetFromPool should succeed with auto-expand");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 4, "Pool should have expanded to 4");

	// Try to get another (should fail - max size reached)
	AActor* Actor5 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NULL(Actor5, "GetFromPool should fail when max pool size is reached");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 4, "Pool should remain at max size of 4");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_AutoExpand");
}

/**
 * Test: Reset Pool
 * Verify that ResetPool returns all objects and calls ResetState
 */
static bool ObjectPoolTest_ResetPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get some actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	ATestPoolableActor* TestActor1 = Cast<ATestPoolableActor>(Actor1);
	ATestPoolableActor* TestActor2 = Cast<ATestPoolableActor>(Actor2);

	TEST_EQUAL(PoolComponent->GetActiveCount(), 2, "Should have 2 active objects");

	// Reset the pool
	PoolComponent->ResetPool();

	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Should have 0 active objects after reset");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Should have 3 available objects after reset");

	// Verify ResetState was called
	if (TestActor1)
	{
		TEST_EQUAL(TestActor1->ResetCount, 1, "ResetState should have been called");
		TEST_EQUAL(TestActor1->ActivationCount, 0, "Activation count should be reset");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ResetPool");
}

// ============================================================================
// GROUND TILE TESTS
// ============================================================================

// Helper function to create a simple war rig stand-in for testing
static AActor* CreateTestWarRig(UWorld* World, const FVector& Location)
{
	if (!World)
	{
		return nullptr;
	}

	AActor* WarRig = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
	if (WarRig)
	{
		WarRig->SetActorLabel(TEXT("WarRig"));
	}
	return WarRig;
}

/**
 * Test: Tile Pool Recycling
 * Verify tiles are reused, not destroyed when they leave the visible area
 */
static bool GroundTileTest_TilePoolRecycling()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create a war rig for position reference
	AActor* WarRig = CreateTestWarRig(World, FVector::ZeroVector);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Create pool directly without accessing protected members
	AActor* PoolOwner = World->SpawnActor<AActor>();
	TEST_NOT_NULL(PoolOwner, "Pool owner should be created");

	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	bool bInitSuccess = Pool->Initialize(AGroundTile::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	TEST_EQUAL(Pool->GetTotalPoolSize(), 5, "Pool should have 5 tiles");

	// Get a tile from the pool
	AActor* Tile1Actor = Pool->GetFromPool(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(Tile1Actor, "First tile should be retrieved");
	AGroundTile* Tile1 = Cast<AGroundTile>(Tile1Actor);
	TEST_NOT_NULL(Tile1, "Should be a GroundTile");

	// Store the pointer to the first tile
	AGroundTile* OriginalTile = Tile1;

	// Return it to the pool
	Pool->ReturnToPool(Tile1);
	TEST_EQUAL(Pool->GetAvailableCount(), 5, "All tiles should be available again");

	// Get a tile again
	AActor* Tile2Actor = Pool->GetFromPool(FVector(3000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	AGroundTile* Tile2 = Cast<AGroundTile>(Tile2Actor);

	// Verify it's the SAME instance (reused, not destroyed and recreated)
	TEST_EQUAL(OriginalTile, Tile2, "Pool should reuse the same tile instance");
	TEST_EQUAL(Pool->GetTotalPoolSize(), 5, "Pool size should remain constant");

	// Cleanup
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (PoolOwner)
	{
		PoolOwner->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TilePoolRecycling");
}

/**
 * Test: Seamless Scrolling
 * Verify no gaps between tiles when positioned in sequence
 */
static bool GroundTileTest_SeamlessScrolling()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create pool
	AActor* PoolOwner = World->SpawnActor<AActor>();
	TEST_NOT_NULL(PoolOwner, "Pool owner should be created");

	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;
	bool bInitSuccess = Pool->Initialize(AGroundTile::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	const float TileSize = 2000.0f;

	// Spawn tiles in sequence
	TArray<AGroundTile*> Tiles;
	for (int32 i = 0; i < 3; ++i)
	{
		float XPosition = i * TileSize;
		AActor* TileActor = Pool->GetFromPool(FVector(XPosition, 0.0f, 0.0f), FRotator::ZeroRotator);
		AGroundTile* Tile = Cast<AGroundTile>(TileActor);
		TEST_NOT_NULL(Tile, "Tile should be created");

		Tile->SetTileLength(TileSize);
		Tiles.Add(Tile);
	}

	// Verify no gaps between tiles
	for (int32 i = 0; i < Tiles.Num() - 1; ++i)
	{
		AGroundTile* CurrentTile = Tiles[i];
		AGroundTile* NextTile = Tiles[i + 1];

		float CurrentTileEnd = CurrentTile->GetActorLocation().X + (TileSize / 2.0f);
		float NextTileStart = NextTile->GetActorLocation().X - (TileSize / 2.0f);

		// Tiles should be adjacent with no gap (allowing small floating point tolerance)
		float Gap = NextTileStart - CurrentTileEnd;
		TEST_NEARLY_EQUAL(Gap, 0.0f, 1.0f, "No gap should exist between tiles");
	}

	// Verify tiles form a continuous line
	float ExpectedTotalLength = TileSize * 3;
	float FirstTileStart = Tiles[0]->GetActorLocation().X - (TileSize / 2.0f);
	float LastTileEnd = Tiles[2]->GetActorLocation().X + (TileSize / 2.0f);
	float ActualTotalLength = LastTileEnd - FirstTileStart;

	TEST_NEARLY_EQUAL(ActualTotalLength, ExpectedTotalLength, 1.0f,
		"Total length should equal expected continuous length");

	// Cleanup
	if (PoolOwner)
	{
		PoolOwner->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_SeamlessScrolling");
}

/**
 * Test: Tile Positioning
 * Verify spawn positions are correct relative to war rig
 */
static bool GroundTileTest_TilePositioning()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create war rig at a specific position
	const FVector WarRigPosition(1000.0f, 0.0f, 0.0f);
	AActor* WarRig = CreateTestWarRig(World, WarRigPosition);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Create pool
	AActor* PoolOwner = World->SpawnActor<AActor>();
	TEST_NOT_NULL(PoolOwner, "Pool owner should be created");

	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	bool bInitSuccess = Pool->Initialize(AGroundTile::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	const float TileSize = 2000.0f;
	const float SpawnDistanceAhead = 5000.0f;

	// Calculate where the furthest tile should spawn
	float ExpectedFurthestPosition = WarRigPosition.X + SpawnDistanceAhead;

	// Spawn a tile at the expected furthest position
	AActor* TileActor = Pool->GetFromPool(FVector(ExpectedFurthestPosition, 0.0f, 0.0f), FRotator::ZeroRotator);
	AGroundTile* Tile = Cast<AGroundTile>(TileActor);
	TEST_NOT_NULL(Tile, "Tile should be created");

	// Verify tile is at correct position
	float ActualTileX = Tile->GetActorLocation().X;
	TEST_NEARLY_EQUAL(ActualTileX, ExpectedFurthestPosition, 1.0f,
		"Tile should be at correct spawn distance");

	// Verify tile is ahead of war rig
	TEST_TRUE(ActualTileX > WarRigPosition.X, "Tile should be ahead of war rig");

	// Calculate distance between war rig and tile
	float DistanceFromWarRig = ActualTileX - WarRigPosition.X;
	TEST_NEARLY_EQUAL(DistanceFromWarRig, SpawnDistanceAhead, 1.0f,
		"Distance should match spawn distance");

	// Test multiple tiles at different positions
	float TestPosition1 = WarRigPosition.X - 1000.0f;
	AActor* TestTileActor1 = Pool->GetFromPool(FVector(TestPosition1, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (TestTileActor1)
	{
		TEST_NEARLY_EQUAL(TestTileActor1->GetActorLocation().X, TestPosition1, 1.0f, "Tile at position 1");
		Pool->ReturnToPool(TestTileActor1);
	}

	float TestPosition2 = WarRigPosition.X;
	AActor* TestTileActor2 = Pool->GetFromPool(FVector(TestPosition2, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (TestTileActor2)
	{
		TEST_NEARLY_EQUAL(TestTileActor2->GetActorLocation().X, TestPosition2, 1.0f, "Tile at position 2");
		Pool->ReturnToPool(TestTileActor2);
	}

	float TestPosition3 = WarRigPosition.X + 2000.0f;
	AActor* TestTileActor3 = Pool->GetFromPool(FVector(TestPosition3, 0.0f, 0.0f), FRotator::ZeroRotator);
	if (TestTileActor3)
	{
		TEST_NEARLY_EQUAL(TestTileActor3->GetActorLocation().X, TestPosition3, 1.0f, "Tile at position 3");
		Pool->ReturnToPool(TestTileActor3);
	}

	// Cleanup
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (PoolOwner)
	{
		PoolOwner->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TilePositioning");
}

/**
 * Test: Pool Size
 * Verify correct number of tiles created and pool respects size limits
 */
static bool GroundTileTest_PoolSize()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	AActor* PoolOwner = World->SpawnActor<AActor>();
	TEST_NOT_NULL(PoolOwner, "Pool owner should be created");

	// Test pool size of 5
	UObjectPoolComponent* Pool1 = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool1->RegisterComponent();

	FObjectPoolConfig Config1;
	Config1.PoolSize = 5;
	Config1.bAutoExpand = false;

	bool bInitSuccess1 = Pool1->Initialize(AGroundTile::StaticClass(), Config1);
	TEST_TRUE(bInitSuccess1, "Pool should initialize with size 5");
	TEST_EQUAL(Pool1->GetTotalPoolSize(), 5, "Pool should have 5 tiles");
	TEST_EQUAL(Pool1->GetAvailableCount(), 5, "All 5 tiles should be available");
	TEST_EQUAL(Pool1->GetActiveCount(), 0, "No tiles should be active initially");

	// Get all tiles from pool
	TArray<AActor*> RetrievedTiles;
	for (int32 i = 0; i < 5; ++i)
	{
		AActor* Tile = Pool1->GetFromPool(FVector(i * 1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
		TEST_NOT_NULL(Tile, "Should retrieve tile from pool");
		RetrievedTiles.Add(Tile);
	}

	// Verify pool is exhausted
	TEST_EQUAL(Pool1->GetActiveCount(), 5, "All 5 tiles should be active");
	TEST_EQUAL(Pool1->GetAvailableCount(), 0, "Pool should be exhausted");

	// Try to get one more (should fail - no auto-expand)
	AActor* ExtraTile = Pool1->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NULL(ExtraTile, "Should not get tile when pool exhausted");

	// Return tiles
	for (AActor* Tile : RetrievedTiles)
	{
		Pool1->ReturnToPool(Tile);
	}

	// Test minimum recommended size (3)
	UObjectPoolComponent* Pool2 = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool2->RegisterComponent();

	FObjectPoolConfig Config2;
	Config2.PoolSize = 3;
	Config2.bAutoExpand = false;
	bool bInitSuccess2 = Pool2->Initialize(AGroundTile::StaticClass(), Config2);
	TEST_TRUE(bInitSuccess2, "Pool should initialize with size 3");
	TEST_EQUAL(Pool2->GetTotalPoolSize(), 3, "Minimum pool size should be 3");

	// Cleanup
	if (PoolOwner)
	{
		PoolOwner->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_PoolSize");
}

/**
 * Test: Tile Despawn
 * Verify tiles are returned to pool at correct distance behind war rig
 */
static bool GroundTileTest_TileDespawn()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create war rig
	const FVector WarRigPosition(5000.0f, 0.0f, 0.0f);
	AActor* WarRig = CreateTestWarRig(World, WarRigPosition);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Create pool
	AActor* PoolOwner = World->SpawnActor<AActor>();
	TEST_NOT_NULL(PoolOwner, "Pool owner should be created");

	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(PoolOwner);
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	bool bInitSuccess = Pool->Initialize(AGroundTile::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	const float DespawnDistance = 1000.0f;
	const float DespawnThreshold = WarRigPosition.X - DespawnDistance;

	// Test tile well behind threshold (should despawn)
	float Position1 = DespawnThreshold - 500.0f;
	AActor* TileActor1 = Pool->GetFromPool(FVector(Position1, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(TileActor1, "Tile 1 should be created");
	AGroundTile* Tile1 = Cast<AGroundTile>(TileActor1);
	bool IsBehind1 = (Tile1->GetActorLocation().X < DespawnThreshold);
	TEST_TRUE(IsBehind1, "Tile 1 should be behind threshold");
	Pool->ReturnToPool(TileActor1);
	TEST_TRUE(Tile1->IsHidden(), "Despawned tile should be hidden");

	// Test tile ahead of threshold (should not despawn)
	float Position2 = DespawnThreshold + 500.0f;
	AActor* TileActor2 = Pool->GetFromPool(FVector(Position2, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(TileActor2, "Tile 2 should be created");
	AGroundTile* Tile2 = Cast<AGroundTile>(TileActor2);
	bool IsBehind2 = (Tile2->GetActorLocation().X < DespawnThreshold);
	TEST_FALSE(IsBehind2, "Tile 2 should be ahead of threshold");
	Pool->ReturnToPool(TileActor2);

	// Test tile at war rig position (should not despawn)
	float Position3 = WarRigPosition.X;
	AActor* TileActor3 = Pool->GetFromPool(FVector(Position3, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(TileActor3, "Tile 3 should be created");
	AGroundTile* Tile3 = Cast<AGroundTile>(TileActor3);
	bool IsBehind3 = (Tile3->GetActorLocation().X < DespawnThreshold);
	TEST_FALSE(IsBehind3, "Tile 3 should be ahead of threshold");
	Pool->ReturnToPool(TileActor3);

	// Verify pool statistics after despawn operations
	TEST_EQUAL(Pool->GetActiveCount(), 0, "All tiles returned");
	TEST_EQUAL(Pool->GetAvailableCount(), 5, "All tiles available");

	// Test distance-based despawn with moving war rig
	AActor* TestTile = Pool->GetFromPool(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(TestTile, "Test tile should be created");

	// Move war rig forward, making tile fall behind despawn threshold
	WarRig->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
	float NewDespawnThreshold = 3000.0f - DespawnDistance;
	float TilePosition = TestTile->GetActorLocation().X;

	// Tile at 1000 should now be behind threshold of 2000
	TEST_TRUE(TilePosition < NewDespawnThreshold, "Tile should be behind new threshold");

	// Cleanup
	Pool->ReturnToPool(TestTile);
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (PoolOwner)
	{
		PoolOwner->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TileDespawn");
}

/**
 * Register all object pool tests with the test manager
 * This function should be called from TestingGameMode::RegisterSampleTests()
 */
void RegisterObjectPoolTests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	// Register generic object pool tests
	TestManager->RegisterTest(TEXT("ObjectPool_Initialization"), ETestCategory::ObjectPool, &ObjectPoolTest_Initialization);
	TestManager->RegisterTest(TEXT("ObjectPool_GetFromPool"), ETestCategory::ObjectPool, &ObjectPoolTest_GetFromPool);
	TestManager->RegisterTest(TEXT("ObjectPool_ReturnToPool"), ETestCategory::ObjectPool, &ObjectPoolTest_ReturnToPool);
	TestManager->RegisterTest(TEXT("ObjectPool_PoolExhaustion"), ETestCategory::ObjectPool, &ObjectPoolTest_PoolExhaustion);
	TestManager->RegisterTest(TEXT("ObjectPool_PoolReuse"), ETestCategory::ObjectPool, &ObjectPoolTest_PoolReuse);
	TestManager->RegisterTest(TEXT("ObjectPool_ActiveCount"), ETestCategory::ObjectPool, &ObjectPoolTest_ActiveCount);
	TestManager->RegisterTest(TEXT("ObjectPool_AutoExpand"), ETestCategory::ObjectPool, &ObjectPoolTest_AutoExpand);
	TestManager->RegisterTest(TEXT("ObjectPool_ResetPool"), ETestCategory::ObjectPool, &ObjectPoolTest_ResetPool);

	// Register ground tile tests
	TestManager->RegisterTest(TEXT("GroundTile_PoolRecycling"), ETestCategory::ObjectPool, &GroundTileTest_TilePoolRecycling);
	TestManager->RegisterTest(TEXT("GroundTile_SeamlessScrolling"), ETestCategory::Movement, &GroundTileTest_SeamlessScrolling);
	TestManager->RegisterTest(TEXT("GroundTile_Positioning"), ETestCategory::Movement, &GroundTileTest_TilePositioning);
	TestManager->RegisterTest(TEXT("GroundTile_PoolSize"), ETestCategory::ObjectPool, &GroundTileTest_PoolSize);
	TestManager->RegisterTest(TEXT("GroundTile_Despawn"), ETestCategory::Movement, &GroundTileTest_TileDespawn);
}

#endif // !UE_BUILD_SHIPPING
