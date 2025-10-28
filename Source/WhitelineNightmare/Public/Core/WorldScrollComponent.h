// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldScrollComponent.generated.h"

// Forward declarations
class UDataTable;
struct FWorldScrollData;

/**
 * UWorldScrollComponent - Manages world scrolling to simulate forward movement
 *
 * CRITICAL: The war rig is STATIONARY. The world scrolls BACKWARD past it.
 *
 * This component manages the world scrolling system that simulates forward movement.
 * It is attached to the GameMode or a persistent level manager actor (NOT the war rig).
 * All scrolling actors (ground tiles, enemies, obstacles, pickups) query this component
 * to get the scroll velocity and move themselves backward.
 *
 * Key Features:
 * - Single source of truth for scroll speed throughout the level
 * - Tracks total virtual distance traveled (used for win condition)
 * - Configurable via data table
 * - Can be paused/resumed at runtime
 * - Can change scroll speed at runtime
 *
 * Usage by Other Systems:
 * - Ground tiles: Query GetScrollVelocity() each tick to move backward
 * - Enemies: Add scroll velocity to their own movement
 * - Obstacles: Move backward at scroll speed
 * - Pickups: Scroll backward toward player
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UWorldScrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWorldScrollComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === SCROLL VELOCITY QUERY FUNCTIONS ===

	/**
	 * Get the current scroll velocity vector
	 * Other systems use this to move objects backward
	 * @return Current scroll velocity (direction * speed)
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	FVector GetScrollVelocity() const;

	/**
	 * Get the current scroll speed magnitude
	 * @return Current scroll speed in units per second
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetScrollSpeed() const { return ScrollSpeed; }

	/**
	 * Get the total virtual distance traveled
	 * Used for win condition tracking
	 * @return Accumulated distance in units
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/**
	 * Check if scrolling is currently active
	 * @return True if world is scrolling
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	bool IsScrolling() const { return bIsScrolling; }

	// === SCROLL CONTROL FUNCTIONS ===

	/**
	 * Change the scroll speed at runtime
	 * @param NewSpeed - New scroll speed in units per second (will be clamped to >= 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollSpeed(float NewSpeed);

	/**
	 * Start or stop scrolling
	 * @param bEnabled - True to start scrolling, false to pause
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrolling(bool bEnabled);

	/**
	 * Reset the distance counter
	 * Used for level restarts or testing
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void ResetDistance();

	// === DEBUG COMMANDS ===

	/**
	 * Debug command to change scroll speed
	 * Usage: DebugSetScrollSpeed 500
	 */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugSetScrollSpeed(float NewSpeed);

	/**
	 * Debug command to toggle scrolling on/off
	 * Usage: DebugToggleScroll
	 */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugToggleScroll();

	/**
	 * Debug command to display current scroll info in log
	 * Usage: DebugShowScrollInfo
	 */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugShowScrollInfo();

	/**
	 * Debug command to reset distance counter
	 * Usage: DebugResetDistance
	 */
	UFUNCTION(Exec, Category = "Debug|World Scroll")
	void DebugResetDistance();

	// === TESTING FUNCTIONS ===

	/**
	 * Test that scroll speed remains constant over time
	 * @return True if test passed
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestScrollSpeedConsistency();

	/**
	 * Test that distance accumulates correctly
	 * @return True if test passed
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestDistanceAccumulation();

	/**
	 * Test that scrolling can be paused and resumed
	 * @return True if test passed
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestScrollPause();

	/**
	 * Test that velocity calculation is correct
	 * @return True if test passed
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestScrollVelocity();

	/**
	 * Test that runtime speed changes work correctly
	 * @return True if test passed
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	bool TestScrollSpeedChange();

	/**
	 * Run all world scroll tests in sequence
	 */
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestWorldScrollAll();

protected:
	// === INTERNAL FUNCTIONS ===

	/**
	 * Load configuration from data table
	 */
	void LoadScrollConfiguration();

	/**
	 * Validate scroll speed (clamp to >= 0)
	 * @param Speed - Speed to validate
	 * @return Validated speed
	 */
	float ValidateScrollSpeed(float Speed) const;

	/**
	 * Log scroll state change for debugging
	 */
	void LogScrollStateChange(const FString& Message) const;

protected:
	// === CONFIGURATION ===

	/**
	 * Reference to world scroll data table
	 * Should point to DT_WorldScrollData
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Configuration")
	TObjectPtr<UDataTable> WorldScrollDataTable;

	/**
	 * Row name to use in the data table
	 * Default: "DefaultScroll"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Configuration")
	FName DataTableRowName;

	// === SCROLL STATE ===

	/**
	 * Current scroll speed in units per second
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	float ScrollSpeed;

	/**
	 * Whether scrolling is currently active
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	bool bIsScrolling;

	/**
	 * Total virtual distance traveled
	 * Used for win condition
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	float DistanceTraveled;

	/**
	 * Scroll velocity vector (calculated from direction and speed)
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	FVector ScrollVelocity;

	/**
	 * Normalized scroll direction vector
	 * Default: (-1, 0, 0) for backward along X axis
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	FVector ScrollDirection;
};
