// Copyright Flatlander81. All Rights Reserved.

#include "World/GroundTile.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AGroundTile::AGroundTile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false; // Disable by default, enable when active

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create tile mesh component
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(RootComponent);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TileMesh->SetCastShadow(false); // Optimize for performance

	// Default tile size (2000x2000)
	TileSize = FVector2D(2000.0f, 2000.0f);

	// Try to load a default plane mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMeshAsset.Succeeded())
	{
		TileMesh->SetStaticMesh(PlaneMeshAsset.Object);

		// Scale the plane to match tile size
		// Engine plane is 100x100, so scale to achieve desired size
		float ScaleX = TileSize.X / 100.0f;
		float ScaleY = TileSize.Y / 100.0f;
		TileMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
	}

	// Default material (grey/brown road color)
	TileMaterial = nullptr;
}

void AGroundTile::BeginPlay()
{
	Super::BeginPlay();
}

void AGroundTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Note: Position updates are handled externally by GroundTilePoolComponent
	// This Tick is kept for potential future per-tile logic
}

void AGroundTile::SetTileSize(FVector2D NewSize)
{
	if (NewSize.X <= 0.0f || NewSize.Y <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("GroundTile: Invalid tile size (%.2f, %.2f). Must be positive."),
			NewSize.X, NewSize.Y);
		return;
	}

	TileSize = NewSize;

	// Update mesh scale
	if (TileMesh && TileMesh->GetStaticMesh())
	{
		// Engine plane is 100x100
		float ScaleX = TileSize.X / 100.0f;
		float ScaleY = TileSize.Y / 100.0f;
		TileMesh->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));
	}

	UE_LOG(LogTemp, Log, TEXT("GroundTile: Tile size set to (%.2f, %.2f)."), TileSize.X, TileSize.Y);
}

void AGroundTile::UpdatePosition(float DeltaTime, FVector ScrollVelocity)
{
	FVector NewLocation = GetActorLocation() + (ScrollVelocity * DeltaTime);
	SetActorLocation(NewLocation);
}

void AGroundTile::OnActivated_Implementation()
{
	// Show the tile
	SetActorHiddenInGame(false);
	SetActorEnableCollision(false); // No collision needed for ground tiles
	SetActorTickEnabled(true);

	UE_LOG(LogTemp, Verbose, TEXT("GroundTile: Activated at location %s."), *GetActorLocation().ToString());
}

void AGroundTile::OnDeactivated_Implementation()
{
	// Hide the tile
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	UE_LOG(LogTemp, Verbose, TEXT("GroundTile: Deactivated."));
}

void AGroundTile::ResetState_Implementation()
{
	// Reset any runtime state
	// For ground tiles, this is minimal since they don't have complex state
	SetActorRotation(FRotator::ZeroRotator);

	UE_LOG(LogTemp, Verbose, TEXT("GroundTile: State reset."));
}
