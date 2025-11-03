// Copyright Flatlander81. All Rights Reserved.

#include "UI/WarRigHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"
#include "AbilitySystemComponent.h"
#include "GAS/WarRigAttributeSet.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigHUDWidget, Log, All);

UWarRigHUDWidget::UWarRigHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RootCanvas(nullptr)
	, FuelProgressBar(nullptr)
	, FuelTextBlock(nullptr)
	, AbilitySystemComponent(nullptr)
	, CurrentFuel(100.0f)
	, CurrentMaxFuel(100.0f)
	, bBindingSuccessful(false)
	, DebugColorIndex(0)
{
}

void UWarRigHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: NativeConstruct called"));

	// Create widget layout programmatically
	CreateWidgetLayout();

	// Initialize with default values
	UpdateFuelDisplay(CurrentFuel, CurrentMaxFuel);

	// Debug: Log geometry information after construction
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Widget constructed, logging geometry..."));
	DebugLogGeometry();
}

void UWarRigHUDWidget::NativeDestruct()
{
	// Clean up GAS delegate bindings
	if (AbilitySystemComponent)
	{
		if (FuelChangedHandle.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UWarRigAttributeSet::GetFuelAttribute()).Remove(FuelChangedHandle);
			FuelChangedHandle.Reset();
			UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Fuel attribute delegate unbound"));
		}

		if (MaxFuelChangedHandle.IsValid())
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UWarRigAttributeSet::GetMaxFuelAttribute()).Remove(MaxFuelChangedHandle);
			MaxFuelChangedHandle.Reset();
			UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: MaxFuel attribute delegate unbound"));
		}
	}

	Super::NativeDestruct();
}

void UWarRigHUDWidget::CreateWidgetLayout()
{
	// Get or create root canvas panel
	RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Created root canvas"));
	}

	// CRITICAL: Ensure Canvas Panel is visible and configured properly
	if (RootCanvas)
	{
		// Must explicitly set canvas to visible!
		RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		RootCanvas->SetClipping(EWidgetClipping::ClipToBoundsAlways);
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Configured root canvas visibility and clipping"));
	}

	// Create fuel text block
	FuelTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("FuelTextBlock"));
	if (FuelTextBlock)
	{
		FuelTextBlock->SetText(FText::FromString(TEXT("Fuel: 100 / 100")));

		// Configure font
		FSlateFontInfo FontInfo = FuelTextBlock->GetFont();
		FontInfo.Size = 18;
		FuelTextBlock->SetFont(FontInfo);
		FuelTextBlock->SetColorAndOpacity(FLinearColor::White);

		// CRITICAL: Explicitly set visibility
		FuelTextBlock->SetVisibility(ESlateVisibility::Visible);

		// Add to canvas at CENTER of screen
		UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(FuelTextBlock);
		if (TextSlot)
		{
			// Center anchor point (0.5, 0.5)
			TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			// Center alignment so text centers on anchor point
			TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			// Position slightly above center
			TextSlot->SetPosition(FVector2D(0.0f, -50.0f));
			TextSlot->SetAutoSize(true);
		}

		UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Created fuel text block"));
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("WarRigHUDWidget: Failed to create fuel text block"));
	}

	// Create fuel progress bar
	FuelProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("FuelProgressBar"));
	if (FuelProgressBar)
	{
		// Configure progress bar
		FuelProgressBar->SetPercent(1.0f); // Start at full
		FuelProgressBar->SetFillColorAndOpacity(FLinearColor::Green);

		// CRITICAL: Explicitly set visibility
		FuelProgressBar->SetVisibility(ESlateVisibility::Visible);

		// Set bar style
		FProgressBarStyle BarStyle = FuelProgressBar->GetWidgetStyle();
		// Use default style (can be customized further if needed)
		FuelProgressBar->SetWidgetStyle(BarStyle);

		// Add to canvas at CENTER of screen, below text
		UCanvasPanelSlot* BarSlot = RootCanvas->AddChildToCanvas(FuelProgressBar);
		if (BarSlot)
		{
			// Center anchor point (0.5, 0.5)
			BarSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			// Center alignment so bar centers on anchor point
			BarSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			// Position slightly below center (below the text)
			BarSlot->SetPosition(FVector2D(0.0f, 0.0f));
			BarSlot->SetSize(FVector2D(ProgressBarWidth, ProgressBarHeight));
		}

		UE_LOG(LogWarRigHUDWidget, Log, TEXT("WarRigHUDWidget: Created fuel progress bar (%.0fx%.0f)"),
			ProgressBarWidth, ProgressBarHeight);
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("WarRigHUDWidget: Failed to create fuel progress bar"));
	}
}

void UWarRigHUDWidget::InitializeWidget(UAbilitySystemComponent* InAbilitySystemComponent)
{
	if (!InAbilitySystemComponent)
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("InitializeWidget: AbilitySystemComponent is null"));
		bBindingSuccessful = false;
		return;
	}

	AbilitySystemComponent = InAbilitySystemComponent;

	// Validate attribute set exists
	const UWarRigAttributeSet* AttributeSet = AbilitySystemComponent->GetSet<UWarRigAttributeSet>();
	if (!AttributeSet)
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("InitializeWidget: WarRigAttributeSet not found on AbilitySystemComponent"));
		bBindingSuccessful = false;
		return;
	}

	// Bind to Fuel attribute changes
	FGameplayAttribute FuelAttribute = UWarRigAttributeSet::GetFuelAttribute();
	if (FuelAttribute.IsValid())
	{
		FuelChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(FuelAttribute)
			.AddUObject(this, &UWarRigHUDWidget::OnFuelChanged);

		// Get initial value
		CurrentFuel = AbilitySystemComponent->GetNumericAttribute(FuelAttribute);

		UE_LOG(LogWarRigHUDWidget, Log, TEXT("InitializeWidget: Bound to Fuel attribute (Initial: %.2f)"), CurrentFuel);
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("InitializeWidget: Fuel attribute is invalid"));
		bBindingSuccessful = false;
		return;
	}

	// Bind to MaxFuel attribute changes
	FGameplayAttribute MaxFuelAttribute = UWarRigAttributeSet::GetMaxFuelAttribute();
	if (MaxFuelAttribute.IsValid())
	{
		MaxFuelChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxFuelAttribute)
			.AddUObject(this, &UWarRigHUDWidget::OnMaxFuelChanged);

		// Get initial value
		CurrentMaxFuel = AbilitySystemComponent->GetNumericAttribute(MaxFuelAttribute);

		UE_LOG(LogWarRigHUDWidget, Log, TEXT("InitializeWidget: Bound to MaxFuel attribute (Initial: %.2f)"), CurrentMaxFuel);
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Error, TEXT("InitializeWidget: MaxFuel attribute is invalid"));
		bBindingSuccessful = false;
		return;
	}

	bBindingSuccessful = true;

	// Update display with initial values
	UpdateFuelDisplay(CurrentFuel, CurrentMaxFuel);

	UE_LOG(LogWarRigHUDWidget, Log, TEXT("InitializeWidget: Successfully initialized and bound to GAS attributes"));
}

void UWarRigHUDWidget::UpdateFuelDisplay(float NewFuel, float NewMaxFuel)
{
	// Validate inputs
	if (NewMaxFuel <= 0.0f)
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("UpdateFuelDisplay: MaxFuel must be positive (%.2f), using default"), NewMaxFuel);
		NewMaxFuel = 100.0f;
	}

	// Clamp fuel to valid range
	NewFuel = FMath::Clamp(NewFuel, 0.0f, NewMaxFuel);

	// Store values
	CurrentFuel = NewFuel;
	CurrentMaxFuel = NewMaxFuel;

	// Calculate percentage
	const float Percentage = (NewMaxFuel > 0.0f) ? (NewFuel / NewMaxFuel) : 0.0f;

	// Update progress bar
	if (FuelProgressBar)
	{
		FuelProgressBar->SetPercent(Percentage);
		UpdateProgressBarColor(Percentage);
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("UpdateFuelDisplay: FuelProgressBar is null"));
	}

	// Update text display
	if (FuelTextBlock)
	{
		const FString FuelText = FString::Printf(TEXT("Fuel: %.0f / %.0f"), NewFuel, NewMaxFuel);
		FuelTextBlock->SetText(FText::FromString(FuelText));
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("UpdateFuelDisplay: FuelTextBlock is null"));
	}

	UE_LOG(LogWarRigHUDWidget, Verbose, TEXT("UpdateFuelDisplay: %.2f / %.2f (%.1f%%)"),
		NewFuel, NewMaxFuel, Percentage * 100.0f);
}

void UWarRigHUDWidget::UpdateProgressBarColor(float Percentage)
{
	if (!FuelProgressBar)
	{
		return;
	}

	const FLinearColor NewColor = GetColorForPercentage(Percentage);
	FuelProgressBar->SetFillColorAndOpacity(NewColor);

	UE_LOG(LogWarRigHUDWidget, VeryVerbose, TEXT("UpdateProgressBarColor: %.1f%% -> Color(R:%.2f, G:%.2f, B:%.2f)"),
		Percentage * 100.0f, NewColor.R, NewColor.G, NewColor.B);
}

FLinearColor UWarRigHUDWidget::GetColorForPercentage(float Percentage) const
{
	// Green: Fuel > 60%
	if (Percentage > HighFuelThreshold)
	{
		return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green
	}
	// Yellow: Fuel 30-60%
	else if (Percentage > MediumFuelThreshold)
	{
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
	}
	// Red: Fuel < 30%
	else
	{
		return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red
	}
}

void UWarRigHUDWidget::OnFuelChanged(const FOnAttributeChangeData& Data)
{
	const float NewFuel = Data.NewValue;

	UE_LOG(LogWarRigHUDWidget, Verbose, TEXT("OnFuelChanged: %.2f -> %.2f"), Data.OldValue, NewFuel);

	// Update display with new fuel value (keep current max fuel)
	UpdateFuelDisplay(NewFuel, CurrentMaxFuel);
}

void UWarRigHUDWidget::OnMaxFuelChanged(const FOnAttributeChangeData& Data)
{
	const float NewMaxFuel = Data.NewValue;

	UE_LOG(LogWarRigHUDWidget, Verbose, TEXT("OnMaxFuelChanged: %.2f -> %.2f"), Data.OldValue, NewMaxFuel);

	// Update display with new max fuel value (keep current fuel)
	UpdateFuelDisplay(CurrentFuel, NewMaxFuel);
}

void UWarRigHUDWidget::ToggleVisibility()
{
	const ESlateVisibility CurrentVisibility = GetVisibility();
	const ESlateVisibility NewVisibility = (CurrentVisibility == ESlateVisibility::Visible) ?
		ESlateVisibility::Hidden : ESlateVisibility::Visible;

	SetVisibility(NewVisibility);

	UE_LOG(LogWarRigHUDWidget, Log, TEXT("ToggleVisibility: %s -> %s"),
		(CurrentVisibility == ESlateVisibility::Visible) ? TEXT("Visible") : TEXT("Hidden"),
		(NewVisibility == ESlateVisibility::Visible) ? TEXT("Visible") : TEXT("Hidden"));
}

void UWarRigHUDWidget::DebugCycleColors()
{
	if (!FuelProgressBar)
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("DebugCycleColors: FuelProgressBar is null"));
		return;
	}

	// Cycle through: Green (0) -> Yellow (1) -> Red (2) -> Green (0)
	FLinearColor NewColor;
	FString ColorName;
	float TestPercentage;

	switch (DebugColorIndex)
	{
		case 0: // Green
			NewColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
			ColorName = TEXT("Green");
			TestPercentage = 0.8f;
			break;
		case 1: // Yellow
			NewColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
			ColorName = TEXT("Yellow");
			TestPercentage = 0.45f;
			break;
		case 2: // Red
			NewColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
			ColorName = TEXT("Red");
			TestPercentage = 0.15f;
			break;
		default:
			DebugColorIndex = 0;
			NewColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
			ColorName = TEXT("Green");
			TestPercentage = 0.8f;
			break;
	}

	// Update progress bar
	FuelProgressBar->SetPercent(TestPercentage);
	FuelProgressBar->SetFillColorAndOpacity(NewColor);

	// Update text
	if (FuelTextBlock)
	{
		const float TestFuel = TestPercentage * CurrentMaxFuel;
		const FString FuelText = FString::Printf(TEXT("Fuel: %.0f / %.0f (DEBUG: %s)"),
			TestFuel, CurrentMaxFuel, *ColorName);
		FuelTextBlock->SetText(FText::FromString(FuelText));
	}

	UE_LOG(LogWarRigHUDWidget, Log, TEXT("DebugCycleColors: Set to %s (%.1f%%)"),
		*ColorName, TestPercentage * 100.0f);

	// Increment for next cycle
	DebugColorIndex = (DebugColorIndex + 1) % 3;
}

void UWarRigHUDWidget::DebugLogGeometry()
{
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("========================================"));
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("Widget Geometry Debug Info"));
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("========================================"));

	// Widget self
	const FGeometry& MyGeometry = GetCachedGeometry();
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("Widget Size: %.1f x %.1f"),
		MyGeometry.GetLocalSize().X, MyGeometry.GetLocalSize().Y);
	UE_LOG(LogWarRigHUDWidget, Log, TEXT("Widget Absolute Position: %.1f, %.1f"),
		MyGeometry.GetAbsolutePosition().X, MyGeometry.GetAbsolutePosition().Y);

	// Root Canvas
	if (RootCanvas)
	{
		const FGeometry& CanvasGeometry = RootCanvas->GetCachedGeometry();
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("Canvas Size: %.1f x %.1f"),
			CanvasGeometry.GetLocalSize().X, CanvasGeometry.GetLocalSize().Y);
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("Canvas Visibility: %s"),
			RootCanvas->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("RootCanvas is null"));
	}

	// Text Block
	if (FuelTextBlock)
	{
		const FGeometry& TextGeometry = FuelTextBlock->GetCachedGeometry();
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("TextBlock Size: %.1f x %.1f"),
			TextGeometry.GetLocalSize().X, TextGeometry.GetLocalSize().Y);
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("TextBlock Visibility: %s"),
			FuelTextBlock->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("TextBlock Text: %s"),
			*FuelTextBlock->GetText().ToString());
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("FuelTextBlock is null"));
	}

	// Progress Bar
	if (FuelProgressBar)
	{
		const FGeometry& BarGeometry = FuelProgressBar->GetCachedGeometry();
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("ProgressBar Size: %.1f x %.1f"),
			BarGeometry.GetLocalSize().X, BarGeometry.GetLocalSize().Y);
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("ProgressBar Visibility: %s"),
			FuelProgressBar->GetVisibility() == ESlateVisibility::Visible ? TEXT("Visible") : TEXT("Not Visible"));
		UE_LOG(LogWarRigHUDWidget, Log, TEXT("ProgressBar Percent: %.1f%%"),
			FuelProgressBar->GetPercent() * 100.0f);
	}
	else
	{
		UE_LOG(LogWarRigHUDWidget, Warning, TEXT("FuelProgressBar is null"));
	}

	UE_LOG(LogWarRigHUDWidget, Log, TEXT("========================================"));
}
