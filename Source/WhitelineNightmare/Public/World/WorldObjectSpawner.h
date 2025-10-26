// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldObjectSpawner.generated.h"

class UObjectPoolComponent;
class UDataTable;
class AWorldScrollManager;

/**
 * World Object Spawner
 * Manages spawning of enemies, obstacles, and pickups ahead of the war rig
 * Works in conjunction with WorldScrollManager and ObjectPoolComponent
 */
UCLASS()
class WHITELINENIGHTMARE_API AWorldObjectSpawner : public AActor
{
	GENERATED_BODY()

public:
	AWorldObjectSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** Initialize spawner with references */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void Initialize(AWorldScrollManager* ScrollManager, AActor* WarRig);

	/** Start spawning objects */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	/** Stop spawning objects */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

protected:
	/** Reference to world scroll manager */
	UPROPERTY(BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<AWorldScrollManager> WorldScrollManager;

	/** Reference to war rig */
	UPROPERTY(BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<AActor> WarRigReference;

	/** Object pool for enemies */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner|Pools")
	TObjectPtr<UObjectPoolComponent> EnemyPool;

	/** Object pool for obstacles */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner|Pools")
	TObjectPtr<UObjectPoolComponent> ObstaclePool;

	/** Object pool for pickups */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner|Pools")
	TObjectPtr<UObjectPoolComponent> PickupPool;

	/** Is spawning active? */
	UPROPERTY(BlueprintReadOnly, Category = "Spawner")
	bool bIsSpawning;

	/** Time between spawn attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float SpawnInterval;

	/** Time since last spawn */
	float TimeSinceLastSpawn;

	/** Number of lanes for spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 NumberOfLanes;

	/** Distance between lanes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float LaneSpacing;

	/** Enemy spawn chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Probabilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemySpawnChance;

	/** Obstacle spawn chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Probabilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ObstacleSpawnChance;

	/** Pickup spawn chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner|Probabilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PickupSpawnChance;

	/** Placeholder enemy class (TODO: load from data table) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Classes")
	TSubclassOf<AActor> DefaultEnemyClass;

	/** Placeholder obstacle class (TODO: load from data table) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Classes")
	TSubclassOf<AActor> DefaultObstacleClass;

	/** Placeholder pickup class (TODO: load from data table) */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner|Classes")
	TSubclassOf<AActor> DefaultPickupClass;

	/** Initialize object pools */
	void InitializePools();

	/** Attempt to spawn objects */
	void TrySpawnObjects();

	/** Spawn an enemy */
	void SpawnEnemy(int32 Lane);

	/** Spawn an obstacle */
	void SpawnObstacle(int32 Lane);

	/** Spawn a pickup */
	void SpawnPickup(int32 Lane);

	/** Get world position for a lane at spawn distance */
	FVector GetLaneSpawnPosition(int32 Lane) const;

	/** Load spawn settings from game mode */
	void LoadSpawnSettings();

	/** Update and cleanup spawned objects */
	void UpdateSpawnedObjects(float DeltaTime);

	/** Active spawned objects */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveObjects;
};
