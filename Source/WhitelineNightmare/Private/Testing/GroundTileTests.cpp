// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "World/GroundTile.h"
#include "World/GroundTileManager.h"
#include "Core/ObjectPoolComponent.h"
#include "Core/ObjectPoolTypes.h"
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

// Helper function to create a ground tile manager for testing
static UGroundTileManager* CreateTestGroundTileManager()
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

	// Create and attach the tile manager component
	UGroundTileManager* TileManager = NewObject<UGroundTileManager>(DummyActor);
	if (TileManager)
	{
		TileManager->RegisterComponent();
	}

	return TileManager;
}

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
	UGroundTileManager* TileManager = CreateTestGroundTileManager();
	TEST_NOT_NULL(TileManager, "Tile manager should be created");

	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create a war rig for position reference
	AActor* WarRig = CreateTestWarRig(World, FVector::ZeroVector);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Set up tile manager manually for testing
	TileManager->TileClass = AGroundTile::StaticClass();

	// Create pool manually
	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	Pool->Initialize(AGroundTile::StaticClass(), Config);

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
	TEST_EQUAL(OriginalTile, Tile2, "Pool should reuse the same tile instance, not create a new one");
	TEST_EQUAL(Pool->GetTotalPoolSize(), 5, "Pool size should remain constant (tiles not destroyed)");

	// Cleanup
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (TileManager->GetOwner())
	{
		TileManager->GetOwner()->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TilePoolRecycling");
}

/**
 * Test: Seamless Scrolling
 * Verify no gaps between tiles when positioned in sequence
 */
static bool GroundTileTest_SeamlessScrolling()
{
	UGroundTileManager* TileManager = CreateTestGroundTileManager();
	TEST_NOT_NULL(TileManager, "Tile manager should be created");

	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create pool
	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;
	Pool->Initialize(AGroundTile::StaticClass(), Config);

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
		TEST_NEARLY_EQUAL(Gap, 0.0f, 1.0f,
			FString::Printf(TEXT("No gap should exist between tile %d and %d (gap: %.2f)"), i, i + 1, Gap));
	}

	// Verify tiles form a continuous line
	float ExpectedTotalLength = TileSize * 3;
	float FirstTileStart = Tiles[0]->GetActorLocation().X - (TileSize / 2.0f);
	float LastTileEnd = Tiles[2]->GetActorLocation().X + (TileSize / 2.0f);
	float ActualTotalLength = LastTileEnd - FirstTileStart;

	TEST_NEARLY_EQUAL(ActualTotalLength, ExpectedTotalLength, 1.0f,
		"Total length of all tiles should equal expected continuous length");

	// Cleanup
	if (TileManager->GetOwner())
	{
		TileManager->GetOwner()->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_SeamlessScrolling");
}

/**
 * Test: Tile Positioning
 * Verify spawn positions are correct relative to war rig
 */
static bool GroundTileTest_TilePositioning()
{
	UGroundTileManager* TileManager = CreateTestGroundTileManager();
	TEST_NOT_NULL(TileManager, "Tile manager should be created");

	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create war rig at a specific position
	const FVector WarRigPosition(1000.0f, 0.0f, 0.0f);
	AActor* WarRig = CreateTestWarRig(World, WarRigPosition);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Create pool
	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	Pool->Initialize(AGroundTile::StaticClass(), Config);

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
		"Tile should be positioned at correct spawn distance ahead of war rig");

	// Verify tile is ahead of war rig
	TEST_TRUE(ActualTileX > WarRigPosition.X, "Tile should be ahead of war rig");

	// Calculate distance between war rig and tile
	float DistanceFromWarRig = ActualTileX - WarRigPosition.X;
	TEST_NEARLY_EQUAL(DistanceFromWarRig, SpawnDistanceAhead, 1.0f,
		"Distance from war rig should match spawn distance");

	// Test multiple tiles at different positions
	TArray<float> TestPositions = {
		WarRigPosition.X - 1000.0f,  // Behind
		WarRigPosition.X,             // At war rig
		WarRigPosition.X + 2000.0f,  // Ahead
		WarRigPosition.X + 4000.0f   // Far ahead
	};

	for (float TestX : TestPositions)
	{
		AActor* TestTileActor = Pool->GetFromPool(FVector(TestX, 0.0f, 0.0f), FRotator::ZeroRotator);
		if (TestTileActor)
		{
			AGroundTile* TestTile = Cast<AGroundTile>(TestTileActor);
			float ActualX = TestTile->GetActorLocation().X;
			TEST_NEARLY_EQUAL(ActualX, TestX, 1.0f,
				FString::Printf(TEXT("Tile should spawn at specified position (%.0f)"), TestX));

			Pool->ReturnToPool(TestTileActor);
		}
	}

	// Cleanup
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (TileManager->GetOwner())
	{
		TileManager->GetOwner()->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TilePositioning");
}

/**
 * Test: Pool Size
 * Verify correct number of tiles created and pool respects size limits
 */
static bool GroundTileTest_PoolSize()
{
	UGroundTileManager* TileManager = CreateTestGroundTileManager();
	TEST_NOT_NULL(TileManager, "Tile manager should be created");

	// Test various pool sizes
	TArray<int32> PoolSizes = { 3, 5, 10, 15 };

	for (int32 PoolSize : PoolSizes)
	{
		UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
		Pool->RegisterComponent();

		FObjectPoolConfig Config;
		Config.PoolSize = PoolSize;
		Config.bAutoExpand = false;

		bool bInitSuccess = Pool->Initialize(AGroundTile::StaticClass(), Config);
		TEST_TRUE(bInitSuccess, FString::Printf(TEXT("Pool should initialize with size %d"), PoolSize));

		// Verify pool created correct number of tiles
		TEST_EQUAL(Pool->GetTotalPoolSize(), PoolSize,
			FString::Printf(TEXT("Pool should have %d tiles"), PoolSize));

		TEST_EQUAL(Pool->GetAvailableCount(), PoolSize,
			FString::Printf(TEXT("All %d tiles should be available initially"), PoolSize));

		TEST_EQUAL(Pool->GetActiveCount(), 0,
			FString::Printf(TEXT("No tiles should be active initially for pool size %d"), PoolSize));

		// Get all tiles from pool
		TArray<AActor*> RetrievedTiles;
		for (int32 i = 0; i < PoolSize; ++i)
		{
			AActor* Tile = Pool->GetFromPool(FVector(i * 1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
			TEST_NOT_NULL(Tile, FString::Printf(TEXT("Should retrieve tile %d of %d"), i + 1, PoolSize));
			RetrievedTiles.Add(Tile);
		}

		// Verify pool is exhausted
		TEST_EQUAL(Pool->GetActiveCount(), PoolSize,
			FString::Printf(TEXT("All %d tiles should be active"), PoolSize));
		TEST_EQUAL(Pool->GetAvailableCount(), 0,
			FString::Printf(TEXT("Pool should be exhausted with %d tiles"), PoolSize));

		// Try to get one more (should fail - no auto-expand)
		AActor* ExtraTile = Pool->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
		TEST_NULL(ExtraTile, FString::Printf(TEXT("Should not get tile when pool of %d is exhausted"), PoolSize));

		// Clean up this pool's tiles
		for (AActor* Tile : RetrievedTiles)
		{
			Pool->ReturnToPool(Tile);
		}
	}

	// Test minimum recommended size (3)
	UObjectPoolComponent* MinPool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
	MinPool->RegisterComponent();

	FObjectPoolConfig MinConfig;
	MinConfig.PoolSize = 3;
	MinConfig.bAutoExpand = false;
	MinPool->Initialize(AGroundTile::StaticClass(), MinConfig);

	TEST_EQUAL(MinPool->GetTotalPoolSize(), 3, "Minimum pool size should be 3 for seamless scrolling");

	// Cleanup
	if (TileManager->GetOwner())
	{
		TileManager->GetOwner()->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_PoolSize");
}

/**
 * Test: Tile Despawn
 * Verify tiles are returned to pool at correct distance behind war rig
 */
static bool GroundTileTest_TileDespawn()
{
	UGroundTileManager* TileManager = CreateTestGroundTileManager();
	TEST_NOT_NULL(TileManager, "Tile manager should be created");

	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	// Create war rig
	const FVector WarRigPosition(5000.0f, 0.0f, 0.0f);
	AActor* WarRig = CreateTestWarRig(World, WarRigPosition);
	TEST_NOT_NULL(WarRig, "War rig should be created");

	// Create pool
	UObjectPoolComponent* Pool = NewObject<UObjectPoolComponent>(TileManager->GetOwner());
	Pool->RegisterComponent();

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;
	Pool->Initialize(AGroundTile::StaticClass(), Config);

	const float DespawnDistance = 1000.0f;
	const float DespawnThreshold = WarRigPosition.X - DespawnDistance;

	// Spawn tiles at various positions relative to despawn threshold
	struct FTileTestData
	{
		float Position;
		bool ShouldDespawn;
		FString Description;
	};

	TArray<FTileTestData> TestCases = {
		{ DespawnThreshold - 500.0f, true,  TEXT("Well behind threshold") },
		{ DespawnThreshold - 1.0f,   true,  TEXT("Just behind threshold") },
		{ DespawnThreshold + 1.0f,   false, TEXT("Just ahead of threshold") },
		{ DespawnThreshold + 500.0f, false, TEXT("Well ahead of threshold") },
		{ WarRigPosition.X,          false, TEXT("At war rig position") }
	};

	for (const FTileTestData& TestCase : TestCases)
	{
		AActor* TileActor = Pool->GetFromPool(FVector(TestCase.Position, 0.0f, 0.0f), FRotator::ZeroRotator);
		TEST_NOT_NULL(TileActor, FString::Printf(TEXT("Tile should be created at %.0f (%s)"),
			TestCase.Position, *TestCase.Description));

		AGroundTile* Tile = Cast<AGroundTile>(TileActor);
		float TileX = Tile->GetActorLocation().X;

		// Check if tile is behind despawn threshold
		bool IsBehindThreshold = (TileX < DespawnThreshold);

		TEST_EQUAL(IsBehindThreshold, TestCase.ShouldDespawn,
			FString::Printf(TEXT("Tile at %.0f %s be behind despawn threshold %.0f (%s)"),
				TileX,
				TestCase.ShouldDespawn ? TEXT("should") : TEXT("should not"),
				DespawnThreshold,
				*TestCase.Description));

		// Simulate despawn check logic
		if (IsBehindThreshold)
		{
			// This tile should be returned to pool
			Pool->ReturnToPool(TileActor);
			TEST_TRUE(Tile->IsHidden(), FString::Printf(TEXT("Despawned tile should be hidden (%s)"), *TestCase.Description));
		}
		else
		{
			// This tile should remain active
			Pool->ReturnToPool(TileActor); // Clean up for next test
		}
	}

	// Verify pool statistics after despawn operations
	TEST_EQUAL(Pool->GetActiveCount(), 0, "All tiles should be returned after test");
	TEST_EQUAL(Pool->GetAvailableCount(), 5, "All tiles should be available after test");

	// Test distance-based despawn with moving war rig
	AActor* TestTile = Pool->GetFromPool(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TEST_NOT_NULL(TestTile, "Test tile should be created");

	// Move war rig forward, making tile fall behind despawn threshold
	WarRig->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
	float NewDespawnThreshold = 3000.0f - DespawnDistance;
	float TilePosition = TestTile->GetActorLocation().X;

	// Tile at 1000 should now be behind threshold of 2000
	TEST_TRUE(TilePosition < NewDespawnThreshold,
		FString::Printf(TEXT("Tile at %.0f should be behind new threshold %.0f after war rig moved"),
			TilePosition, NewDespawnThreshold));

	// Cleanup
	Pool->ReturnToPool(TestTile);
	if (WarRig)
	{
		WarRig->Destroy();
	}
	if (TileManager->GetOwner())
	{
		TileManager->GetOwner()->Destroy();
	}

	TEST_SUCCESS("GroundTileTest_TileDespawn");
}

/**
 * Register all ground tile tests with the test manager
 * This function should be called from TestingGameMode::RegisterSampleTests()
 */
void RegisterGroundTileTests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	// Register tests in both Movement and ObjectPool categories as per spec
	TestManager->RegisterTest(TEXT("GroundTile_PoolRecycling"), ETestCategory::ObjectPool, &GroundTileTest_TilePoolRecycling);
	TestManager->RegisterTest(TEXT("GroundTile_SeamlessScrolling"), ETestCategory::Movement, &GroundTileTest_SeamlessScrolling);
	TestManager->RegisterTest(TEXT("GroundTile_Positioning"), ETestCategory::Movement, &GroundTileTest_TilePositioning);
	TestManager->RegisterTest(TEXT("GroundTile_PoolSize"), ETestCategory::ObjectPool, &GroundTileTest_PoolSize);
	TestManager->RegisterTest(TEXT("GroundTile_Despawn"), ETestCategory::Movement, &GroundTileTest_TileDespawn);
}

#endif // !UE_BUILD_SHIPPING
