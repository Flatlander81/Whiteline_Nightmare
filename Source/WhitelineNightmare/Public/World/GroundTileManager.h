// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GroundTileManager.generated.h"

/**
 * Ground Tile Manager - Manages spawning and scrolling of ground tiles
 *
 * This component creates an infinite scrolling road by managing a pool of tiles.
 * Tiles scroll backward at the world scroll velocity, and are recycled when they
 * pass behind the war rig.
 *
 * Usage:
 * 1. Attach to GameMode or level manager actor
 * 2. Configure via data table (FWorldTileData)
 * 3. Automatically spawns, scrolls, and recycles tiles
 *
 * Key Responsibilities:
 * - Initialize tile object pool
 * - Spawn tiles ahead of war rig
 * - Detect when tiles pass behind war rig
 * - Recycle tiles to front of road
 * - Provide debug visualization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UGroundTileManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UGroundTileManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Get the total number of active tiles
	 * @return Number of tiles currently spawned
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Manager")
	int32 GetActiveTileCount() const { return ActiveTiles.Num(); }

	/**
	 * Get position of the furthest forward tile
	 * @return X position of leading tile
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Manager")
	float GetFurthestTilePosition() const;

	/**
	 * Manually trigger tile recycling check
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile Manager")
	void CheckForTileRecycling();

	// === DEBUG FUNCTIONS ===

	/** Toggle debug visualization of tiles */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTiles();

	/** Log tile manager state */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTileInfo();

	/** Set spawn distance at runtime */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugSetSpawnDistance(float NewDistance);

	/** Set despawn distance at runtime */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugSetDespawnDistance(float NewDistance);

protected:
	// Data table reference for tile configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Manager|Config")
	UDataTable* TileDataTable;

	// Row name in the data table (default: "DefaultTile")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Manager|Config")
	FName DataTableRowName;

	// Class of ground tile to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Manager|Config")
	TSubclassOf<class AGroundTile> TileClass;

	// Reference to war rig (for position calculations)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|State")
	AActor* WarRig;

	// Object pool component for tiles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Components")
	class UObjectPoolComponent* TilePool;

	// Active tiles currently on the road
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|State")
	TArray<class AGroundTile*> ActiveTiles;

	// Tile size (loaded from data table)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	float TileSize;

	// Number of tiles to spawn
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	int32 TilePoolSize;

	// Distance ahead of war rig to spawn tiles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	float TileSpawnDistance;

	// Distance behind war rig to despawn tiles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	float TileDespawnDistance;

	// Optional tile mesh override from data table
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	TSoftObjectPtr<UStaticMesh> ConfiguredTileMesh;

	// Optional tile material override from data table
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Manager|Config")
	TSoftObjectPtr<UMaterialInterface> ConfiguredTileMaterial;

	// Debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Manager|Debug")
	bool bShowDebugVisualization;

private:
	/**
	 * Load configuration from data table
	 * @return True if loaded successfully
	 */
	bool LoadConfigFromDataTable();

	/**
	 * Initialize tile object pool
	 * @return True if initialized successfully
	 */
	bool InitializeTilePool();

	/**
	 * Spawn initial tiles to fill the road
	 */
	void SpawnInitialTiles();

	/**
	 * Spawn a new tile at the specified position
	 * @param Position - World position for the tile
	 * @return Spawned tile or nullptr if pool exhausted
	 */
	class AGroundTile* SpawnTile(const FVector& Position);

	/**
	 * Recycle a tile that has passed behind the war rig
	 * @param Tile - Tile to recycle
	 */
	void RecycleTile(class AGroundTile* Tile);

	/**
	 * Get war rig reference
	 * @return War rig actor or nullptr if not found
	 */
	AActor* GetWarRig();

	/**
	 * Get world scroll component
	 * @return WorldScrollComponent or nullptr if not found
	 */
	class UWorldScrollComponent* GetWorldScrollComponent() const;

	/**
	 * Draw debug visualization
	 */
	void DrawDebugVisualization();

	/**
	 * Log manager state
	 */
	void LogManagerState() const;
};
