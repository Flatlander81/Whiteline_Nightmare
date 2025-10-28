// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugLaneUI.generated.h"

class UButton;
class UTextBlock;

/**
 * Debug UI Widget for Lane System Testing
 * Simple UI with two buttons to change lanes left and right
 *
 * To use: Create a Blueprint widget based on this class (WBP_DebugLaneUI)
 * Add two Button widgets named "LeftButton" and "RightButton"
 * Add a TextBlock named "LaneInfoText" to display current lane
 */
UCLASS()
class WHITELINENIGHTMARE_API UDebugLaneUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UDebugLaneUI(const FObjectInitializer& ObjectInitializer);

protected:
	// UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * Update current lane display
	 */
	void UpdateLaneDisplay();

	/**
	 * Button click handlers
	 */
	UFUNCTION()
	void OnLaneLeftClicked();

	UFUNCTION()
	void OnLaneRightClicked();

protected:
	// UI Elements (Bound from Blueprint - optional, will be created if not bound)
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* LeftButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* RightButton;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* LaneInfoText;

	// Cached reference to war rig
	UPROPERTY()
	class AWarRigPawn* WarRig;

	// Cached reference to lane system component
	UPROPERTY()
	class ULaneSystemComponent* LaneSystem;
};
