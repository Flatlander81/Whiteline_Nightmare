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

	// Draw tile bounds
	FVector TileCenter = GetActorLocation();
	FVector BoxExtent(TileLength / 2.0f, 1000.0f, 50.0f); // Assume width 2000, height 100

	DrawDebugBox(World, TileCenter, BoxExtent, FColor::Green, false, -1.0f, 0, 2.0f);

	// Draw position text
	DrawDebugString(World, TileCenter + FVector(0, 0, 100),
		FString::Printf(TEXT("Tile X: %.0f"), TileCenter.X),
		nullptr, FColor::White, 0.0f, true);
}
