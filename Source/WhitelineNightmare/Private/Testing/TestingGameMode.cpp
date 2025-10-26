// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestingGameMode.h"
#include "Testing/TestManager.h"
#include "Core/GameDataStructs.h"

#if !UE_BUILD_SHIPPING

DEFINE_LOG_CATEGORY_STATIC(LogTestingGameMode, Log, All);

ATestingGameMode::ATestingGameMode()
	: bAutoRunTests(true)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATestingGameMode::StartPlay()
{
	Super::StartPlay();

	UE_LOG(LogTestingGameMode, Display, TEXT("TestingGameMode started"));

	if (!bAutoRunTests)
	{
		UE_LOG(LogTestingGameMode, Display, TEXT("Auto-run tests disabled. Use console command 'RunTests' to run tests manually."));
		return;
	}

	// Register tests
	RegisterTests();

	// Get test manager
	UTestManager* TestManager = UTestManager::Get(GetWorld());
	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("Failed to get TestManager"));
		return;
	}

	// Run tests
	if (TestCategoryFilter.IsEmpty())
	{
		UE_LOG(LogTestingGameMode, Display, TEXT("Running all tests..."));
		TestManager->RunAllTests();
	}
	else
	{
		UE_LOG(LogTestingGameMode, Display, TEXT("Running tests in category: %s"), *TestCategoryFilter);
		TestManager->RunTestsByCategory(TestCategoryFilter);
	}
}

void ATestingGameMode::RegisterTests()
{
	UTestManager* TestManager = UTestManager::Get(GetWorld());
	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("Failed to get TestManager for test registration"));
		return;
	}

	// Register example tests
	TestManager->RegisterTest(
		TEXT("ExampleTest"),
		TEXT("Framework"),
		FTestFunction::CreateUObject(this, &ATestingGameMode::TestExample)
	);

	TestManager->RegisterTest(
		TEXT("DataTableStructDefaults"),
		TEXT("Framework"),
		FTestFunction::CreateUObject(this, &ATestingGameMode::TestDataTableStructDefaults)
	);

	UE_LOG(LogTestingGameMode, Display, TEXT("Registered %d tests"), TestManager->GetAllTests().Num());
}

bool ATestingGameMode::TestExample(FString& OutErrorMessage)
{
	// Example test that always passes
	TEST_ASSERT(true, "This test should always pass");
	TEST_EQUAL(1 + 1, 2, "Basic math");
	TEST_NOT_NULL(this, "This pointer should not be null");

	return true;
}

bool ATestingGameMode::TestDataTableStructDefaults(FString& OutErrorMessage)
{
	// Test that data table struct defaults are sensible

	// GameplayBalanceData
	FGameplayBalanceData BalanceData;
	TEST_GREATER(BalanceData.FuelDrainRate, 0.0f, "Fuel drain rate should be positive");
	TEST_GREATER(BalanceData.WinDistanceMeters, 0.0f, "Win distance should be positive");
	TEST_GREATER(BalanceData.NumLanes, 0, "Number of lanes should be positive");
	TEST_GREATER(BalanceData.LaneSpacing, 0.0f, "Lane spacing should be positive");
	TEST_GREATER(BalanceData.ScrollSpeed, 0.0f, "Scroll speed should be positive");
	TEST_LESS_EQUAL(BalanceData.MinScrollSpeed, BalanceData.MaxScrollSpeed, "Min scroll speed should be <= max");

	// TurretData
	FTurretData TurretData;
	TEST_GREATER_EQUAL(TurretData.BaseDamage, 0.0f, "Base damage should be non-negative");
	TEST_GREATER(TurretData.FireRate, 0.0f, "Fire rate should be positive");
	TEST_GREATER(TurretData.AttackRange, 0.0f, "Attack range should be positive");
	TEST_GREATER_EQUAL(TurretData.BuildCost, 0, "Build cost should be non-negative");
	TEST_GREATER_EQUAL(TurretData.MaxUpgradeLevel, 0, "Max upgrade level should be non-negative");

	// EnemyData
	FEnemyData EnemyData;
	TEST_GREATER(EnemyData.BaseHealth, 0.0f, "Enemy health should be positive");
	TEST_GREATER(EnemyData.SpeedMultiplier, 0.0f, "Speed multiplier should be positive");
	TEST_GREATER_EQUAL(EnemyData.CollisionDamage, 0.0f, "Collision damage should be non-negative");
	TEST_GREATER_EQUAL(EnemyData.ScrapReward, 0, "Scrap reward should be non-negative");
	TEST_GREATER_EQUAL(EnemyData.ScrapDropChance, 0.0f, "Drop chance should be >= 0");
	TEST_LESS_EQUAL(EnemyData.ScrapDropChance, 1.0f, "Drop chance should be <= 1");

	// PickupData
	FPickupData PickupData;
	TEST_GREATER_EQUAL(PickupData.FuelAmount, 0.0f, "Fuel amount should be non-negative");
	TEST_GREATER_EQUAL(PickupData.ScrapAmount, 0, "Scrap amount should be non-negative");
	TEST_GREATER_EQUAL(PickupData.HealthAmount, 0.0f, "Health amount should be non-negative");
	TEST_GREATER_EQUAL(PickupData.Lifetime, 0.0f, "Lifetime should be non-negative");

	// WorldScrollData
	FWorldScrollData WorldData;
	TEST_GREATER(WorldData.TileLength, 0.0f, "Tile length should be positive");
	TEST_GREATER(WorldData.TileWidth, 0.0f, "Tile width should be positive");
	TEST_GREATER(WorldData.TilePoolSize, 0, "Tile pool size should be positive");
	TEST_GREATER(WorldData.EnemyPoolSize, 0, "Enemy pool size should be positive");
	TEST_GREATER(WorldData.ObstaclePoolSize, 0, "Obstacle pool size should be positive");
	TEST_GREATER(WorldData.PickupPoolSize, 0, "Pickup pool size should be positive");

	// WarRigData
	FWarRigData RigData;
	TEST_GREATER(RigData.BaseHealth, 0.0f, "Rig health should be positive");
	TEST_GREATER_EQUAL(RigData.BaseArmor, 0.0f, "Rig armor should be non-negative");
	TEST_GREATER(RigData.BaseFuelCapacity, 0.0f, "Fuel capacity should be positive");
	TEST_GREATER_EQUAL(RigData.UnlockCost, 0, "Unlock cost should be non-negative");

	return true;
}

#endif // !UE_BUILD_SHIPPING
