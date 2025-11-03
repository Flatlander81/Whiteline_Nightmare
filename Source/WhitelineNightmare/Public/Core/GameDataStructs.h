// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Core/WhitelineNightmareGameplayTags.h"
#include "GameDataStructs.generated.h"

// Forward declarations
class USoundBase;
class UNiagaraSystem;

/**
 * Mount Point Data - Defines where turrets can be mounted on the war rig
 * EXTENDED from Prompt 3.2 to include occupancy tracking
 */
USTRUCT(BlueprintType)
struct FMountPointData
{
	GENERATED_BODY()

	// Transform relative to the war rig (position and rotation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FTransform MountTransform;

	// Array of allowed facing directions (0-7 for 8 compass directions)
	// 0 = North (Forward), 1 = NE, 2 = East (Right), 3 = SE, 4 = South (Back), 5 = SW, 6 = West (Left), 7 = NW
	// Empty array = all directions allowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	TArray<int32> AllowedFacingDirections;

	// Tags for special mount restrictions (e.g., "Mount.Heavy", "Mount.Rear")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FGameplayTagContainer MountTags;

	// Display name for UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount Point")
	FText DisplayName;

	// Whether a turret is currently mounted here
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mount Point|State")
	bool bOccupied;

	// Reference to the turret currently occupying this mount (nullptr if unoccupied)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mount Point|State")
	TObjectPtr<ATurretBase> OccupyingTurret;

	FMountPointData()
		: MountTransform(FTransform::Identity)
		, DisplayName(FText::FromString("Mount Point"))
		, bOccupied(false)
		, OccupyingTurret(nullptr)
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
		: DisplayName(FText::FromString("Semi Truck"))
		, Description(FText::FromString("A classic highway semi-truck configured for wasteland combat"))
		, MaxHull(100.0f)
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
		// Default mesh sections for SemiTruck configuration (3 total: cab + 2 trailers)
		// Note: These will be null soft object pointers by default
		// Set actual mesh references in the data table in Unreal Editor
		MeshSections.SetNum(3);

		// Default mount points for SemiTruck configuration (10 total)
		// 2 on cab + 4 on trailer 1 + 4 on trailer 2

		// Mount Point 0: Cab - Left Side
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, -100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {0, 1, 2, 6, 7}; // Forward, Forward-Right, Right, Left, Forward-Left
			MountPoint.DisplayName = FText::FromString("Cab Left");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Cab);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 1: Cab - Right Side
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {0, 1, 2, 3, 4}; // Forward, Forward-Right, Right, Back-Right, Back
			MountPoint.DisplayName = FText::FromString("Cab Right");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Cab);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 2: Trailer 1 - Front Left
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-200.0f, -100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {}; // All directions allowed
			MountPoint.DisplayName = FText::FromString("Trailer 1 Front Left");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 3: Trailer 1 - Front Right
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-200.0f, 100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {}; // All directions allowed
			MountPoint.DisplayName = FText::FromString("Trailer 1 Front Right");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 4: Trailer 1 - Rear Left
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-300.0f, -100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {2, 3, 4, 5, 6}; // Right, Back-Right, Back, Back-Left, Left
			MountPoint.DisplayName = FText::FromString("Trailer 1 Rear Left");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 5: Trailer 1 - Rear Right
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-300.0f, 100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {2, 3, 4, 5, 6}; // Right, Back-Right, Back, Back-Left, Left
			MountPoint.DisplayName = FText::FromString("Trailer 1 Rear Right");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 6: Trailer 2 - Front Left
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-400.0f, -100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {}; // All directions allowed
			MountPoint.DisplayName = FText::FromString("Trailer 2 Front Left");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Rear);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 7: Trailer 2 - Front Right
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-400.0f, 100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {}; // All directions allowed
			MountPoint.DisplayName = FText::FromString("Trailer 2 Front Right");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Rear);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 8: Trailer 2 - Rear Left
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-500.0f, -100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {3, 4, 5}; // Back-Right, Back, Back-Left
			MountPoint.DisplayName = FText::FromString("Trailer 2 Rear Left");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Rear);
			MountPoints.Add(MountPoint);
		}

		// Mount Point 9: Trailer 2 - Rear Right
		{
			FMountPointData MountPoint;
			MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-500.0f, 100.0f, 50.0f), FVector::OneVector);
			MountPoint.AllowedFacingDirections = {3, 4, 5}; // Back-Right, Back, Back-Left
			MountPoint.DisplayName = FText::FromString("Trailer 2 Rear Right");
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Trailer);
			MountPoint.MountTags.AddTag(WhitelineNightmareGameplayTags::Mount_Rear);
			MountPoints.Add(MountPoint);
		}
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

	// Starting fuel amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float FuelStartAmount;

	// Maximum fuel capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Balance|Fuel")
	float MaxFuelCapacity;

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
		: FuelDrainRate(5.0f)
		, FuelStartAmount(100.0f)
		, MaxFuelCapacity(100.0f)
		, LaneChangeFuelCost(5.0f)
		, WinDistance(10000.0f)
		, ObstacleSpawnDistance(2000.0f)
		, ScrollSpeed(500.0f)
		, LaneChangeDuration(1.0f)
		, LaneWidth(400.0f)
	{
	}
};

// Forward declaration for turret class
class ATurretBase;

/**
 * Turret Data - Data table row for different turret types
 */
USTRUCT(BlueprintType)
struct FTurretData : public FTableRowBase
{
	GENERATED_BODY()

	// Internal turret name (used for identification)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FName TurretName;

	// Display name of the turret
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText DisplayName;

	// Description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FText Description;

	// Class reference for the turret type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TSubclassOf<ATurretBase> TurretClass;

	// Reference to the turret mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	TSoftObjectPtr<UStaticMesh> TurretMesh;

	// Icon for UI display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|UI")
	TSoftObjectPtr<UTexture2D> Icon;

	// Base damage per shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float BaseDamage;

	// Shots per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float FireRate;

	// Maximum targeting range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float Range;

	// Base health for the turret
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Combat")
	float BaseHealth;

	// Cost in scrap to build
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 BuildCost;

	// Cost in scrap to upgrade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Economy")
	int32 UpgradeCost;

	FTurretData()
		: TurretName(NAME_None)
		, DisplayName(FText::FromString(TEXT("Default Turret")))
		, Description(FText::FromString(TEXT("A basic defensive turret")))
		, BaseDamage(10.0f)
		, FireRate(1.0f)
		, Range(1000.0f)
		, BaseHealth(100.0f)
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

	// Sound to play when pickup is collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	TSoftObjectPtr<USoundBase> PickupSound;

	// Particle effect to spawn when pickup is collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Effects")
	TSoftObjectPtr<UNiagaraSystem> PickupParticle;

	// Visual color for the pickup sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
	FLinearColor VisualColor;

	// Collision radius for the pickup sphere
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Collision")
	float PickupRadius;

	FPickupData()
		: FuelAmount(0.0f)
		, ScrapAmount(0)
		, SpawnWeight(1.0f)
		, VisualColor(FLinearColor::Green)
		, PickupRadius(50.0f)
	{
	}
};

/**
 * World Tile Data - Data table row for tile management and object pooling settings
 */
USTRUCT(BlueprintType)
struct FWorldTileData : public FTableRowBase
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

	// Optional: Static mesh to use for ground tiles (if set, overrides default cube mesh)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles|Visual")
	TSoftObjectPtr<UStaticMesh> TileMesh;

	// Optional: Material to apply to ground tiles (if set, overrides default material)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Tiles|Visual")
	TSoftObjectPtr<UMaterialInterface> TileMaterial;

	// Pool size for enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 EnemyPoolSize;

	// Pool size for obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 ObstaclePoolSize;

	// Pool size for pickups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Object Pooling")
	int32 PickupPoolSize;

	FWorldTileData()
		: TileSize(2000.0f)
		, TilePoolSize(15)
		, TileSpawnDistance(10000.0f)  // Spawn 10000 units ahead (5 tiles)
		, TileDespawnDistance(5000.0f)  // Despawn 5000 units behind (2.5 tiles)
		, EnemyPoolSize(50)
		, ObstaclePoolSize(30)
		, PickupPoolSize(20)
	{
	}
};

/**
 * World Scroll Data - Data table row for world scrolling velocity configuration
 * Controls the speed and direction of world scrolling to simulate forward movement
 */
USTRUCT(BlueprintType)
struct FWorldScrollData : public FTableRowBase
{
	GENERATED_BODY()

	// Scroll speed in units per second (how fast the world moves backward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	float ScrollSpeed;

	// Whether scrolling is enabled (can pause world movement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	bool bScrollEnabled;

	// Direction of scroll (normalized) - default (-1, 0, 0) for backward along X axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Scroll")
	FVector ScrollDirection;

	FWorldScrollData()
		: ScrollSpeed(1000.0f)
		, bScrollEnabled(true)
		, ScrollDirection(-1.0f, 0.0f, 0.0f)
	{
	}
};

/**
 * Lane System Data - Configuration for lane-based movement system
 *
 * Defines the number of lanes and their Y-axis positions for lateral war rig movement.
 * Can either specify lane count + spacing (automatic calculation) or provide explicit positions.
 */
USTRUCT(BlueprintType)
struct FLaneSystemData : public FTableRowBase
{
	GENERATED_BODY()

	// Number of lanes (default 5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	int32 NumLanes;

	// Distance between lanes (units) - used to auto-calculate positions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	float LaneSpacing;

	// Index of center lane (default 2 for 5 lanes, 0-indexed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	int32 CenterLaneIndex;

	// Optional: Explicitly specify Y positions for each lane
	// If empty, positions are auto-calculated from NumLanes and LaneSpacing
	// Example for 5 lanes: [-400, -200, 0, 200, 400]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	TArray<float> LaneYPositions;

	FLaneSystemData()
		: NumLanes(5)
		, LaneSpacing(200.0f)
		, CenterLaneIndex(2)
	{
		// Default positions will be auto-calculated if LaneYPositions is empty
	}
};
