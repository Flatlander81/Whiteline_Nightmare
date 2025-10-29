// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WhitelineNightmareGameMode.generated.h"

/**
 * Main Game Mode for Whiteline Nightmare
 * Manages core gameplay loop, win/lose conditions, and game state
 */
UCLASS()
class WHITELINENIGHTMARE_API AWhitelineNightmareGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWhitelineNightmareGameMode();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Initialize the game mode
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/**
	 * Track distance traveled by the war rig
	 * @param DeltaDistance - Distance to add (should be positive)
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game Progress")
	void AddDistanceTraveled(float DeltaDistance);

	/**
	 * Get current distance traveled
	 * @return Current distance traveled in units
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game Progress")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/**
	 * Get target win distance
	 * @return Distance needed to win in units
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game Progress")
	float GetWinDistance() const { return WinDistance; }

	/**
	 * Check if player has won
	 * @return True if player has reached win distance
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game Progress")
	bool HasPlayerWon() const { return DistanceTraveled >= WinDistance; }

	/**
	 * Trigger game over (win or lose)
	 * @param bPlayerWon - True if player won, false if player lost
	 */
	UFUNCTION(BlueprintCallable, Category = "Whiteline Nightmare|Game State")
	void TriggerGameOver(bool bPlayerWon);

	/**
	 * Check if game is over
	 * @return True if game has ended
	 */
	UFUNCTION(BlueprintPure, Category = "Whiteline Nightmare|Game State")
	bool IsGameOver() const { return bIsGameOver; }

	// === DEBUG COMMANDS ===

	/** Debug command: Set scroll speed */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugSetScrollSpeed(float NewSpeed);

	/** Debug command: Toggle scrolling on/off */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugToggleScroll();

	/** Debug command: Show scroll info in log */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugShowScrollInfo();

	/** Debug command: Reset distance counter */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugResetDistance();

	/** Debug command: Show tile debug visualization */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTiles();

	/** Debug command: Show tile manager info */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTileInfo();

	/** Debug command: Show tile pool bounds */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTileBounds();

	/** Debug command: Show tile pool stats */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTilePoolStats();

	/** Debug command: Run tile pool tests */
	UFUNCTION(Exec, Category = "Debug|Testing")
	void RunTilePoolTests();

	/** Debug command: Run a specific test by name */
	UFUNCTION(Exec, Category = "Debug|Testing")
	void RunTest(const FString& TestName);

	/** Debug command: Run all tests in a category (Movement, Combat, Economy, etc.) */
	UFUNCTION(Exec, Category = "Debug|Testing")
	void RunTests(const FString& CategoryName);

	/** Debug command: Run all registered tests */
	UFUNCTION(Exec, Category = "Debug|Testing")
	void RunAllTests();

	// World scroll component (manages world scrolling)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Components")
	class UWorldScrollComponent* WorldScrollComponent;

	// Ground tile manager (manages scrolling road tiles - legacy)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Components")
	class UGroundTileManager* GroundTileManager;

	// Ground tile pool component (modern pooled tile system)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Components")
	class UGroundTilePoolComponent* GroundTilePoolComponent;

protected:
	// Current distance traveled
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Game Progress")
	float DistanceTraveled;

	// Distance needed to win (loaded from data table)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Whiteline Nightmare|Game Progress")
	float WinDistance;

	// Whether the game is over
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	bool bIsGameOver;

	// Whether the player won
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Whiteline Nightmare|Game State")
	bool bPlayerWon;

private:
	/**
	 * Internal validation for distance tracking
	 * @param DeltaDistance - Distance to validate
	 * @return True if valid
	 */
	bool ValidateDistanceAddition(float DeltaDistance) const;

	/**
	 * Log game state for debugging
	 */
	void LogGameState() const;
};
