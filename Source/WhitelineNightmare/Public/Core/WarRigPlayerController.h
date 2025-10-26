// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WarRigPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * Player Controller for the War Rig
 * Handles player input for lane changes, turret control, and UI interaction
 *
 * Key responsibilities:
 * - Processing lane change input
 * - Managing turret targeting and firing
 * - Handling pause/menu input
 * - Providing input to the HUD for cursor-based turret placement
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWarRigPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Input Mapping Context for Enhanced Input */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Input Action for lane changes (left/right) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LaneChangeAction;

	/** Input Action for pausing the game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	/** Current lane index (0 to NumberOfLanes-1) */
	UPROPERTY(BlueprintReadOnly, Category = "War Rig")
	int32 CurrentLane;

	/** Total number of lanes */
	UPROPERTY(BlueprintReadOnly, Category = "War Rig")
	int32 TotalLanes;

	/** Is a lane change currently in progress? */
	UPROPERTY(BlueprintReadOnly, Category = "War Rig")
	bool bIsChangingLanes;

	/** Input handler for lane changes */
	void HandleLaneChange(const FInputActionValue& Value);

	/** Input handler for pause */
	void HandlePause(const FInputActionValue& Value);

public:
	/** Request a lane change by direction (-1 for left, +1 for right) */
	UFUNCTION(BlueprintCallable, Category = "War Rig")
	void RequestLaneChange(int32 Direction);

	/** Get current lane index */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	int32 GetCurrentLane() const { return CurrentLane; }

	/** Get total number of lanes */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	int32 GetTotalLanes() const { return TotalLanes; }

	/** Is the rig currently changing lanes? */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	bool IsChangingLanes() const { return bIsChangingLanes; }

	/** Set lane change state (called by abilities or pawns) */
	UFUNCTION(BlueprintCallable, Category = "War Rig")
	void SetIsChangingLanes(bool bChanging) { bIsChangingLanes = bChanging; }

	/** Initialize lane data from game mode */
	UFUNCTION(BlueprintCallable, Category = "War Rig")
	void InitializeLaneData(int32 NumLanes, int32 StartingLane = 1);
};
