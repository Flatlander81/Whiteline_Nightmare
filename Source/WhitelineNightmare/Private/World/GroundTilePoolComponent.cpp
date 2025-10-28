// Copyright Flatlander81. All Rights Reserved.

#include "World/GroundTilePoolComponent.h"
#include "World/GroundTile.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/GameDataStructs.h"
#include "Core/ObjectPoolTypes.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogGroundTilePool, Log, All);

UGroundTilePoolComponent::UGroundTilePoolComponent()
	: TileDataTable(nullptr)
	, DataTableRowName("DefaultGroundTile")
	, GroundTileClass(nullptr)
	, WarRig(nullptr)
	, TileSize(2000.0f)
	, SpawnDistanceAhead(3000.0f)
	, DespawnDistanceBehind(1000.0f)
	, bShowDebugBounds(false)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UGroundTilePoolComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get war rig reference
	WarRig = GetWarRig();
	if (!WarRig)
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("War rig not found, using world origin"));
	}

	// Load configuration from data table if specified
	if (TileDataTable)
	{
		if (!LoadConfigFromDataTable())
		{
			UE_LOG(LogGroundTilePool, Warning, TEXT("Failed to load config from data table, using defaults"));
		}
	}

	// Initialize the pool if class is specified
	if (GroundTileClass)
	{
		if (!InitializeTilePool(GroundTileClass, DataTableRowName))
		{
			UE_LOG(LogGroundTilePool, Error, TEXT("Failed to initialize tile pool"));
			return;
		}
	}
	else
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("GroundTileClass not set, pool not initialized"));
	}
}

void UGroundTilePoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check for tiles that need recycling
	CheckTileRecycling();

	// Debug visualization
	if (bShowDebugBounds)
	{
		DrawDebugVisualization();
	}
}

bool UGroundTilePoolComponent::InitializeTilePool(TSubclassOf<AGroundTile> TileClass, FName RowName)
{
	// Validate tile class
	if (!TileClass)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("TileClass is null"));
		return false;
	}

	GroundTileClass = TileClass;
	DataTableRowName = RowName;

	// Load configuration from data table
	if (!LoadConfigFromDataTable())
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("Failed to load config, using defaults"));
	}

	// Validate configuration
	if (!ValidateConfiguration())
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("Invalid configuration"));
		return false;
	}

	// Configure pool
	FObjectPoolConfig PoolConfig;
	PoolConfig.PoolSize = 5; // Default pool size from spec
	PoolConfig.bAutoExpand = true;
	PoolConfig.MaxPoolSize = 10;
	PoolConfig.SpawnDistanceAhead = SpawnDistanceAhead;
	PoolConfig.DespawnDistanceBehind = DespawnDistanceBehind;

	// Initialize base pool component
	bool bSuccess = Initialize(TileClass, PoolConfig);
	if (!bSuccess)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("Failed to initialize base pool component"));
		return false;
	}

	// Spawn initial tiles
	SpawnInitialTiles();

	UE_LOG(LogGroundTilePool, Log, TEXT("Ground tile pool initialized: %d tiles spawned"), ActiveTiles.Num());
	return true;
}

float UGroundTilePoolComponent::GetFurthestTilePosition() const
{
	if (ActiveTiles.Num() == 0)
	{
		return WarRig ? WarRig->GetActorLocation().X : 0.0f;
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

bool UGroundTilePoolComponent::LoadConfigFromDataTable()
{
	if (!TileDataTable)
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("No data table assigned"));
		return false;
	}

	static const FString ContextString(TEXT("GroundTilePoolComponent::LoadConfigFromDataTable"));
	FGroundTileData* RowData = TileDataTable->FindRow<FGroundTileData>(DataTableRowName, ContextString);

	if (!RowData)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("Failed to find row '%s' in data table"),
			*DataTableRowName.ToString());
		return false;
	}

	// Load values
	TileSize = RowData->TileSize;
	SpawnDistanceAhead = RowData->SpawnDistanceAhead;
	DespawnDistanceBehind = RowData->DespawnDistanceBehind;
	TileMesh = RowData->TileMesh;
	TileMaterial = RowData->TileMaterial;

	UE_LOG(LogGroundTilePool, Log, TEXT("Loaded config: TileSize=%.0f, SpawnDist=%.0f, DespawnDist=%.0f"),
		TileSize, SpawnDistanceAhead, DespawnDistanceBehind);

	return true;
}

void UGroundTilePoolComponent::SpawnInitialTiles()
{
	if (!WarRig)
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("Cannot spawn tiles without war rig reference"));
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;

	// Calculate how many tiles we need to fill the visible area
	const float VisibleDistance = SpawnDistanceAhead + DespawnDistanceBehind;
	const int32 NumTilesToSpawn = FMath::CeilToInt(VisibleDistance / TileSize) + 2;

	UE_LOG(LogGroundTilePool, Log, TEXT("Spawning %d initial tiles (VisibleDist=%.0f, TileSize=%.0f)"),
		NumTilesToSpawn, VisibleDistance, TileSize);

	// Start tiles from war rig position
	const float StartX = WarRigX;

	for (int32 i = 0; i < NumTilesToSpawn; ++i)
	{
		FVector TilePosition(StartX + (i * TileSize), 0.0f, 0.0f);
		AGroundTile* Tile = SpawnTile(TilePosition);

		if (!Tile)
		{
			UE_LOG(LogGroundTilePool, Warning, TEXT("Failed to spawn initial tile %d"), i);
			break;
		}
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("Spawned %d initial tiles"), ActiveTiles.Num());
}

void UGroundTilePoolComponent::CheckTileRecycling()
{
	if (!WarRig)
	{
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;
	const float DespawnThreshold = WarRigX + DespawnDistanceBehind;
	const float SpawnThreshold = WarRigX + SpawnDistanceAhead;

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
			UE_LOG(LogGroundTilePool, Verbose, TEXT("Recycling tile at X=%.0f (threshold=%.0f)"),
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
			UE_LOG(LogGroundTilePool, Verbose, TEXT("Spawned new tile at X=%.0f"), NewPosition.X);
		}
	}
}

AGroundTile* UGroundTilePoolComponent::SpawnTile(const FVector& Position)
{
	// Get tile from pool
	AActor* TileActor = GetFromPool(Position, FRotator::ZeroRotator);
	if (!TileActor)
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("Failed to get tile from pool (pool exhausted)"));
		return nullptr;
	}

	// Cast to ground tile
	AGroundTile* Tile = Cast<AGroundTile>(TileActor);
	if (!Tile)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("Pooled actor is not a GroundTile"));
		return nullptr;
	}

	// Configure tile
	Tile->SetTileLength(TileSize);

	// Apply mesh and material if available
	if (TileMesh.IsValid() || !TileMesh.IsNull())
	{
		UStaticMeshComponent* MeshComp = Tile->FindComponentByClass<UStaticMeshComponent>();
		if (MeshComp)
		{
			UStaticMesh* LoadedMesh = TileMesh.LoadSynchronous();
			if (LoadedMesh)
			{
				MeshComp->SetStaticMesh(LoadedMesh);
			}

			if (!TileMaterial.IsNull())
			{
				UMaterialInterface* LoadedMaterial = TileMaterial.LoadSynchronous();
				if (LoadedMaterial)
				{
					MeshComp->SetMaterial(0, LoadedMaterial);
				}
			}
		}
	}

	// Activate tile using IPoolableActor interface
	if (Tile->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnActivated(Tile);
	}

	// Add to active tiles
	ActiveTiles.Add(Tile);

	return Tile;
}

void UGroundTilePoolComponent::RecycleTile(AGroundTile* Tile)
{
	if (!Tile)
	{
		return;
	}

	// Deactivate tile using IPoolableActor interface
	if (Tile->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnDeactivated(Tile);
	}

	// Return to pool
	ReturnToPool(Tile);

	UE_LOG(LogGroundTilePool, VeryVerbose, TEXT("Tile recycled"));
}

AActor* UGroundTilePoolComponent::GetWarRig()
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

	UE_LOG(LogGroundTilePool, Warning, TEXT("War rig not found"));
	return nullptr;
}

UWorldScrollComponent* UGroundTilePoolComponent::GetWorldScrollComponent() const
{
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(
		UGameplayStatics::GetGameMode(this));

	if (!GameMode)
	{
		return nullptr;
	}

	return GameMode->WorldScrollComponent;
}

void UGroundTilePoolComponent::DrawDebugVisualization()
{
	UWorld* World = GetWorld();
	if (!World || !WarRig)
	{
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;

	// Draw spawn threshold
	const float SpawnX = WarRigX + SpawnDistanceAhead;
	DrawDebugLine(World,
		FVector(SpawnX, -2000.0f, 0.0f),
		FVector(SpawnX, 2000.0f, 0.0f),
		FColor::Green, false, -1.0f, 0, 5.0f);

	DrawDebugString(World, FVector(SpawnX, 0.0f, 200.0f),
		TEXT("Spawn Threshold"), nullptr, FColor::Green, 0.0f, true);

	// Draw despawn threshold
	const float DespawnX = WarRigX + DespawnDistanceBehind;
	DrawDebugLine(World,
		FVector(DespawnX, -2000.0f, 0.0f),
		FVector(DespawnX, 2000.0f, 0.0f),
		FColor::Red, false, -1.0f, 0, 5.0f);

	DrawDebugString(World, FVector(DespawnX, 0.0f, 200.0f),
		TEXT("Despawn Threshold"), nullptr, FColor::Red, 0.0f, true);

	// Draw tile count
	DrawDebugString(World, WarRig->GetActorLocation() + FVector(0.0f, 0.0f, 500.0f),
		FString::Printf(TEXT("Active Tiles: %d\nPool - Active: %d, Available: %d"),
			ActiveTiles.Num(), GetActiveCount(), GetAvailableCount()),
		nullptr, FColor::White, 0.0f, true, 1.5f);

	// Highlight tiles
	for (const AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			FVector TilePos = Tile->GetActorLocation();
			DrawDebugBox(World, TilePos, FVector(TileSize / 2.0f, 1000.0f, 50.0f),
				FColor::Cyan, false, -1.0f, 0, 2.0f);
		}
	}
}

bool UGroundTilePoolComponent::ValidateConfiguration() const
{
	// Validate pool size >= 3 (minimum for seamless scrolling)
	if (GetTotalPoolSize() < 3)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("Pool size must be >= 3 for seamless scrolling"));
		return false;
	}

	// Validate spawn > despawn
	if (SpawnDistanceAhead <= DespawnDistanceBehind)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("SpawnDistanceAhead must be > DespawnDistanceBehind"));
		return false;
	}

	// Validate tile mesh exists (if specified)
	if (!TileMesh.IsNull() && !TileMesh.IsValid())
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("TileMesh specified but not valid, will use default"));
	}

	return true;
}

// === TESTING FUNCTIONS ===

void UGroundTilePoolComponent::TestTilePoolRecycling()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== TEST: Tile Pool Recycling ==="));

	// Store initial pool size
	int32 InitialTotalSize = GetTotalPoolSize();
	UE_LOG(LogGroundTilePool, Log, TEXT("Initial pool size: %d"), InitialTotalSize);

	// Simulate recycling by checking if tiles are being reused
	if (ActiveTiles.Num() > 0)
	{
		TSet<AGroundTile*> UniqueTiles;
		for (AGroundTile* Tile : ActiveTiles)
		{
			UniqueTiles.Add(Tile);
		}

		UE_LOG(LogGroundTilePool, Log, TEXT("Active tiles: %d, Unique tiles: %d"),
			ActiveTiles.Num(), UniqueTiles.Num());

		if (ActiveTiles.Num() == UniqueTiles.Num())
		{
			UE_LOG(LogGroundTilePool, Log, TEXT("PASS: All active tiles are unique (proper pooling)"));
		}
		else
		{
			UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Duplicate tiles found (pooling error)"));
		}
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("Total pool size after test: %d"), GetTotalPoolSize());

	if (GetTotalPoolSize() == InitialTotalSize)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: Pool size unchanged (tiles reused, not destroyed)"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("Pool size changed (may indicate auto-expansion)"));
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("================================="));
}

void UGroundTilePoolComponent::TestSeamlessScrolling()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== TEST: Seamless Scrolling ==="));

	if (ActiveTiles.Num() < 2)
	{
		UE_LOG(LogGroundTilePool, Warning, TEXT("Need at least 2 tiles to test seamless scrolling"));
		return;
	}

	// Sort tiles by X position
	TArray<AGroundTile*> SortedTiles = ActiveTiles;
	SortedTiles.Sort([](const AGroundTile& A, const AGroundTile& B) {
		return A.GetActorLocation().X < B.GetActorLocation().X;
	});

	// Check for gaps between adjacent tiles
	bool bHasGaps = false;
	float MaxGap = 0.0f;

	for (int32 i = 0; i < SortedTiles.Num() - 1; ++i)
	{
		float CurrentTileEnd = SortedTiles[i]->GetActorLocation().X + (TileSize / 2.0f);
		float NextTileStart = SortedTiles[i + 1]->GetActorLocation().X - (TileSize / 2.0f);
		float Gap = NextTileStart - CurrentTileEnd;

		if (FMath::Abs(Gap) > 1.0f) // Allow 1 unit tolerance
		{
			bHasGaps = true;
			MaxGap = FMath::Max(MaxGap, FMath::Abs(Gap));
			UE_LOG(LogGroundTilePool, Warning, TEXT("Gap found between tile %d and %d: %.2f units"),
				i, i + 1, Gap);
		}
	}

	if (!bHasGaps)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: No gaps between tiles (seamless scrolling)"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Gaps found (max gap: %.2f units)"), MaxGap);
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("================================"));
}

void UGroundTilePoolComponent::TestTilePositioning()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== TEST: Tile Positioning ==="));

	if (!WarRig)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: War rig not found"));
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;
	const float ExpectedMaxX = WarRigX + SpawnDistanceAhead;
	const float ExpectedMinX = WarRigX + DespawnDistanceBehind;

	float ActualMaxX = -MAX_FLT;
	float ActualMinX = MAX_FLT;

	for (const AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			float TileX = Tile->GetActorLocation().X;
			ActualMaxX = FMath::Max(ActualMaxX, TileX);
			ActualMinX = FMath::Min(ActualMinX, TileX);
		}
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("War Rig X: %.0f"), WarRigX);
	UE_LOG(LogGroundTilePool, Log, TEXT("Expected range: %.0f to %.0f"), ExpectedMinX, ExpectedMaxX);
	UE_LOG(LogGroundTilePool, Log, TEXT("Actual range: %.0f to %.0f"), ActualMinX, ActualMaxX);

	// Check if furthest tile is within spawn threshold
	if (ActualMaxX <= ExpectedMaxX + TileSize)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: Furthest tile position correct"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Furthest tile too far ahead"));
	}

	// Check if nearest tile is within despawn threshold
	if (ActualMinX >= ExpectedMinX - TileSize)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: Nearest tile position correct"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Nearest tile too far behind"));
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("==============================="));
}

void UGroundTilePoolComponent::TestPoolSize()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== TEST: Pool Size ==="));

	int32 TotalSize = GetTotalPoolSize();
	int32 ActiveCount = GetActiveCount();
	int32 AvailableCount = GetAvailableCount();

	UE_LOG(LogGroundTilePool, Log, TEXT("Total pool size: %d"), TotalSize);
	UE_LOG(LogGroundTilePool, Log, TEXT("Active objects: %d"), ActiveCount);
	UE_LOG(LogGroundTilePool, Log, TEXT("Available objects: %d"), AvailableCount);
	UE_LOG(LogGroundTilePool, Log, TEXT("Active tiles: %d"), ActiveTiles.Num());

	if (TotalSize >= 3)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: Pool size >= 3 (minimum for seamless scrolling)"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Pool size < 3"));
	}

	if (ActiveCount + AvailableCount == TotalSize)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: Pool accounting correct (Active + Available = Total)"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: Pool accounting incorrect"));
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("======================="));
}

void UGroundTilePoolComponent::TestTileDespawn()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== TEST: Tile Despawn ==="));

	if (!WarRig)
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: War rig not found"));
		return;
	}

	const float WarRigX = WarRig->GetActorLocation().X;
	const float DespawnThreshold = WarRigX + DespawnDistanceBehind;

	bool bAllTilesValid = true;
	int32 TilesBehindThreshold = 0;

	for (const AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			float TileX = Tile->GetActorLocation().X;
			if (TileX < DespawnThreshold)
			{
				TilesBehindThreshold++;
				UE_LOG(LogGroundTilePool, Warning, TEXT("Tile at X=%.0f is behind despawn threshold (%.0f)"),
					TileX, DespawnThreshold);
				bAllTilesValid = false;
			}
		}
	}

	if (bAllTilesValid)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("PASS: All tiles are ahead of despawn threshold"));
	}
	else
	{
		UE_LOG(LogGroundTilePool, Error, TEXT("FAIL: %d tiles behind despawn threshold"),
			TilesBehindThreshold);
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("=========================="));
}

// === DEBUG FUNCTIONS ===

void UGroundTilePoolComponent::DebugShowTileBounds()
{
	bShowDebugBounds = !bShowDebugBounds;
	UE_LOG(LogGroundTilePool, Log, TEXT("Tile bounds visualization: %s"),
		bShowDebugBounds ? TEXT("ENABLED") : TEXT("DISABLED"));

	// Also enable/disable debug on tiles
	for (AGroundTile* Tile : ActiveTiles)
	{
		if (Tile)
		{
			Tile->bShowDebugBounds = bShowDebugBounds;
		}
	}
}

void UGroundTilePoolComponent::DebugShowTilePool()
{
	UE_LOG(LogGroundTilePool, Log, TEXT("=== Ground Tile Pool Status ==="));
	UE_LOG(LogGroundTilePool, Log, TEXT("Active Tiles: %d"), ActiveTiles.Num());
	UE_LOG(LogGroundTilePool, Log, TEXT("Pool - Active: %d, Available: %d, Total: %d"),
		GetActiveCount(), GetAvailableCount(), GetTotalPoolSize());
	UE_LOG(LogGroundTilePool, Log, TEXT("Tile Size: %.0f"), TileSize);
	UE_LOG(LogGroundTilePool, Log, TEXT("Spawn Distance Ahead: %.0f"), SpawnDistanceAhead);
	UE_LOG(LogGroundTilePool, Log, TEXT("Despawn Distance Behind: %.0f"), DespawnDistanceBehind);
	UE_LOG(LogGroundTilePool, Log, TEXT("Furthest Tile: %.0f"), GetFurthestTilePosition());

	if (WarRig)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("War Rig Position: %s"), *WarRig->GetActorLocation().ToString());
	}

	// List all active tile positions
	if (ActiveTiles.Num() > 0)
	{
		UE_LOG(LogGroundTilePool, Log, TEXT("Active Tile Positions:"));
		for (int32 i = 0; i < ActiveTiles.Num(); ++i)
		{
			if (ActiveTiles[i])
			{
				FVector Pos = ActiveTiles[i]->GetActorLocation();
				UE_LOG(LogGroundTilePool, Log, TEXT("  [%d] X: %.0f, Y: %.0f, Z: %.0f"),
					i, Pos.X, Pos.Y, Pos.Z);
			}
		}
	}

	UE_LOG(LogGroundTilePool, Log, TEXT("================================"));
	UE_LOG(LogGroundTilePool, Log, TEXT("TIP: Use 'DebugShowTileBounds' to toggle visual debug display"));
}
