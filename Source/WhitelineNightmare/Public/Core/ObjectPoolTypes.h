// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ObjectPoolTypes.generated.h"

/**
 * Object Pool Configuration - Defines how an object pool should behave
 */
USTRUCT(BlueprintType)
struct FObjectPoolConfig
{
	GENERATED_BODY()

	// Number of objects to pre-allocate in the pool
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	int32 PoolSize;

	// Whether to create more objects if pool is exhausted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	bool bAutoExpand;

	// Maximum size if auto-expand is enabled (0 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool")
	int32 MaxPoolSize;

	// How far ahead of the war rig to spawn objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool|Spawning")
	float SpawnDistanceAhead;

	// How far behind the war rig to despawn/recycle objects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Object Pool|Despawning")
	float DespawnDistanceBehind;

	FObjectPoolConfig()
		: PoolSize(10)
		, bAutoExpand(false)
		, MaxPoolSize(0)
		, SpawnDistanceAhead(2000.0f)
		, DespawnDistanceBehind(1000.0f)
	{
	}
};

/**
 * Poolable Actor Interface - UInterface class
 * Actors that can be pooled should implement IPoolableActor
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPoolableActor : public UInterface
{
	GENERATED_BODY()
};

/**
 * Poolable Actor Interface - Interface class
 * Provides callbacks for when actors are taken from or returned to the pool
 */
class IPoolableActor
{
	GENERATED_BODY()

public:
	/**
	 * Called when the actor is taken from the pool and activated
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnActivated();
	virtual void OnActivated_Implementation() {}

	/**
	 * Called when the actor is returned to the pool and deactivated
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void OnDeactivated();
	virtual void OnDeactivated_Implementation() {}

	/**
	 * Called to reset the actor to its default state
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void ResetState();
	virtual void ResetState_Implementation() {}
};
