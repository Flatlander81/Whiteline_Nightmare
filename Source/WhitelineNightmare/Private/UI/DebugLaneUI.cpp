// Copyright Flatlander81. All Rights Reserved.

#include "UI/DebugLaneUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
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

	// Bind button click events if buttons exist
	if (LeftButton)
	{
		LeftButton->OnClicked.AddDynamic(this, &UDebugLaneUI::OnLaneLeftClicked);
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Left button bound"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugLaneUI: LeftButton not found - make sure widget is named 'LeftButton' in Blueprint"));
	}

	if (RightButton)
	{
		RightButton->OnClicked.AddDynamic(this, &UDebugLaneUI::OnLaneRightClicked);
		UE_LOG(LogTemp, Log, TEXT("DebugLaneUI: Right button bound"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugLaneUI: RightButton not found - make sure widget is named 'RightButton' in Blueprint"));
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
