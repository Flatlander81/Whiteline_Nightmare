// Copyright Flatlander81. All Rights Reserved.

#include "UI/GameOverWidget.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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

	// Create widget layout programmatically (EXACT WarRigHUDWidget pattern!)
	CreateWidgetLayout();

	// Update the stats display
	UpdateStatsDisplay();

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Widget constructed successfully"));
}

void UGameOverWidget::CreateWidgetLayout()
{
	// Get or create root canvas panel (SAME AS WarRigHUDWidget)
	RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created root canvas"));
	}

	// CRITICAL: Use SAME visibility as WarRigHUDWidget
	if (RootCanvas)
	{
		RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Configured root canvas visibility"));
	}

	// Create semi-transparent background overlay (fullscreen)
	BackgroundOverlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BackgroundOverlay"));
	if (BackgroundOverlay)
	{
		BackgroundOverlay->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f));
		BackgroundOverlay->SetVisibility(ESlateVisibility::Visible);

		UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundOverlay);
		if (BackgroundSlot)
		{
			BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			BackgroundSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created background overlay"));
	}

	// Create "GAME OVER" text - DIRECTLY on Canvas (SAME AS WarRigHUDWidget)
	GameOverText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("GameOverText"));
	if (GameOverText)
	{
		GameOverText->SetText(FText::FromString(TEXT("GAME OVER")));
		FSlateFontInfo FontInfo = GameOverText->GetFont();
		FontInfo.Size = 72;
		GameOverText->SetFont(FontInfo);
		GameOverText->SetColorAndOpacity(FLinearColor(1.0f, 0.2f, 0.0f, 1.0f));
		GameOverText->SetJustification(ETextJustify::Center);
		GameOverText->SetVisibility(ESlateVisibility::Visible);

		UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(GameOverText);
		if (TextSlot)
		{
			TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			TextSlot->SetPosition(FVector2D(0.0f, -200.0f)); // Above center
			TextSlot->SetAutoSize(true);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created GAME OVER text"));
	}

	// Create reason text - DIRECTLY on Canvas
	ReasonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReasonText"));
	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(GameOverReason));
		FSlateFontInfo FontInfo = ReasonText->GetFont();
		FontInfo.Size = 36;
		ReasonText->SetFont(FontInfo);
		ReasonText->SetColorAndOpacity(FLinearColor::White);
		ReasonText->SetJustification(ETextJustify::Center);
		ReasonText->SetVisibility(ESlateVisibility::Visible);

		UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(ReasonText);
		if (TextSlot)
		{
			TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			TextSlot->SetPosition(FVector2D(0.0f, -100.0f)); // Below "GAME OVER"
			TextSlot->SetAutoSize(true);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created reason text"));
	}

	// Create stats text - DIRECTLY on Canvas
	StatsText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatsText"));
	if (StatsText)
	{
		StatsText->SetText(FText::FromString(TEXT("Stats loading...")));
		FSlateFontInfo FontInfo = StatsText->GetFont();
		FontInfo.Size = 24;
		StatsText->SetFont(FontInfo);
		StatsText->SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f));
		StatsText->SetJustification(ETextJustify::Center);
		StatsText->SetVisibility(ESlateVisibility::Visible);

		UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(StatsText);
		if (TextSlot)
		{
			TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			TextSlot->SetPosition(FVector2D(0.0f, 0.0f)); // Center
			TextSlot->SetAutoSize(true);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created stats text"));
	}

	// Create restart instruction - DIRECTLY on Canvas
	RestartInstructionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartInstructionText"));
	if (RestartInstructionText)
	{
		RestartInstructionText->SetText(FText::FromString(TEXT("Press R to Restart")));
		FSlateFontInfo FontInfo = RestartInstructionText->GetFont();
		FontInfo.Size = 20;
		RestartInstructionText->SetFont(FontInfo);
		RestartInstructionText->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
		RestartInstructionText->SetJustification(ETextJustify::Center);
		RestartInstructionText->SetVisibility(ESlateVisibility::Visible);

		UCanvasPanelSlot* TextSlot = RootCanvas->AddChildToCanvas(RestartInstructionText);
		if (TextSlot)
		{
			TextSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			TextSlot->SetPosition(FVector2D(0.0f, 150.0f)); // Below stats
			TextSlot->SetAutoSize(true);
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created restart instruction text"));
	}

	// Create restart button - DIRECTLY on Canvas
	RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartButton"));
	if (RestartButton)
	{
		RestartButton->SetVisibility(ESlateVisibility::Visible);

		// Create button text
		RestartButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartButtonText"));
		if (RestartButtonText)
		{
			RestartButtonText->SetText(FText::FromString(TEXT("Restart")));
			FSlateFontInfo FontInfo = RestartButtonText->GetFont();
			FontInfo.Size = 24;
			RestartButtonText->SetFont(FontInfo);
			RestartButtonText->SetColorAndOpacity(FLinearColor::Black);
			RestartButtonText->SetJustification(ETextJustify::Center);
			RestartButtonText->SetVisibility(ESlateVisibility::Visible);

			RestartButton->AddChild(RestartButtonText);
		}

		// Bind button click
		RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartButtonClicked);

		UCanvasPanelSlot* ButtonSlot = RootCanvas->AddChildToCanvas(RestartButton);
		if (ButtonSlot)
		{
			ButtonSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			ButtonSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			ButtonSlot->SetPosition(FVector2D(0.0f, 200.0f)); // Below instruction
			ButtonSlot->SetSize(FVector2D(200.0f, 50.0f));
		}

		UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: Created restart button"));
	}

	UE_LOG(LogGameOverWidget, Log, TEXT("GameOverWidget: UI layout created successfully (WarRigHUDWidget pattern)"));
}

void UGameOverWidget::SetGameOverReason(const FString& Reason)
{
	GameOverReason = Reason;

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

	UpdateStatsDisplay();
}

void UGameOverWidget::UpdateStatsDisplay()
{
	if (!StatsText)
	{
		return;
	}

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

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogGameOverWidget, Error, TEXT("GameOverWidget: No player controller found"));
		return;
	}

	AWarRigPlayerController* WarRigPC = Cast<AWarRigPlayerController>(PC);
	if (WarRigPC)
	{
		WarRigPC->RestartGame();
	}
	else
	{
		UWorld* World = GetWorld();
		if (World)
		{
			UGameplayStatics::OpenLevel(World, FName(*World->GetName()));
		}
	}
}
