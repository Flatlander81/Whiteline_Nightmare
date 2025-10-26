// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameplayDataStructs.generated.h"

/**
 * Struct defining a turret mounting point on the war rig
 * Used to specify where turrets can be placed and what directions they can face
 */
USTRUCT(BlueprintType)
struct FMountPointData
{
	GENERATED_BODY()

	/** Transform relative to the war rig root */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FTransform Transform;

	/** Allowed facing directions (0-7 for 8 compass directions). Empty = all directions allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	TArray<int32> AllowedFacingDirections;

	/** Gameplay tags for special restrictions (e.g., heavy weapons only, special ammo types) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FGameplayTagContainer MountTags;

	FMountPointData()
		: Transform(FTransform::Identity)
	{
	}
};

/**
 * Data table row for gameplay balance values
 * Contains all tunable parameters for core gameplay mechanics
 */
USTRUCT(BlueprintType)
struct FGameplayBalanceData : public FTableRowBase
{
	GENERATED_BODY()

	/** Rate at which fuel drains per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float FuelDrainRate;

	/** Fuel cost for changing lanes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float LaneChangeFuelCost;

	/** Distance required to win the level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float WinDistance;

	/** Time in seconds before obstacle collision to show warning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float ObstacleWarningTime;

	/** Base speed at which the world scrolls backward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float BaseScrollSpeed;

	/** Number of lanes available for the war rig */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 NumberOfLanes;

	/** Starting fuel amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	float StartingFuel;

	/** Starting scrap amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance")
	int32 StartingScrap;

	FGameplayBalanceData()
		: FuelDrainRate(1.0f)
		, LaneChangeFuelCost(5.0f)
		, WinDistance(10000.0f)
		, ObstacleWarningTime(2.0f)
		, BaseScrollSpeed(1000.0f)
		, NumberOfLanes(3)
		, StartingFuel(100.0f)
		, StartingScrap(0)
	{
	}
};

/**
 * Data table row for turret definitions
 * Contains all stats and properties for each turret type
 */
USTRUCT(BlueprintType)
struct FTurretData : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name of the turret */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText TurretName;

	/** Description of the turret */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText Description;

	/** Damage per shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float Damage;

	/** Shots per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float FireRate;

	/** Maximum range in units */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float Range;

	/** Scrap cost to build */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Cost")
	int32 ScrapCost;

	/** Fuel cost to operate per shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Cost")
	float FuelCostPerShot;

	/** Visual mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Visual")
	TSoftObjectPtr<UStaticMesh> TurretMesh;

	/** Projectile class reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	TSoftClassPtr<AActor> ProjectileClass;

	/** Gameplay tags for this turret type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FGameplayTagContainer TurretTags;

	FTurretData()
		: Damage(10.0f)
		, FireRate(1.0f)
		, Range(1000.0f)
		, ScrapCost(10)
		, FuelCostPerShot(0.5f)
	{
	}
};

/**
 * Data table row for enemy definitions
 * Contains all stats and behavior parameters for enemy types
 */
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name of the enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FText EnemyName;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float MaxHealth;

	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Movement")
	float Speed;

	/** Damage dealt to war rig on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float CollisionDamage;

	/** Base spawn weight for random spawning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spawning")
	float SpawnWeight;

	/** Minimum distance from player to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spawning")
	float MinSpawnDistance;

	/** Scrap reward on kill */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
	int32 ScrapReward;

	/** Visual mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Visual")
	TSoftObjectPtr<USkeletalMesh> EnemyMesh;

	/** Enemy behavior tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FGameplayTagContainer EnemyTags;

	FEnemyData()
		: MaxHealth(100.0f)
		, Speed(500.0f)
		, CollisionDamage(20.0f)
		, SpawnWeight(1.0f)
		, MinSpawnDistance(2000.0f)
		, ScrapReward(5)
	{
	}
};

/**
 * Data table row for pickup definitions
 * Contains properties for fuel and scrap pickups
 */
USTRUCT(BlueprintType)
struct FPickupData : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name of the pickup */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText PickupName;

	/** Fuel amount restored (0 if scrap pickup) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Values")
	float FuelAmount;

	/** Scrap amount given (0 if fuel pickup) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Values")
	int32 ScrapAmount;

	/** Spawn weight for random generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Spawning")
	float SpawnWeight;

	/** Visual mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	TSoftObjectPtr<UStaticMesh> PickupMesh;

	FPickupData()
		: FuelAmount(0.0f)
		, ScrapAmount(0)
		, SpawnWeight(1.0f)
	{
	}
};

/**
 * Data table row for world scrolling parameters
 * Controls how the world moves and objects are pooled
 */
USTRUCT(BlueprintType)
struct FWorldScrollData : public FTableRowBase
{
	GENERATED_BODY()

	/** Base scroll speed (world units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	float ScrollSpeed;

	/** Size of each ground tile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Tiles")
	float TileSize;

	/** Number of tiles to keep in the pool */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Tiles")
	int32 TilePoolSize;

	/** Distance ahead of rig to spawn new objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Spawning")
	float SpawnDistance;

	/** Distance behind rig to despawn objects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Spawning")
	float DespawnDistance;

	/** Maximum number of obstacles pooled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Pools")
	int32 ObstaclePoolSize;

	/** Maximum number of enemies pooled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Pools")
	int32 EnemyPoolSize;

	/** Maximum number of pickups pooled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll|Pools")
	int32 PickupPoolSize;

	FWorldScrollData()
		: ScrollSpeed(1000.0f)
		, TileSize(2000.0f)
		, TilePoolSize(10)
		, SpawnDistance(5000.0f)
		, DespawnDistance(2000.0f)
		, ObstaclePoolSize(20)
		, EnemyPoolSize(30)
		, PickupPoolSize(15)
	{
	}
};

/**
 * Data table row for war rig configurations
 * Contains mesh references, mounting points, and stats for different rig types
 */
USTRUCT(BlueprintType)
struct FWarRigData : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name of the war rig */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig")
	FText RigName;

	/** Description */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig")
	FText Description;

	/** Visual mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Visual")
	TSoftObjectPtr<USkeletalMesh> RigMesh;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float MaxHealth;

	/** Maximum fuel capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float MaxFuel;

	/** Array of turret mounting points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Mounting")
	TArray<FMountPointData> MountPoints;

	/** Scrap cost to unlock this rig */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Unlock")
	int32 UnlockCost;

	/** Whether this rig is available from start */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Unlock")
	bool bStartingRig;

	FWarRigData()
		: MaxHealth(1000.0f)
		, MaxFuel(100.0f)
		, UnlockCost(0)
		, bStartingRig(true)
	{
	}
};

/**
 * Data table row for obstacle definitions
 * Contains properties for environmental hazards
 */
USTRUCT(BlueprintType)
struct FObstacleData : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name of the obstacle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle")
	FText ObstacleName;

	/** Damage dealt on collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Combat")
	float CollisionDamage;

	/** Can this obstacle be destroyed by turret fire? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Combat")
	bool bDestructible;

	/** Health if destructible */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Combat", meta = (EditCondition = "bDestructible"))
	float Health;

	/** Spawn weight for random generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Spawning")
	float SpawnWeight;

	/** Visual mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle|Visual")
	TSoftObjectPtr<UStaticMesh> ObstacleMesh;

	FObstacleData()
		: CollisionDamage(30.0f)
		, bDestructible(false)
		, Health(100.0f)
		, SpawnWeight(1.0f)
	{
	}
};
