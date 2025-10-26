// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameDataStructs.generated.h"

/**
 * Mount point data for turret placement on war rig
 * Defines where turrets can be attached and what restrictions apply
 */
USTRUCT(BlueprintType)
struct FMountPointData
{
	GENERATED_BODY()

	// Transform relative to the war rig root
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountPoint")
	FTransform RelativeTransform;

	// Allowed facing directions (0-7 for 8 compass directions)
	// Empty array means all directions allowed
	// 0=Forward, 1=ForwardRight, 2=Right, 3=BackRight, 4=Back, 5=BackLeft, 6=Left, 7=ForwardLeft
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountPoint")
	TArray<int32> AllowedFacingDirections;

	// Tags for special turret restrictions (e.g., heavy weapons only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountPoint")
	FGameplayTagContainer MountTags;

	// Display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MountPoint")
	FText DisplayName;

	FMountPointData()
		: RelativeTransform(FTransform::Identity)
	{
	}
};

/**
 * War rig configuration data
 * Defines different war rig types with their stats and mounting points
 */
USTRUCT(BlueprintType)
struct FWarRigData : public FTableRowBase
{
	GENERATED_BODY()

	// Rig display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig")
	FText RigName;

	// Rig description for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig")
	FText Description;

	// Reference to the skeletal mesh for this rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig")
	TSoftObjectPtr<USkeletalMesh> RigMesh;

	// Base health points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Stats")
	float BaseHealth;

	// Base armor value (damage reduction)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Stats")
	float BaseArmor;

	// Base fuel capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Stats")
	float BaseFuelCapacity;

	// Available mounting points for turrets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Mounting")
	TArray<FMountPointData> MountingPoints;

	// Cost to unlock this rig (scrap)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Economy")
	int32 UnlockCost;

	// Is this rig unlocked by default?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WarRig|Economy")
	bool bUnlockedByDefault;

	FWarRigData()
		: BaseHealth(1000.0f)
		, BaseArmor(0.0f)
		, BaseFuelCapacity(100.0f)
		, UnlockCost(0)
		, bUnlockedByDefault(true)
	{
	}
};

/**
 * Core gameplay balance data
 * All tunable values for core game mechanics
 */
USTRUCT(BlueprintType)
struct FGameplayBalanceData : public FTableRowBase
{
	GENERATED_BODY()

	// Fuel drain rate per second when moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float FuelDrainRate;

	// Fuel cost to change lanes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float LaneChangeFuelCost;

	// Total distance needed to win (in meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Victory")
	float WinDistanceMeters;

	// Warning time before obstacle collision (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Obstacles")
	float ObstacleWarningTime;

	// World scroll speed (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|World")
	float ScrollSpeed;

	// Minimum scroll speed (can't go below this)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|World")
	float MinScrollSpeed;

	// Maximum scroll speed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|World")
	float MaxScrollSpeed;

	// Number of lanes available
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Lanes")
	int32 NumLanes;

	// Distance between lanes (in units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Lanes")
	float LaneSpacing;

	// Lane change duration (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Lanes")
	float LaneChangeDuration;

	FGameplayBalanceData()
		: FuelDrainRate(1.0f)
		, LaneChangeFuelCost(5.0f)
		, WinDistanceMeters(10000.0f)
		, ObstacleWarningTime(2.0f)
		, ScrollSpeed(1000.0f)
		, MinScrollSpeed(500.0f)
		, MaxScrollSpeed(2000.0f)
		, NumLanes(3)
		, LaneSpacing(400.0f)
		, LaneChangeDuration(0.5f)
	{
	}
};

/**
 * Turret type data
 * Defines stats and costs for each turret type
 */
USTRUCT(BlueprintType)
struct FTurretData : public FTableRowBase
{
	GENERATED_BODY()

	// Turret display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText TurretName;

	// Turret description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText Description;

	// Base damage per shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
	float BaseDamage;

	// Fire rate (shots per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
	float FireRate;

	// Attack range (in units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
	float AttackRange;

	// Projectile speed (0 for hitscan)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Stats")
	float ProjectileSpeed;

	// Cost to build (scrap)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 BuildCost;

	// Cost to upgrade to next level (scrap)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 UpgradeCost;

	// Maximum upgrade level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 MaxUpgradeLevel;

	// Tags required on mount point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Mounting")
	FGameplayTagContainer RequiredMountTags;

	FTurretData()
		: BaseDamage(10.0f)
		, FireRate(1.0f)
		, AttackRange(1000.0f)
		, ProjectileSpeed(2000.0f)
		, BuildCost(10)
		, UpgradeCost(20)
		, MaxUpgradeLevel(3)
	{
	}
};

/**
 * Enemy/raider type data
 * Defines stats and behavior for different enemy types
 */
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

	// Enemy display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FText EnemyName;

	// Base health points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float BaseHealth;

	// Movement speed (relative to world scroll speed, 1.0 = same speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float SpeedMultiplier;

	// Damage dealt to war rig on collision
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float CollisionDamage;

	// Attack damage (if enemy has ranged attack)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float AttackDamage;

	// Attack range (0 for melee only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float AttackRange;

	// Attack rate (attacks per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float AttackRate;

	// Scrap dropped on death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
	int32 ScrapReward;

	// Chance to drop scrap (0.0 to 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
	float ScrapDropChance;

	// Base spawn weight (higher = more common)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spawning")
	float SpawnWeight;

	FEnemyData()
		: BaseHealth(100.0f)
		, SpeedMultiplier(1.2f)
		, CollisionDamage(50.0f)
		, AttackDamage(10.0f)
		, AttackRange(0.0f)
		, AttackRate(1.0f)
		, ScrapReward(5)
		, ScrapDropChance(0.8f)
		, SpawnWeight(1.0f)
	{
	}
};

/**
 * Pickup item data
 * Defines pickup types and their effects
 */
USTRUCT(BlueprintType)
struct FPickupData : public FTableRowBase
{
	GENERATED_BODY()

	// Pickup display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText PickupName;

	// Fuel amount restored (0 if not a fuel pickup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	float FuelAmount;

	// Scrap amount granted (0 if not a scrap pickup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	int32 ScrapAmount;

	// Health amount restored (0 if not a health pickup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	float HealthAmount;

	// Spawn weight (higher = more common)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Spawning")
	float SpawnWeight;

	// Lifetime before despawning (seconds, 0 = never despawn)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Spawning")
	float Lifetime;

	FPickupData()
		: FuelAmount(0.0f)
		, ScrapAmount(0)
		, HealthAmount(0.0f)
		, SpawnWeight(1.0f)
		, Lifetime(10.0f)
	{
	}
};

/**
 * World scrolling system data
 * Defines parameters for the scrolling world system
 */
USTRUCT(BlueprintType)
struct FWorldScrollData : public FTableRowBase
{
	GENERATED_BODY()

	// Ground tile size (length in direction of travel)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles")
	float TileLength;

	// Ground tile width
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles")
	float TileWidth;

	// Number of tiles in the object pool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles")
	int32 TilePoolSize;

	// Distance ahead of rig to spawn tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float TileSpawnDistance;

	// Distance behind rig to despawn tiles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float TileDespawnDistance;

	// Distance ahead of rig to spawn enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float EnemySpawnDistance;

	// Distance ahead of rig to spawn obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float ObstacleSpawnDistance;

	// Distance ahead of rig to spawn pickups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float PickupSpawnDistance;

	// Enemy pool size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Pooling")
	int32 EnemyPoolSize;

	// Obstacle pool size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Pooling")
	int32 ObstaclePoolSize;

	// Pickup pool size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Pooling")
	int32 PickupPoolSize;

	FWorldScrollData()
		: TileLength(2000.0f)
		, TileWidth(2000.0f)
		, TilePoolSize(10)
		, TileSpawnDistance(4000.0f)
		, TileDespawnDistance(2000.0f)
		, EnemySpawnDistance(3000.0f)
		, ObstacleSpawnDistance(3000.0f)
		, PickupSpawnDistance(2500.0f)
		, EnemyPoolSize(50)
		, ObstaclePoolSize(30)
		, PickupPoolSize(20)
	{
	}
};
