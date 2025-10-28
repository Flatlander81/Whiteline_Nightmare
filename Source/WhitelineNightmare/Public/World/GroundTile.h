// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundTile.generated.h"

/**
 * Ground Tile - Represents a scrolling road segment
 *
 * This actor is pooled and recycled to create an infinite scrolling road.
 * It moves backward at the world scroll velocity to simulate forward movement.
 *
 * Usage:
 * 1. Created by GroundTileManager using object pool
 * 2. Positioned ahead of the war rig
 * 3. Scrolls backward each tick at scroll velocity
 * 4. Recycled when it passes behind the war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API AGroundTile : public AActor
{
	GENERATED_BODY()

public:
	AGroundTile();

	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when tile is activated from pool
	 * Sets initial position and makes visible
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void OnActivated();

	/**
	 * Called when tile is returned to pool
	 * Hides tile and resets state
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void OnDeactivated();

	/**
	 * Reset tile state (for pool reset)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void ResetState();

	/**
	 * Get tile length (for spawning calculations)
	 * @return Length of tile along X axis
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile")
	float GetTileLength() const { return TileLength; }

	/**
	 * Set tile length (configurable per tile type)
	 * @param Length - New tile length
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void SetTileLength(float Length) { TileLength = Length; }

protected:
	virtual void BeginPlay() override;

	// Mesh component for the tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile|Components")
	UStaticMeshComponent* TileMesh;

	// Length of the tile along the X axis (for spawning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile|Config")
	float TileLength;

	// Debug: Show tile bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile|Debug")
	bool bShowDebugBounds;

private:
	/**
	 * Update tile position based on scroll velocity
	 * @param DeltaTime - Time since last frame
	 */
	void UpdateScrollPosition(float DeltaTime);

	/**
	 * Get world scroll component from game mode
	 * @return WorldScrollComponent or nullptr if not found
	 */
	class UWorldScrollComponent* GetWorldScrollComponent() const;

	/**
	 * Draw debug visualization
	 */
	void DrawDebugInfo();
};
