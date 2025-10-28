// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "UI/DebugLaneUI.h"
#include "Blueprint/UserWidget.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigHUD, Log, All);

AWarRigHUD::AWarRigHUD()
	: FuelPercentage(1.0f)
	, ArmorPercentage(1.0f)
	, ScrapAmount(0)
	, DistancePercentage(0.0f)
	, bShowingGameOver(false)
	, bPlayerWonGame(false)
	, DebugLaneUIClass(nullptr)
	, DebugLaneUIWidget(nullptr)
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWarRigHUD::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWarRigHUD, Log, TEXT("WarRigHUD: Initialized"));

	// TODO: Create UI widgets programmatically using UMG
}

void AWarRigHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw debug HUD until proper UI widgets are implemented
	DrawDebugHUD();
}

void AWarRigHUD::UpdateFuelDisplay(float CurrentFuel, float MaxFuel)
{
	// Input validation
	if (MaxFuel <= 0.0f)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("UpdateFuelDisplay: MaxFuel must be positive: %.2f"), MaxFuel);
		return;
	}

	if (CurrentFuel < 0.0f)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("UpdateFuelDisplay: CurrentFuel is negative: %.2f"), CurrentFuel);
		CurrentFuel = 0.0f;
	}

	// Calculate percentage
	const float NewPercentage = FMath::Clamp(CurrentFuel / MaxFuel, 0.0f, 1.0f);
	FuelPercentage = ValidatePercentage(NewPercentage);

	UE_LOG(LogWarRigHUD, Verbose, TEXT("UpdateFuelDisplay: %.2f / %.2f (%.1f%%)"),
		CurrentFuel, MaxFuel, FuelPercentage * 100.0f);
}

void AWarRigHUD::UpdateArmorDisplay(float CurrentArmor, float MaxArmor)
{
	// Input validation
	if (MaxArmor <= 0.0f)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("UpdateArmorDisplay: MaxArmor must be positive: %.2f"), MaxArmor);
		return;
	}

	if (CurrentArmor < 0.0f)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("UpdateArmorDisplay: CurrentArmor is negative: %.2f"), CurrentArmor);
		CurrentArmor = 0.0f;
	}

	// Calculate percentage
	const float NewPercentage = FMath::Clamp(CurrentArmor / MaxArmor, 0.0f, 1.0f);
	ArmorPercentage = ValidatePercentage(NewPercentage);

	UE_LOG(LogWarRigHUD, Verbose, TEXT("UpdateArmorDisplay: %.2f / %.2f (%.1f%%)"),
		CurrentArmor, MaxArmor, ArmorPercentage * 100.0f);
}

void AWarRigHUD::UpdateScrapDisplay(int32 CurrentScrap)
{
	// Input validation
	if (CurrentScrap < 0)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("UpdateScrapDisplay: Scrap is negative: %d"), CurrentScrap);
		CurrentScrap = 0;
	}

	ScrapAmount = CurrentScrap;

	UE_LOG(LogWarRigHUD, Verbose, TEXT("UpdateScrapDisplay: %d"), ScrapAmount);
}

void AWarRigHUD::UpdateDistanceDisplay(float CurrentDistance, float TargetDistance)
{
	// Input validation
	if (TargetDistance <= 0.0f)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("UpdateDistanceDisplay: TargetDistance must be positive: %.2f"), TargetDistance);
		return;
	}

	if (CurrentDistance < 0.0f)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("UpdateDistanceDisplay: CurrentDistance is negative: %.2f"), CurrentDistance);
		CurrentDistance = 0.0f;
	}

	// Calculate percentage
	const float NewPercentage = FMath::Clamp(CurrentDistance / TargetDistance, 0.0f, 1.0f);
	DistancePercentage = ValidatePercentage(NewPercentage);

	UE_LOG(LogWarRigHUD, Verbose, TEXT("UpdateDistanceDisplay: %.2f / %.2f (%.1f%%)"),
		CurrentDistance, TargetDistance, DistancePercentage * 100.0f);
}

void AWarRigHUD::ShowGameOverScreen(bool bPlayerWon)
{
	if (bShowingGameOver)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("ShowGameOverScreen: Game over screen already showing"));
		return;
	}

	bShowingGameOver = true;
	bPlayerWonGame = bPlayerWon;

	UE_LOG(LogWarRigHUD, Log, TEXT("ShowGameOverScreen: Player %s"), bPlayerWon ? TEXT("WON") : TEXT("LOST"));

	// TODO: Create and show game over UI widget
}

void AWarRigHUD::HideGameOverScreen()
{
	if (!bShowingGameOver)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("HideGameOverScreen: Game over screen not showing"));
		return;
	}

	bShowingGameOver = false;
	bPlayerWonGame = false;

	UE_LOG(LogWarRigHUD, Log, TEXT("HideGameOverScreen: Hiding game over screen"));

	// TODO: Hide game over UI widget
}

float AWarRigHUD::ValidatePercentage(float Value) const
{
	// Clamp to valid range
	const float ClampedValue = FMath::Clamp(Value, 0.0f, 1.0f);

	// Warn if clamping occurred
	if (!FMath::IsNearlyEqual(Value, ClampedValue))
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("ValidatePercentage: Value %.4f clamped to %.4f"), Value, ClampedValue);
	}

	return ClampedValue;
}

void AWarRigHUD::DrawDebugHUD()
{
	// This is a temporary debug HUD - will be replaced with proper UMG widgets

	if (!Canvas)
	{
		return;
	}

	// Set up font and colors
	const float LineHeight = 20.0f;
	float YPos = 50.0f;
	const float XPos = 50.0f;

	// Draw fuel
	FString FuelText = FString::Printf(TEXT("Fuel: %.1f%%"), FuelPercentage * 100.0f);
	DrawText(FuelText, FLinearColor::Green, XPos, YPos, nullptr, 1.0f);
	YPos += LineHeight;

	// Draw armor
	FString ArmorText = FString::Printf(TEXT("Armor: %.1f%%"), ArmorPercentage * 100.0f);
	DrawText(ArmorText, FLinearColor::Blue, XPos, YPos, nullptr, 1.0f);
	YPos += LineHeight;

	// Draw scrap
	FString ScrapText = FString::Printf(TEXT("Scrap: %d"), ScrapAmount);
	DrawText(ScrapText, FLinearColor::Yellow, XPos, YPos, nullptr, 1.0f);
	YPos += LineHeight;

	// Draw distance
	FString DistanceText = FString::Printf(TEXT("Distance: %.1f%%"), DistancePercentage * 100.0f);
	DrawText(DistanceText, FLinearColor::White, XPos, YPos, nullptr, 1.0f);
	YPos += LineHeight;

	// Draw game over if applicable
	if (bShowingGameOver)
	{
		YPos += LineHeight;
		FString GameOverText = bPlayerWonGame ? TEXT("YOU WIN!") : TEXT("GAME OVER");
		FLinearColor GameOverColor = bPlayerWonGame ? FLinearColor::Green : FLinearColor::Red;
		DrawText(GameOverText, GameOverColor, Canvas->SizeX * 0.5f - 100.0f, Canvas->SizeY * 0.5f, nullptr, 2.0f);
	}
}

void AWarRigHUD::ShowDebugLaneUI()
{
	// Create widget if it doesn't exist
	if (!DebugLaneUIWidget)
	{
		if (!DebugLaneUIClass)
		{
			// Use default C++ class if no Blueprint is specified
			DebugLaneUIClass = UDebugLaneUI::StaticClass();
		}

		if (DebugLaneUIClass)
		{
			DebugLaneUIWidget = CreateWidget<UDebugLaneUI>(GetWorld(), DebugLaneUIClass);
			if (DebugLaneUIWidget)
			{
				UE_LOG(LogWarRigHUD, Log, TEXT("ShowDebugLaneUI: Created debug lane UI widget"));
			}
			else
			{
				UE_LOG(LogWarRigHUD, Error, TEXT("ShowDebugLaneUI: Failed to create debug lane UI widget"));
				return;
			}
		}
		else
		{
			UE_LOG(LogWarRigHUD, Error, TEXT("ShowDebugLaneUI: DebugLaneUIClass is null"));
			return;
		}
	}

	// Add to viewport if not already visible
	if (DebugLaneUIWidget && !DebugLaneUIWidget->IsInViewport())
	{
		DebugLaneUIWidget->AddToViewport();
		UE_LOG(LogWarRigHUD, Log, TEXT("ShowDebugLaneUI: Added debug lane UI to viewport"));
	}
}

void AWarRigHUD::HideDebugLaneUI()
{
	if (DebugLaneUIWidget && DebugLaneUIWidget->IsInViewport())
	{
		DebugLaneUIWidget->RemoveFromParent();
		UE_LOG(LogWarRigHUD, Log, TEXT("HideDebugLaneUI: Removed debug lane UI from viewport"));
	}
}

void AWarRigHUD::ToggleDebugLaneUI()
{
	if (DebugLaneUIWidget && DebugLaneUIWidget->IsInViewport())
	{
		HideDebugLaneUI();
	}
	else
	{
		ShowDebugLaneUI();
	}
}
