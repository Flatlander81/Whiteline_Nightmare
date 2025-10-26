// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "Data/GameplayDataStructs.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

AWhitelineNightmareGameMode::AWhitelineNightmareGameMode()
{
	// Set default player controller and HUD classes
	PlayerControllerClass = AWarRigPlayerController::StaticClass();
	HUDClass = AWarRigHUD::StaticClass();

	// Initialize game state
	DistanceTraveled = 0.0f;
	bHasWon = false;
	bGameOver = false;

	// Data tables will be set in Blueprint or via Config
	GameplayBalanceTable = nullptr;
	TurretDataTable = nullptr;
	EnemyDataTable = nullptr;
	PickupDataTable = nullptr;
	WorldScrollDataTable = nullptr;
	WarRigDataTable = nullptr;
	ObstacleDataTable = nullptr;
}

void AWhitelineNightmareGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Validate that data tables are set
	if (!GameplayBalanceTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameplayBalanceTable is not set in GameMode!"));
	}

	if (!TurretDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurretDataTable is not set in GameMode!"));
	}

	if (!EnemyDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyDataTable is not set in GameMode!"));
	}

	if (!PickupDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupDataTable is not set in GameMode!"));
	}

	if (!WorldScrollDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldScrollDataTable is not set in GameMode!"));
	}

	if (!WarRigDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("WarRigDataTable is not set in GameMode!"));
	}

	if (!ObstacleDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObstacleDataTable is not set in GameMode!"));
	}
}

const FGameplayBalanceData* AWhitelineNightmareGameMode::GetGameplayBalanceData() const
{
	if (!GameplayBalanceTable)
	{
		UE_LOG(LogTemp, Error, TEXT("GameplayBalanceTable is not set!"));
		return nullptr;
	}

	// Get the first row (we typically only have one balance config per level)
	static const FString ContextString(TEXT("GetGameplayBalanceData"));
	TArray<FName> RowNames = GameplayBalanceTable->GetRowNames();

	if (RowNames.Num() > 0)
	{
		return GameplayBalanceTable->FindRow<FGameplayBalanceData>(RowNames[0], ContextString);
	}

	return nullptr;
}

void AWhitelineNightmareGameMode::UpdateDistanceTraveled(float DeltaDistance)
{
	if (bGameOver)
	{
		return;
	}

	DistanceTraveled += DeltaDistance;
	CheckWinCondition();
}

void AWhitelineNightmareGameMode::CheckWinCondition()
{
	if (bGameOver || bHasWon)
	{
		return;
	}

	const FGameplayBalanceData* BalanceData = GetGameplayBalanceData();
	if (BalanceData && DistanceTraveled >= BalanceData->WinDistance)
	{
		HandleGameOver(true);
	}
}

void AWhitelineNightmareGameMode::HandleGameOver(bool bPlayerWon)
{
	if (bGameOver)
	{
		return;
	}

	bGameOver = true;
	bHasWon = bPlayerWon;

	UE_LOG(LogTemp, Log, TEXT("Game Over! Player %s"), bPlayerWon ? TEXT("Won") : TEXT("Lost"));

	// TODO: Trigger game over UI, stop world scrolling, etc.
	// This will be implemented when we create the world scrolling system
}
