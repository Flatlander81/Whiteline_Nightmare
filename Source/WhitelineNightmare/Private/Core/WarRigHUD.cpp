// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Core/WarRigPawn.h"
#include "Core/LaneSystemComponent.h"
#include "UI/WarRigHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigHUD, Log, All);

AWarRigHUD::AWarRigHUD()
	: FuelPercentage(1.0f)
	, ArmorPercentage(1.0f)
	, ScrapAmount(0)
	, DistancePercentage(0.0f)
	, bShowingGameOver(false)
	, bPlayerWonGame(false)
	, bShowDebugLaneUI(true) // Show by default
	, FuelWidget(nullptr)
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWarRigHUD::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWarRigHUD, Log, TEXT("WarRigHUD: Initialized (Debug Lane UI: %s)"),
		bShowDebugLaneUI ? TEXT("Enabled") : TEXT("Disabled"));

	// Create fuel HUD widget
	if (!FuelWidget)
	{
		FuelWidget = CreateWidget<UWarRigHUDWidget>(GetWorld(), UWarRigHUDWidget::StaticClass());
		if (FuelWidget)
		{
			// Add widget to viewport
			FuelWidget->AddToViewport(0); // Z-order 0 (behind other UI)

			// CRITICAL: Set visibility to Visible so the widget renders!
			FuelWidget->SetVisibility(ESlateVisibility::Visible);

			UE_LOG(LogWarRigHUD, Log, TEXT("WarRigHUD: Created fuel HUD widget and set visibility to Visible"));

			// Get war rig pawn and initialize widget with its AbilitySystemComponent
			AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetOwningPawn());
			if (WarRig)
			{
				UAbilitySystemComponent* ASC = WarRig->GetAbilitySystemComponent();
				if (ASC)
				{
					FuelWidget->InitializeWidget(ASC);
					UE_LOG(LogWarRigHUD, Log, TEXT("WarRigHUD: Fuel widget initialized with AbilitySystemComponent"));
				}
				else
				{
					UE_LOG(LogWarRigHUD, Error, TEXT("WarRigHUD: War Rig has no AbilitySystemComponent"));
				}
			}
			else
			{
				UE_LOG(LogWarRigHUD, Warning, TEXT("WarRigHUD: Could not get War Rig pawn, fuel widget not bound to GAS"));
			}
		}
		else
		{
			UE_LOG(LogWarRigHUD, Error, TEXT("WarRigHUD: Failed to create fuel HUD widget"));
		}
	}
}

void AWarRigHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw debug HUD until proper UI widgets are implemented
	DrawDebugHUD();

	// Draw debug lane UI buttons
	if (bShowDebugLaneUI)
	{
		DrawDebugLaneUI();
	}
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
	bShowDebugLaneUI = true;
	UE_LOG(LogWarRigHUD, Log, TEXT("ShowDebugLaneUI: Debug lane UI enabled"));
}

void AWarRigHUD::HideDebugLaneUI()
{
	bShowDebugLaneUI = false;
	UE_LOG(LogWarRigHUD, Log, TEXT("HideDebugLaneUI: Debug lane UI disabled"));
}

void AWarRigHUD::ToggleDebugLaneUI()
{
	bShowDebugLaneUI = !bShowDebugLaneUI;
	UE_LOG(LogWarRigHUD, Log, TEXT("ToggleDebugLaneUI: Debug lane UI %s"),
		bShowDebugLaneUI ? TEXT("enabled") : TEXT("disabled"));
}

void AWarRigHUD::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);

	if (BoxName == "LaneLeftButton")
	{
		// Get war rig pawn
		AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetOwningPawn());
		if (WarRig)
		{
			ULaneSystemComponent* LaneSystem = WarRig->FindComponentByClass<ULaneSystemComponent>();
			if (LaneSystem && LaneSystem->CanChangeLaneLeft())
			{
				LaneSystem->ChangeLaneLeft();
				UE_LOG(LogWarRigHUD, Log, TEXT("NotifyHitBoxClick: Changed to left lane"));
			}
		}
	}
	else if (BoxName == "LaneRightButton")
	{
		// Get war rig pawn
		AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetOwningPawn());
		if (WarRig)
		{
			ULaneSystemComponent* LaneSystem = WarRig->FindComponentByClass<ULaneSystemComponent>();
			if (LaneSystem && LaneSystem->CanChangeLaneRight())
			{
				LaneSystem->ChangeLaneRight();
				UE_LOG(LogWarRigHUD, Log, TEXT("NotifyHitBoxClick: Changed to right lane"));
			}
		}
	}
}

void AWarRigHUD::DrawDebugLaneUI()
{
	if (!Canvas)
	{
		return;
	}

	// Get war rig and lane system
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetOwningPawn());
	ULaneSystemComponent* LaneSystem = WarRig ? WarRig->FindComponentByClass<ULaneSystemComponent>() : nullptr;

	if (!LaneSystem)
	{
		return;
	}

	// Button dimensions
	const float ButtonWidth = 150.0f;
	const float ButtonHeight = 50.0f;
	const float ButtonSpacing = 20.0f;
	const float BottomMargin = 100.0f;

	// Calculate button positions (bottom center of screen)
	const float CenterX = Canvas->SizeX * 0.5f;
	const float ButtonY = Canvas->SizeY - BottomMargin;

	// Left button position
	const float LeftButtonX = CenterX - ButtonWidth - ButtonSpacing;
	const float RightButtonX = CenterX + ButtonSpacing;

	// Update hit boxes for click detection
	LeftLaneButtonBox = FBox2D(
		FVector2D(LeftButtonX, ButtonY),
		FVector2D(LeftButtonX + ButtonWidth, ButtonY + ButtonHeight)
	);

	RightLaneButtonBox = FBox2D(
		FVector2D(RightButtonX, ButtonY),
		FVector2D(RightButtonX + ButtonWidth, ButtonY + ButtonHeight)
	);

	// Draw left button
	{
		FLinearColor ButtonColor = LaneSystem->CanChangeLaneLeft() ?
			FLinearColor(0.2f, 0.6f, 0.2f, 0.8f) : // Green if enabled
			FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);   // Gray if disabled

		DrawRect(ButtonColor, LeftButtonX, ButtonY, ButtonWidth, ButtonHeight);

		// Draw button text
		FString ButtonText = TEXT("<< Lane Left");
		DrawText(ButtonText, FLinearColor::White, LeftButtonX + 10, ButtonY + 15);

		// Add hit box
		AddHitBox(LeftLaneButtonBox.Min, LeftLaneButtonBox.GetSize(), "LaneLeftButton", false, 0);
	}

	// Draw right button
	{
		FLinearColor ButtonColor = LaneSystem->CanChangeLaneRight() ?
			FLinearColor(0.2f, 0.6f, 0.2f, 0.8f) : // Green if enabled
			FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);   // Gray if disabled

		DrawRect(ButtonColor, RightButtonX, ButtonY, ButtonWidth, ButtonHeight);

		// Draw button text
		FString ButtonText = TEXT("Lane Right >>");
		DrawText(ButtonText, FLinearColor::White, RightButtonX + 10, ButtonY + 15);

		// Add hit box
		AddHitBox(RightLaneButtonBox.Min, RightLaneButtonBox.GetSize(), "LaneRightButton", false, 0);
	}

	// Draw current lane info above buttons
	const int32 CurrentLane = LaneSystem->GetCurrentLane();
	const FString LaneText = FString::Printf(TEXT("Current Lane: %d"), CurrentLane);
	DrawText(LaneText, FLinearColor::Yellow, CenterX - 60, ButtonY - 30, nullptr, 1.2f);
}

void AWarRigHUD::DebugToggleFuelUI()
{
	if (FuelWidget)
	{
		FuelWidget->ToggleVisibility();
		UE_LOG(LogWarRigHUD, Log, TEXT("DebugToggleFuelUI: Toggled fuel UI visibility"));
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("DebugToggleFuelUI: Fuel widget is null"));
	}
}

void AWarRigHUD::DebugTestFuelColors()
{
	if (FuelWidget)
	{
		FuelWidget->DebugCycleColors();
		UE_LOG(LogWarRigHUD, Log, TEXT("DebugTestFuelColors: Cycled fuel colors"));
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("DebugTestFuelColors: Fuel widget is null"));
	}
}

void AWarRigHUD::DebugShowFuelBindings()
{
	if (FuelWidget)
	{
		const bool bBindingSuccessful = FuelWidget->IsBindingSuccessful();
		UE_LOG(LogWarRigHUD, Log, TEXT("DebugShowFuelBindings: Fuel widget binding status: %s"),
			bBindingSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"));

		// Also log widget visibility
		const ESlateVisibility Visibility = FuelWidget->GetVisibility();
		FString VisibilityStr;
		switch (Visibility)
		{
			case ESlateVisibility::Visible: VisibilityStr = TEXT("Visible"); break;
			case ESlateVisibility::Collapsed: VisibilityStr = TEXT("Collapsed"); break;
			case ESlateVisibility::Hidden: VisibilityStr = TEXT("Hidden"); break;
			case ESlateVisibility::HitTestInvisible: VisibilityStr = TEXT("HitTestInvisible"); break;
			case ESlateVisibility::SelfHitTestInvisible: VisibilityStr = TEXT("SelfHitTestInvisible"); break;
			default: VisibilityStr = TEXT("Unknown"); break;
		}
		UE_LOG(LogWarRigHUD, Log, TEXT("DebugShowFuelBindings: Widget visibility: %s"), *VisibilityStr);
		UE_LOG(LogWarRigHUD, Log, TEXT("DebugShowFuelBindings: Widget is in viewport: %s"),
			FuelWidget->IsInViewport() ? TEXT("YES") : TEXT("NO"));

		// Log detailed geometry information
		FuelWidget->DebugLogGeometry();
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("DebugShowFuelBindings: Fuel widget is null"));
	}
}

void AWarRigHUD::DebugForceCreateFuelWidget()
{
	UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: Attempting to create fuel widget..."));

	if (FuelWidget)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("DebugForceCreateFuelWidget: Fuel widget already exists!"));
		UE_LOG(LogWarRigHUD, Log, TEXT("  -> Visibility: %s"),
			FuelWidget->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Hidden/Other"));
		UE_LOG(LogWarRigHUD, Log, TEXT("  -> In Viewport: %s"),
			FuelWidget->IsInViewport() ? TEXT("YES") : TEXT("NO"));
		UE_LOG(LogWarRigHUD, Log, TEXT("  -> Binding Status: %s"),
			FuelWidget->IsBindingSuccessful() ? TEXT("SUCCESS") : TEXT("FAILED"));
		return;
	}

	// Get world
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("DebugForceCreateFuelWidget: World is null!"));
		return;
	}

	// Create widget
	FuelWidget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	if (!FuelWidget)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("DebugForceCreateFuelWidget: Failed to create widget!"));
		return;
	}

	UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: Widget created successfully"));

	// Add to viewport
	FuelWidget->AddToViewport(0);

	// CRITICAL: Set visibility to Visible so the widget renders!
	FuelWidget->SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: Widget added to viewport and set to Visible"));

	// Try to initialize with War Rig's ASC
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetOwningPawn());
	if (WarRig)
	{
		UAbilitySystemComponent* ASC = WarRig->GetAbilitySystemComponent();
		if (ASC)
		{
			FuelWidget->InitializeWidget(ASC);
			UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: Widget initialized with ASC"));
		}
		else
		{
			UE_LOG(LogWarRigHUD, Warning, TEXT("DebugForceCreateFuelWidget: War Rig has no ASC"));
		}
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("DebugForceCreateFuelWidget: No War Rig pawn found"));
	}

	// Force a test update to make it visible
	FuelWidget->UpdateFuelDisplay(75.0f, 100.0f);
	UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: Forced test update (75/100)"));

	UE_LOG(LogWarRigHUD, Log, TEXT("DebugForceCreateFuelWidget: DONE - Widget should now be visible at top-left!"));
}
