// Copyright Flatlander81. All Rights Reserved.

#include "World/GroundTilePoolComponent.h"
#include "World/GroundTile.h"
#include "World/WorldScrollComponent.h"
#include "DrawDebugHelpers.h"

UGroundTilePoolComponent::UGroundTilePoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Default configuration
	TileSize = FVector2D(2000.0f, 2000.0f);
	SpawnDistanceAhead = 3000.0f;
	DespawnDistanceBehind = 1000.0f;
	WarRigLocation = FVector::ZeroVector;
	FurthestTilePosition = 0.0f;
	bIsTilePoolInitialized = false;
	WorldScrollComponent = nullptr;
}

void UGroundTilePoolComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGroundTilePoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsTilePoolInitialized || !WorldScrollComponent)
	{
		return;
	}

	// Update tiles
	UpdateTiles(DeltaTime);
}

bool UGroundTilePoolComponent::InitializeTilePool(
	TSubclassOf<AGroundTile> InTileClass,
	FVector2D InTileSize,
	int32 InPoolSize,
	float InSpawnDistance,
	float InDespawnDistance,
	UWorldScrollComponent* InWorldScrollComponent)
{
	// Validate parameters
	if (!InTileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: Tile class is null."));
		return false;
	}

	if (InTileSize.X <= 0.0f || InTileSize.Y <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: Invalid tile size (%.2f, %.2f)."),
			InTileSize.X, InTileSize.Y);
		return false;
	}

	if (InPoolSize <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: Invalid pool size (%d). Must be greater than 0."), InPoolSize);
		return false;
	}

	if (!InWorldScrollComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: World scroll component is null."));
		return false;
	}

	// Store configuration
	TileSize = InTileSize;
	SpawnDistanceAhead = InSpawnDistance;
	DespawnDistanceBehind = InDespawnDistance;
	WorldScrollComponent = InWorldScrollComponent;
	WarRigLocation = FVector::ZeroVector; // Assume war rig at origin

	// Initialize base object pool
	FObjectPoolConfig Config;
	Config.PoolSize = InPoolSize;
	Config.bAutoExpand = false; // Fixed pool size for ground tiles
	Config.MaxPoolSize = InPoolSize;
	Config.SpawnDistanceAhead = InSpawnDistance;
	Config.DespawnDistanceBehind = InDespawnDistance;

	if (!Initialize(InTileClass, Config))
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: Failed to initialize base object pool."));
		return false;
	}

	bIsTilePoolInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("GroundTilePoolComponent: Initialized with %d tiles of size (%.2f, %.2f)."),
		InPoolSize, TileSize.X, TileSize.Y);

	return true;
}

void UGroundTilePoolComponent::SpawnInitialTiles()
{
	if (!bIsTilePoolInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundTilePoolComponent: Cannot spawn initial tiles - not initialized."));
		return;
	}

	// Calculate how many tiles we need to spawn to fill the visible area + ahead distance
	float TotalDistanceToFill = SpawnDistanceAhead + DespawnDistanceBehind;
	int32 NumTilesToSpawn = FMath::CeilToInt(TotalDistanceToFill / TileSize.X);

	// Clamp to available pool size
	int32 AvailableTiles = GetAvailableCount();
	NumTilesToSpawn = FMath::Min(NumTilesToSpawn, AvailableTiles);

	UE_LOG(LogTemp, Log, TEXT("GroundTilePoolComponent: Spawning %d initial tiles."), NumTilesToSpawn);

	// Start spawning from behind war rig
	float CurrentSpawnX = WarRigLocation.X - DespawnDistanceBehind;
	FurthestTilePosition = CurrentSpawnX;

	for (int32 i = 0; i < NumTilesToSpawn; ++i)
	{
		FVector SpawnPosition = FVector(CurrentSpawnX, WarRigLocation.Y, WarRigLocation.Z);
		SpawnTileAt(SpawnPosition);

		CurrentSpawnX += TileSize.X;
		FurthestTilePosition = CurrentSpawnX;
	}

	UE_LOG(LogTemp, Log, TEXT("GroundTilePoolComponent: Initial tiles spawned. Furthest position: %.2f"), FurthestTilePosition);
}

void UGroundTilePoolComponent::UpdateTiles(float DeltaTime)
{
	if (!WorldScrollComponent)
	{
		return;
	}

	// Get scroll velocity
	FVector ScrollVelocity = WorldScrollComponent->GetScrollVelocity();

	// Update all active tiles
	TArray<AActor*> TilesToRecycle;

	for (AActor* Actor : GetActiveCount() > 0 ? TArray<AActor*>() : TArray<AActor*>())
	{
		if (!Actor)
		{
			continue;
		}

		AGroundTile* Tile = Cast<AGroundTile>(Actor);
		if (!Tile)
		{
			continue;
		}

		// Update tile position
		Tile->UpdatePosition(DeltaTime, ScrollVelocity);

		// Check if tile should be recycled
		if (ShouldRecycleTile(Tile))
		{
			TilesToRecycle.Add(Tile);
		}
	}

	// Recycle tiles that passed behind
	for (AActor* TileToRecycle : TilesToRecycle)
	{
		UE_LOG(LogTemp, Verbose, TEXT("GroundTilePoolComponent: Recycling tile at X=%.2f"), TileToRecycle->GetActorLocation().X);

		// Return to pool
		ReturnToPool(TileToRecycle);

		// Spawn new tile ahead
		FVector SpawnPosition = GetNextSpawnPosition();
		SpawnTileAt(SpawnPosition);
	}
}

bool UGroundTilePoolComponent::ShouldRecycleTile(AActor* Tile) const
{
	if (!Tile)
	{
		return false;
	}

	FVector TileLocation = Tile->GetActorLocation();
	float DistanceBehindWarRig = WarRigLocation.X - TileLocation.X;

	// Recycle if tile is behind the despawn threshold
	return DistanceBehindWarRig > DespawnDistanceBehind;
}

FVector UGroundTilePoolComponent::GetNextSpawnPosition() const
{
	// Spawn at the furthest tile position + tile size
	return FVector(FurthestTilePosition, WarRigLocation.Y, WarRigLocation.Z);
}

void UGroundTilePoolComponent::SpawnTileAt(FVector SpawnPosition)
{
	if (!HasAvailable())
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundTilePoolComponent: No tiles available in pool."));
		return;
	}

	// Get tile from pool
	AActor* Actor = GetFromPool(SpawnPosition, FRotator::ZeroRotator);
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundTilePoolComponent: Failed to get tile from pool."));
		return;
	}

	AGroundTile* Tile = Cast<AGroundTile>(Actor);
	if (!Tile)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTilePoolComponent: Pooled actor is not a GroundTile."));
		return;
	}

	// Configure tile
	Tile->SetTileSize(TileSize);

	// Update furthest position if this tile is ahead
	if (SpawnPosition.X > FurthestTilePosition)
	{
		FurthestTilePosition = SpawnPosition.X + TileSize.X;
	}

	UE_LOG(LogTemp, Verbose, TEXT("GroundTilePoolComponent: Spawned tile at X=%.2f"), SpawnPosition.X);
}
