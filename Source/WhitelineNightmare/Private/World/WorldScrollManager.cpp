// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/WorldScrollManager.h"
#include "World/GroundTile.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Data/GameplayDataStructs.h"
#include "Kismet/GameplayStatics.h"

AWorldScrollManager::AWorldScrollManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default values
	BaseScrollSpeed = 1000.0f;
	CurrentScrollSpeed = 1000.0f;
	ScrollSpeedMultiplier = 1.0f;
	bIsScrolling = false;
	bIsPaused = false;
	DistanceTraveled = 0.0f;
	TileSize = 2000.0f;
	InitialTileCount = 10;
	SpawnDistance = 5000.0f;
	DespawnDistance = 2000.0f;
	WarRigReference = nullptr;
	NextTilePosition = FVector::ZeroVector;
}

void AWorldScrollManager::BeginPlay()
{
	Super::BeginPlay();

	LoadScrollSettings();
	InitializeTiles();

	// Auto-start scrolling
	StartScrolling();
}

void AWorldScrollManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsScrolling && !bIsPaused)
	{
		UpdateTiles(DeltaTime);

		// Update distance traveled
		const float DeltaDistance = CurrentScrollSpeed * DeltaTime;
		UpdateDistance(DeltaDistance);
	}
}

void AWorldScrollManager::StartScrolling()
{
	bIsScrolling = true;
	bIsPaused = false;
	UE_LOG(LogTemp, Log, TEXT("World scrolling started at speed %.2f"), CurrentScrollSpeed);
}

void AWorldScrollManager::StopScrolling()
{
	bIsScrolling = false;
	UE_LOG(LogTemp, Log, TEXT("World scrolling stopped"));
}

void AWorldScrollManager::PauseScrolling()
{
	bIsPaused = true;
	UE_LOG(LogTemp, Log, TEXT("World scrolling paused"));
}

void AWorldScrollManager::ResumeScrolling()
{
	bIsPaused = false;
	UE_LOG(LogTemp, Log, TEXT("World scrolling resumed"));
}

void AWorldScrollManager::SetScrollSpeedMultiplier(float Multiplier)
{
	ScrollSpeedMultiplier = FMath::Clamp(Multiplier, 0.1f, 5.0f);
	CurrentScrollSpeed = BaseScrollSpeed * ScrollSpeedMultiplier;
	UE_LOG(LogTemp, Log, TEXT("Scroll speed multiplier set to %.2f (speed: %.2f)"), ScrollSpeedMultiplier, CurrentScrollSpeed);
}

float AWorldScrollManager::GetSpawnDistance() const
{
	return SpawnDistance;
}

float AWorldScrollManager::GetDespawnDistance() const
{
	return DespawnDistance;
}

void AWorldScrollManager::InitializeTiles()
{
	if (!GroundTileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("GroundTileClass is not set in WorldScrollManager!"));
		return;
	}

	// Find war rig if not set
	if (!WarRigReference)
	{
		// For now, use world origin as reference
		// TODO: Find actual war rig pawn
		NextTilePosition = FVector(0, 0, 0);
	}
	else
	{
		NextTilePosition = WarRigReference->GetActorLocation();
	}

	// Spawn initial tiles
	// Start tiles behind the war rig and extend forward
	const float StartOffset = -TileSize * 2.0f; // Start 2 tiles behind
	NextTilePosition.X = StartOffset;

	for (int32 i = 0; i < InitialTileCount; ++i)
	{
		SpawnTileAhead();
	}

	UE_LOG(LogTemp, Log, TEXT("Initialized %d ground tiles"), ActiveTiles.Num());
}

void AWorldScrollManager::UpdateTiles(float DeltaTime)
{
	const float ScrollDelta = CurrentScrollSpeed * DeltaTime;
	FVector WarRigLocation = WarRigReference ? WarRigReference->GetActorLocation() : FVector::ZeroVector;

	// Move all active tiles backward (toward negative X)
	TArray<AGroundTile*> TilesToRecycle;

	for (AGroundTile* Tile : ActiveTiles)
	{
		if (!Tile)
		{
			continue;
		}

		// Move tile backward
		FVector NewLocation = Tile->GetActorLocation();
		NewLocation.X -= ScrollDelta;
		Tile->SetActorLocation(NewLocation);

		// Check if tile has moved too far behind the war rig
		const float DistanceBehindRig = WarRigLocation.X - NewLocation.X;
		if (DistanceBehindRig > DespawnDistance)
		{
			TilesToRecycle.Add(Tile);
		}
	}

	// Recycle tiles that went too far back
	for (AGroundTile* Tile : TilesToRecycle)
	{
		ActiveTiles.Remove(Tile);
		ReturnTileToPool(Tile);
	}

	// Spawn new tiles ahead if needed
	if (ActiveTiles.Num() > 0)
	{
		// Find the furthest forward tile
		float FurthestX = -99999.0f;
		for (AGroundTile* Tile : ActiveTiles)
		{
			if (Tile && Tile->GetActorLocation().X > FurthestX)
			{
				FurthestX = Tile->GetActorLocation().X;
			}
		}

		// Spawn new tiles if the furthest is not far enough ahead
		const float DistanceAheadOfRig = FurthestX - WarRigLocation.X;
		if (DistanceAheadOfRig < SpawnDistance)
		{
			NextTilePosition.X = FurthestX + TileSize;
			SpawnTileAhead();
		}
	}
}

AGroundTile* AWorldScrollManager::GetTileFromPool()
{
	if (TilePool.Num() > 0)
	{
		AGroundTile* Tile = TilePool.Pop();
		return Tile;
	}

	// No tiles in pool, create a new one
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGroundTile* NewTile = GetWorld()->SpawnActor<AGroundTile>(GroundTileClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	return NewTile;
}

void AWorldScrollManager::ReturnTileToPool(AGroundTile* Tile)
{
	if (!Tile)
	{
		return;
	}

	// Deactivate tile
	Tile->SetActorHiddenInGame(true);
	Tile->SetActorEnableCollision(false);
	Tile->SetActorTickEnabled(false);

	TilePool.Add(Tile);
}

void AWorldScrollManager::SpawnTileAhead()
{
	AGroundTile* Tile = GetTileFromPool();
	if (!Tile)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get tile from pool!"));
		return;
	}

	// Position the tile
	Tile->SetActorLocation(NextTilePosition);
	Tile->SetActorRotation(FRotator::ZeroRotator);

	// Activate tile
	Tile->SetActorHiddenInGame(false);
	Tile->SetActorEnableCollision(true);
	Tile->SetActorTickEnabled(true);

	ActiveTiles.Add(Tile);

	// Update next position
	NextTilePosition.X += TileSize;
}

void AWorldScrollManager::UpdateDistance(float DeltaDistance)
{
	DistanceTraveled += DeltaDistance;

	// Notify game mode of distance update
	if (AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GameMode->UpdateDistanceTraveled(DeltaDistance);
	}
}

void AWorldScrollManager::LoadScrollSettings()
{
	if (AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		if (UDataTable* WorldScrollTable = GameMode->GetWorldScrollDataTable())
		{
			static const FString ContextString(TEXT("LoadScrollSettings"));
			TArray<FName> RowNames = WorldScrollTable->GetRowNames();

			if (RowNames.Num() > 0)
			{
				if (const FWorldScrollData* ScrollData = WorldScrollTable->FindRow<FWorldScrollData>(RowNames[0], ContextString))
				{
					BaseScrollSpeed = ScrollData->ScrollSpeed;
					CurrentScrollSpeed = BaseScrollSpeed * ScrollSpeedMultiplier;
					TileSize = ScrollData->TileSize;
					SpawnDistance = ScrollData->SpawnDistance;
					DespawnDistance = ScrollData->DespawnDistance;
					InitialTileCount = ScrollData->TilePoolSize;

					UE_LOG(LogTemp, Log, TEXT("Loaded scroll settings: Speed=%.2f, TileSize=%.2f"), BaseScrollSpeed, TileSize);
				}
			}
		}

		// Also load gameplay balance for base speed
		FGameplayBalanceData BalanceData;
		if (GameMode->GetGameplayBalanceData(BalanceData))
		{
			BaseScrollSpeed = BalanceData.BaseScrollSpeed;
			CurrentScrollSpeed = BaseScrollSpeed * ScrollSpeedMultiplier;
		}
	}
}
