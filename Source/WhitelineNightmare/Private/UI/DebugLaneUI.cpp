// Copyright Flatlander81. All Rights Reserved.

#include "UI/DebugLaneUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/WarRigPawn.h"
#include "Core/LaneSystemComponent.h"
#include "Kismet/GameplayStatics.h"

UDebugLaneUI::UDebugLaneUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, LeftButton(nullptr)
	, RightButton(nullptr)
	, LaneInfoText(nullptr)
	, WarRig(nullptr)
	, LaneSystem(nullptr)
{
}

void UDebugLaneUI::NativeConstruct()
{
	Super::NativeConstruct();

	// If widgets aren't bound from Blueprint, create them programmatically
	if (!LeftButton || !RightButton || !LaneInfoText)
	{
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Widgets not bound from Blueprint, creating programmatically"));

		// Get or create root canvas panel
		UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(GetRootWidget());
		if (!RootCanvas)
		{
			RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
			WidgetTree->RootWidget = RootCanvas;
		}

		// Create horizontal box for button layout
		UHorizontalBox* ButtonContainer = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("ButtonContainer"));

		// Create lane info text
		if (!LaneInfoText)
		{
			LaneInfoText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LaneInfoText"));
			LaneInfoText->SetText(FText::FromString(TEXT("Lane: --")));

			FSlateFontInfo FontInfo = LaneInfoText->GetFont();
			FontInfo.Size = 24;
			LaneInfoText->SetFont(FontInfo);
			LaneInfoText->SetColorAndOpacity(FLinearColor::White);

			UHorizontalBoxSlot* InfoSlot = ButtonContainer->AddChildToHorizontalBox(LaneInfoText);
			InfoSlot->SetPadding(FMargin(10.0f, 5.0f));
			InfoSlot->SetHorizontalAlignment(HAlign_Center);
			InfoSlot->SetVerticalAlignment(VAlign_Center);
		}

		// Create left button
		if (!LeftButton)
		{
			LeftButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LeftButton"));

			// Create text for button
			UTextBlock* LeftButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("LeftButtonText"));
			LeftButtonText->SetText(FText::FromString(TEXT("<< Lane Left")));

			FSlateFontInfo FontInfo = LeftButtonText->GetFont();
			FontInfo.Size = 20;
			LeftButtonText->SetFont(FontInfo);
			LeftButtonText->SetJustification(ETextJustify::Center);

			LeftButton->AddChild(LeftButtonText);

			UHorizontalBoxSlot* LeftSlot = ButtonContainer->AddChildToHorizontalBox(LeftButton);
			LeftSlot->SetPadding(FMargin(10.0f, 5.0f));
			LeftSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}

		// Create right button
		if (!RightButton)
		{
			RightButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RightButton"));

			// Create text for button
			UTextBlock* RightButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RightButtonText"));
			RightButtonText->SetText(FText::FromString(TEXT("Lane Right >>")));

			FSlateFontInfo FontInfo = RightButtonText->GetFont();
			FontInfo.Size = 20;
			RightButtonText->SetFont(FontInfo);
			RightButtonText->SetJustification(ETextJustify::Center);

			RightButton->AddChild(RightButtonText);

			UHorizontalBoxSlot* RightSlot = ButtonContainer->AddChildToHorizontalBox(RightButton);
			RightSlot->SetPadding(FMargin(10.0f, 5.0f));
			RightSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}

		// Add horizontal box to canvas panel at bottom center
		UCanvasPanelSlot* CanvasSlot = RootCanvas->AddChildToCanvas(ButtonContainer);
		if (CanvasSlot)
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f)); // Bottom center
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f)); // Align from center-bottom
			CanvasSlot->SetPosition(FVector2D(0.0f, -50.0f)); // 50 pixels from bottom
			CanvasSlot->SetAutoSize(true);
		}

		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Programmatic widgets created"));
	}

	// Bind button click events
	if (LeftButton)
	{
		LeftButton->OnClicked.AddDynamic(this, &UDebugLaneUI::OnLaneLeftClicked);
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Left button bound"));
	}

	if (RightButton)
	{
		RightButton->OnClicked.AddDynamic(this, &UDebugLaneUI::OnLaneRightClicked);
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Right button bound"));
	}

	// Find the war rig pawn
	WarRig = Cast<AWarRigPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	if (WarRig)
	{
		LaneSystem = WarRig->FindComponentByClass<ULaneSystemComponent>();
		if (LaneSystem)
		{
			UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Found LaneSystemComponent"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DebugLaneUI: War Rig found but no LaneSystemComponent!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugLaneUI: War Rig pawn not found. UI will not function."));
	}

	UpdateLaneDisplay();
}

void UDebugLaneUI::NativeDestruct()
{
	// Unbind button events
	if (LeftButton)
	{
		LeftButton->OnClicked.RemoveDynamic(this, &UDebugLaneUI::OnLaneLeftClicked);
	}

	if (RightButton)
	{
		RightButton->OnClicked.RemoveDynamic(this, &UDebugLaneUI::OnLaneRightClicked);
	}

	Super::NativeDestruct();
}

void UDebugLaneUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update lane display every tick to show real-time changes
	UpdateLaneDisplay();
}

void UDebugLaneUI::UpdateLaneDisplay()
{
	if (!LaneSystem)
	{
		if (LaneInfoText)
		{
			LaneInfoText->SetText(FText::FromString(TEXT("Lane: --")));
		}
		return;
	}

	const int32 CurrentLane = LaneSystem->GetCurrentLane();

	// Update lane info text
	if (LaneInfoText)
	{
		const FString LaneText = FString::Printf(TEXT("Lane: %d"), CurrentLane);
		LaneInfoText->SetText(FText::FromString(LaneText));
	}

	// Update button enabled state
	if (LeftButton)
	{
		LeftButton->SetIsEnabled(LaneSystem->CanChangeLaneLeft());
	}

	if (RightButton)
	{
		RightButton->SetIsEnabled(LaneSystem->CanChangeLaneRight());
	}
}

void UDebugLaneUI::OnLaneLeftClicked()
{
	UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Lane Left button clicked"));

	if (!LaneSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("DebugLaneUI: Cannot change lane - LaneSystem is null"));
		return;
	}

	if (LaneSystem->CanChangeLaneLeft())
	{
		LaneSystem->ChangeLaneLeft();
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Changed to left lane"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugLaneUI: Cannot change to left lane (already at leftmost)"));
	}
}

void UDebugLaneUI::OnLaneRightClicked()
{
	UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Lane Right button clicked"));

	if (!LaneSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("DebugLaneUI: Cannot change lane - LaneSystem is null"));
		return;
	}

	if (LaneSystem->CanChangeLaneRight())
	{
		LaneSystem->ChangeLaneRight();
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Changed to right lane"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugLaneUI: Cannot change to right lane (already at rightmost)"));
	}
}
