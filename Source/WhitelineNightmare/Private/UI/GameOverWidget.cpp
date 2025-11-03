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
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize default values
	GameOverReason = TEXT("Game Over");
	DistanceTraveled = 0.0f;
	EnemiesKilled = 0;
	FuelCollected = 0.0f;
	ScrapCollected = 0;

	// Fetch stats from GameMode
	FetchStatsFromGameMode();

	// Create the UI
	CreateUI();

	// Update the stats display
	UpdateStatsDisplay();

	UE_LOG(LogTemp, Log, TEXT("UGameOverWidget::NativeConstruct - Game over widget constructed"));
}

void UGameOverWidget::SetGameOverReason(const FString& Reason)
{
	GameOverReason = Reason;

	// Update the reason text if it exists
	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(Reason));
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

void UGameOverWidget::CreateUI()
{
	// Create the root canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
	if (!RootCanvas)
	{
		// If there's no root widget, create one
		RootCanvas = NewObject<UCanvasPanel>(this, UCanvasPanel::StaticClass());
		if (RootCanvas)
		{
			// This won't work at runtime, but we'll create our own structure
			UE_LOG(LogTemp, Warning, TEXT("UGameOverWidget::CreateUI - No root widget found, creating canvas"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UGameOverWidget::CreateUI - Failed to create canvas panel"));
			return;
		}
	}

	// Create a semi-transparent background overlay
	UBorder* BackgroundBorder = NewObject<UBorder>(this, UBorder::StaticClass(), TEXT("BackgroundBorder"));
	if (BackgroundBorder)
	{
		// Set background color (semi-transparent black)
		FLinearColor BackgroundColor(0.0f, 0.0f, 0.0f, 0.8f);
		BackgroundBorder->SetBrushColor(BackgroundColor);

		// Add to canvas
		UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(BackgroundBorder);
		if (BackgroundSlot)
		{
			BackgroundSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			BackgroundSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 0.0f));
		}
	}

	// Create a vertical box to hold all UI elements
	UVerticalBox* MainVerticalBox = NewObject<UVerticalBox>(this, UVerticalBox::StaticClass(), TEXT("MainVerticalBox"));
	if (!MainVerticalBox)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameOverWidget::CreateUI - Failed to create main vertical box"));
		return;
	}

	// Add vertical box to canvas (centered)
	UCanvasPanelSlot* MainBoxSlot = RootCanvas->AddChildToCanvas(MainVerticalBox);
	if (MainBoxSlot)
	{
		MainBoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		MainBoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		MainBoxSlot->SetAutoSize(true);
	}

	// Create "GAME OVER" text
	GameOverText = NewObject<UTextBlock>(this, UTextBlock::StaticClass(), TEXT("GameOverText"));
	if (GameOverText)
	{
		GameOverText->SetText(FText::FromString(TEXT("GAME OVER")));

		// Set font size and color
		FSlateFontInfo FontInfo = GameOverText->GetFont();
		FontInfo.Size = 72;
		GameOverText->SetFont(FontInfo);
		GameOverText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.0f, 1.0f))); // Red/orange
		GameOverText->SetJustification(ETextJustify::Center);

		// Add to vertical box
		UVerticalBoxSlot* GameOverSlot = MainVerticalBox->AddChildToVerticalBox(GameOverText);
		if (GameOverSlot)
		{
			GameOverSlot->SetPadding(FMargin(0.0f, 20.0f, 0.0f, 40.0f));
			GameOverSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// Create reason text
	ReasonText = NewObject<UTextBlock>(this, UTextBlock::StaticClass(), TEXT("ReasonText"));
	if (ReasonText)
	{
		ReasonText->SetText(FText::FromString(GameOverReason));

		// Set font size and color
		FSlateFontInfo FontInfo = ReasonText->GetFont();
		FontInfo.Size = 36;
		ReasonText->SetFont(FontInfo);
		ReasonText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		ReasonText->SetJustification(ETextJustify::Center);

		// Add to vertical box
		UVerticalBoxSlot* ReasonSlot = MainVerticalBox->AddChildToVerticalBox(ReasonText);
		if (ReasonSlot)
		{
			ReasonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 40.0f));
			ReasonSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// Create stats text
	StatsText = NewObject<UTextBlock>(this, UTextBlock::StaticClass(), TEXT("StatsText"));
	if (StatsText)
	{
		StatsText->SetText(FText::FromString(TEXT("Stats will be displayed here")));

		// Set font size and color
		FSlateFontInfo FontInfo = StatsText->GetFont();
		FontInfo.Size = 24;
		StatsText->SetFont(FontInfo);
		StatsText->SetColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f, 1.0f)));
		StatsText->SetJustification(ETextJustify::Center);

		// Add to vertical box
		UVerticalBoxSlot* StatsSlot = MainVerticalBox->AddChildToVerticalBox(StatsText);
		if (StatsSlot)
		{
			StatsSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 40.0f));
			StatsSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// Create "Press R to Restart" instruction
	RestartInstructionText = NewObject<UTextBlock>(this, UTextBlock::StaticClass(), TEXT("RestartInstructionText"));
	if (RestartInstructionText)
	{
		RestartInstructionText->SetText(FText::FromString(TEXT("Press R to Restart")));

		// Set font size and color
		FSlateFontInfo FontInfo = RestartInstructionText->GetFont();
		FontInfo.Size = 20;
		RestartInstructionText->SetFont(FontInfo);
		RestartInstructionText->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)));
		RestartInstructionText->SetJustification(ETextJustify::Center);

		// Add to vertical box
		UVerticalBoxSlot* InstructionSlot = MainVerticalBox->AddChildToVerticalBox(RestartInstructionText);
		if (InstructionSlot)
		{
			InstructionSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 20.0f));
			InstructionSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	// Create restart button
	RestartButton = NewObject<UButton>(this, UButton::StaticClass(), TEXT("RestartButton"));
	if (RestartButton)
	{
		// Create button text
		RestartButtonText = NewObject<UTextBlock>(this, UTextBlock::StaticClass(), TEXT("RestartButtonText"));
		if (RestartButtonText)
		{
			RestartButtonText->SetText(FText::FromString(TEXT("Restart")));

			// Set font size and color
			FSlateFontInfo FontInfo = RestartButtonText->GetFont();
			FontInfo.Size = 24;
			RestartButtonText->SetFont(FontInfo);
			RestartButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
			RestartButtonText->SetJustification(ETextJustify::Center);

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
	}

	UE_LOG(LogTemp, Log, TEXT("UGameOverWidget::CreateUI - UI created successfully"));
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
}

void UGameOverWidget::FetchStatsFromGameMode()
{
	// Get the game mode
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameOverWidget::FetchStatsFromGameMode - No world found"));
		return;
	}

	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameOverWidget::FetchStatsFromGameMode - GameMode not found"));
		return;
	}

	// Get stats from GameMode
	DistanceTraveled = GameMode->GetDistanceTraveled();
	EnemiesKilled = GameMode->GetEnemiesKilled();
	FuelCollected = GameMode->GetFuelCollected();
	ScrapCollected = GameMode->GetScrapCollected();

	UE_LOG(LogTemp, Log, TEXT("UGameOverWidget::FetchStatsFromGameMode - Fetched stats: Distance=%.0f, Enemies=%d, Fuel=%.0f, Scrap=%d"),
		DistanceTraveled, EnemiesKilled, FuelCollected, ScrapCollected);
}

void UGameOverWidget::OnRestartButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("UGameOverWidget::OnRestartButtonClicked - Restart button clicked"));

	// Get the player controller
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameOverWidget::OnRestartButtonClicked - No player controller found"));
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
