// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

/**
 * UGameOverWidget - Game over screen UI
 *
 * Displays game over information and allows the player to restart.
 * UI is created programmatically in NativeConstruct().
 *
 * Components:
 * - "GAME OVER" text (large, centered)
 * - Reason text (e.g., "Out of Fuel")
 * - Stats display (distance, enemies killed, etc.)
 * - "Press R to Restart" instruction
 * - Restart button (optional)
 */
UCLASS()
class WHITELINENIGHTMARE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Set the game over reason
	 * @param Reason - Reason for game over (e.g., "Out of Fuel")
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Over")
	void SetGameOverReason(const FString& Reason);

	/**
	 * Set the stats to display
	 * @param DistanceTraveled - Distance traveled in units
	 * @param EnemiesKilled - Number of enemies killed
	 * @param FuelCollected - Amount of fuel collected
	 * @param ScrapCollected - Amount of scrap collected
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Over")
	void SetStats(float DistanceTraveled, int32 EnemiesKilled, float FuelCollected, int32 ScrapCollected);

protected:
	// Called when the widget is constructed
	virtual void NativeConstruct() override;

	// Game over reason string
	UPROPERTY(BlueprintReadOnly, Category = "Game Over")
	FString GameOverReason;

	// Stats
	UPROPERTY(BlueprintReadOnly, Category = "Game Over|Stats")
	float DistanceTraveled;

	UPROPERTY(BlueprintReadOnly, Category = "Game Over|Stats")
	int32 EnemiesKilled;

	UPROPERTY(BlueprintReadOnly, Category = "Game Over|Stats")
	float FuelCollected;

	UPROPERTY(BlueprintReadOnly, Category = "Game Over|Stats")
	int32 ScrapCollected;

	// UI Text Blocks (created programmatically)
	UPROPERTY()
	TObjectPtr<class UTextBlock> GameOverText;

	UPROPERTY()
	TObjectPtr<class UTextBlock> ReasonText;

	UPROPERTY()
	TObjectPtr<class UTextBlock> StatsText;

	UPROPERTY()
	TObjectPtr<class UTextBlock> RestartInstructionText;

	UPROPERTY()
	TObjectPtr<class UButton> RestartButton;

	UPROPERTY()
	TObjectPtr<class UTextBlock> RestartButtonText;

	// Button click handlers
	UFUNCTION()
	void OnRestartButtonClicked();

private:
	/**
	 * Create the UI programmatically
	 */
	void CreateUI();

	/**
	 * Update the displayed stats
	 */
	void UpdateStatsDisplay();

	/**
	 * Get stats from GameMode
	 */
	void FetchStatsFromGameMode();
};
