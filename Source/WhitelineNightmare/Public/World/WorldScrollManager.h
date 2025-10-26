// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldScrollManager.generated.h"

class UDataTable;
struct FWorldScrollData;
class AGroundTile;
class UObjectPoolComponent;

/**
 * World Scroll Manager
 * Manages the scrolling world system where the war rig is stationary and the world moves backward
 *
 * Key responsibilities:
 * - Scrolling ground tiles from ahead to behind the war rig
 * - Managing tile pooling and recycling
 * - Coordinating with spawners for enemies, obstacles, and pickups
 * - Distance tracking for win condition
 */
UCLASS()
class WHITELINENIGHTMARE_API AWorldScrollManager : public AActor
{
	GENERATED_BODY()

public:
	AWorldScrollManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Start the world scrolling */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void StartScrolling();

	/** Stop the world scrolling */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void StopScrolling();

	/** Pause the world scrolling */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void PauseScrolling();

	/** Resume the world scrolling */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void ResumeScrolling();

	/** Get current scroll speed */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetCurrentScrollSpeed() const { return CurrentScrollSpeed; }

	/** Set scroll speed multiplier */
	UFUNCTION(BlueprintCallable, Category = "World Scroll")
	void SetScrollSpeedMultiplier(float Multiplier);

	/** Get total distance traveled */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetDistanceTraveled() const { return DistanceTraveled; }

	/** Get spawn distance ahead of war rig */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetSpawnDistance() const;

	/** Get despawn distance behind war rig */
	UFUNCTION(BlueprintPure, Category = "World Scroll")
	float GetDespawnDistance() const;

protected:
	/** Reference to the war rig (stationary in world space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	TObjectPtr<AActor> WarRigReference;

	/** Tile class to spawn for ground */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "World Scroll|Tiles")
	TSubclassOf<AGroundTile> GroundTileClass;

	/** Base scroll speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	float BaseScrollSpeed;

	/** Current scroll speed (base * multiplier) */
	UPROPERTY(BlueprintReadOnly, Category = "World Scroll")
	float CurrentScrollSpeed;

	/** Scroll speed multiplier (can be modified by abilities/pickups) */
	UPROPERTY(BlueprintReadOnly, Category = "World Scroll")
	float ScrollSpeedMultiplier;

	/** Is scrolling currently active? */
	UPROPERTY(BlueprintReadOnly, Category = "World Scroll")
	bool bIsScrolling;

	/** Is scrolling paused? */
	UPROPERTY(BlueprintReadOnly, Category = "World Scroll")
	bool bIsPaused;

	/** Total distance traveled since start */
	UPROPERTY(BlueprintReadOnly, Category = "World Scroll")
	float DistanceTraveled;

	/** Size of each ground tile (should match tile mesh size) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Tiles")
	float TileSize;

	/** Number of tiles to pre-spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Tiles")
	int32 InitialTileCount;

	/** Distance ahead of war rig to spawn new objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Spawning")
	float SpawnDistance;

	/** Distance behind war rig to despawn objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Spawning")
	float DespawnDistance;

	/** Active ground tiles */
	UPROPERTY()
	TArray<TObjectPtr<AGroundTile>> ActiveTiles;

	/** Pool of inactive tiles for reuse */
	UPROPERTY()
	TArray<TObjectPtr<AGroundTile>> TilePool;

	/** Position where next tile should be placed */
	FVector NextTilePosition;

	/** Initialize the tile system */
	void InitializeTiles();

	/** Update tile positions based on scroll speed */
	void UpdateTiles(float DeltaTime);

	/** Get a tile from the pool or create a new one */
	AGroundTile* GetTileFromPool();

	/** Return a tile to the pool */
	void ReturnTileToPool(AGroundTile* Tile);

	/** Spawn a new tile ahead of the war rig */
	void SpawnTileAhead();

	/** Update distance traveled and notify game mode */
	void UpdateDistance(float DeltaDistance);

	/** Load settings from data table */
	void LoadScrollSettings();
};
