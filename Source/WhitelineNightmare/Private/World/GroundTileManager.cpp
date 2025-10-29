// Copyright Flatlander81. All Rights Reserved.

#include "World/GroundTileManager.h"
#include "World/GroundTile.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/ObjectPoolComponent.h"
#include "Core/ObjectPoolTypes.h"
#include "Core/GameDataStructs.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogGroundTileManager, Log, All);

UGroundTileManager::UGroundTileManager()
	: TileDataTable(nullptr)
	, DataTableRowName("DefaultTile")
	, TileClass(nullptr)
	, WarRig(nullptr)
	, TilePool(nullptr)
	, TileSize(2000.0f)
	, TilePoolSize(15)
	, TileSpawnDistance(10000.0f)  // Spawn 10000 units ahead (5 tiles)
	, TileDespawnDistance(5000.0f)  // Despawn 5000 units behind (2.5 tiles)
	, bShowDebugVisualization(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UGroundTileManager::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogGroundTileManager, Log, TEXT("=== GroundTileManager BeginPlay ==="));
	UE_LOG(LogGroundTileManager, Log, TEXT("Initial defaults: TileSize=%.0f, PoolSize=%d, SpawnDist=%.0f, DespawnDist=%.0f"),
		TileSize, TilePoolSize, TileSpawnDistance, TileDespawnDistance);

	// Load configuration from data table
	if (!LoadConfigFromDataTable())
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("Failed to load config from data table, using defaults"));
	}

	UE_LOG(LogGroundTileManager, Log, TEXT("After config load: TileSize=%.0f, PoolSize=%d, SpawnDist=%.0f, DespawnDist=%.0f"),
		TileSize, TilePoolSize, TileSpawnDistance, TileDespawnDistance);

	// Initialize tile pool
	if (!InitializeTilePool())
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("Failed to initialize tile pool"));
		return;
	}

	// Get war rig reference
	WarRig = GetWarRig();
	if (!WarRig)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("War rig not found, using world origin"));
	}
	else
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("War rig found at position: %s"), *WarRig->GetActorLocation().ToString());
	}

	// Spawn initial tiles
	SpawnInitialTiles();

	UE_LOG(LogGroundTileManager, Log, TEXT("GroundTileManager initialized: %d tiles spawned"), ActiveTiles.Num());
	UE_LOG(LogGroundTileManager, Log, TEXT("Pool state: Active=%d, Available=%d, Total=%d"),
		TilePool ? TilePool->GetActiveCount() : 0,
		TilePool ? TilePool->GetAvailableCount() : 0,
		TilePool ? TilePool->GetTotalPoolSize() : 0);
	UE_LOG(LogGroundTileManager, Log, TEXT("=== GroundTileManager Initialization Complete ==="));
}

void UGroundTileManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check for tiles that need recycling
	CheckForTileRecycling();

	// Debug visualization
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
}

float UGroundTileManager::GetFurthestTilePosition() const
{
	if (ActiveTiles.Num() == 0)
	{
		return 0.0f;
	}

	float FurthestX = -MAX_FLT;
	for (const AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			float TileX = Tile->GetActorLocation().X;
			if (TileX > FurthestX)
			{
				FurthestX = TileX;
			}
		}
	}

	return FurthestX;
}

void UGroundTileManager::CheckForTileRecycling()
{
	if (!WarRig)
	{
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;
	const float DespawnThreshold = WarRigX - TileDespawnDistance;
	const float SpawnThreshold = WarRigX + TileSpawnDistance;

	// Check for tiles that passed behind war rig
	for (int32 i = ActiveTiles.Num() - 1; i >= 0; --i)
	{
		AGroundTile* Tile = ActiveTiles[i];
		if (!Tile)
		{
			continue;
		}

		const float TileX = Tile->GetActorLocation().X;

		// If tile passed behind war rig, recycle it
		if (TileX < DespawnThreshold)
		{
			UE_LOG(LogGroundTileManager, Verbose, TEXT("Recycling tile at X=%.0f (threshold=%.0f)"),
				TileX, DespawnThreshold);

			RecycleTile(Tile);
			ActiveTiles.RemoveAt(i);
		}
	}

	// Spawn new tiles if furthest tile is too close
	const float FurthestX = GetFurthestTilePosition();
	if (FurthestX < SpawnThreshold)
	{
		// Spawn new tile at furthest position + tile size
		FVector NewPosition(FurthestX + TileSize, 0.0f, 0.0f);
		AGroundTile* NewTile = SpawnTile(NewPosition);

		if (NewTile)
		{
			UE_LOG(LogGroundTileManager, Verbose, TEXT("Spawned new tile at X=%.0f"), NewPosition.X);
		}
	}
}

void UGroundTileManager::DebugShowTiles()
{
	bShowDebugVisualization = !bShowDebugVisualization;
	UE_LOG(LogGroundTileManager, Log, TEXT("Tile debug visualization: %s"),
		bShowDebugVisualization ? TEXT("ENABLED") : TEXT("DISABLED"));

	// Also enable/disable debug on tiles
	for (AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			Tile->bShowDebugBounds = bShowDebugVisualization;
		}
	}
}

void UGroundTileManager::DebugShowTileInfo()
{
	LogManagerState();
}

void UGroundTileManager::DebugSetSpawnDistance(float NewDistance)
{
	TileSpawnDistance = NewDistance;
	UE_LOG(LogGroundTileManager, Log, TEXT("Set TileSpawnDistance to %.0f"), NewDistance);
}

void UGroundTileManager::DebugSetDespawnDistance(float NewDistance)
{
	TileDespawnDistance = NewDistance;
	UE_LOG(LogGroundTileManager, Log, TEXT("Set TileDespawnDistance to %.0f"), NewDistance);
}

bool UGroundTileManager::LoadConfigFromDataTable()
{
	if (!TileDataTable)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("No data table assigned"));
		return false;
	}

	static const FString ContextString(TEXT("GroundTileManager::LoadConfigFromDataTable"));
	FWorldTileData* RowData = TileDataTable->FindRow<FWorldTileData>(DataTableRowName, ContextString);

	if (!RowData)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("Failed to find row '%s' in data table"),
			*DataTableRowName.ToString());
		return false;
	}

	// Load values
	TileSize = RowData->TileSize;
	TilePoolSize = RowData->TilePoolSize;
	TileSpawnDistance = RowData->TileSpawnDistance;
	TileDespawnDistance = RowData->TileDespawnDistance;

	// Load optional visual overrides
	ConfiguredTileMesh = RowData->TileMesh;
	ConfiguredTileMaterial = RowData->TileMaterial;

	UE_LOG(LogGroundTileManager, Log, TEXT("Loaded config: TileSize=%.0f, PoolSize=%d, SpawnDist=%.0f, DespawnDist=%.0f"),
		TileSize, TilePoolSize, TileSpawnDistance, TileDespawnDistance);

	if (!ConfiguredTileMesh.IsNull())
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("  TileMesh override: %s"), *ConfiguredTileMesh.ToString());
	}
	if (!ConfiguredTileMaterial.IsNull())
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("  TileMaterial override: %s"), *ConfiguredTileMaterial.ToString());
	}

	// Validate configuration
	bool bConfigValid = true;

	// Validate pool size (minimum 3 for seamless scrolling)
	if (TilePoolSize < 3)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("TilePoolSize (%d) is less than minimum required (3). Seamless scrolling requires at least 3 tiles."), TilePoolSize);
		TilePoolSize = 3; // Clamp to minimum
		bConfigValid = false;
	}

	// Validate spawn/despawn distances
	if (TileSpawnDistance <= TileDespawnDistance)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("TileSpawnDistance (%.0f) must be greater than TileDespawnDistance (%.0f). Tiles would spawn behind despawn threshold!"),
			TileSpawnDistance, TileDespawnDistance);
		bConfigValid = false;
	}

	// Validate tile size is positive
	if (TileSize <= 0.0f)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("TileSize (%.0f) must be positive. Using default 2000.0f"), TileSize);
		TileSize = 2000.0f;
		bConfigValid = false;
	}

	if (!bConfigValid)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("Configuration validation failed. Some values have been corrected. Please fix data table."));
	}

	return true;
}

bool UGroundTileManager::InitializeTilePool()
{
	// Validate tile class
	if (!TileClass)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("TileClass not set. Please assign a valid AGroundTile class in the component properties."));
		return false;
	}

	// Validate tile class implements IPoolableActor (warning only, not fatal)
	if (!TileClass->ImplementsInterface(UPoolableActor::StaticClass()))
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("TileClass does not implement IPoolableActor interface. Pool lifecycle callbacks may not work correctly."));
	}

	// Create pool component if not exists
	if (!TilePool)
	{
		AActor* Owner = GetOwner();
		if (!Owner)
		{
			UE_LOG(LogGroundTileManager, Error, TEXT("No owner actor"));
			return false;
		}

		TilePool = NewObject<UObjectPoolComponent>(Owner, UObjectPoolComponent::StaticClass());
		if (!TilePool)
		{
			UE_LOG(LogGroundTileManager, Error, TEXT("Failed to create tile pool component"));
			return false;
		}

		TilePool->RegisterComponent();
	}

	// Configure pool
	FObjectPoolConfig PoolConfig;
	PoolConfig.PoolSize = TilePoolSize;
	PoolConfig.bAutoExpand = true;
	PoolConfig.MaxPoolSize = TilePoolSize * 2;

	UE_LOG(LogGroundTileManager, Log, TEXT("=== Pool Configuration ==="));
	UE_LOG(LogGroundTileManager, Log, TEXT("Initial Size: %d"), PoolConfig.PoolSize);
	UE_LOG(LogGroundTileManager, Log, TEXT("Auto-Expand: %s"), PoolConfig.bAutoExpand ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogGroundTileManager, Log, TEXT("Max Size: %d"), PoolConfig.MaxPoolSize);

	// Validate pool configuration
	if (PoolConfig.PoolSize < 3)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("Pool size is %d, minimum recommended is 3 for seamless scrolling"), PoolConfig.PoolSize);
	}

	// Initialize pool
	bool bSuccess = TilePool->Initialize(TileClass, PoolConfig);
	if (!bSuccess)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("Failed to initialize tile pool"));
		return false;
	}

	UE_LOG(LogGroundTileManager, Log, TEXT("Tile pool initialized successfully"));
	UE_LOG(LogGroundTileManager, Log, TEXT("Pool state after init: Total=%d, Available=%d, Active=%d"),
		TilePool->GetTotalPoolSize(), TilePool->GetAvailableCount(), TilePool->GetActiveCount());
	return true;
}

void UGroundTileManager::SpawnInitialTiles()
{
	if (!WarRig)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("Cannot spawn tiles without war rig reference"));
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;

	UE_LOG(LogGroundTileManager, Log, TEXT("=== SpawnInitialTiles Calculation ==="));
	UE_LOG(LogGroundTileManager, Log, TEXT("Config: TileSize=%.0f, SpawnDist=%.0f, DespawnDist=%.0f, PoolSize=%d"),
		TileSize, TileSpawnDistance, TileDespawnDistance, TilePoolSize);

	// Calculate how many tiles we need to fill the visible area
	const float VisibleDistance = TileSpawnDistance + TileDespawnDistance;
	const int32 NumTilesToSpawn = FMath::CeilToInt(VisibleDistance / TileSize) + 2;

	UE_LOG(LogGroundTileManager, Log, TEXT("VisibleDistance = SpawnDist + DespawnDist = %.0f + %.0f = %.0f"),
		TileSpawnDistance, TileDespawnDistance, VisibleDistance);
	UE_LOG(LogGroundTileManager, Log, TEXT("NumTilesToSpawn = Ceil(%.0f / %.0f) + 2 = %d"),
		VisibleDistance, TileSize, NumTilesToSpawn);

	// Spawn tiles from well behind to ahead
	// Add extra margin (10000 units) behind despawn distance to ensure road looks complete from any camera angle
	const float ExtraBackMargin = 10000.0f;
	const float StartX = WarRigX - TileDespawnDistance - ExtraBackMargin;

	UE_LOG(LogGroundTileManager, Log, TEXT("WarRigX=%.0f, ExtraBackMargin=%.0f"), WarRigX, ExtraBackMargin);
	UE_LOG(LogGroundTileManager, Log, TEXT("StartX = %.0f - %.0f - %.0f = %.0f"),
		WarRigX, TileDespawnDistance, ExtraBackMargin, StartX);
	UE_LOG(LogGroundTileManager, Log, TEXT("Attempting to spawn %d tiles (Pool max: %d)"),
		NumTilesToSpawn, TilePool ? TilePool->GetTotalPoolSize() : 0);

	int32 SuccessCount = 0;
	int32 FailCount = 0;

	for (int32 i = 0; i < NumTilesToSpawn; ++i)
	{
		FVector TilePosition(StartX + (i * TileSize), 0.0f, 0.0f);

		if (TilePool)
		{
			UE_LOG(LogGroundTileManager, Verbose, TEXT("  [%d/%d] Attempting spawn at X=%.0f (Pool: %d/%d available)"),
				i + 1, NumTilesToSpawn, TilePosition.X,
				TilePool->GetAvailableCount(), TilePool->GetTotalPoolSize());
		}

		AGroundTile* Tile = SpawnTile(TilePosition);

		if (!Tile)
		{
			FailCount++;
			UE_LOG(LogGroundTileManager, Warning, TEXT("  [%d/%d] FAILED to spawn tile at X=%.0f"), i + 1, NumTilesToSpawn, TilePosition.X);
		}
		else
		{
			SuccessCount++;
		}
	}

	UE_LOG(LogGroundTileManager, Log, TEXT("=== Spawn Results ==="));
	UE_LOG(LogGroundTileManager, Log, TEXT("Attempted: %d, Success: %d, Failed: %d"), NumTilesToSpawn, SuccessCount, FailCount);
	UE_LOG(LogGroundTileManager, Log, TEXT("Active tiles: %d"), ActiveTiles.Num());
	if (TilePool)
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("Pool: Active=%d, Available=%d, Total=%d"),
			TilePool->GetActiveCount(), TilePool->GetAvailableCount(), TilePool->GetTotalPoolSize());
	}
}

AGroundTile* UGroundTileManager::SpawnTile(const FVector& Position)
{
	if (!TilePool)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("Tile pool not initialized"));
		return nullptr;
	}

	// Get tile from pool
	AActor* TileActor = TilePool->GetFromPool(Position, FRotator::ZeroRotator);
	if (!TileActor)
	{
		UE_LOG(LogGroundTileManager, Warning, TEXT("Failed to get tile from pool (pool exhausted)"));
		return nullptr;
	}

	// Cast to ground tile
	AGroundTile* Tile = Cast<AGroundTile>(TileActor);
	if (!Tile)
	{
		UE_LOG(LogGroundTileManager, Error, TEXT("Pooled actor is not a GroundTile"));
		return nullptr;
	}

	// Configure tile
	Tile->SetTileLength(TileSize);
	IPoolableActor::Execute_OnActivated(Tile);
	Tile->bShowDebugBounds = bShowDebugVisualization;

	// Apply mesh override from data table if configured
	if (!ConfiguredTileMesh.IsNull())
	{
		UStaticMesh* MeshToApply = ConfiguredTileMesh.LoadSynchronous();
		UStaticMeshComponent* MeshComponent = Tile->GetTileMesh();
		if (MeshToApply && MeshComponent)
		{
			MeshComponent->SetStaticMesh(MeshToApply);
			UE_LOG(LogGroundTileManager, Verbose, TEXT("Applied mesh override to tile: %s"), *MeshToApply->GetName());
		}
	}

	// Apply material override from data table if configured
	if (!ConfiguredTileMaterial.IsNull())
	{
		UMaterialInterface* MaterialToApply = ConfiguredTileMaterial.LoadSynchronous();
		UStaticMeshComponent* MeshComponent = Tile->GetTileMesh();
		if (MaterialToApply && MeshComponent)
		{
			MeshComponent->SetMaterial(0, MaterialToApply);
			UE_LOG(LogGroundTileManager, Verbose, TEXT("Applied material override to tile: %s"), *MaterialToApply->GetName());
		}
	}

	// Add to active tiles
	ActiveTiles.Add(Tile);

	return Tile;
}

void UGroundTileManager::RecycleTile(AGroundTile* Tile)
{
	if (!Tile || !TilePool)
	{
		return;
	}

	// Deactivate tile
	IPoolableActor::Execute_OnDeactivated(Tile);

	// Return to pool
	TilePool->ReturnToPool(Tile);

	UE_LOG(LogGroundTileManager, VeryVerbose, TEXT("Tile recycled"));
}

AActor* UGroundTileManager::GetWarRig()
{
	// Try to get war rig from player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		APawn* Pawn = PC->GetPawn();
		if (Pawn)
		{
			return Pawn;
		}
	}

	// Fallback: search for war rig by class name
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor->GetName().Contains(TEXT("WarRig")))
		{
			return Actor;
		}
	}

	UE_LOG(LogGroundTileManager, Warning, TEXT("War rig not found"));
	return nullptr;
}

UWorldScrollComponent* UGroundTileManager::GetWorldScrollComponent() const
{
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(
		UGameplayStatics::GetGameMode(this));

	if (!GameMode)
	{
		return nullptr;
	}

	return GameMode->WorldScrollComponent;
}

void UGroundTileManager::DrawDebugVisualization()
{
	UWorld* World = GetWorld();
	if (!World || !WarRig)
	{
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;

	// Draw spawn threshold
	const float SpawnX = WarRigX + TileSpawnDistance;
	DrawDebugLine(World,
		FVector(SpawnX, -2000.0f, 0.0f),
		FVector(SpawnX, 2000.0f, 0.0f),
		FColor::Green, false, -1.0f, 0, 5.0f);

	DrawDebugString(World, FVector(SpawnX, 0.0f, 200.0f),
		TEXT("Spawn Threshold"), nullptr, FColor::Green, 0.0f, true);

	// Draw despawn threshold
	const float DespawnX = WarRigX - TileDespawnDistance;
	DrawDebugLine(World,
		FVector(DespawnX, -2000.0f, 0.0f),
		FVector(DespawnX, 2000.0f, 0.0f),
		FColor::Red, false, -1.0f, 0, 5.0f);

	DrawDebugString(World, FVector(DespawnX, 0.0f, 200.0f),
		TEXT("Despawn Threshold"), nullptr, FColor::Red, 0.0f, true);

	// Draw tile count
	DrawDebugString(World, WarRig->GetActorLocation() + FVector(0.0f, 0.0f, 500.0f),
		FString::Printf(TEXT("Active Tiles: %d"), ActiveTiles.Num()),
		nullptr, FColor::White, 0.0f, true);
}

void UGroundTileManager::LogManagerState() const
{
	UE_LOG(LogGroundTileManager, Log, TEXT("=== Ground Tile Manager State ==="));
	UE_LOG(LogGroundTileManager, Log, TEXT("Active Tiles: %d"), ActiveTiles.Num());
	UE_LOG(LogGroundTileManager, Log, TEXT("Tile Size: %.0f"), TileSize);
	UE_LOG(LogGroundTileManager, Log, TEXT("Spawn Distance: %.0f"), TileSpawnDistance);
	UE_LOG(LogGroundTileManager, Log, TEXT("Despawn Distance: %.0f"), TileDespawnDistance);
	UE_LOG(LogGroundTileManager, Log, TEXT("Furthest Tile: %.0f"), GetFurthestTilePosition());
	UE_LOG(LogGroundTileManager, Log, TEXT("Debug Visualization: %s"), bShowDebugVisualization ? TEXT("ENABLED") : TEXT("DISABLED"));

	if (WarRig)
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("War Rig Position: %s"), *WarRig->GetActorLocation().ToString());
	}

	if (TilePool)
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("Pool - Active: %d, Available: %d, Total: %d"),
			TilePool->GetActiveCount(), TilePool->GetAvailableCount(), TilePool->GetTotalPoolSize());
	}

	// List all active tile positions
	if (ActiveTiles.Num() > 0)
	{
		UE_LOG(LogGroundTileManager, Log, TEXT("Active Tile Positions:"));
		for (int32 i = 0; i < ActiveTiles.Num(); ++i)
		{
			if (ActiveTiles[i])
			{
				FVector Pos = ActiveTiles[i]->GetActorLocation();
				UE_LOG(LogGroundTileManager, Log, TEXT("  [%d] X: %.0f, Y: %.0f, Z: %.0f"),
					i, Pos.X, Pos.Y, Pos.Z);
			}
		}
	}

	UE_LOG(LogGroundTileManager, Log, TEXT("=================================="));
	UE_LOG(LogGroundTileManager, Log, TEXT("TIP: Use 'DebugShowTiles' to toggle visual debug display"));
}
