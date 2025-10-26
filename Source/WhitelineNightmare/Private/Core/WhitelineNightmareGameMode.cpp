// Copyright Flatlander81. All Rights Reserved.

#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "Kismet/GameplayStatics.h"

#if !UE_BUILD_SHIPPING
#include "Testing/TestManager.h"

// Forward declaration of test registration functions
void RegisterObjectPoolTests(class UTestManager* TestManager);
#endif

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWhitelineNightmare, Log, All);

AWhitelineNightmareGameMode::AWhitelineNightmareGameMode()
	: DistanceTraveled(0.0f)
	, WinDistance(10000.0f)
	, bIsGameOver(false)
	, bPlayerWon(false)
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set default pawn and controller classes (will be set in Blueprints)
	PlayerControllerClass = AWarRigPlayerController::StaticClass();
	HUDClass = AWarRigHUD::StaticClass();
}

void AWhitelineNightmareGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Initialize game state
	DistanceTraveled = 0.0f;
	bIsGameOver = false;
	bPlayerWon = false;

#if !UE_BUILD_SHIPPING
	// Register tests for non-shipping builds
	UTestManager* TestManager = UTestManager::Get(this);
	if (TestManager)
	{
		RegisterObjectPoolTests(TestManager);
		UE_LOG(LogWhitelineNightmare, Log, TEXT("WhitelineNightmareGameMode: Registered ObjectPool tests"));
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
