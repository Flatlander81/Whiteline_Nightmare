// Copyright Flatlander81. All Rights Reserved.

#include "Pickups/PickupPoolComponent.h"
#include "Pickups/FuelPickup.h"
#include "Core/WarRigPawn.h"
#include "Core/WorldScrollComponent.h"
#include "Core/LaneSystemComponent.h"
#include "DrawDebugHelpers.h"

#if !UE_BUILD_SHIPPING
// Static debug instance
UPickupPoolComponent* UPickupPoolComponent::DebugInstance = nullptr;

// Console command auto-registers
static FAutoConsoleCommand DebugSpawnFuelPickupCmd(
	TEXT("DebugSpawnFuelPickup"),
	TEXT("Spawns a fuel pickup in the specified lane (0-4). Usage: DebugSpawnFuelPickup <lane_index>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&UPickupPoolComponent::DebugSpawnFuelPickup)
);

static FAutoConsoleCommand DebugShowPickupsCmd(
	TEXT("DebugShowPickups"),
	TEXT("Toggles debug visualization for pickups"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&UPickupPoolComponent::DebugShowPickups)
);

static FAutoConsoleCommand DebugShowPickupPoolCmd(
	TEXT("DebugShowPickupPool"),
	TEXT("Displays pool statistics for pickups"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&UPickupPoolComponent::DebugShowPickupPool)
);
#endif

UPickupPoolComponent::UPickupPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	SpawnDistanceAhead = 2000.0f;
	DespawnDistanceBehind = -1000.0f;
	SpawnHeight = 0.0f;

	// Default lane positions (5 lanes)
	LaneYPositions = { -400.0f, -200.0f, 0.0f, 200.0f, 400.0f };

#if !UE_BUILD_SHIPPING
	bShowDebugVisualization = false;
#endif
}

void UPickupPoolComponent::BeginPlay()
{
	Super::BeginPlay();

#if !UE_BUILD_SHIPPING
	// Set as debug instance
	DebugInstance = this;
#endif
}

void UPickupPoolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check and despawn pickups that have passed behind the war rig
	CheckAndDespawnPickups();

#if !UE_BUILD_SHIPPING
	// Draw debug visualization if enabled
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
#endif
}

bool UPickupPoolComponent::InitializePickupPool(AWarRigPawn* WarRig, UWorldScrollComponent* ScrollComponent,
	TSubclassOf<AFuelPickup> PickupClass, int32 PoolSize)
{
	if (!WarRig)
	{
		UE_LOG(LogTemp, Error, TEXT("UPickupPoolComponent::InitializePickupPool - WarRig is null"));
		return false;
	}

	if (!ScrollComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UPickupPoolComponent::InitializePickupPool - ScrollComponent is null"));
		return false;
	}

	if (!PickupClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UPickupPoolComponent::InitializePickupPool - PickupClass is null"));
		return false;
	}

	// Store references
	WarRigPawn = WarRig;
	WorldScrollComponent = ScrollComponent;

	// Get lane positions from war rig's lane system if available
	if (ULaneSystemComponent* LaneSystem = WarRig->FindComponentByClass<ULaneSystemComponent>())
	{
		LaneYPositions = LaneSystem->GetLaneYPositions();
	}

	// Initialize the base object pool
	FObjectPoolConfig PoolConfig;
	PoolConfig.PoolSize = PoolSize;
	PoolConfig.bAutoExpand = true;
	PoolConfig.MaxPoolSize = PoolSize * 2;  // Allow expansion up to 2x initial size
	PoolConfig.SpawnDistanceAhead = SpawnDistanceAhead;
	PoolConfig.DespawnDistanceBehind = DespawnDistanceBehind;

	bool bSuccess = Initialize(PickupClass, PoolConfig);
	if (bSuccess)
	{
		// Set world scroll component and pool component reference on all pooled pickups
		for (AActor* PooledActor : AllPooledObjects)
		{
			if (AFuelPickup* Pickup = Cast<AFuelPickup>(PooledActor))
			{
				Pickup->SetWorldScrollComponent(WorldScrollComponent);
				Pickup->SetPoolComponent(this);
			}
		}
	}

	return bSuccess;
}

AFuelPickup* UPickupPoolComponent::SpawnPickupInLane(int32 LaneIndex)
{
	// Validate lane index
	if (LaneIndex < 0 || LaneIndex >= LaneYPositions.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UPickupPoolComponent::SpawnPickupInLane - Invalid lane index %d (valid range: 0-%d)"),
			LaneIndex, LaneYPositions.Num() - 1);
		return nullptr;
	}

	// Get spawn location for the lane
	FVector SpawnLocation = GetSpawnLocationForLane(LaneIndex);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Get pickup from pool
	AActor* PickupActor = GetFromPool(SpawnLocation, SpawnRotation);
	AFuelPickup* Pickup = Cast<AFuelPickup>(PickupActor);

	if (Pickup)
	{
		// Ensure world scroll component and pool component are set
		// (in case this is a newly created actor from auto-expand)
		Pickup->SetWorldScrollComponent(WorldScrollComponent);
		Pickup->SetPoolComponent(this);
	}

	return Pickup;
}

AFuelPickup* UPickupPoolComponent::SpawnPickupInRandomLane()
{
	// Select a random lane
	int32 RandomLaneIndex = FMath::RandRange(0, LaneYPositions.Num() - 1);
	return SpawnPickupInLane(RandomLaneIndex);
}

void UPickupPoolComponent::CheckAndDespawnPickups()
{
	if (!WarRigPawn)
	{
		return;
	}

	// Get war rig position
	const FVector WarRigLocation = WarRigPawn->GetActorLocation();
	const float DespawnThreshold = WarRigLocation.X + DespawnDistanceBehind;

	// Check each active pickup
	TArray<AActor*> PickupsToDespawn;
	for (AActor* ActivePickup : ActiveObjects)
	{
		if (!ActivePickup)
		{
			continue;
		}

		// Check if pickup has passed behind war rig
		if (ActivePickup->GetActorLocation().X < DespawnThreshold)
		{
			PickupsToDespawn.Add(ActivePickup);
		}
	}

	// Return pickups to pool
	for (AActor* Pickup : PickupsToDespawn)
	{
		ReturnToPool(Pickup);
	}
}

FVector UPickupPoolComponent::GetSpawnLocationForLane(int32 LaneIndex) const
{
	if (!WarRigPawn)
	{
		return FVector::ZeroVector;
	}

	// Validate lane index
	if (LaneIndex < 0 || LaneIndex >= LaneYPositions.Num())
	{
		return FVector::ZeroVector;
	}

	// Get war rig position
	const FVector WarRigLocation = WarRigPawn->GetActorLocation();

	// Calculate spawn position
	// X: Ahead of war rig by SpawnDistanceAhead
	// Y: Lane position
	// Z: Ground level
	FVector SpawnLocation;
	SpawnLocation.X = WarRigLocation.X + SpawnDistanceAhead;
	SpawnLocation.Y = LaneYPositions[LaneIndex];
	SpawnLocation.Z = SpawnHeight;

	return SpawnLocation;
}

#if !UE_BUILD_SHIPPING
void UPickupPoolComponent::DrawDebugVisualization() const
{
	if (!WarRigPawn || !GetWorld())
	{
		return;
	}

	const FVector WarRigLocation = WarRigPawn->GetActorLocation();

	// Draw spawn boundary (green line)
	const float SpawnX = WarRigLocation.X + SpawnDistanceAhead;
	FVector SpawnLineStart(SpawnX, -1000.0f, 0.0f);
	FVector SpawnLineEnd(SpawnX, 1000.0f, 0.0f);
	DrawDebugLine(GetWorld(), SpawnLineStart, SpawnLineEnd, FColor::Green, false, -1.0f, 0, 5.0f);

	// Draw despawn boundary (red line)
	const float DespawnX = WarRigLocation.X + DespawnDistanceBehind;
	FVector DespawnLineStart(DespawnX, -1000.0f, 0.0f);
	FVector DespawnLineEnd(DespawnX, 1000.0f, 0.0f);
	DrawDebugLine(GetWorld(), DespawnLineStart, DespawnLineEnd, FColor::Red, false, -1.0f, 0, 5.0f);

	// Draw active pickups (green spheres)
	for (AActor* ActivePickup : ActiveObjects)
	{
		if (ActivePickup && !ActivePickup->IsHidden())
		{
			DrawDebugSphere(GetWorld(), ActivePickup->GetActorLocation(), 50.0f, 12, FColor::Green, false, -1.0f, 0, 2.0f);
		}
	}

	// Draw lane markers
	for (int32 i = 0; i < LaneYPositions.Num(); ++i)
	{
		FVector LaneStart(WarRigLocation.X - 500.0f, LaneYPositions[i], 0.0f);
		FVector LaneEnd(WarRigLocation.X + SpawnDistanceAhead + 500.0f, LaneYPositions[i], 0.0f);
		DrawDebugLine(GetWorld(), LaneStart, LaneEnd, FColor::Cyan, false, -1.0f, 0, 1.0f);

		// Draw lane number
		FVector TextLocation(WarRigLocation.X, LaneYPositions[i], 50.0f);
		DrawDebugString(GetWorld(), TextLocation, FString::Printf(TEXT("Lane %d"), i), nullptr, FColor::White, 0.0f, true);
	}

	// Draw pool statistics
	FVector StatsLocation = WarRigLocation + FVector(0.0f, 0.0f, 300.0f);
	FString StatsText = FString::Printf(TEXT("Pickup Pool:\nActive: %d\nAvailable: %d\nTotal: %d"),
		ActiveObjects.Num(), AvailableObjects.Num(), AllPooledObjects.Num());
	DrawDebugString(GetWorld(), StatsLocation, StatsText, nullptr, FColor::Yellow, 0.0f, true, 1.5f);
}

void UPickupPoolComponent::DebugSpawnFuelPickup(const TArray<FString>& Args)
{
	if (!DebugInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugSpawnFuelPickup - No active PickupPoolComponent instance"));
		return;
	}

	if (Args.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugSpawnFuelPickup - Usage: DebugSpawnFuelPickup <lane_index>"));
		return;
	}

	int32 LaneIndex = FCString::Atoi(*Args[0]);
	AFuelPickup* Pickup = DebugInstance->SpawnPickupInLane(LaneIndex);

	if (Pickup)
	{
		UE_LOG(LogTemp, Log, TEXT("DebugSpawnFuelPickup - Spawned pickup in lane %d at location %s"),
			LaneIndex, *Pickup->GetActorLocation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugSpawnFuelPickup - Failed to spawn pickup in lane %d"), LaneIndex);
	}
}

void UPickupPoolComponent::DebugShowPickups(const TArray<FString>& Args)
{
	if (!DebugInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugShowPickups - No active PickupPoolComponent instance"));
		return;
	}

	DebugInstance->bShowDebugVisualization = !DebugInstance->bShowDebugVisualization;

	UE_LOG(LogTemp, Log, TEXT("DebugShowPickups - Debug visualization %s"),
		DebugInstance->bShowDebugVisualization ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UPickupPoolComponent::DebugShowPickupPool(const TArray<FString>& Args)
{
	if (!DebugInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugShowPickupPool - No active PickupPoolComponent instance"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== Pickup Pool Statistics ==="));
	UE_LOG(LogTemp, Log, TEXT("Active Pickups: %d"), DebugInstance->ActiveObjects.Num());
	UE_LOG(LogTemp, Log, TEXT("Available Pickups: %d"), DebugInstance->AvailableObjects.Num());
	UE_LOG(LogTemp, Log, TEXT("Total Pool Size: %d"), DebugInstance->AllPooledObjects.Num());
	UE_LOG(LogTemp, Log, TEXT("Spawn Distance Ahead: %.1f"), DebugInstance->SpawnDistanceAhead);
	UE_LOG(LogTemp, Log, TEXT("Despawn Distance Behind: %.1f"), DebugInstance->DespawnDistanceBehind);
	UE_LOG(LogTemp, Log, TEXT("Number of Lanes: %d"), DebugInstance->LaneYPositions.Num());
	UE_LOG(LogTemp, Log, TEXT("=============================="));
}
#endif
