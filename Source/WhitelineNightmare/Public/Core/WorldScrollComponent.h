// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "WorldScrollComponent.generated.h"

struct FWorldScrollData;

/**
 * World Scroll Component - Manages world scrolling to simulate forward movement
 *
 * This component is the single source of truth for world scroll velocity.
 * Attach to GameMode or a persistent level manager actor (NOT the war rig).
 * The war rig remains STATIONARY while the world scrolls BACKWARD past it.
 *
 * Key Responsibilities:
 * - Provide scroll velocity to all systems (ground tiles, enemies, obstacles, pickups)
 * - Track total virtual distance traveled (for win condition)
 * - Manage scroll speed configuration via data table
 * - Support runtime speed changes and pause/resume
 *
 * Usage:
 * 1. Add component to GameMode or level manager actor
 * 2. Configure DataTableRowName to point to data table row
 * 3. Other systems query GetScrollVelocity() to move backward
 * 4. GameMode queries GetDistanceTraveled() for win condition
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UWorldScrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWorldScrollComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Get current scroll velocity vector
	 * Other systems use this to move objects backward
	 * @return Scroll velocity (direction * speed)
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	FVector GetScrollVelocity() const;

	/**
	 * Get current scroll speed magnitude
	 * @return Speed in units per second
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetScrollSpeed() const { return ScrollSpeed; }

	/**
	 * Get total virtual distance traveled
	 * Used by GameMode for win condition
	 * @return Accumulated distance in units
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/**
	 * Set scroll speed at runtime
	 * @param NewSpeed - New speed in units per second (clamped to >= 0)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollSpeed(float NewSpeed);

	/**
	 * Enable or disable scrolling
	 * @param bEnabled - True to start scrolling, false to pause
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrolling(bool bEnabled);

	/**
	 * Check if scrolling is currently active
	 * @return True if scrolling is active
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	bool IsScrolling() const { return bIsScrolling; }

	/**
	 * Reset distance counter to zero
	 * Used for game restarts
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void ResetDistance();

	/**
	 * Get scroll direction vector (normalized)
	 * @return Normalized scroll direction
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	FVector GetScrollDirection() const { return ScrollDirection; }

	/**
	 * Set scroll direction at runtime
	 * @param NewDirection - New direction (will be normalized)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollDirection(FVector NewDirection);

protected:
	// Data table reference for scroll configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config")
	UDataTable* ScrollDataTable;

	// Row name in the data table (default: "DefaultScroll")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config")
	FName DataTableRowName;

	// Current scroll speed in units per second
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	float ScrollSpeed;

	// Whether scrolling is currently active
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	bool bIsScrolling;

	// Total virtual distance traveled (accumulated)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	float DistanceTraveled;

	// Normalized scroll direction vector
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|State")
	FVector ScrollDirection;

private:
	/**
	 * Load configuration from data table
	 * @return True if loaded successfully
	 */
	bool LoadConfigFromDataTable();

	/**
	 * Validate scroll speed
	 * @param Speed - Speed to validate
	 * @return Clamped valid speed (>= 0)
	 */
	float ValidateScrollSpeed(float Speed) const;

	/**
	 * Validate and normalize direction vector
	 * @param Direction - Direction to validate
	 * @return Normalized direction (or default if invalid)
	 */
	FVector ValidateScrollDirection(FVector Direction) const;

	/**
	 * Log scroll state for debugging
	 */
	void LogScrollState() const;
};
