// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/GroundTile.h"
#include "Components/StaticMeshComponent.h"

AGroundTile::AGroundTile()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create tile mesh component
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// Default settings
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	TileMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	bShowLaneMarkers = false;
}

void AGroundTile::BeginPlay()
{
	Super::BeginPlay();
}

void AGroundTile::ResetTile()
{
	// Reset any state when tile is reused from pool
	// Override in subclasses if needed
}
