// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WarRigHUD.generated.h"

class UUserWidget;

/**
 * HUD for the War Rig
 * Manages all UI elements during gameplay
 *
 * Key responsibilities:
 * - Displaying fuel, health, scrap, and distance meters
 * - Showing turret placement UI
 * - Rendering warning indicators for obstacles
 * - Managing pause menu and game over screens
 * - Displaying minimap or radar
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWarRigHUD();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

protected:
	/** Main gameplay HUD widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> MainHUDWidgetClass;

	/** Main gameplay HUD widget instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> MainHUDWidget;

	/** Pause menu widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	/** Pause menu widget instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> PauseMenuWidget;

	/** Game over widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> GameOverWidgetClass;

	/** Game over widget instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> GameOverWidget;

	/** Turret placement widget class */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> TurretPlacementWidgetClass;

	/** Turret placement widget instance */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	TObjectPtr<UUserWidget> TurretPlacementWidget;

public:
	/** Show the main HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainHUD();

	/** Hide the main HUD */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideMainHUD();

	/** Show the pause menu */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowPauseMenu();

	/** Hide the pause menu */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HidePauseMenu();

	/** Show the game over screen */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowGameOverScreen(bool bPlayerWon);

	/** Show the turret placement UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowTurretPlacementUI();

	/** Hide the turret placement UI */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void HideTurretPlacementUI();

	/** Update fuel display */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateFuelDisplay(float CurrentFuel, float MaxFuel);

	/** Update health display */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateHealthDisplay(float CurrentHealth, float MaxHealth);

	/** Update scrap display */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateScrapDisplay(int32 CurrentScrap);

	/** Update distance display */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateDistanceDisplay(float CurrentDistance, float TargetDistance);

protected:
	/** Create a widget from class if it doesn't exist */
	UUserWidget* CreateWidgetIfNeeded(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& WidgetInstance);
};
