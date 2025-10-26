// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ObjectPoolComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectSpawnedFromPool, AActor*, SpawnedActor, int32, PoolIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectReturnedToPool, AActor*, ReturnedActor, int32, PoolIndex);

/**
 * Object Pool Component
 * Generic object pooling system for efficient actor reuse
 * Used for enemies, obstacles, pickups, and projectiles
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UObjectPoolComponent();

	virtual void BeginPlay() override;

	/**
	 * Initialize the pool with a specific actor class and size
	 * @param ActorClass - Class of actors to pool
	 * @param PoolSize - Number of actors to pre-spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void InitializePool(TSubclassOf<AActor> ActorClass, int32 PoolSize);

	/**
	 * Get an actor from the pool
	 * @param SpawnLocation - Location to spawn/activate the actor
	 * @param SpawnRotation - Rotation for the actor
	 * @return The pooled actor, or nullptr if pool is exhausted
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	AActor* GetFromPool(const FVector& SpawnLocation, const FRotator& SpawnRotation);

	/**
	 * Return an actor to the pool
	 * @param Actor - Actor to return to the pool
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnToPool(AActor* Actor);

	/**
	 * Get number of available actors in pool
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetAvailableCount() const { return InactivePool.Num(); }

	/**
	 * Get number of active actors from this pool
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetActiveCount() const { return ActivePool.Num(); }

	/**
	 * Get total pool size
	 */
	UFUNCTION(BlueprintPure, Category = "Object Pool")
	int32 GetTotalPoolSize() const { return InactivePool.Num() + ActivePool.Num(); }

	/**
	 * Clear the entire pool (destroys all actors)
	 */
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearPool();

	/** Event fired when an object is spawned from the pool */
	UPROPERTY(BlueprintAssignable, Category = "Object Pool")
	FOnObjectSpawnedFromPool OnObjectSpawnedFromPool;

	/** Event fired when an object is returned to the pool */
	UPROPERTY(BlueprintAssignable, Category = "Object Pool")
	FOnObjectReturnedToPool OnObjectReturnedToPool;

protected:
	/** Class of actors in this pool */
	UPROPERTY(BlueprintReadOnly, Category = "Object Pool")
	TSubclassOf<AActor> PooledActorClass;

	/** Inactive actors ready to be used */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> InactivePool;

	/** Currently active actors from this pool */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActivePool;

	/** Maximum pool size (hard limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	int32 MaxPoolSize;

	/** Allow pool to grow beyond initial size? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	bool bAllowGrowth;

	/** Create a new pooled actor */
	AActor* CreatePooledActor();

	/** Deactivate an actor for pooling */
	void DeactivateActor(AActor* Actor);

	/** Activate an actor from the pool */
	void ActivateActor(AActor* Actor, const FVector& Location, const FRotator& Rotation);
};
