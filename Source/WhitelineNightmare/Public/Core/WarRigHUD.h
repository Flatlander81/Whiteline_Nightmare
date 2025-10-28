// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WarRigHUD.generated.h"

/**
 * HUD for the War Rig
 * Manages UI display and user interface elements
 * All UI widgets should be created in C++ (UMG widgets created programmatically)
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWarRigHUD();

	// Called when the HUD is created
	virtual void BeginPlay() override;

	// Called every frame to draw the HUD
	virtual void DrawHUD() override;

	// Handle hit box clicks (for debug lane UI buttons)
	virtual void NotifyHitBoxClick(FName BoxName) override;

	/**
	 * Update fuel display
	 * @param CurrentFuel - Current fuel amount
	 * @param MaxFuel - Maximum fuel capacity
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void UpdateFuelDisplay(float CurrentFuel, float MaxFuel);

	/**
	 * Update armor/health display
	 * @param CurrentArmor - Current armor amount
	 * @param MaxArmor - Maximum armor capacity
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void UpdateArmorDisplay(float CurrentArmor, float MaxArmor);

	/**
	 * Update scrap display
	 * @param CurrentScrap - Current scrap amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void UpdateScrapDisplay(int32 CurrentScrap);

	/**
	 * Update distance display
	 * @param CurrentDistance - Current distance traveled
	 * @param TargetDistance - Target distance to win
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void UpdateDistanceDisplay(float CurrentDistance, float TargetDistance);

	/**
	 * Show game over screen
	 * @param bPlayerWon - True if player won
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void ShowGameOverScreen(bool bPlayerWon);

	/**
	 * Hide game over screen
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD")
	void HideGameOverScreen();

	/**
	 * Show debug lane UI (for testing lane system)
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD|Debug")
	void ShowDebugLaneUI();

	/**
	 * Hide debug lane UI
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD|Debug")
	void HideDebugLaneUI();

	/**
	 * Toggle debug lane UI visibility
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|HUD|Debug")
	void ToggleDebugLaneUI();

protected:
	// Current fuel percentage (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	float FuelPercentage;

	// Current armor percentage (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	float ArmorPercentage;

	// Current scrap amount
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	int32 ScrapAmount;

	// Current distance percentage (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	float DistancePercentage;

	// Whether game over screen is shown
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	bool bShowingGameOver;

	// Whether player won (only valid if bShowingGameOver is true)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|State")
	bool bPlayerWonGame;

	// Debug lane UI enabled
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|HUD|Debug")
	bool bShowDebugLaneUI;

	// Button hit boxes for lane controls
	FBox2D LeftLaneButtonBox;
	FBox2D RightLaneButtonBox;

private:
	/**
	 * Validate percentage values
	 * @param Value - Value to validate
	 * @return Clamped value between 0 and 1
	 */
	float ValidatePercentage(float Value) const;

	/**
	 * Draw debug HUD (temporary until UI widgets are implemented)
	 */
	void DrawDebugHUD();

	/**
	 * Draw debug lane UI buttons
	 */
	void DrawDebugLaneUI();
};
