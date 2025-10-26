// Copyright Flatlander81. All Rights Reserved.

#include "Core/ObjectPoolComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // Only tick if debug visualization is enabled
	bIsInitialized = false;
	bShowDebugVisualization = false;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();

	// Component doesn't auto-initialize on BeginPlay
	// Initialize() must be called manually with desired configuration
}

void UObjectPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Draw debug visualization if enabled
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
}

bool UObjectPoolComponent::Initialize(TSubclassOf<AActor> ActorClass, const FObjectPoolConfig& Config)
{
	// Validate actor class
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Cannot initialize with null ActorClass"));
		return false;
	}

	// Validate pool size
	if (Config.PoolSize <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: PoolSize must be greater than 0"));
		return false;
	}

	// Validate max pool size if auto-expand is enabled
	if (Config.bAutoExpand && Config.MaxPoolSize > 0 && Config.MaxPoolSize < Config.PoolSize)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: MaxPoolSize must be greater than or equal to PoolSize when auto-expand is enabled"));
		return false;
	}

	// Store configuration
	PooledActorClass = ActorClass;
	PoolConfig = Config;

	// Clear any existing pool
	ClearPool();
	AvailableObjects.Empty();
	ActiveObjects.Empty();
	AllPooledObjects.Empty();

	// Pre-spawn the pool
	if (!PreSpawnPool(Config.PoolSize))
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Failed to pre-spawn pool"));
		return false;
	}

	bIsInitialized = true;
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolComponent: Initialized pool with %d objects of class %s"),
		Config.PoolSize, *ActorClass->GetName());

	return true;
}

bool UObjectPoolComponent::PreSpawnPool(int32 NumToSpawn)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: No valid world for spawning"));
		return false;
	}

	// Spawn actors at the origin, deactivated
	for (int32 i = 0; i < NumToSpawn; ++i)
	{
		AActor* NewActor = SpawnPooledActor();
		if (!NewActor)
		{
			UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Failed to spawn pooled actor %d of %d"), i + 1, NumToSpawn);
			return false;
		}

		// Deactivate and add to available pool
		DeactivateActor(NewActor);
		AvailableObjects.Add(NewActor);
		AllPooledObjects.Add(NewActor);
	}

	return true;
}

AActor* UObjectPoolComponent::SpawnPooledActor()
{
	UWorld* World = GetWorld();
	if (!World || !PooledActorClass)
	{
		return nullptr;
	}

	// Spawn at origin with default rotation
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewActor = World->SpawnActor<AActor>(PooledActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	return NewActor;
}

AActor* UObjectPoolComponent::GetFromPool(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Cannot get from pool - not initialized"));
		return nullptr;
	}

	AActor* Actor = nullptr;

	// Check if we have available objects
	if (AvailableObjects.Num() > 0)
	{
		// Get last object from available pool (faster than removing from front)
		Actor = AvailableObjects.Pop();
	}
	else if (PoolConfig.bAutoExpand)
	{
		// Check if we can expand the pool
		int32 CurrentPoolSize = GetTotalPoolSize();
		if (PoolConfig.MaxPoolSize == 0 || CurrentPoolSize < PoolConfig.MaxPoolSize)
		{
			// Spawn a new actor
			Actor = SpawnPooledActor();
			if (Actor)
			{
				AllPooledObjects.Add(Actor);
				UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Pool exhausted, auto-expanding (new size: %d)"), CurrentPoolSize + 1);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Failed to spawn new actor for pool expansion"));
				return nullptr;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Pool exhausted and max size reached (%d)"), PoolConfig.MaxPoolSize);
			return nullptr;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Pool exhausted and auto-expand is disabled"));
		return nullptr;
	}

	// Set actor location and rotation
	Actor->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);

	// Activate the actor
	ActivateActor(Actor);

	// Move to active pool
	ActiveObjects.Add(Actor);

	// Call OnActivated if actor implements IPoolableActor
	if (Actor->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnActivated(Actor);
	}

	return Actor;
}

bool UObjectPoolComponent::ReturnToPool(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Cannot return null actor to pool"));
		return false;
	}

	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolComponent: Cannot return to pool - not initialized"));
		return false;
	}

	// Validate actor is from this pool
	if (!ValidatePooledActor(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Trying to return actor that is not from this pool: %s"), *Actor->GetName());
		return false;
	}

	// Check if already in available pool
	if (AvailableObjects.Contains(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Actor %s is already in available pool"), *Actor->GetName());
		return false;
	}

	// Remove from active pool
	int32 RemovedCount = ActiveObjects.Remove(Actor);
	if (RemovedCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ObjectPoolComponent: Actor %s was not in active pool"), *Actor->GetName());
	}

	// Call OnDeactivated if actor implements IPoolableActor
	if (Actor->Implements<UPoolableActor>())
	{
		IPoolableActor::Execute_OnDeactivated(Actor);
	}

	// Deactivate the actor
	DeactivateActor(Actor);

	// Move actor to origin to avoid confusion
	Actor->SetActorLocation(FVector::ZeroVector);

	// Add to available pool
	AvailableObjects.Add(Actor);

	return true;
}

void UObjectPoolComponent::ClearPool()
{
	if (!bIsInitialized)
	{
		return;
	}

	// Return all active objects to the pool
	TArray<AActor*> ActiveCopy = ActiveObjects; // Copy because ReturnToPool modifies ActiveObjects
	for (AActor* Actor : ActiveCopy)
	{
		if (Actor)
		{
			ReturnToPool(Actor);
		}
	}
}

void UObjectPoolComponent::ResetPool()
{
	if (!bIsInitialized)
	{
		return;
	}

	// Clear all active objects
	ClearPool();

	// Reset state on all pooled objects
	for (AActor* Actor : AllPooledObjects)
	{
		if (Actor && Actor->Implements<UPoolableActor>())
		{
			IPoolableActor::Execute_ResetState(Actor);
		}
	}
}

void UObjectPoolComponent::DeactivateActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Hide the actor
	Actor->SetActorHiddenInGame(true);

	// Disable collision
	Actor->SetActorEnableCollision(false);

	// Disable tick to save performance
	Actor->SetActorTickEnabled(false);
}

void UObjectPoolComponent::ActivateActor(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// Show the actor
	Actor->SetActorHiddenInGame(false);

	// Enable collision
	Actor->SetActorEnableCollision(true);

	// Enable tick
	Actor->SetActorTickEnabled(true);
}

bool UObjectPoolComponent::ValidatePooledActor(AActor* Actor) const
{
	return AllPooledObjects.Contains(Actor);
}

void UObjectPoolComponent::DrawDebugVisualization()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float SphereRadius = 50.0f;
	const float LifeTime = 0.0f; // 0 means one frame

	// Draw green spheres for available objects
	for (AActor* Actor : AvailableObjects)
	{
		if (Actor)
		{
			DrawDebugSphere(World, Actor->GetActorLocation(), SphereRadius, 12, FColor::Green, false, LifeTime);
		}
	}

	// Draw red spheres for active objects
	for (AActor* Actor : ActiveObjects)
	{
		if (Actor)
		{
			DrawDebugSphere(World, Actor->GetActorLocation(), SphereRadius, 12, FColor::Red, false, LifeTime);
		}
	}
}

// Console command to visualize all object pools
#if !UE_BUILD_SHIPPING

static FAutoConsoleCommand DebugShowPoolsCommand(
	TEXT("DebugShowPools"),
	TEXT("Toggle debug visualization for all object pools in the world. Green = available, Red = active"),
	FConsoleCommandDelegate::CreateStatic([]()
	{
		UE_LOG(LogTemp, Log, TEXT("Console: DebugShowPools command executed"));

		// Get the world
		UWorld* World = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}

		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("Console: No valid world found"));
			return;
		}

		// Find all object pool components and toggle visualization
		int32 PoolCount = 0;
		bool bNewState = true; // Default to enabling

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor)
			{
				TArray<UObjectPoolComponent*> PoolComponents;
				Actor->GetComponents<UObjectPoolComponent>(PoolComponents);

				for (UObjectPoolComponent* PoolComponent : PoolComponents)
				{
					if (PoolComponent)
					{
						// Toggle visualization (if first pool, determine new state)
						if (PoolCount == 0)
						{
							bNewState = !PoolComponent->bShowDebugVisualization;
						}

						PoolComponent->SetDebugVisualization(bNewState);

						// Enable ticking for debug visualization
						PoolComponent->SetComponentTickEnabled(bNewState);

						PoolCount++;

						UE_LOG(LogTemp, Log, TEXT("Pool #%d: %s (Active: %d, Available: %d, Total: %d)"),
							PoolCount,
							*PoolComponent->GetOwner()->GetName(),
							PoolComponent->GetActiveCount(),
							PoolComponent->GetAvailableCount(),
							PoolComponent->GetTotalPoolSize());
					}
				}
			}
		}

		if (PoolCount == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("No object pools found in the world"));
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Debug visualization %s for %d object pool(s)"),
				bNewState ? TEXT("ENABLED") : TEXT("DISABLED"), PoolCount);
		}
	})
);

#endif // !UE_BUILD_SHIPPING
