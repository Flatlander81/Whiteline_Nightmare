// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LaneSystemComponent.generated.h"

/**
 * Lane System Component - Manages lane-based movement for the war rig
 *
 * The war rig is stationary but can move between 5 fixed lanes.
 * This component handles:
 * - Tracking current lane position
 * - Smooth lane transitions (lerp-based)
 * - Lane bounds validation
 * - Debug visualization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API ULaneSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULaneSystemComponent();

	/**
	 * Initialize the lane system with configuration
	 * @param InLaneWidth - Distance between lanes in world units
	 * @param InNumLanes - Number of lanes (default 5)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	void Initialize(float InLaneWidth, int32 InNumLanes = 5);

	/**
	 * Request a lane change (left or right)
	 * @param Direction - Negative for left, positive for right
	 * @return True if lane change is valid and initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool RequestLaneChange(int32 Direction);

	/**
	 * Set the lane change speed
	 * @param Speed - Units per second for lane transitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	void SetLaneChangeSpeed(float Speed);

	/**
	 * Get the current lane index (0-4 for 5 lanes)
	 * @return Current lane index
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System")
	int32 GetCurrentLane() const { return CurrentLaneIndex; }

	/**
	 * Get the target lane index (where we're moving to)
	 * @return Target lane index
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System")
	int32 GetTargetLane() const { return TargetLaneIndex; }

	/**
	 * Check if currently transitioning between lanes
	 * @return True if in transition
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System")
	bool IsChangingLanes() const { return bIsChangingLanes; }

	/**
	 * Get the current Y position (lateral position)
	 * @return Current Y offset from center
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System")
	float GetCurrentYPosition() const { return CurrentYPosition; }

	/**
	 * Get the Y position for a specific lane index
	 * @param LaneIndex - Lane index (0 = center)
	 * @return Y position offset for that lane
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System")
	float GetLaneYPosition(int32 LaneIndex) const;

	/**
	 * Enable or disable debug visualization
	 * @param bEnabled - Whether to show debug lines
	 */
	UFUNCTION(BlueprintCallable, Category = "Lane System|Debug")
	void SetDebugVisualization(bool bEnabled) { bShowDebugVisualization = bEnabled; }

	/**
	 * Check if debug visualization is enabled
	 * @return True if debug visualization is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Lane System|Debug")
	bool IsDebugVisualizationEnabled() const { return bShowDebugVisualization; }

#if !UE_BUILD_SHIPPING
	/**
	 * Simulate a tick for testing purposes (only available in non-shipping builds)
	 * @param DeltaTime - Time to simulate
	 */
	void SimulateTick(float DeltaTime) { TickComponent(DeltaTime, ELevelTick::LEVELTICK_All, nullptr); }
#endif

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/**
	 * Update lane transition interpolation
	 * @param DeltaTime - Time since last frame
	 */
	void UpdateLaneTransition(float DeltaTime);

	/**
	 * Validate that a lane index is within bounds
	 * @param LaneIndex - Lane to validate
	 * @return True if lane is valid
	 */
	bool IsValidLane(int32 LaneIndex) const;

	/**
	 * Draw debug visualization for lanes
	 */
	void DrawDebugVisualization();

	// Number of lanes (typically 5)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	int32 NumLanes;

	// Distance between lanes in world units
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	float LaneWidth;

	// Current lane index (0 = center lane for 5 lanes)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	int32 CurrentLaneIndex;

	// Target lane index (where we're moving to)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	int32 TargetLaneIndex;

	// Current Y position (interpolated during transitions)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	float CurrentYPosition;

	// Speed of lane transitions (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	float LaneChangeSpeed;

	// Whether currently transitioning between lanes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	bool bIsChangingLanes;

	// Whether the system has been initialized
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System", meta = (AllowPrivateAccess = "true"))
	bool bIsInitialized;

	// Whether to show debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugVisualization;

	// Length of debug lane lines to draw
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System|Debug", meta = (AllowPrivateAccess = "true"))
	float DebugLineLength;
};
