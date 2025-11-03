// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarRigPlayerController.generated.h"

/**
 * Player Controller for the War Rig
 * Handles input, UI interaction, and player state management
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWarRigPlayerController();

	// Called when the controller possesses a pawn
	virtual void OnPossess(APawn* InPawn) override;

	// Called when the controller unpossesses a pawn
	virtual void OnUnPossess() override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

protected:
	// Handler for any key press during game over
	void OnAnyKeyPressed(FKey Key);

public:
	/**
	 * Add scrap to the player's inventory
	 * @param Amount - Amount of scrap to add (can be negative to subtract)
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Economy")
	bool AddScrap(int32 Amount);

	/**
	 * Get current scrap amount
	 * @return Current scrap count
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Economy")
	int32 GetScrap() const { return CurrentScrap; }

	/**
	 * Can the player afford a purchase?
	 * @param Cost - Cost in scrap
	 * @return True if player has enough scrap
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Economy")
	bool CanAfford(int32 Cost) const;

	/**
	 * Attempt to spend scrap
	 * @param Cost - Amount to spend
	 * @return True if successful (player had enough scrap)
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Economy")
	bool SpendScrap(int32 Cost);

	/**
	 * Handle game over state
	 * @param bPlayerWon - True if player won
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void OnGameOver(bool bPlayerWon);

	/**
	 * Restart the game (reload current level)
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void RestartGame();

	/**
	 * Check if game is currently over
	 * @return True if game over
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game State")
	bool IsGameOver() const { return bIsGameOver; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Current scrap resources
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Economy")
	int32 CurrentScrap;

	// Starting scrap amount (can be configured in data table)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Economy")
	int32 StartingScrap;

	// Game over flag
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	bool bIsGameOver;

private:
	/**
	 * Validate scrap amount changes
	 * @param NewAmount - Proposed new scrap amount
	 * @return True if valid
	 */
	bool ValidateScrapAmount(int32 NewAmount) const;

	/**
	 * Log player state for debugging
	 */
	void LogPlayerState() const;
};
