// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WhitelineNightmareGameMode.generated.h"

class UDataTable;
struct FGameplayBalanceData;

/**
 * Game Mode for Whiteline Nightmare
 * Manages game flow, scoring, and core gameplay rules
 *
 * Key responsibilities:
 * - Loading and providing access to gameplay data tables
 * - Tracking game state (distance traveled, fuel, scrap)
 * - Win/loss conditions
 * - Difficulty scaling over time
 */
UCLASS()
class WHITELINENIGHTMARE_API AWhitelineNightmareGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWhitelineNightmareGameMode();

	virtual void BeginPlay() override;

	/** Get the current gameplay balance data (returns default values if not set) */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Data")
	bool GetGameplayBalanceData(FGameplayBalanceData& OutData) const;

protected:
	/** Data table containing gameplay balance values */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> GameplayBalanceTable;

	/** Data table containing turret definitions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> TurretDataTable;

	/** Data table containing enemy definitions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> EnemyDataTable;

	/** Data table containing pickup definitions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> PickupDataTable;

	/** Data table containing world scroll parameters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> WorldScrollDataTable;

	/** Data table containing war rig configurations */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> WarRigDataTable;

	/** Data table containing obstacle definitions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Data")
	TObjectPtr<UDataTable> ObstacleDataTable;

	/** Current distance traveled */
	UPROPERTY(BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	float DistanceTraveled;

	/** Has the player won the current run? */
	UPROPERTY(BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	bool bHasWon;

	/** Has the game ended (win or loss)? */
	UPROPERTY(BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	bool bGameOver;

public:
	/** Called when distance traveled updates */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void UpdateDistanceTraveled(float DeltaDistance);

	/** Check and handle win condition */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void CheckWinCondition();

	/** Handle game over (player death) */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void HandleGameOver(bool bPlayerWon);

	/** Get current distance traveled */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game State")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/** Get data tables for external access */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetTurretDataTable() const { return TurretDataTable; }

	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetEnemyDataTable() const { return EnemyDataTable; }

	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetPickupDataTable() const { return PickupDataTable; }

	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetWorldScrollDataTable() const { return WorldScrollDataTable; }

	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetWarRigDataTable() const { return WarRigDataTable; }

	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Data")
	UDataTable* GetObstacleDataTable() const { return ObstacleDataTable; }
};
