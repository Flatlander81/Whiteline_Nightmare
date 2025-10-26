// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "Engine/DataTable.h"

DEFINE_LOG_CATEGORY_STATIC(LogWhitelineGameMode, Log, All);

AWhitelineNightmareGameMode::AWhitelineNightmareGameMode()
	: CurrentScore(0)
	, DistanceTraveled(0.0f)
	, bIsGameOver(false)
	, bIsVictory(false)
	, CurrentScrollSpeed(0.0f)
	, CachedBalanceData(nullptr)
	, CachedWorldScrollData(nullptr)
{
	// Set default classes
	PlayerControllerClass = AWarRigPlayerController::StaticClass();
	HUDClass = AWarRigHUD::StaticClass();

	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWhitelineNightmareGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogWhitelineGameMode, Log, TEXT("Initializing Whiteline Nightmare game mode for map: %s"), *MapName);

	// Initialize data tables
	InitializeDataTables();
}

void AWhitelineNightmareGameMode::StartPlay()
{
	Super::StartPlay();

	UE_LOG(LogWhitelineGameMode, Log, TEXT("Game started"));

	// Initialize scroll speed from balance data
	const FGameplayBalanceData* BalanceData = GetBalanceData();
	if (BalanceData)
	{
		CurrentScrollSpeed = BalanceData->ScrollSpeed;
		UE_LOG(LogWhitelineGameMode, Log, TEXT("Initial scroll speed: %.2f"), CurrentScrollSpeed);
	}
	else
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Balance data not available, using default scroll speed"));
		CurrentScrollSpeed = 1000.0f;
	}
}

void AWhitelineNightmareGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsGameOver)
	{
		UpdateDistanceTraveled(DeltaSeconds);

		// Check for victory
		if (IsGameWon())
		{
			TriggerGameOver(true);
		}
	}
}

const FGameplayBalanceData* AWhitelineNightmareGameMode::GetBalanceData() const
{
	// Return cached data if available
	if (CachedBalanceData)
	{
		return CachedBalanceData;
	}

	// Validate table
	if (!BalanceDataTable)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Balance data table is not set"));
		return nullptr;
	}

	// Get first row (we expect only one row for global balance)
	TArray<FName> RowNames = BalanceDataTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Balance data table is empty"));
		return nullptr;
	}

	// Cache the data
	CachedBalanceData = BalanceDataTable->FindRow<FGameplayBalanceData>(RowNames[0], TEXT("GetBalanceData"));
	if (!CachedBalanceData)
	{
		UE_LOG(LogWhitelineGameMode, Error, TEXT("Failed to get balance data from table"));
	}

	return CachedBalanceData;
}

const FWorldScrollData* AWhitelineNightmareGameMode::GetWorldScrollData() const
{
	// Return cached data if available
	if (CachedWorldScrollData)
	{
		return CachedWorldScrollData;
	}

	// Validate table
	if (!WorldScrollDataTable)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("World scroll data table is not set"));
		return nullptr;
	}

	// Get first row
	TArray<FName> RowNames = WorldScrollDataTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("World scroll data table is empty"));
		return nullptr;
	}

	// Cache the data
	CachedWorldScrollData = WorldScrollDataTable->FindRow<FWorldScrollData>(RowNames[0], TEXT("GetWorldScrollData"));
	if (!CachedWorldScrollData)
	{
		UE_LOG(LogWhitelineGameMode, Error, TEXT("Failed to get world scroll data from table"));
	}

	return CachedWorldScrollData;
}

void AWhitelineNightmareGameMode::AddScore(int32 Points)
{
	if (Points < 0)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Attempted to add negative score: %d"), Points);
		return;
	}

	CurrentScore += Points;
	UE_LOG(LogWhitelineGameMode, Log, TEXT("Score added: %d, Total: %d"), Points, CurrentScore);
}

float AWhitelineNightmareGameMode::GetWinDistance() const
{
	const FGameplayBalanceData* BalanceData = GetBalanceData();
	if (BalanceData)
	{
		return BalanceData->WinDistanceMeters;
	}

	// Default fallback
	UE_LOG(LogWhitelineGameMode, Warning, TEXT("Balance data not available, using default win distance"));
	return 10000.0f;
}

bool AWhitelineNightmareGameMode::IsGameWon() const
{
	if (bIsGameOver && bIsVictory)
	{
		return true;
	}

	// Check if distance traveled exceeds win distance
	return DistanceTraveled >= GetWinDistance();
}

void AWhitelineNightmareGameMode::TriggerGameOver(bool bVictory)
{
	if (bIsGameOver)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Game over already triggered"));
		return;
	}

	bIsGameOver = true;
	bIsVictory = bVictory;

	if (bVictory)
	{
		UE_LOG(LogWhitelineGameMode, Display, TEXT("=== VICTORY! ==="));
		UE_LOG(LogWhitelineGameMode, Display, TEXT("Distance traveled: %.2f meters"), DistanceTraveled);
		UE_LOG(LogWhitelineGameMode, Display, TEXT("Final score: %d"), CurrentScore);
		OnGameWon();
	}
	else
	{
		UE_LOG(LogWhitelineGameMode, Display, TEXT("=== GAME OVER ==="));
		UE_LOG(LogWhitelineGameMode, Display, TEXT("Distance traveled: %.2f meters"), DistanceTraveled);
		UE_LOG(LogWhitelineGameMode, Display, TEXT("Final score: %d"), CurrentScore);
		OnGameLost();
	}
}

void AWhitelineNightmareGameMode::InitializeDataTables()
{
	UE_LOG(LogWhitelineGameMode, Log, TEXT("Initializing data tables"));

	// Validate all data tables
	ValidateDataTable(BalanceDataTable, TEXT("BalanceDataTable"));
	ValidateDataTable(TurretDataTable, TEXT("TurretDataTable"));
	ValidateDataTable(EnemyDataTable, TEXT("EnemyDataTable"));
	ValidateDataTable(PickupDataTable, TEXT("PickupDataTable"));
	ValidateDataTable(WorldScrollDataTable, TEXT("WorldScrollDataTable"));
	ValidateDataTable(WarRigDataTable, TEXT("WarRigDataTable"));

	// Pre-cache frequently accessed data
	GetBalanceData();
	GetWorldScrollData();
}

bool AWhitelineNightmareGameMode::ValidateDataTable(UDataTable* DataTable, const FString& TableName) const
{
	if (!DataTable)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Data table '%s' is not set"), *TableName);
		return false;
	}

	TArray<FName> RowNames = DataTable->GetRowNames();
	if (RowNames.Num() == 0)
	{
		UE_LOG(LogWhitelineGameMode, Warning, TEXT("Data table '%s' is empty"), *TableName);
		return false;
	}

	UE_LOG(LogWhitelineGameMode, Log, TEXT("Data table '%s' validated: %d rows"), *TableName, RowNames.Num());
	return true;
}

void AWhitelineNightmareGameMode::UpdateDistanceTraveled(float DeltaSeconds)
{
	if (CurrentScrollSpeed <= 0.0f)
	{
		return;
	}

	// Distance = Speed * Time
	// Convert from units per second to meters (assuming 1 unit = 1 cm, so divide by 100)
	float DistanceDelta = (CurrentScrollSpeed * DeltaSeconds) / 100.0f;
	DistanceTraveled += DistanceDelta;
}

void AWhitelineNightmareGameMode::OnGameWon()
{
	// Override in subclasses for custom victory logic
	// e.g., show victory screen, save progress, unlock achievements
}

void AWhitelineNightmareGameMode::OnGameLost()
{
	// Override in subclasses for custom game over logic
	// e.g., show game over screen, retry option
}
