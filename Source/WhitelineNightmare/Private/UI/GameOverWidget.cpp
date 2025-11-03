// Copyright Flatlander81. All Rights Reserved.

#include "UI/GameOverWidget.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogGameOverWidget, Log, All);

UGameOverWidget::UGameOverWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, GameOverReason(TEXT("Game Over"))
	, DistanceTraveled(0.0f)
	, EnemiesKilled(0)
	, FuelCollected(0.0f)
	, ScrapCollected(0)
	, RootCanvas(nullptr)
	, BackgroundOverlay(nullptr)
	, MainVerticalBox(nullptr)
	, GameOverText(nullptr)
	, ReasonText(nullptr)
	, StatsText(nullptr)
	, RestartInstructionText(nullptr)
	, RestartButton(nullptr)
	, RestartButtonText(nullptr)
{
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: NativeConstruct called"));

	// Fetch stats from GameMode before creating UI
	FetchStatsFromGameMode();

	// Create widget layout programmatically (same as WarRigHUDWidget!)
	CreateWidgetLayout();

	// Update the stats display
	UpdateStatsDisplay();

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Widget constructed successfully"));
}

void UGameOverWidget::CreateWidgetLayout()
{
	// Get or create root canvas panel
	RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created root canvas"));
	}

	// CRITICAL: Root canvas must be Visible to render children
	if (RootCanvas)
	{
		RootCanvas->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Configured root canvas visibility"));
	}

	// Create semi-transparent background overlay (this will block input)
	BackgroundOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundOverlay"));
	if (BackgroundOverlay)
	{
		// Set background color (semi-transparent black)
		BackgroundOverlay->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
		// Visible means it blocks input AND renders
		BackgroundOverlay->SetVisibility(ESlateVisibility::Visible);

		// Add to canvas (fullscreen)
		UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundOverlay);
		if (BackgroundSlot)
		{
			BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			BackgroundSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created background overlay"));
	}

	// Create vertical box to hold all UI elements
	MainVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MainVerticalBox"));
	if (MainVerticalBox)
	{
		MainVerticalBox->SetVisibility(ESlateVisibility::Visible);

		// Add to canvas (centered, with explicit size)
		UCanvasPanelSlot* VBoxSlot = RootCanvas->AddChildToCanvas(MainVerticalBox);
		if (VBoxSlot)
		{
			VBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			VBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			// Give it a reasonable size instead of auto-size
			VBoxSlot->SetSize(FVector2D(800.0f, 600.0f));
			VBoxSlot->SetPosition(FVector2D(0.0f, 0.0f));
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created main vertical box"));
	}

	// Create "GAME OVER" text
	GameOverText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameOverText"));
	if (GameOverText)
	{
		GameOverText->SetText(FText::FromString(TEXT("GAME OVER")));

		// Set font size and color
		FSlateFontInfo FontInfo = GameOverText->GetFont();
		FontInfo.Size = 72;
		GameOverText->SetFont(FontInfo);
		GameOverText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.0f, 1.0f))); // Red/orange
		GameOverText->SetJustification(ETextJustify::Center);
		GameOverText->SetVisibility(ESlateVisibility::Visible);

		// Add to vertical box
		UVerticalBoxSlot* GameOverSlot = MainVerticalBox->AddChildToVerticalBox(GameOverText);
		if (GameOverSlot)
		{
			GameOverSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 40.0f));
			GameOverSlot->SetHorizontalAlignment(HAlign_Center);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created GAME OVER text"));
	}

	// Create reason text
	ReasonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReasonText"));
	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(GameOverReason));

		// Set font size and color
		FSlateFontInfo FontInfo = ReasonText->GetFont();
		FontInfo.Size = 36;
		ReasonText->SetFont(FontInfo);
		ReasonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		ReasonText->SetJustification(ETextJustify::Center);
		ReasonText->SetVisibility(ESlateVisibility::Visible);

		// Add to vertical box
		UVerticalBoxSlot* ReasonSlot = MainVerticalBox->AddChildToVerticalBox(ReasonText);
		if (ReasonSlot)
		{
			ReasonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 40.0f));
			ReasonSlot->SetHorizontalAlignment(HAlign_Center);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created reason text"));
	}

	// Create stats text
	StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatsText"));
	if (StatsText)
	{
		StatsText->SetText(FText::FromString(TEXT("Stats loading...")));

		// Set font size and color
		FSlateFontInfo FontInfo = StatsText->GetFont();
		FontInfo.Size = 24;
		StatsText->SetFont(FontInfo);
		StatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
		StatsText->SetJustification(ETextJustify::Center);
		StatsText->SetVisibility(ESlateVisibility::Visible);

		// Add to vertical box
		UVerticalBoxSlot* StatsSlot = MainVerticalBox->AddChildToVerticalBox(StatsText);
		if (StatsSlot)
		{
			StatsSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 40.0f));
			StatsSlot->SetHorizontalAlignment(HAlign_Center);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created stats text"));
	}

	// Create "Press R to Restart" instruction
	RestartInstructionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartInstructionText"));
	if (RestartInstructionText)
	{
		RestartInstructionText->SetText(FText::FromString(TEXT("Press R to Restart")));

		// Set font size and color
		FSlateFontInfo FontInfo = RestartInstructionText->GetFont();
		FontInfo.Size = 20;
		RestartInstructionText->SetFont(FontInfo);
		RestartInstructionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
		RestartInstructionText->SetJustification(ETextJustify::Center);
		RestartInstructionText->SetVisibility(ESlateVisibility::Visible);

		// Add to vertical box
		UVerticalBoxSlot* InstructionSlot = MainVerticalBox->AddChildToVerticalBox(RestartInstructionText);
		if (InstructionSlot)
		{
			InstructionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
			InstructionSlot->SetHorizontalAlignment(HAlign_Center);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created restart instruction text"));
	}

	// Create restart button
	RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartButton"));
	if (RestartButton)
	{
		RestartButton->SetVisibility(ESlateVisibility::Visible);

		// Create button text
		RestartButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartButtonText"));
		if (RestartButtonText)
		{
			RestartButtonText->SetText(FText::FromString(TEXT("Restart")));

			// Set font size and color
			FSlateFontInfo FontInfo = RestartButtonText->GetFont();
			FontInfo.Size = 24;
			RestartButtonText->SetFont(FontInfo);
			RestartButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
			RestartButtonText->SetJustification(ETextJustify::Center);
			RestartButtonText->SetVisibility(ESlateVisibility::Visible);

			// Add text to button
			RestartButton->AddChild(RestartButtonText);
		}

		// Bind button click event
		RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartButtonClicked);

		// Add to vertical box
		UVerticalBoxSlot* ButtonSlot = MainVerticalBox->AddChildToVerticalBox(RestartButton);
		if (ButtonSlot)
		{
			ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
			ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created restart button"));
	}

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: UI layout created successfully"));
}

void UGameOverWidget::SetGameOverReason(const FString& Reason)
{
	GameOverReason = Reason;

	// Update the reason text if it exists
	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(Reason));
		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Set game over reason to '%s'"), *Reason);
	}
}

void UGameOverWidget::SetStats(float InDistanceTraveled, int32 InEnemiesKilled, float InFuelCollected, int32 InScrapCollected)
{
	DistanceTraveled = InDistanceTraveled;
	EnemiesKilled = InEnemiesKilled;
	FuelCollected = InFuelCollected;
	ScrapCollected = InScrapCollected;

	// Update the stats display
	UpdateStatsDisplay();
}

void UGameOverWidget::UpdateStatsDisplay()
{
	if (!StatsText)
	{
		return;
	}

	// Format stats as multi-line text
	FString StatsString = FString::Printf(TEXT(
		"Distance Traveled: %.0f units\n"
		"Enemies Killed: %d\n"
		"Fuel Collected: %.0f\n"
		"Scrap Collected: %d"
	), DistanceTraveled, EnemiesKilled, FuelCollected, ScrapCollected);

	StatsText->SetText(FText::FromString(StatsString));

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Updated stats display"));
}

void UGameOverWidget::FetchStatsFromGameMode()
{
	// Get the game mode
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogGameOverWidget, Warning, TEXT("GameOverWidget: No world found"));
		return;
	}

	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogGameOverWidget, Warning, TEXT("GameOverWidget: GameMode not found"));
		return;
	}

	// Get stats from GameMode
	DistanceTraveled = GameMode->GetDistanceTraveled();
	EnemiesKilled = GameMode->GetEnemiesKilled();
	FuelCollected = GameMode->GetFuelCollected();
	ScrapCollected = GameMode->GetScrapCollected();

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Fetched stats - Distance=%.0f, Enemies=%d, Fuel=%.0f, Scrap=%d"),
		DistanceTraveled, EnemiesKilled, FuelCollected, ScrapCollected);
}

void UGameOverWidget::OnRestartButtonClicked()
{
	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Restart button clicked"));

	// Get the player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogGameOverWidget, Error, TEXT("GameOverWidget: No player controller found"));
		return;
	}

	// Cast to WarRigPlayerController to call RestartGame
	AWarRigPlayerController* WarRigPC = Cast<AWarRigPlayerController>(PC);
	if (WarRigPC)
	{
		WarRigPC->RestartGame();
	}
	else
	{
		// Fallback: Reload level directly
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::OpenLevel(World, FName(*World->GetName()));
		}
	}
}
