// Copyright Flatlander81. All Rights Reserved.

#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "Core/WorldScrollComponent.h"
#include "World/GroundTileManager.h"
#include "Kismet/GameplayStatics.h"

#if !UE_BUILD_SHIPPING
#include "Testing/TestManager.h"

// Forward declaration of test registration function
void RegisterObjectPoolTests(class UTestManager* TestManager);
#endif

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWhitelineNightmare, Log, All);

AWhitelineNightmareGameMode::AWhitelineNightmareGameMode()
	: WorldScrollComponent(nullptr)
	, GroundTileManager(nullptr)
	, DistanceTraveled(0.0f)
	, WinDistance(10000.0f)
	, bIsGameOver(false)
	, bPlayerWon(false)
	, EnemiesKilled(0)
	, FuelCollected(0.0f)
	, ScrapCollected(0)
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set default pawn and controller classes (will be set in Blueprints)
	PlayerControllerClass = AWarRigPlayerController::StaticClass();
	HUDClass = AWarRigHUD::StaticClass();

	// Create world scroll component
	WorldScrollComponent = CreateDefaultSubobject<UWorldScrollComponent>(TEXT("WorldScrollComponent"));

	// Create ground tile manager
	GroundTileManager = CreateDefaultSubobject<UGroundTileManager>(TEXT("GroundTileManager"));
}

void AWhitelineNightmareGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize game state
	DistanceTraveled = 0.0f;
	bIsGameOver = false;
	bPlayerWon = false;

	// Initialize stats
	EnemiesKilled = 0;
	FuelCollected = 0.0f;
	ScrapCollected = 0;

#if !UE_BUILD_SHIPPING
	// Register tests for non-shipping builds
	UTestManager* TestManager = UTestManager::Get(this);
	if (TestManager)
	{
		RegisterObjectPoolTests(TestManager);
		UE_LOG(LogWhitelineNightmare, Log, TEXT("WhitelineNightmareGameMode: Registered all tests (ObjectPool, GroundTile, Turret, WorldScroll)"));
	}
#endif

	UE_LOG(LogWhitelineNightmare, Log, TEXT("WhitelineNightmareGameMode: Game started. Target distance: %.2f"), WinDistance);
}

void AWhitelineNightmareGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check win condition each frame (could be optimized to only check when distance changes)
	if (!bIsGameOver && HasPlayerWon())
	{
		TriggerGameOver(true);
	}
}

void AWhitelineNightmareGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogWhitelineNightmare, Log, TEXT("WhitelineNightmareGameMode: Initializing game for map: %s"), *MapName);

	// TODO: Load gameplay balance data from data table
	// For now, using default values set in constructor
}

void AWhitelineNightmareGameMode::AddDistanceTraveled(float DeltaDistance)
{
	// Input validation
	if (!ValidateDistanceAddition(DeltaDistance))
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("AddDistanceTraveled: Invalid delta distance %.2f"), DeltaDistance);
		return;
	}

	// Don't track distance if game is over
	if (bIsGameOver)
	{
		UE_LOG(LogWhitelineNightmare, Verbose, TEXT("AddDistanceTraveled: Game is over, ignoring distance addition"));
		return;
	}

	// Add distance
	const float OldDistance = DistanceTraveled;
	DistanceTraveled += DeltaDistance;

	UE_LOG(LogWhitelineNightmare, Verbose, TEXT("AddDistanceTraveled: %.2f -> %.2f (delta: %.2f)"),
		OldDistance, DistanceTraveled, DeltaDistance);

	// Check win condition
	if (HasPlayerWon())
	{
		TriggerGameOver(true);
	}
}

void AWhitelineNightmareGameMode::TriggerGameOver(bool bWon)
{
	// Prevent multiple game over triggers
	if (bIsGameOver)
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("TriggerGameOver: Game is already over"));
		return;
	}

	bIsGameOver = true;
	bPlayerWon = bWon;

	UE_LOG(LogWhitelineNightmare, Log, TEXT("TriggerGameOver: Game ended. Player %s"),
		bPlayerWon ? TEXT("WON") : TEXT("LOST"));

	LogGameState();

	// TODO: Notify player controller and HUD
	// TODO: Trigger gameplay abilities or effects
	// TODO: Save statistics
}

bool AWhitelineNightmareGameMode::ValidateDistanceAddition(float DeltaDistance) const
{
	// Distance should be positive
	if (DeltaDistance < 0.0f)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("ValidateDistanceAddition: Negative distance not allowed: %.2f"),
			DeltaDistance);
		return false;
	}

	// Sanity check for extremely large values (probably a bug)
	const float MaxReasonableDistance = 10000.0f; // 100 meters per frame at 60fps would be insane
	if (DeltaDistance > MaxReasonableDistance)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("ValidateDistanceAddition: Distance too large, possible bug: %.2f"),
			DeltaDistance);
		return false;
	}

	return true;
}

void AWhitelineNightmareGameMode::LogGameState() const
{
	UE_LOG(LogWhitelineNightmare, Log, TEXT("=== Game State ==="));
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Distance Traveled: %.2f / %.2f (%.1f%%)"),
		DistanceTraveled, WinDistance, (DistanceTraveled / WinDistance) * 100.0f);
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Game Over: %s"), bIsGameOver ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Player Won: %s"), bPlayerWon ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogWhitelineNightmare, Log, TEXT("=================="));
}

// === DEBUG COMMANDS ===

void AWhitelineNightmareGameMode::DebugSetScrollSpeed(float NewSpeed)
{
	if (!WorldScrollComponent)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugSetScrollSpeed: WorldScrollComponent is null"));
		return;
	}

	WorldScrollComponent->SetScrollSpeed(NewSpeed);
	UE_LOG(LogWhitelineNightmare, Log, TEXT("DebugSetScrollSpeed: Set scroll speed to %.2f"), NewSpeed);
}

void AWhitelineNightmareGameMode::DebugToggleScroll()
{
	if (!WorldScrollComponent)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugToggleScroll: WorldScrollComponent is null"));
		return;
	}

	const bool bNewState = !WorldScrollComponent->IsScrolling();
	WorldScrollComponent->SetScrolling(bNewState);
	UE_LOG(LogWhitelineNightmare, Log, TEXT("DebugToggleScroll: Scrolling is now %s"),
		bNewState ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AWhitelineNightmareGameMode::DebugShowScrollInfo()
{
	if (!WorldScrollComponent)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugShowScrollInfo: WorldScrollComponent is null"));
		return;
	}

	UE_LOG(LogWhitelineNightmare, Log, TEXT("=== World Scroll Info ==="));
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Scroll Speed: %.2f units/second"), WorldScrollComponent->GetScrollSpeed());
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Scroll Direction: %s"), *WorldScrollComponent->GetScrollDirection().ToString());
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Scroll Velocity: %s"), *WorldScrollComponent->GetScrollVelocity().ToString());
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Is Scrolling: %s"), WorldScrollComponent->IsScrolling() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogWhitelineNightmare, Log, TEXT("Distance Traveled: %.2f units"), WorldScrollComponent->GetDistanceTraveled());
	UE_LOG(LogWhitelineNightmare, Log, TEXT("========================"));
}

void AWhitelineNightmareGameMode::DebugResetDistance()
{
	if (!WorldScrollComponent)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugResetDistance: WorldScrollComponent is null"));
		return;
	}

	const float OldDistance = WorldScrollComponent->GetDistanceTraveled();
	WorldScrollComponent->ResetDistance();
	UE_LOG(LogWhitelineNightmare, Log, TEXT("DebugResetDistance: Reset distance from %.2f to 0.0"), OldDistance);
}

void AWhitelineNightmareGameMode::DebugShowTiles()
{
	if (!GroundTileManager)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugShowTiles: GroundTileManager is null"));
		return;
	}

	GroundTileManager->DebugShowTiles();
}

void AWhitelineNightmareGameMode::DebugShowTileInfo()
{
	if (!GroundTileManager)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("DebugShowTileInfo: GroundTileManager is null"));
		return;
	}

	GroundTileManager->DebugShowTileInfo();
}

void AWhitelineNightmareGameMode::RunTest(const FString& TestName)
{
#if !UE_BUILD_SHIPPING
	UTestManager* TestManager = UTestManager::Get(this);
	if (!TestManager)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("RunTest: TestManager is null"));
		return;
	}

	UE_LOG(LogWhitelineNightmare, Log, TEXT("RunTest: Running test '%s'"), *TestName);
	bool bSuccess = TestManager->RunTest(TestName);

	if (!bSuccess)
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("RunTest: Test '%s' not found or failed"), *TestName);
	}
#else
	UE_LOG(LogWhitelineNightmare, Warning, TEXT("RunTest: Tests are only available in non-shipping builds"));
#endif
}

void AWhitelineNightmareGameMode::RunTests(const FString& CategoryName)
{
#if !UE_BUILD_SHIPPING
	UTestManager* TestManager = UTestManager::Get(this);
	if (!TestManager)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("RunTests: TestManager is null"));
		return;
	}

	UE_LOG(LogWhitelineNightmare, Log, TEXT("RunTests: Running tests for category '%s'"), *CategoryName);

	// Map category name to enum
	ETestCategory Category = ETestCategory::All;
	if (CategoryName.Equals(TEXT("Movement"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::Movement;
	}
	else if (CategoryName.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::Combat;
	}
	else if (CategoryName.Equals(TEXT("Economy"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::Economy;
	}
	else if (CategoryName.Equals(TEXT("Spawning"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::Spawning;
	}
	else if (CategoryName.Equals(TEXT("ObjectPool"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::ObjectPool;
	}
	else if (CategoryName.Equals(TEXT("GAS"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::GAS;
	}
	else if (CategoryName.Equals(TEXT("All"), ESearchCase::IgnoreCase))
	{
		Category = ETestCategory::All;
	}
	else
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("RunTests: Unknown category '%s'. Valid categories: Movement, Combat, Economy, Spawning, ObjectPool, GAS, All"), *CategoryName);
		return;
	}

	TestManager->RunTestCategory(Category);
#else
	UE_LOG(LogWhitelineNightmare, Warning, TEXT("RunTests: Tests are only available in non-shipping builds"));
#endif
}

void AWhitelineNightmareGameMode::RunAllTests()
{
#if !UE_BUILD_SHIPPING
	UTestManager* TestManager = UTestManager::Get(this);
	if (!TestManager)
	{
		UE_LOG(LogWhitelineNightmare, Error, TEXT("RunAllTests: TestManager is null"));
		return;
	}

	UE_LOG(LogWhitelineNightmare, Log, TEXT("RunAllTests: Running all registered tests"));
	TestManager->RunAllTests();
#else
	UE_LOG(LogWhitelineNightmare, Warning, TEXT("RunAllTests: Tests are only available in non-shipping builds"));
#endif
}

// === STAT TRACKING IMPLEMENTATIONS ===

void AWhitelineNightmareGameMode::IncrementEnemiesKilled()
{
	EnemiesKilled++;
	UE_LOG(LogWhitelineNightmare, Log, TEXT("IncrementEnemiesKilled: Enemies killed: %d"), EnemiesKilled);
}

void AWhitelineNightmareGameMode::AddFuelCollected(float Amount)
{
	if (Amount < 0.0f)
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("AddFuelCollected: Negative amount not allowed: %.2f"), Amount);
		return;
	}

	FuelCollected += Amount;
	UE_LOG(LogWhitelineNightmare, Log, TEXT("AddFuelCollected: Fuel collected: %.2f (total: %.2f)"), Amount, FuelCollected);
}

void AWhitelineNightmareGameMode::AddScrapCollected(int32 Amount)
{
	if (Amount < 0)
	{
		UE_LOG(LogWhitelineNightmare, Warning, TEXT("AddScrapCollected: Negative amount not allowed: %d"), Amount);
		return;
	}

	ScrapCollected += Amount;
	UE_LOG(LogWhitelineNightmare, Log, TEXT("AddScrapCollected: Scrap collected: %d (total: %d)"), Amount, ScrapCollected);
}
