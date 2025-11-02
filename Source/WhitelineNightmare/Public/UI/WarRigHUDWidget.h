// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "WarRigHUDWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UCanvasPanel;
class UAbilitySystemComponent;
class AWarRigPawn;

/**
 * War Rig HUD Widget - Pure C++ widget for displaying fuel status
 *
 * Features:
 * - Programmatically created UI (no Blueprint required)
 * - Progress bar with color coding (Green > 60%, Yellow 30-60%, Red < 30%)
 * - Numeric fuel display (e.g., "Fuel: 75 / 100")
 * - Real-time updates via GAS attribute change delegates
 * - Positioned at top of screen
 *
 * Usage:
 * - Created by AWarRigHUD in BeginPlay
 * - Automatically binds to War Rig's Fuel and MaxFuel attributes
 * - Updates automatically when attributes change
 */
UCLASS()
class WHITELINENIGHTMARE_API UWarRigHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWarRigHUDWidget(const FObjectInitializer& ObjectInitializer);

	/**
	 * Initialize widget with reference to War Rig's Ability System Component
	 * Must be called after widget creation to bind to GAS attributes
	 * @param InAbilitySystemComponent - The war rig's ability system component
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|UI")
	void InitializeWidget(UAbilitySystemComponent* InAbilitySystemComponent);

	/**
	 * Update fuel display with new values
	 * Called automatically via GAS attribute change delegates
	 * @param NewFuel - Current fuel amount
	 * @param NewMaxFuel - Maximum fuel capacity
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|UI")
	void UpdateFuelDisplay(float NewFuel, float NewMaxFuel);

	/**
	 * Toggle widget visibility (for debug commands)
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|UI")
	void ToggleVisibility();

	/**
	 * Manually cycle through color states (for testing)
	 * Cycles: Green -> Yellow -> Red -> Green
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|UI|Debug")
	void DebugCycleColors();

	/**
	 * Get current fuel binding status
	 * @return True if successfully bound to GAS attributes
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|UI|Debug")
	bool IsBindingSuccessful() const { return bBindingSuccessful; }

protected:
	// UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Create UI elements programmatically
	 */
	void CreateWidgetLayout();

	/**
	 * Update progress bar color based on fuel percentage
	 * @param Percentage - Fuel percentage (0-1)
	 */
	void UpdateProgressBarColor(float Percentage);

	/**
	 * Get color for given fuel percentage
	 * @param Percentage - Fuel percentage (0-1)
	 * @return Color (Green > 60%, Yellow 30-60%, Red < 30%)
	 */
	FLinearColor GetColorForPercentage(float Percentage) const;

	/**
	 * Callback for when Fuel attribute changes
	 */
	void OnFuelChanged(const FOnAttributeChangeData& Data);

	/**
	 * Callback for when MaxFuel attribute changes
	 */
	void OnMaxFuelChanged(const FOnAttributeChangeData& Data);

protected:
	// UI Elements (created programmatically)
	UPROPERTY()
	UCanvasPanel* RootCanvas;

	UPROPERTY()
	UProgressBar* FuelProgressBar;

	UPROPERTY()
	UTextBlock* FuelTextBlock;

	// Reference to Ability System Component
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	// Cached attribute values
	UPROPERTY()
	float CurrentFuel;

	UPROPERTY()
	float CurrentMaxFuel;

	// Binding state
	UPROPERTY()
	bool bBindingSuccessful;

	// Delegate handles for cleanup
	FDelegateHandle FuelChangedHandle;
	FDelegateHandle MaxFuelChangedHandle;

	// Debug color cycling state
	int32 DebugColorIndex;

	// UI Layout constants
	static constexpr float ProgressBarWidth = 300.0f;
	static constexpr float ProgressBarHeight = 30.0f;
	static constexpr float TextTopMargin = 10.0f;
	static constexpr float ProgressBarTopMargin = 40.0f;
	static constexpr float LeftMargin = 50.0f;

	// Color thresholds
	static constexpr float HighFuelThreshold = 0.6f;  // Green above this
	static constexpr float MediumFuelThreshold = 0.3f; // Yellow above this, Red below
};
