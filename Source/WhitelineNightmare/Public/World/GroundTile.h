// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ObjectPoolTypes.h"
#include "GroundTile.generated.h"

/**
 * Ground Tile - Represents a scrolling road segment
 *
 * This actor is pooled and recycled to create an infinite scrolling road.
 * It moves backward at the world scroll velocity to simulate forward movement.
 * Implements IPoolableActor interface for object pooling integration.
 *
 * Usage:
 * 1. Created by GroundTilePoolComponent using object pool
 * 2. Positioned ahead of the war rig
 * 3. Scrolls backward each tick at scroll velocity
 * 4. Recycled when it passes behind the war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API AGroundTile : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	AGroundTile();

	virtual void Tick(float DeltaTime) override;

	// IPoolableActor interface implementation
	/**
	 * Called when tile is activated from pool
	 * Sets initial position, makes visible, begins scrolling
	 */
	virtual void OnActivated_Implementation() override;

	/**
	 * Called when tile is returned to pool
	 * Stops scrolling, makes invisible, hides tile
	 */
	virtual void OnDeactivated_Implementation() override;

	/**
	 * Reset tile state (for pool reset)
	 * Clears any runtime modifications
	 */
	virtual void ResetState_Implementation() override;

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

	// Debug: Show tile bounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile|Debug")
	bool bShowDebugBounds;

protected:
	virtual void BeginPlay() override;

	// Mesh component for the tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile|Components")
	UStaticMeshComponent* TileMesh;

	// Length of the tile along the X axis (for spawning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile|Config")
	float TileLength;

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
