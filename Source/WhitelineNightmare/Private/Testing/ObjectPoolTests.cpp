// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "Core/ObjectPoolComponent.h"
#include "Core/ObjectPoolTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#if !UE_BUILD_SHIPPING

// Define a simple test actor that implements IPoolableActor
UCLASS()
class ATestPoolableActor : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	ATestPoolableActor()
	{
		ActivationCount = 0;
		DeactivationCount = 0;
		ResetCount = 0;
	}

	// IPoolableActor interface
	virtual void OnActivated_Implementation() override
	{
		ActivationCount++;
	}

	virtual void OnDeactivated_Implementation() override
	{
		DeactivationCount++;
	}

	virtual void ResetState_Implementation() override
	{
		ResetCount++;
		ActivationCount = 0;
		DeactivationCount = 0;
	}

	int32 ActivationCount;
	int32 DeactivationCount;
	int32 ResetCount;
};

// Helper function to get a valid world for testing
static UWorld* GetTestWorld()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
		{
			return Context.World();
		}
	}
	return nullptr;
}

// Helper function to create a pool component for testing
static UObjectPoolComponent* CreateTestPoolComponent()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	// Create a dummy actor to hold the component
	AActor* DummyActor = World->SpawnActor<AActor>();
	if (!DummyActor)
	{
		return nullptr;
	}

	// Create and attach the pool component
	UObjectPoolComponent* PoolComponent = NewObject<UObjectPoolComponent>(DummyActor);
	if (PoolComponent)
	{
		PoolComponent->RegisterComponent();
	}

	return PoolComponent;
}

/**
 * Test: Pool Initialization
 * Verify that the pool creates the correct number of actors on initialization
 */
static bool ObjectPoolTest_Initialization()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	// Create a pool configuration
	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;

	// Initialize the pool
	bool bInitSuccess = PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);
	TEST_TRUE(bInitSuccess, "Pool should initialize successfully");

	// Verify pool size
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Pool should have 5 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Pool should have 0 active objects");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 5, "Pool total size should be 5");
	TEST_TRUE(PoolComponent->HasAvailable(), "Pool should have available objects");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_Initialization");
}

/**
 * Test: Get From Pool
 * Verify that GetFromPool returns a valid actor and updates counts correctly
 */
static bool ObjectPoolTest_GetFromPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	FVector SpawnLocation(100.0f, 200.0f, 300.0f);
	FRotator SpawnRotation(0.0f, 90.0f, 0.0f);
	AActor* Actor = PoolComponent->GetFromPool(SpawnLocation, SpawnRotation);

	TEST_NOT_NULL(Actor, "GetFromPool should return a valid actor");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 2, "Pool should have 2 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 1, "Pool should have 1 active object");

	// Verify actor location and rotation
	FVector ActorLocation = Actor->GetActorLocation();
	TEST_NEARLY_EQUAL(ActorLocation.X, SpawnLocation.X, 0.1f, "Actor X location should match spawn location");
	TEST_NEARLY_EQUAL(ActorLocation.Y, SpawnLocation.Y, 0.1f, "Actor Y location should match spawn location");
	TEST_NEARLY_EQUAL(ActorLocation.Z, SpawnLocation.Z, 0.1f, "Actor Z location should match spawn location");

	// Verify actor is visible and has collision enabled
	TEST_FALSE(Actor->IsHidden(), "Actor should be visible");

	// Verify OnActivated was called
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->ActivationCount, 1, "OnActivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_GetFromPool");
}

/**
 * Test: Return To Pool
 * Verify that ReturnToPool deactivates the actor and returns it to the available pool
 */
static bool ObjectPoolTest_ReturnToPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	AActor* Actor = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor, "GetFromPool should return a valid actor");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 1, "Pool should have 1 active object");

	// Return the actor to the pool
	bool bReturnSuccess = PoolComponent->ReturnToPool(Actor);
	TEST_TRUE(bReturnSuccess, "ReturnToPool should succeed");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Pool should have 3 available objects");
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Pool should have 0 active objects");

	// Verify actor is hidden
	TEST_TRUE(Actor->IsHidden(), "Actor should be hidden");

	// Verify OnDeactivated was called
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->DeactivationCount, 1, "OnDeactivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ReturnToPool");
}

/**
 * Test: Pool Exhaustion
 * Verify behavior when the pool is exhausted (no auto-expand)
 */
static bool ObjectPoolTest_PoolExhaustion()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 2;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get all actors from the pool
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_NOT_NULL(Actor1, "First actor should be retrieved");
	TEST_NOT_NULL(Actor2, "Second actor should be retrieved");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 0, "Pool should be exhausted");
	TEST_FALSE(PoolComponent->HasAvailable(), "Pool should not have available objects");

	// Try to get another actor (should fail)
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NULL(Actor3, "GetFromPool should return null when pool is exhausted");

	// Return one actor and try again
	PoolComponent->ReturnToPool(Actor1);
	AActor* Actor4 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor4, "GetFromPool should succeed after returning an actor");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_PoolExhaustion");
}

/**
 * Test: Pool Reuse
 * Verify that the same actor instance is reused when returned to the pool
 */
static bool ObjectPoolTest_PoolReuse()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 1;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get an actor from the pool
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor1, "First GetFromPool should return a valid actor");

	// Return the actor to the pool
	PoolComponent->ReturnToPool(Actor1);

	// Get an actor from the pool again
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor2, "Second GetFromPool should return a valid actor");

	// Verify it's the same instance
	TEST_EQUAL(Actor1, Actor2, "Pool should reuse the same actor instance");

	// Verify activation/deactivation counts
	ATestPoolableActor* TestActor = Cast<ATestPoolableActor>(Actor2);
	if (TestActor)
	{
		TEST_EQUAL(TestActor->ActivationCount, 2, "OnActivated should have been called twice");
		TEST_EQUAL(TestActor->DeactivationCount, 1, "OnDeactivated should have been called once");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_PoolReuse");
}

/**
 * Test: Active Count Tracking
 * Verify that active/available counts are accurate as objects move between pools
 */
static bool ObjectPoolTest_ActiveCount()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 5;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Initial state
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Initial active count should be 0");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Initial available count should be 5");

	// Get 3 actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_EQUAL(PoolComponent->GetActiveCount(), 3, "Active count should be 3");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 2, "Available count should be 2");

	// Return 1 actor
	PoolComponent->ReturnToPool(Actor2);
	TEST_EQUAL(PoolComponent->GetActiveCount(), 2, "Active count should be 2 after returning one");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Available count should be 3 after returning one");

	// Clear pool (return all active)
	PoolComponent->ClearPool();
	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Active count should be 0 after clearing");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 5, "Available count should be 5 after clearing");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ActiveCount");
}

/**
 * Test: Auto-Expand
 * Verify that the pool can auto-expand when exhausted
 */
static bool ObjectPoolTest_AutoExpand()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 2;
	Config.bAutoExpand = true;
	Config.MaxPoolSize = 4;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get all initial actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 2, "Initial pool size should be 2");

	// Get another actor (should auto-expand)
	AActor* Actor3 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor3, "GetFromPool should succeed with auto-expand");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 3, "Pool should have expanded to 3");

	// Get one more (should expand again)
	AActor* Actor4 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NOT_NULL(Actor4, "GetFromPool should succeed with auto-expand");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 4, "Pool should have expanded to 4");

	// Try to get another (should fail - max size reached)
	AActor* Actor5 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	TEST_NULL(Actor5, "GetFromPool should fail when max pool size is reached");
	TEST_EQUAL(PoolComponent->GetTotalPoolSize(), 4, "Pool should remain at max size of 4");

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_AutoExpand");
}

/**
 * Test: Reset Pool
 * Verify that ResetPool returns all objects and calls ResetState
 */
static bool ObjectPoolTest_ResetPool()
{
	UObjectPoolComponent* PoolComponent = CreateTestPoolComponent();
	TEST_NOT_NULL(PoolComponent, "Pool component should be created");

	FObjectPoolConfig Config;
	Config.PoolSize = 3;
	Config.bAutoExpand = false;

	PoolComponent->Initialize(ATestPoolableActor::StaticClass(), Config);

	// Get some actors
	AActor* Actor1 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);
	AActor* Actor2 = PoolComponent->GetFromPool(FVector::ZeroVector, FRotator::ZeroRotator);

	ATestPoolableActor* TestActor1 = Cast<ATestPoolableActor>(Actor1);
	ATestPoolableActor* TestActor2 = Cast<ATestPoolableActor>(Actor2);

	TEST_EQUAL(PoolComponent->GetActiveCount(), 2, "Should have 2 active objects");

	// Reset the pool
	PoolComponent->ResetPool();

	TEST_EQUAL(PoolComponent->GetActiveCount(), 0, "Should have 0 active objects after reset");
	TEST_EQUAL(PoolComponent->GetAvailableCount(), 3, "Should have 3 available objects after reset");

	// Verify ResetState was called
	if (TestActor1)
	{
		TEST_EQUAL(TestActor1->ResetCount, 1, "ResetState should have been called");
		TEST_EQUAL(TestActor1->ActivationCount, 0, "Activation count should be reset");
	}

	// Cleanup
	if (PoolComponent->GetOwner())
	{
		PoolComponent->GetOwner()->Destroy();
	}

	TEST_SUCCESS("ObjectPoolTest_ResetPool");
}

/**
 * Register all object pool tests with the test manager
 * This function should be called from TestingGameMode::RegisterSampleTests()
 */
void RegisterObjectPoolTests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	TestManager->RegisterTest(TEXT("ObjectPool_Initialization"), ETestCategory::ObjectPool, &ObjectPoolTest_Initialization);
	TestManager->RegisterTest(TEXT("ObjectPool_GetFromPool"), ETestCategory::ObjectPool, &ObjectPoolTest_GetFromPool);
	TestManager->RegisterTest(TEXT("ObjectPool_ReturnToPool"), ETestCategory::ObjectPool, &ObjectPoolTest_ReturnToPool);
	TestManager->RegisterTest(TEXT("ObjectPool_PoolExhaustion"), ETestCategory::ObjectPool, &ObjectPoolTest_PoolExhaustion);
	TestManager->RegisterTest(TEXT("ObjectPool_PoolReuse"), ETestCategory::ObjectPool, &ObjectPoolTest_PoolReuse);
	TestManager->RegisterTest(TEXT("ObjectPool_ActiveCount"), ETestCategory::ObjectPool, &ObjectPoolTest_ActiveCount);
	TestManager->RegisterTest(TEXT("ObjectPool_AutoExpand"), ETestCategory::ObjectPool, &ObjectPoolTest_AutoExpand);
	TestManager->RegisterTest(TEXT("ObjectPool_ResetPool"), ETestCategory::ObjectPool, &ObjectPoolTest_ResetPool);
}

#endif // !UE_BUILD_SHIPPING
