// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/ObjectPoolComponent.h"
#include "Engine/World.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxPoolSize = 50;
	bAllowGrowth = true;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UObjectPoolComponent::InitializePool(TSubclassOf<AActor> ActorClass, int32 PoolSize)
{
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Cannot initialize pool with null ActorClass!"));
		return;
	}

	PooledActorClass = ActorClass;

	// Clear any existing pool
	ClearPool();

	// Pre-spawn actors
	const int32 ActorsToSpawn = FMath::Min(PoolSize, MaxPoolSize);
	for (int32 i = 0; i < ActorsToSpawn; ++i)
	{
		AActor* NewActor = CreatePooledActor();
		if (NewActor)
		{
			InactivePool.Add(NewActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ObjectPoolComponent: Initialized pool with %d actors of class %s"),
		InactivePool.Num(), *ActorClass->GetName());
}

AActor* UObjectPoolComponent::GetFromPool(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (!PooledActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Pool not initialized!"));
		return nullptr;
	}

	AActor* PooledActor = nullptr;

	// Try to get from inactive pool
	if (InactivePool.Num() > 0)
	{
		PooledActor = InactivePool.Pop();
	}
	// If pool is empty and growth is allowed, create new actor
	else if (bAllowGrowth && GetTotalPoolSize() < MaxPoolSize)
	{
		PooledActor = CreatePooledActor();
		UE_LOG(LogTemp, Verbose, TEXT("ObjectPoolComponent: Growing pool (new size: %d)"), GetTotalPoolSize() + 1);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Pool exhausted! Consider increasing pool size."));
		return nullptr;
	}

	if (PooledActor)
	{
		ActivateActor(PooledActor, SpawnLocation, SpawnRotation);
		ActivePool.Add(PooledActor);
		OnObjectSpawnedFromPool.Broadcast(PooledActor, ActivePool.Num() - 1);
	}

	return PooledActor;
}

void UObjectPoolComponent::ReturnToPool(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Remove from active pool
	const int32 RemovedCount = ActivePool.Remove(Actor);
	if (RemovedCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Trying to return actor that's not in active pool!"));
		return;
	}

	// Deactivate and add to inactive pool
	DeactivateActor(Actor);
	InactivePool.Add(Actor);

	OnObjectReturnedToPool.Broadcast(Actor, InactivePool.Num() - 1);
}

void UObjectPoolComponent::ClearPool()
{
	// Destroy all pooled actors
	for (AActor* Actor : InactivePool)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	for (AActor* Actor : ActivePool)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}

	InactivePool.Empty();
	ActivePool.Empty();
}

AActor* UObjectPoolComponent::CreatePooledActor()
{
	if (!PooledActorClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = GetOwner();

	AActor* NewActor = World->SpawnActor<AActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (NewActor)
	{
		// Initially deactivate
		DeactivateActor(NewActor);
	}

	return NewActor;
}

void UObjectPoolComponent::DeactivateActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Hide actor
	Actor->SetActorHiddenInGame(true);

	// Disable collision
	Actor->SetActorEnableCollision(false);

	// Disable tick
	Actor->SetActorTickEnabled(false);

	// Move far away (to avoid any lingering interactions)
	Actor->SetActorLocation(FVector(0, 0, -10000.0f));
}

void UObjectPoolComponent::ActivateActor(AActor* Actor, const FVector& Location, const FRotator& Rotation)
{
	if (!Actor)
	{
		return;
	}

	// Set transform
	Actor->SetActorLocationAndRotation(Location, Rotation);

	// Show actor
	Actor->SetActorHiddenInGame(false);

	// Enable collision
	Actor->SetActorEnableCollision(true);

	// Enable tick
	Actor->SetActorTickEnabled(true);

	// Call reset function if actor implements it
	// This allows pooled actors to reset their state
	if (Actor->GetClass()->ImplementsInterface(UInterface::StaticClass()))
	{
		// TODO: Create IPoolable interface with Reset() method
	}
}
