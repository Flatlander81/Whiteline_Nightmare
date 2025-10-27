// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/GameDataStructs.h"
#include "WorldScrollComponent.generated.h"

/**
 * World Scroll Component - Manages world scrolling to simulate forward movement
 *
 * The war rig is stationary, but the world scrolls backward past it to create
 * the illusion of forward movement. This component:
 * - Tracks scroll speed
 * - Tracks total virtual distance traveled
 * - Provides scroll velocity to other systems
 * - Configurable via data table
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UWorldScrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWorldScrollComponent();

	/**
	 * Initialize the world scroll system with configuration from data table
	 * @param WorldScrollDataTable - Data table containing scroll configuration
	 * @param RowName - Row name to load (default: "Default")
	 * @return True if initialization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	bool Initialize(UDataTable* WorldScrollDataTable, FName RowName = "Default");

	/**
	 * Initialize with explicit scroll speed (for testing or simple setup)
	 * @param InScrollSpeed - Units per second to scroll
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void InitializeWithSpeed(float InScrollSpeed);

	/**
	 * Set the scroll speed
	 * @param NewSpeed - New scroll speed in units per second
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollSpeed(float NewSpeed);

	/**
	 * Get the current scroll speed
	 * @return Scroll speed in units per second
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetScrollSpeed() const { return ScrollSpeed; }

	/**
	 * Get the scroll velocity (directional)
	 * @return Velocity vector (typically negative X for backward scroll)
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	FVector GetScrollVelocity() const;

	/**
	 * Get the total distance traveled (virtual)
	 * @return Distance in world units
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/**
	 * Reset the distance traveled counter
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void ResetDistanceTraveled() { DistanceTraveled = 0.0f; }

	/**
	 * Enable or disable scrolling
	 * @param bEnabled - Whether scrolling is active
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollEnabled(bool bEnabled) { bIsScrollEnabled = bEnabled; }

	/**
	 * Check if scrolling is enabled
	 * @return True if scrolling is active
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	bool IsScrollEnabled() const { return bIsScrollEnabled; }

	/**
	 * Get the scroll direction (unit vector)
	 * @return Direction vector (typically -X for backward scroll)
	 */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	FVector GetScrollDirection() const { return ScrollDirection; }

	/**
	 * Set the scroll direction (for custom scroll scenarios)
	 * @param NewDirection - New scroll direction (will be normalized)
	 */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollDirection(FVector NewDirection);

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
	 * Update distance traveled
	 * @param DeltaTime - Time since last frame
	 */
	void UpdateDistanceTraveled(float DeltaTime);

	// Data table for gameplay balance (contains scroll speed) - PRIMARY SOURCE
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config", meta = (AllowPrivateAccess = "true"))
	UDataTable* GameplayBalanceDataTable;

	// Row name in gameplay balance data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config", meta = (AllowPrivateAccess = "true"))
	FName BalanceDataRowName;

	// Fallback scroll speed if data table is not set (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config", meta = (AllowPrivateAccess = "true"))
	float FallbackScrollSpeed;

	// Current scroll speed in units per second
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|Runtime", meta = (AllowPrivateAccess = "true"))
	float ScrollSpeed;

	// Total distance traveled (virtual)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|Runtime", meta = (AllowPrivateAccess = "true"))
	float DistanceTraveled;

	// Direction of scroll (normalized, typically -X)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config", meta = (AllowPrivateAccess = "true"))
	FVector ScrollDirection;

	// Whether scrolling is enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Config", meta = (AllowPrivateAccess = "true"))
	bool bIsScrollEnabled;

	// Whether the component has been initialized
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll|Runtime", meta = (AllowPrivateAccess = "true"))
	bool bIsInitialized;
};
