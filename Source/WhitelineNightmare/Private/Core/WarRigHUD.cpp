// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WarRigHUD.h"
#include "Blueprint/UserWidget.h"

AWarRigHUD::AWarRigHUD()
{
	// Widget classes will be set in Blueprint
	MainHUDWidgetClass = nullptr;
	PauseMenuWidgetClass = nullptr;
	GameOverWidgetClass = nullptr;
	TurretPlacementWidgetClass = nullptr;

	// Initialize widget instances
	MainHUDWidget = nullptr;
	PauseMenuWidget = nullptr;
	GameOverWidget = nullptr;
	TurretPlacementWidget = nullptr;
}

void AWarRigHUD::BeginPlay()
{
	Super::BeginPlay();

	// Create and show main HUD by default
	ShowMainHUD();
}

void AWarRigHUD::DrawHUD()
{
	Super::DrawHUD();

	// TODO: Draw any custom HUD elements here (crosshairs, targeting reticles, etc.)
	// Most UI will be handled via UMG widgets, but this can be used for performance-critical overlays
}

void AWarRigHUD::ShowMainHUD()
{
	if (MainHUDWidget)
	{
		MainHUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
	else if (MainHUDWidgetClass)
	{
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDWidgetClass);
		if (MainHUDWidget)
		{
			MainHUDWidget->AddToViewport();
		}
	}
}

void AWarRigHUD::HideMainHUD()
{
	if (MainHUDWidget)
	{
		MainHUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AWarRigHUD::ShowPauseMenu()
{
	PauseMenuWidget = CreateWidgetIfNeeded(PauseMenuWidgetClass, PauseMenuWidget);
	if (PauseMenuWidget)
	{
		PauseMenuWidget->AddToViewport(1); // Higher Z-order than main HUD
		PauseMenuWidget->SetVisibility(ESlateVisibility::Visible);

		// Set input mode to UI only
		if (APlayerController* PC = GetOwningPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}
}

void AWarRigHUD::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();

		// Restore game input mode
		if (APlayerController* PC = GetOwningPlayerController())
		{
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(false);
		}
	}
}

void AWarRigHUD::ShowGameOverScreen(bool bPlayerWon)
{
	GameOverWidget = CreateWidgetIfNeeded(GameOverWidgetClass, GameOverWidget);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport(2); // Highest Z-order
		GameOverWidget->SetVisibility(ESlateVisibility::Visible);

		// Set input mode to UI only
		if (APlayerController* PC = GetOwningPlayerController())
		{
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(GameOverWidget->TakeWidget());
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}

		// TODO: Pass win/loss state to widget via Blueprint interface
		// GameOverWidget->SetGameResult(bPlayerWon);
	}
}

void AWarRigHUD::ShowTurretPlacementUI()
{
	TurretPlacementWidget = CreateWidgetIfNeeded(TurretPlacementWidgetClass, TurretPlacementWidget);
	if (TurretPlacementWidget)
	{
		TurretPlacementWidget->AddToViewport(1);
		TurretPlacementWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AWarRigHUD::HideTurretPlacementUI()
{
	if (TurretPlacementWidget)
	{
		TurretPlacementWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AWarRigHUD::UpdateFuelDisplay(float CurrentFuel, float MaxFuel)
{
	// TODO: Call Blueprint implementable event or interface method to update fuel bar
	// This will be connected to the MainHUDWidget via Blueprint interface
	UE_LOG(LogTemp, VeryVerbose, TEXT("Fuel: %.2f / %.2f"), CurrentFuel, MaxFuel);
}

void AWarRigHUD::UpdateHealthDisplay(float CurrentHealth, float MaxHealth)
{
	// TODO: Call Blueprint implementable event or interface method to update health bar
	UE_LOG(LogTemp, VeryVerbose, TEXT("Health: %.2f / %.2f"), CurrentHealth, MaxHealth);
}

void AWarRigHUD::UpdateScrapDisplay(int32 CurrentScrap)
{
	// TODO: Call Blueprint implementable event or interface method to update scrap counter
	UE_LOG(LogTemp, VeryVerbose, TEXT("Scrap: %d"), CurrentScrap);
}

void AWarRigHUD::UpdateDistanceDisplay(float CurrentDistance, float TargetDistance)
{
	// TODO: Call Blueprint implementable event or interface method to update distance meter
	UE_LOG(LogTemp, VeryVerbose, TEXT("Distance: %.2f / %.2f"), CurrentDistance, TargetDistance);
}

UUserWidget* AWarRigHUD::CreateWidgetIfNeeded(TSubclassOf<UUserWidget> WidgetClass, TObjectPtr<UUserWidget>& WidgetInstance)
{
	if (!WidgetInstance && WidgetClass)
	{
		WidgetInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
	}
	return WidgetInstance;
}
