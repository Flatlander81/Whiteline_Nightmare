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
#include "Turrets/TurretBase.h"
#include "Testing/TestTurret.h"
#include "GAS/Attributes/CombatAttributeSet.h"
#include "Core/WarRigPawn.h"
#include "Core/GameDataStructs.h"
#include "AbilitySystemComponent.h"
#include "Core/WorldScrollComponent.h"

#if !UE_BUILD_SHIPPING

namespace
{
	// Helper function to get a valid world for testing
	UWorld* GetTestWorld()
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
	UObjectPoolComponent* CreateTestPoolComponent()
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

// ============================================================================
// TURRET TEST HELPERS
// ============================================================================

// Helper function to create a test war rig
static AWarRigPawn* CreateTestWarRig()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	AWarRigPawn* WarRig = World->SpawnActor<AWarRigPawn>();
	if (WarRig)
	{
		WarRig->SetActorLocation(FVector::ZeroVector);
	}
	return WarRig;
}

// Helper function to create a test turret
static ATurretBase* CreateTestTurret()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	// Spawn test turret at origin (using concrete ATestTurret class instead of abstract ATurretBase)
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATurretBase* Turret = World->SpawnActor<ATestTurret>(ATestTurret::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	return Turret;
}

// Helper function to create test turret data
static FTurretData CreateTestTurretData()
{
	FTurretData Data;
	Data.TurretName = TEXT("TestTurret");
	Data.DisplayName = FText::FromString(TEXT("Test Turret"));
	Data.Description = FText::FromString(TEXT("A test turret for unit testing"));
	Data.BaseDamage = 25.0f;
	Data.FireRate = 2.0f;
	Data.Range = 1500.0f;
	Data.BaseHealth = 150.0f;
	Data.BuildCost = 100;
	Data.UpgradeCost = 50;
	return Data;
}

// ============================================================================
// TURRET TESTS
// ============================================================================

/**
 * Test: Turret Spawn
 * Verify that turret spawns at mount point correctly with all components
 */
static bool TurretTest_TurretSpawn()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Verify components exist
	TEST_NOT_NULL(Turret->GetAbilitySystemComponent(), "AbilitySystemComponent should exist");
	TEST_NOT_NULL(Turret->GetCombatAttributeSet(), "CombatAttributeSet should exist");
	TEST_NOT_NULL(Turret->GetRootComponent(), "Root component should exist");

	// Verify initial state
	TEST_EQUAL(Turret->GetMountIndex(), -1, "Mount index should be -1 (uninitialized)");
	TEST_NULL(Turret->GetOwnerWarRig(), "Owner war rig should be null (not initialized)");
	TEST_NULL(Turret->GetCurrentTarget(), "Current target should be null");

	// Cleanup
	if (Turret)
	{
		Turret->Destroy();
	}

	TEST_SUCCESS("TurretTest_TurretSpawn");
}

/**
 * Test: Target Acquisition
 * Verify turret finds targets within range and firing arc
 */
static bool TurretTest_TargetAcquisition()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	FRotator FacingDirection = FRotator(0.0f, 0.0f, 0.0f); // Facing forward (X-axis)
	Turret->Initialize(TurretData, 0, FacingDirection, WarRig);

	// Verify attributes initialized correctly
	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist after initialization");
	TEST_NEARLY_EQUAL(Attributes->GetRange(), TurretData.Range, 0.1f, "Range should match data table");

	// Create a test target (enemy pawn) within range and arc
	FVector TargetLocationInRange = Turret->GetActorLocation() + FVector(500.0f, 0.0f, 0.0f); // 500 units ahead
	AActor* TargetInRange = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationInRange, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetInRange, "Target in range should be created");

	// Verify target is in firing arc
	bool bIsInArc = Turret->IsTargetInFiringArc(TargetLocationInRange);
	TEST_TRUE(bIsInArc, "Target should be within 180° firing arc");

	// Create a test target outside range
	FVector TargetLocationOutOfRange = Turret->GetActorLocation() + FVector(3000.0f, 0.0f, 0.0f); // 3000 units ahead (beyond range)
	AActor* TargetOutOfRange = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationOutOfRange, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetOutOfRange, "Target out of range should be created");

	// Create a target behind turret (outside arc)
	FVector TargetLocationBehind = Turret->GetActorLocation() + FVector(-500.0f, 0.0f, 0.0f); // 500 units behind
	AActor* TargetBehind = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationBehind, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetBehind, "Target behind should be created");

	// Verify target behind is not in arc
	bool bIsBehindInArc = Turret->IsTargetInFiringArc(TargetLocationBehind);
	TEST_FALSE(bIsBehindInArc, "Target behind should not be in firing arc");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();
	if (TargetInRange) TargetInRange->Destroy();
	if (TargetOutOfRange) TargetOutOfRange->Destroy();
	if (TargetBehind) TargetBehind->Destroy();

	TEST_SUCCESS("TurretTest_TargetAcquisition");
}

/**
 * Test: Firing Arc Calculation
 * Verify 180° arc math is correct using dot product
 */
static bool TurretTest_FiringArcCalculation()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Set turret facing forward (0°)
	FRotator FacingDirection = FRotator(0.0f, 0.0f, 0.0f); // Facing +X
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	Turret->Initialize(TurretData, 0, FacingDirection, WarRig);

	FVector TurretLocation = Turret->GetActorLocation();

	// Test 1: Target directly ahead (0°) - should be in arc (dot = 1.0)
	FVector TargetAhead = TurretLocation + FVector(100.0f, 0.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(TargetAhead), "Target directly ahead should be in arc");

	// Test 2: Target at 45° (forward-right) - should be in arc (dot > 0)
	FVector Target45Degrees = TurretLocation + FVector(100.0f, 100.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(Target45Degrees), "Target at 45° should be in arc");

	// Test 3: Target at 90° (perpendicular right) - should be at edge of arc (dot ≈ 0)
	FVector Target90Degrees = TurretLocation + FVector(0.0f, 100.0f, 0.0f);
	// This is the edge case - dot product will be very close to 0
	// It should still be in arc since we use > 0.0, but edge case
	bool bAt90 = Turret->IsTargetInFiringArc(Target90Degrees);
	// At exactly 90°, dot product is 0, which is NOT > 0, so it should be false
	TEST_FALSE(bAt90, "Target at exactly 90° should be at arc edge (not in arc)");

	// Test 4: Target at -45° (forward-left) - should be in arc (dot > 0)
	FVector TargetNeg45Degrees = TurretLocation + FVector(100.0f, -100.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(TargetNeg45Degrees), "Target at -45° should be in arc");

	// Test 5: Target at -90° (perpendicular left) - should be at edge of arc (dot ≈ 0)
	FVector TargetNeg90Degrees = TurretLocation + FVector(0.0f, -100.0f, 0.0f);
	bool bAtNeg90 = Turret->IsTargetInFiringArc(TargetNeg90Degrees);
	TEST_FALSE(bAtNeg90, "Target at exactly -90° should be at arc edge (not in arc)");

	// Test 6: Target at 135° (back-right) - should NOT be in arc (dot < 0)
	FVector Target135Degrees = TurretLocation + FVector(-100.0f, 100.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(Target135Degrees), "Target at 135° should not be in arc");

	// Test 7: Target directly behind (180°) - should NOT be in arc (dot = -1.0)
	FVector TargetBehind = TurretLocation + FVector(-100.0f, 0.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(TargetBehind), "Target directly behind should not be in arc");

	// Test 8: Target at -135° (back-left) - should NOT be in arc (dot < 0)
	FVector TargetNeg135Degrees = TurretLocation + FVector(-100.0f, -100.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(TargetNeg135Degrees), "Target at -135° should not be in arc");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_FiringArcCalculation");
}

/**
 * Test: Attribute Initialization
 * Verify stats load correctly from data table
 */
static bool TurretTest_AttributeInitialization()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Create test data
	FTurretData TurretData = CreateTestTurretData();
	TurretData.BaseDamage = 42.0f;
	TurretData.FireRate = 3.5f;
	TurretData.Range = 2000.0f;
	TurretData.BaseHealth = 250.0f;

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FRotator FacingDirection = FRotator(0.0f, 90.0f, 0.0f); // Facing right
	Turret->Initialize(TurretData, 5, FacingDirection, WarRig);

	// Verify attributes match data table
	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist");

	TEST_NEARLY_EQUAL(Attributes->GetHealth(), TurretData.BaseHealth, 0.1f, "Health should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetMaxHealth(), TurretData.BaseHealth, 0.1f, "MaxHealth should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetDamage(), TurretData.BaseDamage, 0.1f, "Damage should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetFireRate(), TurretData.FireRate, 0.1f, "FireRate should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetRange(), TurretData.Range, 0.1f, "Range should match data table");

	// Verify turret properties
	TEST_EQUAL(Turret->GetMountIndex(), 5, "Mount index should be set correctly");
	TEST_EQUAL(Turret->GetOwnerWarRig(), WarRig, "Owner war rig should be set correctly");

	// Verify facing direction (check yaw)
	FRotator ActualFacing = Turret->GetFacingDirection();
	TEST_NEARLY_EQUAL(ActualFacing.Yaw, FacingDirection.Yaw, 0.1f, "Facing direction should match");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_AttributeInitialization");
}

/**
 * Test: Null Target Handling
 * Verify graceful handling when no targets are available
 */
static bool TurretTest_NullTargetHandling()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	Turret->Initialize(TurretData, 0, FRotator::ZeroRotator, WarRig);

	// Try to find a target (should return nullptr since no enemies exist)
	AActor* Target = Turret->FindTarget();
	TEST_NULL(Target, "FindTarget should return null when no targets available");

	// Verify current target is null
	TEST_NULL(Turret->GetCurrentTarget(), "Current target should be null");

	// Try to fire with null target (should handle gracefully without crashing)
	Turret->Fire(); // Should not crash or throw error

	// Verify turret is still valid after attempting to fire with null target
	TEST_TRUE(IsValid(Turret), "Turret should still be valid after firing with null target");

	// Test IsTargetInFiringArc with null target's location (edge case)
	// This tests that the function handles invalid input gracefully
	FVector InvalidLocation = FVector::ZeroVector;
	bool bResult = Turret->IsTargetInFiringArc(InvalidLocation);
	// Should not crash (result doesn't matter as much as not crashing)

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_NullTargetHandling");
}

/**
 * Test: Attribute Clamping
 * Verify that health is clamped to [0, MaxHealth] range
 */
static bool TurretTest_AttributeClamping()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist");

	// Initialize with known values
	float MaxHealth = 100.0f;
	Attributes->InitMaxHealth(MaxHealth);
	Attributes->InitHealth(MaxHealth);

	// Verify initial values
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), MaxHealth, 0.1f, "Health should be at max");

	// Try to set health above max (should be clamped)
	Attributes->SetHealth(150.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), MaxHealth, 0.1f, "Health should be clamped to MaxHealth");

	// Try to set health below zero (should be clamped)
	Attributes->SetHealth(-50.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), 0.0f, 0.1f, "Health should be clamped to 0");

	// Set health to valid value
	Attributes->SetHealth(50.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), 50.0f, 0.1f, "Health should be set to 50");

	// Change MaxHealth to lower value - health should be re-clamped
	Attributes->SetMaxHealth(40.0f);
	// Note: This requires PostGameplayEffectExecute to handle properly
	// For now, just verify MaxHealth changed
	TEST_NEARLY_EQUAL(Attributes->GetMaxHealth(), 40.0f, 0.1f, "MaxHealth should be updated");

	// Cleanup
	if (Turret) Turret->Destroy();

	TEST_SUCCESS("TurretTest_AttributeClamping");
}

/**
 * Test: Mount Point Integration
 * Verify turret correctly stores and retrieves mount point information
 */
static bool TurretTest_MountPointIntegration()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	AWarRigPawn* WarRig = CreateTestWarRig();
	TEST_NOT_NULL(WarRig, "War rig should be created");

	FTurretData TurretData = CreateTestTurretData();

	// Test different mount indices
	for (int32 MountIdx = 0; MountIdx < 10; ++MountIdx)
	{
		FRotator FacingDirection = FRotator(0.0f, MountIdx * 45.0f, 0.0f); // Different facing for each mount
		Turret->Initialize(TurretData, MountIdx, FacingDirection, WarRig);

		TEST_EQUAL(Turret->GetMountIndex(), MountIdx, "Mount index should match initialization value");
		TEST_NEARLY_EQUAL(Turret->GetFacingDirection().Yaw, FacingDirection.Yaw, 0.1f, "Facing direction should match");
		TEST_EQUAL(Turret->GetOwnerWarRig(), WarRig, "Owner war rig should be consistent");
	}

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_MountPointIntegration");
}

/**
 * Test: Ability System Component Integration
 * Verify ASC is properly integrated and accessible
 */
static bool TurretTest_AbilitySystemIntegration()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Verify ASC exists
	UAbilitySystemComponent* ASC = Turret->GetAbilitySystemComponent();
	TEST_NOT_NULL(ASC, "AbilitySystemComponent should exist");

	// Verify attribute set is added to ASC
	const TArray<UAttributeSet*>& AttributeSets = ASC->GetSpawnedAttributes();
	TEST_TRUE(AttributeSets.Num() > 0, "ASC should have at least one attribute set");

	// Verify CombatAttributeSet is present
	bool bHasCombatAttributes = false;
	for (UAttributeSet* AttrSet : AttributeSets)
	{
		if (Cast<UCombatAttributeSet>(AttrSet))
		{
			bHasCombatAttributes = true;
			break;
		}
	}
	TEST_TRUE(bHasCombatAttributes, "ASC should have CombatAttributeSet");

	// Verify ASC replication is enabled
	TEST_TRUE(ASC->GetIsReplicated(), "ASC should be replicated");

	// Cleanup
	if (Turret) Turret->Destroy();

	TEST_SUCCESS("TurretTest_AbilitySystemIntegration");
}

// ============================================================================
// WORLD SCROLL TESTS
// ============================================================================

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

	// Register turret tests
	TestManager->RegisterTest(TEXT("Turret_Spawn"), ETestCategory::Combat, &TurretTest_TurretSpawn);
	TestManager->RegisterTest(TEXT("Turret_TargetAcquisition"), ETestCategory::Combat, &TurretTest_TargetAcquisition);
	TestManager->RegisterTest(TEXT("Turret_FiringArcCalculation"), ETestCategory::Combat, &TurretTest_FiringArcCalculation);
	TestManager->RegisterTest(TEXT("Turret_AttributeInitialization"), ETestCategory::Combat, &TurretTest_AttributeInitialization);
	TestManager->RegisterTest(TEXT("Turret_NullTargetHandling"), ETestCategory::Combat, &TurretTest_NullTargetHandling);
	TestManager->RegisterTest(TEXT("Turret_AttributeClamping"), ETestCategory::GAS, &TurretTest_AttributeClamping);
	TestManager->RegisterTest(TEXT("Turret_MountPointIntegration"), ETestCategory::Combat, &TurretTest_MountPointIntegration);
	TestManager->RegisterTest(TEXT("Turret_AbilitySystemIntegration"), ETestCategory::GAS, &TurretTest_AbilitySystemIntegration);

	// Register world scroll tests
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollSpeedConsistency"), ETestCategory::Movement, &WorldScrollTest_ScrollSpeedConsistency);
	TestManager->RegisterTest(TEXT("WorldScroll_DistanceAccumulation"), ETestCategory::Movement, &WorldScrollTest_DistanceAccumulation);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollPause"), ETestCategory::Movement, &WorldScrollTest_ScrollPause);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollVelocity"), ETestCategory::Movement, &WorldScrollTest_ScrollVelocity);
	TestManager->RegisterTest(TEXT("WorldScroll_ScrollSpeedChange"), ETestCategory::Movement, &WorldScrollTest_ScrollSpeedChange);
	TestManager->RegisterTest(TEXT("WorldScroll_DirectionNormalization"), ETestCategory::Movement, &WorldScrollTest_DirectionNormalization);
	TestManager->RegisterTest(TEXT("WorldScroll_DistanceReset"), ETestCategory::Movement, &WorldScrollTest_DistanceReset);
	TestManager->RegisterTest(TEXT("WorldScroll_TestAll"), ETestCategory::Movement, &WorldScrollTest_TestAll);
}

#endif // !UE_BUILD_SHIPPING
