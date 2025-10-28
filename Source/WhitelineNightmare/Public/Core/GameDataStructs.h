// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "GameDataStructs.generated.h"

/**
 * Mount Point Data - Defines where turrets can be mounted on the war rig
 */
USTRUCT(BlueprintType)
struct FMountPointData
{
	GENERATED_BODY()

	// Transform relative to the war rig (position and rotation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FTransform MountTransform;

	// Array of allowed facing directions (0-7 for 8 compass directions)
	// 0 = Forward, 1 = Forward-Right, 2 = Right, 3 = Back-Right, etc.
	// Empty array = all directions allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	TArray<int32> AllowedFacingDirections;

	// Tags for special mount restrictions (e.g., "Mount.Heavy", "Mount.Rear")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FGameplayTagContainer MountTags;

	// Display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FText DisplayName;

	FMountPointData()
		: MountTransform(FTransform::Identity)
		, DisplayName(FText::FromString("Mount Point"))
	{
	}
};

/**
 * War Rig Data - Data table row for different war rig configurations
 */
USTRUCT(BlueprintType)
struct FWarRigData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of the war rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig")
	FText DisplayName;

	// Description of the war rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig")
	FText Description;

	// Array of mesh sections (cab + trailers) - each mesh spawned as a separate component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Mesh")
	TArray<TSoftObjectPtr<UStaticMesh>> MeshSections;

	// Array of mount points for turrets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Mount Points")
	TArray<FMountPointData> MountPoints;

	// Base maximum hull/health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float MaxHull;

	// Fuel cost to change lanes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float LaneChangeFuelCost;

	// Speed of lane change movement (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float LaneChangeSpeed;

	// Base maximum fuel capacity (legacy - may be removed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float MaxFuel;

	// Base maximum armor/health (legacy - use MaxHull instead)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Stats")
	float MaxArmor;

	// Primary material for the rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Visual")
	TSoftObjectPtr<UMaterialInterface> PrimaryMaterial;

	// Secondary material for the rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Visual")
	TSoftObjectPtr<UMaterialInterface> SecondaryMaterial;

	// Primary color tint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Visual")
	FLinearColor PrimaryColor;

	// Secondary color tint
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Visual")
	FLinearColor SecondaryColor;

	// Camera distance from war rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Camera")
	float CameraDistance;

	// Camera pitch angle (negative = looking down)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Camera")
	float CameraPitch;

	// Cost in scrap to unlock this rig
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Economy")
	int32 UnlockCost;

	FWarRigData()
		: MaxHull(100.0f)
		, LaneChangeFuelCost(0.0f)
		, LaneChangeSpeed(500.0f)
		, MaxFuel(100.0f)
		, MaxArmor(100.0f)
		, PrimaryColor(FLinearColor::Red)
		, SecondaryColor(FLinearColor::Gray)
		, CameraDistance(2000.0f)
		, CameraPitch(-75.0f)
		, UnlockCost(0)
	{
	}
};

/**
 * Gameplay Balance Data - Data table row for core gameplay tuning
 */
USTRUCT(BlueprintType)
struct FGameplayBalanceData : public FTableRowBase
{
	GENERATED_BODY()

	// How much fuel drains per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float FuelDrainRate;

	// Fuel cost to change lanes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float LaneChangeFuelCost;

	// Distance needed to travel to win
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Win Condition")
	float WinDistance;

	// How far ahead obstacles spawn (warning time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Spawning")
	float ObstacleSpawnDistance;

	// Base world scroll speed (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|World Movement")
	float ScrollSpeed;

	// Time duration for lane change in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Movement")
	float LaneChangeDuration;

	// Distance between lanes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Movement")
	float LaneWidth;

	FGameplayBalanceData()
		: FuelDrainRate(1.0f)
		, LaneChangeFuelCost(5.0f)
		, WinDistance(10000.0f)
		, ObstacleSpawnDistance(2000.0f)
		, ScrollSpeed(500.0f)
		, LaneChangeDuration(1.0f)
		, LaneWidth(400.0f)
	{
	}
};

/**
 * Turret Data - Data table row for different turret types
 */
USTRUCT(BlueprintType)
struct FTurretData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of the turret
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText Description;

	// Reference to the turret mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TSoftObjectPtr<UStaticMesh> TurretMesh;

	// Base damage per shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float BaseDamage;

	// Shots per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float FireRate;

	// Maximum targeting range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float Range;

	// Cost in scrap to build
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 BuildCost;

	// Cost in scrap to upgrade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 UpgradeCost;

	FTurretData()
		: BaseDamage(10.0f)
		, FireRate(1.0f)
		, Range(1000.0f)
		, BuildCost(50)
		, UpgradeCost(25)
	{
	}
};

/**
 * Enemy Data - Data table row for different enemy raider types
 */
USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FText DisplayName;

	// Reference to the enemy mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	TSoftObjectPtr<USkeletalMesh> EnemyMesh;

	// Base health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MaxHealth;

	// Movement speed (relative to world scroll speed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Stats")
	float MovementSpeed;

	// Damage per attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackDamage;

	// Time between attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackRate;

	// Base spawn weight (higher = more common)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spawning")
	float SpawnWeight;

	// Scrap dropped on death
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
	int32 ScrapReward;

	FEnemyData()
		: MaxHealth(50.0f)
		, MovementSpeed(100.0f)
		, AttackDamage(10.0f)
		, AttackRate(1.0f)
		, SpawnWeight(1.0f)
		, ScrapReward(10)
	{
	}
};

/**
 * Pickup Data - Data table row for fuel and scrap pickups
 */
USTRUCT(BlueprintType)
struct FPickupData : public FTableRowBase
{
	GENERATED_BODY()

	// Display name of the pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FText DisplayName;

	// Reference to the pickup mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	TSoftObjectPtr<UStaticMesh> PickupMesh;

	// Amount of fuel restored (0 if not a fuel pickup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Rewards")
	float FuelAmount;

	// Amount of scrap given (0 if not a scrap pickup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Rewards")
	int32 ScrapAmount;

	// Spawn weight (higher = more common)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Spawning")
	float SpawnWeight;

	FPickupData()
		: FuelAmount(0.0f)
		, ScrapAmount(0)
		, SpawnWeight(1.0f)
	{
	}
};

/**
 * World Scroll Data - Data table row for world scrolling and object pooling settings
 */
USTRUCT(BlueprintType)
struct FWorldScrollData : public FTableRowBase
{
	GENERATED_BODY()

	// Size of each ground tile (length along forward axis)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles")
	float TileSize;

	// Number of tiles to keep in the pool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles")
	int32 TilePoolSize;

	// Distance ahead of war rig where new tiles spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Spawning")
	float TileSpawnDistance;

	// Distance behind war rig where tiles despawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Despawning")
	float TileDespawnDistance;

	// Pool size for enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 EnemyPoolSize;

	// Pool size for obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 ObstaclePoolSize;

	// Pool size for pickups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 PickupPoolSize;

	FWorldScrollData()
		: TileSize(2000.0f)
		, TilePoolSize(10)
		, TileSpawnDistance(3000.0f)
		, TileDespawnDistance(1000.0f)
		, EnemyPoolSize(50)
		, ObstaclePoolSize(30)
		, PickupPoolSize(20)
	{
	}
};
