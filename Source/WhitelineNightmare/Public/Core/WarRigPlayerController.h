// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarRigPlayerController.generated.h"

class AWhitelineNightmareGameMode;

/**
 * Player controller for war rig
 * Handles player input and manages game flow
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWarRigPlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaSeconds) override;

	// Get the game mode (cached)
	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	AWhitelineNightmareGameMode* GetWhitelineGameMode() const;

	// Notify player of game over
	UFUNCTION(BlueprintCallable, Category = "PlayerController|GameState")
	void NotifyGameOver(bool bVictory);

	// Get current scrap amount
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerController|Economy")
	int32 GetScrap() const { return CurrentScrap; }

	// Add scrap
	UFUNCTION(BlueprintCallable, Category = "PlayerController|Economy")
	void AddScrap(int32 Amount);

	// Spend scrap (returns true if successful)
	UFUNCTION(BlueprintCallable, Category = "PlayerController|Economy")
	bool SpendScrap(int32 Amount);

protected:
	// Cached game mode reference
	UPROPERTY(BlueprintReadOnly, Category = "PlayerController")
	AWhitelineNightmareGameMode* CachedGameMode;

	// Player resources
	UPROPERTY(BlueprintReadOnly, Category = "PlayerController|Economy")
	int32 CurrentScrap;

	// Input handlers (to be implemented with Enhanced Input System)
	void OnLaneChangeLeft();
	void OnLaneChangeRight();
	void OnPause();

	// Initialize cached references
	virtual void InitializeReferences();

	// Validate player state
	virtual bool ValidatePlayerState() const;
};
