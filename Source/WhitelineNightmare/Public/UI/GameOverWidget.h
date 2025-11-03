// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UTextBlock;
class UButton;
class UCanvasPanel;
class UBorder;

/**
 * UGameOverWidget - Game over screen UI
 *
 * Pure C++ widget created programmatically (same pattern as WarRigHUDWidget).
 * No Blueprint required!
 *
 * Displays:
 * - Large "GAME OVER" text (72pt, red/orange, centered)
 * - Game over reason (e.g., "Out of Fuel")
 * - Stats display (distance, enemies killed, fuel/scrap collected)
 * - "Press R to Restart" instruction
 * - Restart button with click handler
 */
UCLASS()
class WHITELINENIGHTMARE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UGameOverWidget(const FObjectInitializer& ObjectInitializer);

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
	// UUserWidget interface
	virtual void NativeConstruct() override;

	/**
	 * Create UI elements programmatically using WidgetTree
	 * Same pattern as WarRigHUDWidget!
	 */
	void CreateWidgetLayout();

	/**
	 * Update the displayed stats
	 */
	void UpdateStatsDisplay();

	/**
	 * Get stats from GameMode
	 */
	void FetchStatsFromGameMode();

	// Button click handlers
	UFUNCTION()
	void OnRestartButtonClicked();

protected:
	// Game over reason string
	UPROPERTY()
	FString GameOverReason;

	// Stats
	UPROPERTY()
	float DistanceTraveled;

	UPROPERTY()
	int32 EnemiesKilled;

	UPROPERTY()
	float FuelCollected;

	UPROPERTY()
	int32 ScrapCollected;

	// UI Elements (created programmatically via WidgetTree - SAME AS WarRigHUDWidget)
	UPROPERTY()
	UCanvasPanel* RootCanvas;

	UPROPERTY()
	UBorder* BackgroundOverlay;

	UPROPERTY()
	UTextBlock* GameOverText;

	UPROPERTY()
	UTextBlock* ReasonText;

	UPROPERTY()
	UTextBlock* StatsText;

	UPROPERTY()
	UTextBlock* RestartInstructionText;

	UPROPERTY()
	UButton* RestartButton;

	UPROPERTY()
	UTextBlock* RestartButtonText;
};
