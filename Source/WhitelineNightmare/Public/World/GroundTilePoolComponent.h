// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ObjectPoolComponent.h"
#include "GroundTilePoolComponent.generated.h"

class AGroundTile;
class UWorldScrollComponent;

/**
 * Ground Tile Pool Component - Specialized pool for managing scrolling ground tiles
 *
 * Extends UObjectPoolComponent with tile-specific logic:
 * - Manages tile positioning for seamless scrolling
 * - Automatically recycles tiles that pass behind war rig
 * - Spawns new tiles ahead of war rig
 * - Configurable spawn/despawn distances
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UGroundTilePoolComponent : public UObjectPoolComponent
{
	GENERATED_BODY()

public:
	UGroundTilePoolComponent();

	/**
	 * Initialize the tile pool with configuration
	 * @param InTileClass - Class of ground tile to pool
	 * @param InTileSize - Size of each tile
	 * @param InPoolSize - Number of tiles to pre-allocate
	 * @param InSpawnDistance - Distance ahead to spawn tiles
	 * @param InDespawnDistance - Distance behind to despawn tiles
	 * @param InWorldScrollComponent - Reference to world scroll component
	 * @return True if initialization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile Pool")
	bool InitializeTilePool(
		TSubclassOf<AGroundTile> InTileClass,
		FVector2D InTileSize,
		int32 InPoolSize,
		float InSpawnDistance,
		float InDespawnDistance,
		UWorldScrollComponent* InWorldScrollComponent);

	/**
	 * Spawn initial tiles to fill the visible area
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile Pool")
	void SpawnInitialTiles();

	/**
	 * Get the tile size
	 * @return Size of tiles
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Pool")
	FVector2D GetTileSize() const { return TileSize; }

	/**
	 * Get the war rig location (reference point for spawning/despawning)
	 * @return War rig location
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Pool")
	FVector GetWarRigLocation() const { return WarRigLocation; }

	/**
	 * Set the war rig location (reference point)
	 * @param NewLocation - New war rig location
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile Pool")
	void SetWarRigLocation(FVector NewLocation) { WarRigLocation = NewLocation; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/**
	 * Update tiles: move them, recycle those behind, spawn new ones ahead
	 * @param DeltaTime - Time since last frame
	 */
	void UpdateTiles(float DeltaTime);

	/**
	 * Check if a tile should be recycled (passed behind despawn threshold)
	 * @param Tile - Tile to check
	 * @return True if tile should be recycled
	 */
	bool ShouldRecycleTile(AActor* Tile) const;

	/**
	 * Get the next spawn position for a tile
	 * @return Spawn position
	 */
	FVector GetNextSpawnPosition() const;

	/**
	 * Spawn a tile at a specific position
	 * @param SpawnPosition - Position to spawn the tile
	 */
	void SpawnTileAt(FVector SpawnPosition);

	// Size of each tile
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	FVector2D TileSize;

	// Distance ahead of war rig to spawn tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	float SpawnDistanceAhead;

	// Distance behind war rig to despawn tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	float DespawnDistanceBehind;

	// Reference to world scroll component
	UPROPERTY()
	UWorldScrollComponent* WorldScrollComponent;

	// War rig location (reference point for spawning/despawning)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	FVector WarRigLocation;

	// Position of the furthest tile ahead (for tracking spawn position)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	float FurthestTilePosition;

	// Whether the tile pool has been initialized
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool", meta = (AllowPrivateAccess = "true"))
	bool bIsTilePoolInitialized;
};
