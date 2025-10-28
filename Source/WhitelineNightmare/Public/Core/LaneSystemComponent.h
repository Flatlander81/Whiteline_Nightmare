// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LaneSystemComponent.generated.h"

// Forward declarations
class UDataTable;
struct FLaneSystemData;

/**
 * Lane transition state
 */
UENUM(BlueprintType)
enum class ELaneTransitionState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Transitioning UMETA(DisplayName = "Transitioning")
};

/**
 * ULaneSystemComponent - Manages lane positions and transitions for the war rig
 *
 * The war rig is stationary in forward/backward movement but can move laterally
 * between discrete lanes (Y-axis only). This component manages:
 * - 5 fixed lane positions (configurable via data table)
 * - Lane positions are Y-axis offsets from center (0)
 * - Default lane positions: -400, -200, 0, 200, 400 (units)
 * - Current lane index (0-4, starts at 2 = center)
 * - Smooth lane transitions using interpolation
 * - Lane change validation (boundaries, already transitioning)
 *
 * Lane indexing: 0 = leftmost, 2 = center, 4 = rightmost
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API ULaneSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULaneSystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === LANE CHANGE FUNCTIONS ===

	/**
	 * Attempt to change lane by direction
	 * @param Direction -1 for left, +1 for right
	 * @return true if lane change initiated successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool ChangeLane(int32 Direction);

	/** Attempt to move one lane to the left */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	void ChangeLaneLeft();

	/** Attempt to move one lane to the right */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	void ChangeLaneRight();

	// === LANE QUERY FUNCTIONS ===

	/** Check if can change lane to the left */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool CanChangeLaneLeft() const;

	/** Check if can change lane to the right */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool CanChangeLaneRight() const;

	/** Get the current lane index (0-4, 2 = center) */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	int32 GetCurrentLane() const { return CurrentLaneIndex; }

	/** Get the Y offset for a specific lane index */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	float GetLaneYPosition(int32 LaneIndex) const;

	/** Check if currently transitioning between lanes */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool IsTransitioning() const { return TransitionState == ELaneTransitionState::Transitioning; }

	// === TESTING FUNCTIONS ===

	/** Test that lane changes are blocked at boundaries */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestLaneSystemBounds();

	/** Test that lane transitions use correct speed */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestLaneTransitionSpeed();

	/** Test that invalid lane changes are rejected */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestLaneChangeValidation();

	/** Test that lane index updates correctly after transitions */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestCurrentLaneTracking();

	/** Test that only Y axis changes during lane transitions */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestStationaryInOtherAxes();

	/** Run all lane system tests in sequence with comprehensive summary */
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestLaneSystemAll();

	// === DEBUG FUNCTIONS ===

	/** Toggle debug visualization of lanes */
	UFUNCTION(Exec, Category = "Debug|Lane System")
	void DebugShowLanes();

protected:
	// === INTERNAL FUNCTIONS ===

	/** Initialize lane configuration from data table */
	void InitializeLaneConfiguration();

	/** Validate lane index is within bounds */
	bool IsValidLaneIndex(int32 LaneIndex) const;

	/** Update war rig Y position during lane transition */
	void UpdateLaneTransition(float DeltaTime);

	/** Draw debug visualization for lanes */
	void DrawDebugLanes() const;

protected:
	// === LANE CONFIGURATION ===

	/** Reference to lane system data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Configuration")
	TObjectPtr<UDataTable> LaneSystemDataTable;

	/** Number of lanes (default 5) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Configuration")
	int32 NumLanes;

	/** Distance between lanes (default 200) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Configuration")
	float LaneSpacing;

	/** Center lane index (default 2 for 5 lanes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Configuration")
	int32 CenterLaneIndex;

	/** Precalculated Y positions for each lane */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System|Configuration")
	TArray<float> LaneYPositions;

	/** Speed of lane change movement (units per second) - loaded from war rig data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Configuration")
	float LaneChangeSpeed;

	// === LANE STATE ===

	/** Current lane index (0-4, starts at 2 = center) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System|State")
	int32 CurrentLaneIndex;

	/** Target lane index (for smooth transitions) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System|State")
	int32 TargetLaneIndex;

	/** Current lane transition state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System|State")
	ELaneTransitionState TransitionState;

	/** Current Y position during transition */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System|State")
	float CurrentYPosition;

	// === DEBUG ===

	/** Show lane debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Debug")
	bool bShowLaneDebug;
};
