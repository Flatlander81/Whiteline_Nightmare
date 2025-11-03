// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ObjectPoolComponent.h"
#include "PickupPoolComponent.generated.h"

// Forward declarations
class AFuelPickup;
class AWarRigPawn;
class UWorldScrollComponent;

/**
 * UPickupPoolComponent - Specialized object pool for fuel and scrap pickups
 *
 * Extends UObjectPoolComponent with pickup-specific functionality:
 * - Automatic despawning when pickups pass behind war rig
 * - Lane-based spawning
 * - Integration with world scroll system
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UPickupPoolComponent : public UObjectPoolComponent
{
	GENERATED_BODY()

public:
	UPickupPoolComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Initialize the pickup pool
	 * @param WarRig - Reference to the war rig pawn
	 * @param ScrollComponent - Reference to the world scroll component
	 * @param PickupClass - Class of pickup actors to pool
	 * @param PoolSize - Number of pickups to pre-spawn
	 * @return True if initialization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Pickup Pool")
	bool InitializePickupPool(AWarRigPawn* WarRig, UWorldScrollComponent* ScrollComponent,
		TSubclassOf<AFuelPickup> PickupClass, int32 PoolSize = 20);

	/**
	 * Spawn a pickup in a specific lane ahead of the war rig
	 * @param LaneIndex - Index of the lane (0-4 for 5 lanes)
	 * @return The spawned pickup actor, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Pickup Pool")
	AFuelPickup* SpawnPickupInLane(int32 LaneIndex);

	/**
	 * Spawn a pickup in a random lane ahead of the war rig
	 * @return The spawned pickup actor, or nullptr if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Pickup Pool")
	AFuelPickup* SpawnPickupInRandomLane();

	/**
	 * Get the number of active pickups in the pool
	 */
	UFUNCTION(BlueprintPure, Category = "Pickup Pool")
	int32 GetActivePickupCount() const { return GetActiveCount(); }

	/**
	 * Get the number of available pickups in the pool
	 */
	UFUNCTION(BlueprintPure, Category = "Pickup Pool")
	int32 GetAvailablePickupCount() const { return GetAvailableCount(); }

protected:
	/** Reference to the war rig pawn */
	UPROPERTY()
	TObjectPtr<AWarRigPawn> WarRigPawn;

	/** Reference to world scroll component */
	UPROPERTY()
	TObjectPtr<UWorldScrollComponent> WorldScrollComponent;

	/** Distance ahead of war rig where pickups spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Pool|Spawning")
	float SpawnDistanceAhead;

	/** Distance behind war rig where pickups despawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Pool|Despawning")
	float DespawnDistanceBehind;

	/** Lane Y-axis positions (default: -400, -200, 0, 200, 400) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Pool|Spawning")
	TArray<float> LaneYPositions;

	/** Z position for pickup spawning (ground level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Pool|Spawning")
	float SpawnHeight;

	/**
	 * Check active pickups and despawn those that have passed behind the war rig
	 */
	void CheckAndDespawnPickups();

	/**
	 * Get spawn location for a pickup in a specific lane
	 * @param LaneIndex - Index of the lane
	 * @return Spawn location in world space
	 */
	FVector GetSpawnLocationForLane(int32 LaneIndex) const;

	// Debug visualization functions (bShowDebugVisualization inherited from UObjectPoolComponent)
#if !UE_BUILD_SHIPPING
	/** Draw debug visualization for pickups and spawn/despawn boundaries */
	void DrawDebugVisualization() const;

	/** Console command: Spawn pickup in specific lane for testing */
	static void DebugSpawnFuelPickup(const TArray<FString>& Args);

	/** Console command: Toggle pickup visualization */
	static void DebugShowPickups(const TArray<FString>& Args);

	/** Console command: Display pool statistics */
	static void DebugShowPickupPool(const TArray<FString>& Args);

	/** Static reference to active instance for console commands */
	static UPickupPoolComponent* DebugInstance;
#endif
};
