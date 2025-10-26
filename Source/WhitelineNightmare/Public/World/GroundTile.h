// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundTile.generated.h"

/**
 * Ground Tile Actor
 * Represents a section of scrolling ground that moves past the war rig
 * Managed by the WorldScrollManager for pooling and reuse
 */
UCLASS()
class WHITELINENIGHTMARE_API AGroundTile : public AActor
{
	GENERATED_BODY()

public:
	AGroundTile();

	/** Reset tile to default state when reused from pool */
	UFUNCTION(BlueprintCallable, Category = "Ground Tile")
	virtual void ResetTile();

protected:
	virtual void BeginPlay() override;

	/** Static mesh component for the tile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TileMesh;

	/** Optional lanes markers or visual guides */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground Tile")
	bool bShowLaneMarkers;
};
