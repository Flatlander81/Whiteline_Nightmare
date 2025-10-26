// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolTypes.h"
#include "ObjectPoolComponent.generated.h"

/**
 * Object Pool Component - Manages a pool of reusable actors
 *
 * This component provides efficient object pooling for frequently spawned/despawned actors.
 * Instead of destroying and creating new actors, it recycles them from a pool.
 *
 * Usage:
 * 1. Add component to your manager actor
 * 2. Call Initialize() with the actor class and config
 * 3. Use GetFromPool() to get an actor when needed
 * 4. Use ReturnToPool() when done with the actor
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectPoolComponent();

	/**
	 * Initialize the object pool with a specific actor class and configuration
	 * @param ActorClass - Class of actors to pool
	 * @param Config - Pool configuration settings
	 * @return True if initialization succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	bool Initialize(TSubclassOf<AActor> ActorClass, const FObjectPoolConfig& Config);

	/**
	 * Get an actor from the pool
	 * @param SpawnLocation - Location to place the actor
	 * @param SpawnRotation - Rotation to apply to the actor
	 * @return Pooled actor or nullptr if pool is exhausted and cannot expand
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* GetFromPool(FVector SpawnLocation, FRotator SpawnRotation);

	/**
	 * Return an actor to the pool
	 * @param Actor - Actor to return to the pool
	 * @return True if actor was successfully returned
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	bool ReturnToPool(AActor* Actor);

	/**
	 * Get the number of currently active (in-use) objects
	 * @return Number of active objects
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetActiveCount() const { return ActiveObjects.Num(); }

	/**
	 * Get the number of available (unused) objects in the pool
	 * @return Number of available objects
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetAvailableCount() const { return AvailableObjects.Num(); }

	/**
	 * Check if there are any available objects in the pool
	 * @return True if at least one object is available
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	bool HasAvailable() const { return AvailableObjects.Num() > 0; }

	/**
	 * Return all active objects to the pool
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearPool();

	/**
	 * Return all objects to the pool and reset their state
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ResetPool();

	/**
	 * Get the total size of the pool (active + available)
	 * @return Total pool size
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetTotalPoolSize() const { return ActiveObjects.Num() + AvailableObjects.Num(); }

	/**
	 * Check if debug visualization is enabled
	 * @return True if debug visualization is enabled
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool|Debug")
	bool IsDebugVisualizationEnabled() const { return bShowDebugVisualization; }

	/**
	 * Enable or disable debug visualization for this pool
	 * @param bEnabled - Whether to show debug visualization
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool|Debug")
	void SetDebugVisualization(bool bEnabled) { bShowDebugVisualization = bEnabled; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/**
	 * Pre-spawn pool of actors
	 * @param NumToSpawn - Number of actors to spawn
	 * @return True if all actors spawned successfully
	 */
	bool PreSpawnPool(int32 NumToSpawn);

	/**
	 * Spawn a single pooled actor
	 * @return Spawned actor or nullptr if spawn failed
	 */
	AActor* SpawnPooledActor();

	/**
	 * Deactivate an actor (hide and disable collision)
	 * @param Actor - Actor to deactivate
	 */
	void DeactivateActor(AActor* Actor);

	/**
	 * Activate an actor (show and enable collision)
	 * @param Actor - Actor to activate
	 */
	void ActivateActor(AActor* Actor);

	/**
	 * Validate that an actor belongs to this pool
	 * @param Actor - Actor to validate
	 * @return True if actor is from this pool
	 */
	bool ValidatePooledActor(AActor* Actor) const;

	/**
	 * Draw debug visualization for the pool
	 */
	void DrawDebugVisualization();

	// Configuration for this pool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
	FObjectPoolConfig PoolConfig;

	// Class of actors to pool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> PooledActorClass;

	// Available (inactive) objects in the pool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> AvailableObjects;

	// Active (in-use) objects from the pool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> ActiveObjects;

	// All pooled objects (for validation)
	UPROPERTY()
	TSet<AActor*> AllPooledObjects;

	// Whether the pool has been initialized
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Pool", meta = (AllowPrivateAccess = "true"))
	bool bIsInitialized;

	// Whether to show debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool|Debug", meta = (AllowPrivateAccess = "true"))
	bool bShowDebugVisualization;
};
