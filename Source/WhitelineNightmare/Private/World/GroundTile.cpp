// Copyright Flatlander81. All Rights Reserved.

#include "World/GroundTile.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogGroundTile, Log, All);

AGroundTile::AGroundTile()
	: bShowDebugBounds(false)
	, TileLength(2000.0f)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create tile mesh
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(RootComponent);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void AGroundTile::BeginPlay()
{
	Super::BeginPlay();

	// Initial state: deactivated (will be activated by pool manager)
	OnDeactivated();
}

void AGroundTile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update position based on scroll velocity
	UpdateScrollPosition(DeltaTime);

	// Debug visualization
	if (bShowDebugBounds)
	{
		DrawDebugInfo();
	}
}

void AGroundTile::OnActivated()
{
	// Make visible
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	UE_LOG(LogGroundTile, Verbose, TEXT("GroundTile activated at: %s"), *GetActorLocation().ToString());
}

void AGroundTile::OnDeactivated()
{
	// Hide and disable
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	UE_LOG(LogGroundTile, Verbose, TEXT("GroundTile deactivated"));
}

void AGroundTile::ResetState()
{
	// Reset to default state
	SetActorLocation(FVector::ZeroVector);
	SetActorRotation(FRotator::ZeroRotator);
	OnDeactivated();

	UE_LOG(LogGroundTile, Verbose, TEXT("GroundTile state reset"));
}

void AGroundTile::UpdateScrollPosition(float DeltaTime)
{
	// Get scroll component
	UWorldScrollComponent* ScrollComponent = GetWorldScrollComponent();
	if (!ScrollComponent)
	{
		UE_LOG(LogGroundTile, Warning, TEXT("UpdateScrollPosition: WorldScrollComponent not found"));
		return;
	}

	// Get scroll velocity
	FVector ScrollVelocity = ScrollComponent->GetScrollVelocity();

	// Move tile by scroll velocity
	FVector DeltaLocation = ScrollVelocity * DeltaTime;
	AddActorWorldOffset(DeltaLocation);

	UE_LOG(LogGroundTile, VeryVerbose, TEXT("Tile position: %s, Velocity: %s"),
		*GetActorLocation().ToString(), *ScrollVelocity.ToString());
}

UWorldScrollComponent* AGroundTile::GetWorldScrollComponent() const
{
	// Get game mode
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(
		UGameplayStatics::GetGameMode(this));

	if (!GameMode)
	{
		return nullptr;
	}

	return GameMode->WorldScrollComponent;
}

void AGroundTile::DrawDebugInfo()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Draw tile bounds as a box
	FVector TileCenter = GetActorLocation();
	FVector BoxExtent(TileLength / 2.0f, 1000.0f, 50.0f); // Half-extents: length/2, width 1000, height 50

	// Draw persistent box (visible each frame)
	DrawDebugBox(World, TileCenter, BoxExtent, FColor::Green, false, 0.0f, 0, 3.0f);

	// Draw a cross at the tile center for easy visibility
	DrawDebugCrosshairs(World, TileCenter, FRotator::ZeroRotator, 200.0f, FColor::Yellow, false, 0.0f, 0, 2.0f);

	// Draw position text above the tile
	DrawDebugString(World, TileCenter + FVector(0, 0, 150),
		FString::Printf(TEXT("Tile\nX: %.0f\nLength: %.0f"), TileCenter.X, TileLength),
		nullptr, FColor::White, 0.0f, true, 1.5f);

	// Draw line showing scroll direction
	FVector ScrollEnd = TileCenter + FVector(-500.0f, 0.0f, 0.0f);
	DrawDebugDirectionalArrow(World, TileCenter, ScrollEnd, 50.0f, FColor::Cyan, false, 0.0f, 0, 2.0f);
}
