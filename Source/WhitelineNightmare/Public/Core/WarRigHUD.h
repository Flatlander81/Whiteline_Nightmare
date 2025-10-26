// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WarRigHUD.generated.h"

class UUserWidget;

/**
 * HUD for war rig
 * Manages UI widgets and provides debug visualization
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWarRigHUD();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	virtual void Tick(float DeltaSeconds) override;

	// Show/hide main game UI
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameUI();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideGameUI();

	// Show victory screen
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowVictoryScreen();

	// Show game over screen
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameOverScreen();

	// Toggle debug info
	UFUNCTION(BlueprintCallable, Category = "HUD|Debug")
	void ToggleDebugInfo();

protected:
	// Widget classes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<UUserWidget> GameUIWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<UUserWidget> VictoryScreenWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<UUserWidget> GameOverScreenWidgetClass;

	// Widget instances
	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	UUserWidget* GameUIWidget;

	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	UUserWidget* VictoryScreenWidget;

	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	UUserWidget* GameOverScreenWidget;

	// Debug settings
	UPROPERTY(BlueprintReadWrite, Category = "HUD|Debug")
	bool bShowDebugInfo;

	// Create widget instances
	virtual void CreateWidgets();

	// Draw debug information
	virtual void DrawDebugInfo();

	// Validate HUD state
	virtual bool ValidateHUDState() const;
};
