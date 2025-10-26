// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/GameDataStructs.h"
#include "WhitelineNightmareGameMode.generated.h"

class UDataTable;

/**
 * Main game mode for Whiteline Nightmare
 * Manages game state, spawning, and game rules
 */
UCLASS()
class WHITELINENIGHTMARE_API AWhitelineNightmareGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWhitelineNightmareGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// Get gameplay balance data
	UFUNCTION(BlueprintCallable, Category = "GameMode|Data")
	const FGameplayBalanceData* GetBalanceData() const;

	// Get world scroll data
	UFUNCTION(BlueprintCallable, Category = "GameMode|Data")
	const FWorldScrollData* GetWorldScrollData() const;

	// Add score
	UFUNCTION(BlueprintCallable, Category = "GameMode|Score")
	void AddScore(int32 Points);

	// Get current score
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Score")
	int32 GetScore() const { return CurrentScore; }

	// Get distance traveled
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Progress")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	// Get win distance
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Progress")
	float GetWinDistance() const;

	// Check if game is won
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Victory")
	bool IsGameWon() const;

	// Trigger game over
	UFUNCTION(BlueprintCallable, Category = "GameMode|Victory")
	void TriggerGameOver(bool bVictory);

protected:
	// Data tables
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* BalanceDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* TurretDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* EnemyDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* PickupDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* WorldScrollDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode|Data")
	UDataTable* WarRigDataTable;

	// Game state
	UPROPERTY(BlueprintReadOnly, Category = "GameMode|State")
	int32 CurrentScore;

	UPROPERTY(BlueprintReadOnly, Category = "GameMode|State")
	float DistanceTraveled;

	UPROPERTY(BlueprintReadOnly, Category = "GameMode|State")
	bool bIsGameOver;

	UPROPERTY(BlueprintReadOnly, Category = "GameMode|State")
	bool bIsVictory;

	// Current scroll speed
	UPROPERTY(BlueprintReadOnly, Category = "GameMode|State")
	float CurrentScrollSpeed;

	// Cached data
	const FGameplayBalanceData* CachedBalanceData;
	const FWorldScrollData* CachedWorldScrollData;

	// Initialize data tables
	virtual void InitializeDataTables();

	// Validate data table
	bool ValidateDataTable(UDataTable* DataTable, const FString& TableName) const;

	// Update distance traveled
	virtual void UpdateDistanceTraveled(float DeltaSeconds);

	// Called when game is won
	virtual void OnGameWon();

	// Called when game is lost
	virtual void OnGameLost();
};
