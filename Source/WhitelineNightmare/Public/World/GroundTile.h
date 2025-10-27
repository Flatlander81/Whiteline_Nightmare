// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ObjectPoolTypes.h"
#include "GroundTile.generated.h"

/**
 * Ground Tile - Poolable actor representing a section of the scrolling road
 *
 * This actor implements IPoolableActor for efficient recycling.
 * Features:
 * - Simple plane mesh with configurable size
 * - Scrolls backward at world scroll speed
 * - No collision needed (purely visual)
 * - Automatically recycled when passing behind war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API AGroundTile : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	AGroundTile();

	/**
	 * Set the size of the ground tile
	 * @param NewSize - Size of the tile (typically 2000x2000)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void SetTileSize(FVector2D NewSize);

	/**
	 * Get the size of the ground tile
	 * @return Size of the tile
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile")
	FVector2D GetTileSize() const { return TileSize; }

	/**
	 * Update tile position based on scroll speed
	 * @param DeltaTime - Time since last frame
	 * @param ScrollVelocity - World scroll velocity
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	void UpdatePosition(float DeltaTime, FVector ScrollVelocity);

	// IPoolableActor interface
	virtual void OnActivated_Implementation() override;
	virtual void OnDeactivated_Implementation() override;
	virtual void ResetState_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/**
	 * Create the tile mesh
	 */
	void CreateTileMesh();

	// Static mesh component for the ground tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* TileMesh;

	// Size of the tile (X = length, Y = width)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile", meta = (AllowPrivateAccess = "true"))
	FVector2D TileSize;

	// Material for the tile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* TileMaterial;
};
