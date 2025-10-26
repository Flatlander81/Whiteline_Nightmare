// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/WorldObjectSpawner.h"
#include "World/WorldScrollManager.h"
#include "World/ObjectPoolComponent.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Data/GameplayDataStructs.h"
#include "Kismet/GameplayStatics.h"

AWorldObjectSpawner::AWorldObjectSpawner()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create object pool components
	EnemyPool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("EnemyPool"));
	ObstaclePool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("ObstaclePool"));
	PickupPool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("PickupPool"));

	// Default values
	bIsSpawning = false;
	SpawnInterval = 2.0f;
	TimeSinceLastSpawn = 0.0f;
	NumberOfLanes = 3;
	LaneSpacing = 400.0f;
	EnemySpawnChance = 0.3f;
	ObstacleSpawnChance = 0.2f;
	PickupSpawnChance = 0.15f;
}

void AWorldObjectSpawner::BeginPlay()
{
	Super::BeginPlay();

	LoadSpawnSettings();
	InitializePools();
}

void AWorldObjectSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSpawning)
	{
		TimeSinceLastSpawn += DeltaTime;

		if (TimeSinceLastSpawn >= SpawnInterval)
		{
			TrySpawnObjects();
			TimeSinceLastSpawn = 0.0f;
		}

		UpdateSpawnedObjects(DeltaTime);
	}
}

void AWorldObjectSpawner::Initialize(AWorldScrollManager* ScrollManager, AActor* WarRig)
{
	WorldScrollManager = ScrollManager;
	WarRigReference = WarRig;

	UE_LOG(LogTemp, Log, TEXT("WorldObjectSpawner initialized"));
}

void AWorldObjectSpawner::StartSpawning()
{
	bIsSpawning = true;
	TimeSinceLastSpawn = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Started spawning objects"));
}

void AWorldObjectSpawner::StopSpawning()
{
	bIsSpawning = false;
	UE_LOG(LogTemp, Log, TEXT("Stopped spawning objects"));
}

void AWorldObjectSpawner::InitializePools()
{
	// Load pool sizes from data table
	int32 EnemyPoolSize = 30;
	int32 ObstaclePoolSize = 20;
	int32 PickupPoolSize = 15;

	if (AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		if (UDataTable* WorldScrollTable = GameMode->GetWorldScrollDataTable())
		{
			static const FString ContextString(TEXT("InitializePools"));
			TArray<FName> RowNames = WorldScrollTable->GetRowNames();

			if (RowNames.Num() > 0)
			{
				if (const FWorldScrollData* ScrollData = WorldScrollTable->FindRow<FWorldScrollData>(RowNames[0], ContextString))
				{
					EnemyPoolSize = ScrollData->EnemyPoolSize;
					ObstaclePoolSize = ScrollData->ObstaclePoolSize;
					PickupPoolSize = ScrollData->PickupPoolSize;
				}
			}
		}
	}

	// Initialize pools (if classes are set)
	if (DefaultEnemyClass)
	{
		EnemyPool->InitializePool(DefaultEnemyClass, EnemyPoolSize);
	}

	if (DefaultObstacleClass)
	{
		ObstaclePool->InitializePool(DefaultObstacleClass, ObstaclePoolSize);
	}

	if (DefaultPickupClass)
	{
		PickupPool->InitializePool(DefaultPickupClass, PickupPoolSize);
	}

	UE_LOG(LogTemp, Log, TEXT("Initialized spawner pools: Enemies=%d, Obstacles=%d, Pickups=%d"),
		EnemyPoolSize, ObstaclePoolSize, PickupPoolSize);
}

void AWorldObjectSpawner::TrySpawnObjects()
{
	if (!WorldScrollManager || !WarRigReference)
	{
		return;
	}

	// Randomly decide which lanes to spawn in
	for (int32 Lane = 0; Lane < NumberOfLanes; ++Lane)
	{
		// Roll for enemy
		if (FMath::FRand() < EnemySpawnChance)
		{
			SpawnEnemy(Lane);
		}
		// Roll for obstacle (if no enemy spawned)
		else if (FMath::FRand() < ObstacleSpawnChance)
		{
			SpawnObstacle(Lane);
		}
		// Roll for pickup (if neither enemy nor obstacle spawned)
		else if (FMath::FRand() < PickupSpawnChance)
		{
			SpawnPickup(Lane);
		}
	}
}

void AWorldObjectSpawner::SpawnEnemy(int32 Lane)
{
	if (!EnemyPool || !DefaultEnemyClass)
	{
		return;
	}

	FVector SpawnPosition = GetLaneSpawnPosition(Lane);
	AActor* Enemy = EnemyPool->GetFromPool(SpawnPosition, FRotator::ZeroRotator);

	if (Enemy)
	{
		ActiveObjects.Add(Enemy);
		UE_LOG(LogTemp, Verbose, TEXT("Spawned enemy in lane %d at position %s"), Lane, *SpawnPosition.ToString());

		// TODO: Initialize enemy with data from data table
		// TODO: Set enemy attributes via GAS
	}
}

void AWorldObjectSpawner::SpawnObstacle(int32 Lane)
{
	if (!ObstaclePool || !DefaultObstacleClass)
	{
		return;
	}

	FVector SpawnPosition = GetLaneSpawnPosition(Lane);
	AActor* Obstacle = ObstaclePool->GetFromPool(SpawnPosition, FRotator::ZeroRotator);

	if (Obstacle)
	{
		ActiveObjects.Add(Obstacle);
		UE_LOG(LogTemp, Verbose, TEXT("Spawned obstacle in lane %d"), Lane);

		// TODO: Initialize obstacle with data from data table
	}
}

void AWorldObjectSpawner::SpawnPickup(int32 Lane)
{
	if (!PickupPool || !DefaultPickupClass)
	{
		return;
	}

	FVector SpawnPosition = GetLaneSpawnPosition(Lane);
	AActor* Pickup = PickupPool->GetFromPool(SpawnPosition, FRotator::ZeroRotator);

	if (Pickup)
	{
		ActiveObjects.Add(Pickup);
		UE_LOG(LogTemp, Verbose, TEXT("Spawned pickup in lane %d"), Lane);

		// TODO: Initialize pickup with data from data table
	}
}

FVector AWorldObjectSpawner::GetLaneSpawnPosition(int32 Lane) const
{
	if (!WarRigReference || !WorldScrollManager)
	{
		return FVector::ZeroVector;
	}

	FVector WarRigLocation = WarRigReference->GetActorLocation();
	float SpawnDistance = WorldScrollManager->GetSpawnDistance();

	// Calculate lane offset from center
	// Lane 0 is leftmost, center lane is NumberOfLanes / 2
	const int32 CenterLane = NumberOfLanes / 2;
	const float LaneOffset = (Lane - CenterLane) * LaneSpacing;

	// Spawn ahead of war rig (positive X)
	FVector SpawnPosition = WarRigLocation;
	SpawnPosition.X += SpawnDistance;
	SpawnPosition.Y = LaneOffset; // Y axis is left/right

	return SpawnPosition;
}

void AWorldObjectSpawner::LoadSpawnSettings()
{
	if (AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		FGameplayBalanceData BalanceData;
		if (GameMode->GetGameplayBalanceData(BalanceData))
		{
			NumberOfLanes = BalanceData.NumberOfLanes;
		}
	}
}

void AWorldObjectSpawner::UpdateSpawnedObjects(float DeltaTime)
{
	if (!WorldScrollManager || !WarRigReference)
	{
		return;
	}

	const float DespawnDistance = WorldScrollManager->GetDespawnDistance();
	const FVector WarRigLocation = WarRigReference->GetActorLocation();

	// Check for objects that have passed behind the war rig
	TArray<AActor*> ObjectsToReturn;

	for (AActor* Object : ActiveObjects)
	{
		if (!Object)
		{
			continue;
		}

		// Check if object is too far behind the war rig
		const float DistanceBehindRig = WarRigLocation.X - Object->GetActorLocation().X;
		if (DistanceBehindRig > DespawnDistance)
		{
			ObjectsToReturn.Add(Object);
		}
	}

	// Return objects to their respective pools
	for (AActor* Object : ObjectsToReturn)
	{
		ActiveObjects.Remove(Object);

		// Determine which pool to return to
		// TODO: Better way to identify object type (interface or tag?)
		if (EnemyPool && Object->IsA(DefaultEnemyClass))
		{
			EnemyPool->ReturnToPool(Object);
		}
		else if (ObstaclePool && Object->IsA(DefaultObstacleClass))
		{
			ObstaclePool->ReturnToPool(Object);
		}
		else if (PickupPool && Object->IsA(DefaultPickupClass))
		{
			PickupPool->ReturnToPool(Object);
		}
	}
}
