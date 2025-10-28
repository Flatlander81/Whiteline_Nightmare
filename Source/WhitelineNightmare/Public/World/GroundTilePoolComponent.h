// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ObjectPoolComponent.h"
#include "Engine/DataTable.h"
#include "GroundTilePoolComponent.generated.h"

/**
 * Ground Tile Pool Component - Specialized pool component for ground tiles
 *
 * Extends UObjectPoolComponent with tile-specific functionality:
 * - Manages tile positioning for seamless scrolling
 * - Handles spawn/despawn based on war rig distance
 * - Configures tiles from FGroundTileData table
 * - Provides automatic tile recycling
 *
 * Usage:
 * 1. Attach to GameMode or level manager actor
 * 2. Configure DataTableRowName to point to FGroundTileData
 * 3. Automatically manages tiles based on war rig position
 * 4. Provides seamless infinite scrolling road
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UGroundTilePoolComponent : public UObjectPoolComponent
{
	GENERATED_BODY()

public:
	UGroundTilePoolComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Initialize the ground tile pool from data table
	 * @param TileClass - Class of ground tiles to spawn
	 * @param RowName - Row name in the data table
	 * @return True if initialization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile Pool")
	bool InitializeTilePool(TSubclassOf<class AGroundTile> TileClass, FName RowName);

	/**
	 * Get position of the furthest forward tile
	 * @return X position of leading tile
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Pool")
	float GetFurthestTilePosition() const;

	/**
	 * Get tile size from configuration
	 * @return Size of tiles in pool
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Pool")
	float GetTileSize() const { return TileSize; }

	/**
	 * Get number of active tiles
	 * @return Number of tiles currently visible
	 */
	UFUNCTION(BlueprintPure, Category = "Ground Tile Pool")
	int32 GetActiveTileCount() const { return ActiveTiles.Num(); }

	// === TESTING FUNCTIONS ===

	/**
	 * Test: Verify tiles are reused, not destroyed
	 * Category: ObjectPool, Movement
	 */
	UFUNCTION(Exec, Category = "Testing|Ground Tiles")
	void TestTilePoolRecycling();

	/**
	 * Test: Verify no gaps between tiles
	 * Category: ObjectPool, Movement
	 */
	UFUNCTION(Exec, Category = "Testing|Ground Tiles")
	void TestSeamlessScrolling();

	/**
	 * Test: Verify spawn positions correct
	 * Category: ObjectPool, Movement
	 */
	UFUNCTION(Exec, Category = "Testing|Ground Tiles")
	void TestTilePositioning();

	/**
	 * Test: Verify correct number of tiles created
	 * Category: ObjectPool
	 */
	UFUNCTION(Exec, Category = "Testing|Ground Tiles")
	void TestPoolSize();

	/**
	 * Test: Verify tiles returned at correct distance
	 * Category: ObjectPool, Movement
	 */
	UFUNCTION(Exec, Category = "Testing|Ground Tiles")
	void TestTileDespawn();

	// === DEBUG FUNCTIONS ===

	/**
	 * Debug: Visualize spawn/despawn zones
	 */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTileBounds();

	/**
	 * Debug: Display pool statistics
	 */
	UFUNCTION(Exec, Category = "Debug|Ground Tiles")
	void DebugShowTilePool();

protected:
	// Data table reference for tile configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool|Config")
	UDataTable* TileDataTable;

	// Row name in the data table (default: "DefaultGroundTile")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool|Config")
	FName DataTableRowName;

	// Class of ground tile to spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool|Config")
	TSubclassOf<class AGroundTile> GroundTileClass;

	// Reference to war rig (for position calculations)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|State")
	AActor* WarRig;

	// Active tiles currently on the road
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|State")
	TArray<class AGroundTile*> ActiveTiles;

	// Tile size (loaded from data table)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|Config")
	float TileSize;

	// Distance ahead of war rig to spawn tiles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|Config")
	float SpawnDistanceAhead;

	// Distance behind war rig to despawn tiles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|Config")
	float DespawnDistanceBehind;

	// Tile mesh (loaded from data table)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|Config")
	TSoftObjectPtr<UStaticMesh> TileMesh;

	// Tile material (loaded from data table)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ground Tile Pool|Config")
	TSoftObjectPtr<UMaterialInterface> TileMaterial;

	// Whether to show debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ground Tile Pool|Debug")
	bool bShowDebugBounds;

private:
	/**
	 * Load configuration from data table
	 * @return True if loaded successfully
	 */
	bool LoadConfigFromDataTable();

	/**
	 * Spawn initial tiles to fill the road
	 */
	void SpawnInitialTiles();

	/**
	 * Check for tiles that need recycling
	 */
	void CheckTileRecycling();

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
	 * Validate configuration
	 * @return True if configuration is valid
	 */
	bool ValidateConfiguration() const;
};
