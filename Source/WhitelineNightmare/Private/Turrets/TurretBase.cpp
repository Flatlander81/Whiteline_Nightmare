// Copyright Flatlander81. All Rights Reserved.

#include "Turrets/TurretBase.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/CombatAttributeSet.h"
#include "Core/WarRigPawn.h"
#include "Core/GameDataStructs.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATurretBase::ATurretBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component for mount point attachment
	TurretRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TurretRoot"));
	RootComponent = TurretRoot;

	// Create visual mesh component
	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TurretMesh"));
	TurretMesh->SetupAttachment(TurretRoot);
	TurretMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Turrets don't need collision

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create Combat Attribute Set
	CombatAttributes = CreateDefaultSubobject<UCombatAttributeSet>(TEXT("CombatAttributes"));

	// Initialize properties
	MountIndex = -1;
	FacingDirection = FRotator::ZeroRotator;
	OwnerWarRig = nullptr;
	CurrentTarget = nullptr;
	TimeSinceLastFire = 0.0f;

	// Debug visualization defaults
	bShowDebugVisualization = false;
	FiringArcDebugColor = FColor::Yellow;
	RangeDebugColor = FColor::Green;
	TargetLineDebugColor = FColor::Red;
}

void ATurretBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Ability System Component
	if (AbilitySystemComponent)
	{
		// Add the attribute set to the ASC
		AbilitySystemComponent->GetSpawnedAttributes_Mutable().AddUnique(CombatAttributes);
		CombatAttributes->InitHealth(CombatAttributes->GetMaxHealth());
	}

	// Validate setup
	if (!ValidateTurretSetup())
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::BeginPlay: Turret setup validation failed for %s"), *GetName());
	}
}

void ATurretBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update time since last fire
	TimeSinceLastFire += DeltaTime;

	// Find and track target
	AActor* NewTarget = FindTarget();
	if (NewTarget != CurrentTarget)
	{
		CurrentTarget = NewTarget;
	}

	// Auto-fire if we have a target and fire rate allows
	if (CurrentTarget && CombatAttributes)
	{
		const float FireRate = CombatAttributes->GetFireRate();
		if (FireRate > 0.0f)
		{
			const float FireInterval = 1.0f / FireRate;
			if (TimeSinceLastFire >= FireInterval)
			{
				Fire();
				TimeSinceLastFire = 0.0f;
			}
		}
	}

	// Draw debug visualization if enabled
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
}

UAbilitySystemComponent* ATurretBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATurretBase::Initialize(const FTurretData& TurretData, int32 InMountIndex, const FRotator& InFacingDirection, AWarRigPawn* InOwnerWarRig)
{
	// Validate inputs
	if (InMountIndex < 0)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::Initialize: Invalid mount index %d for turret %s"), InMountIndex, *GetName());
		return;
	}

	if (!InOwnerWarRig)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::Initialize: Null OwnerWarRig provided for turret %s"), *GetName());
		return;
	}

	// Store properties
	MountIndex = InMountIndex;
	FacingDirection = InFacingDirection;
	OwnerWarRig = InOwnerWarRig;

	// Set turret mesh if provided
	if (TurretData.TurretMesh.IsValid() || !TurretData.TurretMesh.IsNull())
	{
		if (UStaticMesh* Mesh = TurretData.TurretMesh.LoadSynchronous())
		{
			TurretMesh->SetStaticMesh(Mesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATurretBase::Initialize: Failed to load turret mesh for %s"), *TurretData.DisplayName.ToString());
		}
	}

	// Initialize attributes from data table
	if (CombatAttributes && AbilitySystemComponent)
	{
		// Set max values first, then current values
		CombatAttributes->InitMaxHealth(TurretData.BaseHealth);
		CombatAttributes->InitHealth(TurretData.BaseHealth);
		CombatAttributes->InitDamage(TurretData.BaseDamage);
		CombatAttributes->InitFireRate(TurretData.FireRate);
		CombatAttributes->InitRange(TurretData.Range);

		UE_LOG(LogTemp, Log, TEXT("ATurretBase::Initialize: Initialized turret '%s' with Health=%.1f, Damage=%.1f, FireRate=%.1f, Range=%.1f"),
			*TurretData.DisplayName.ToString(),
			CombatAttributes->GetHealth(),
			CombatAttributes->GetDamage(),
			CombatAttributes->GetFireRate(),
			CombatAttributes->GetRange());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::Initialize: Missing CombatAttributes or AbilitySystemComponent for turret %s"), *GetName());
	}
}

void ATurretBase::Fire()
{
	// Base implementation - override in subclasses
	if (!CurrentTarget)
	{
		return; // Graceful handling of null target
	}

	if (!CombatAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::Fire: CombatAttributes is null for %s"), *GetName());
		return;
	}

	// Log fire event (subclasses will implement actual projectile/damage logic)
	UE_LOG(LogTemp, Log, TEXT("ATurretBase::Fire: Turret firing at target %s (Damage: %.1f)"),
		*CurrentTarget->GetName(),
		CombatAttributes->GetDamage());

	// Future implementation will:
	// 1. Spawn projectile
	// 2. Apply damage via gameplay effect
	// 3. Play firing animation/sound
	// 4. Apply recoil/visual feedback
}

AActor* ATurretBase::FindTarget()
{
	if (!CombatAttributes)
	{
		return nullptr; // Graceful null handling
	}

	// Get all potential targets in range
	TArray<AActor*> PotentialTargets = GetPotentialTargets();

	// Filter by firing arc and find closest
	AActor* BestTarget = nullptr;
	float ClosestDistance = FLT_MAX;

	for (AActor* Target : PotentialTargets)
	{
		if (!IsTargetValid(Target))
		{
			continue;
		}

		// Check if target is within 180° firing arc
		if (!IsTargetInFiringArc(Target->GetActorLocation()))
		{
			continue;
		}

		// Check distance
		const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			BestTarget = Target;
		}
	}

	return BestTarget; // Returns nullptr if no valid targets
}

bool ATurretBase::IsTargetInFiringArc(const FVector& TargetLocation) const
{
	// Get turret forward vector from facing direction
	const FVector TurretForward = FacingDirection.Vector();

	// Get direction to target
	const FVector ToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();

	// Calculate dot product
	const float DotProduct = FVector::DotProduct(TurretForward, ToTarget);

	// If dot product > 0.0, target is within 180° arc
	// dot = 1.0 means directly ahead (0° angle)
	// dot = 0.0 means perpendicular (90° angle)
	// dot = -1.0 means directly behind (180° angle)
	return DotProduct > 0.0f;
}

TArray<AActor*> ATurretBase::GetPotentialTargets() const
{
	TArray<AActor*> PotentialTargets;

	if (!CombatAttributes)
	{
		return PotentialTargets; // Empty array if no attributes
	}

	// Get range from attributes
	const float Range = CombatAttributes->GetRange();

	// Perform sphere overlap query
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(Range);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (OwnerWarRig)
	{
		QueryParams.AddIgnoredActor(OwnerWarRig);
	}

	// Query for overlapping actors
	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn, // Look for pawns (enemies)
		SphereShape,
		QueryParams
	);

	if (bHasOverlaps)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AActor* Actor = Result.GetActor())
			{
				PotentialTargets.Add(Actor);
			}
		}
	}

	return PotentialTargets;
}

void ATurretBase::DrawDebugVisualization() const
{
	if (!GetWorld() || !CombatAttributes)
	{
		return;
	}

	const FVector TurretLocation = GetActorLocation();
	const float Range = CombatAttributes->GetRange();
	const FVector TurretForward = FacingDirection.Vector();

	// Draw range sphere
	DrawDebugSphere(
		GetWorld(),
		TurretLocation,
		Range,
		16,
		RangeDebugColor,
		false,
		-1.0f,
		0,
		2.0f
	);

	// Draw firing arc as a cone
	// Calculate cone direction and angle (180° = half sphere)
	DrawDebugCone(
		GetWorld(),
		TurretLocation,
		TurretForward,
		Range,
		PI / 2.0f, // 90° half-angle = 180° full arc
		PI / 2.0f,
		16,
		FiringArcDebugColor,
		false,
		-1.0f,
		0,
		2.0f
	);

	// Draw line to current target if we have one
	if (CurrentTarget)
	{
		DrawDebugLine(
			GetWorld(),
			TurretLocation,
			CurrentTarget->GetActorLocation(),
			TargetLineDebugColor,
			false,
			-1.0f,
			0,
			3.0f
		);

		// Draw sphere at target location
		DrawDebugSphere(
			GetWorld(),
			CurrentTarget->GetActorLocation(),
			25.0f,
			8,
			TargetLineDebugColor,
			false,
			-1.0f,
			0,
			2.0f
		);
	}

	// Draw forward direction arrow
	DrawDebugDirectionalArrow(
		GetWorld(),
		TurretLocation,
		TurretLocation + (TurretForward * 200.0f),
		50.0f,
		FColor::Blue,
		false,
		-1.0f,
		0,
		3.0f
	);
}

void ATurretBase::ToggleDebugVisualization()
{
	bShowDebugVisualization = !bShowDebugVisualization;
	UE_LOG(LogTemp, Log, TEXT("ATurretBase::ToggleDebugVisualization: Debug visualization %s for %s"),
		bShowDebugVisualization ? TEXT("ENABLED") : TEXT("DISABLED"),
		*GetName());
}

void ATurretBase::DebugShowTurretInfo()
{
	UE_LOG(LogTemp, Display, TEXT("=== TURRET DEBUG INFO ==="));
	UE_LOG(LogTemp, Display, TEXT("Turret: %s"), *GetName());
	UE_LOG(LogTemp, Display, TEXT("Mount Index: %d"), MountIndex);
	UE_LOG(LogTemp, Display, TEXT("Facing Direction: %s"), *FacingDirection.ToString());
	UE_LOG(LogTemp, Display, TEXT("Owner War Rig: %s"), OwnerWarRig ? *OwnerWarRig->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Display, TEXT("Current Target: %s"), CurrentTarget ? *CurrentTarget->GetName() : TEXT("NULL"));

	if (CombatAttributes)
	{
		UE_LOG(LogTemp, Display, TEXT("--- Attributes ---"));
		UE_LOG(LogTemp, Display, TEXT("Health: %.1f / %.1f"), CombatAttributes->GetHealth(), CombatAttributes->GetMaxHealth());
		UE_LOG(LogTemp, Display, TEXT("Damage: %.1f"), CombatAttributes->GetDamage());
		UE_LOG(LogTemp, Display, TEXT("Fire Rate: %.1f shots/sec"), CombatAttributes->GetFireRate());
		UE_LOG(LogTemp, Display, TEXT("Range: %.1f units"), CombatAttributes->GetRange());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("--- Attributes: NULL ---"));
	}

	UE_LOG(LogTemp, Display, TEXT("========================"));
}

bool ATurretBase::ValidateTurretSetup() const
{
	bool bIsValid = true;

	// Validate AbilitySystemComponent
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::ValidateTurretSetup: AbilitySystemComponent is null for %s"), *GetName());
		bIsValid = false;
	}

	// Validate CombatAttributes
	if (!CombatAttributes)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::ValidateTurretSetup: CombatAttributes is null for %s"), *GetName());
		bIsValid = false;
	}

	// Validate TurretMesh
	if (!TurretMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::ValidateTurretSetup: TurretMesh is null for %s"), *GetName());
		bIsValid = false;
	}

	// Validate TurretRoot
	if (!TurretRoot)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase::ValidateTurretSetup: TurretRoot is null for %s"), *GetName());
		bIsValid = false;
	}

	// Mount index validation (if initialized)
	if (MountIndex >= 0 && !OwnerWarRig)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATurretBase::ValidateTurretSetup: Mount index set but OwnerWarRig is null for %s"), *GetName());
		// Not a critical error - might not be initialized yet
	}

	return bIsValid;
}

bool ATurretBase::IsTargetValid(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	// Check if target is pending kill or being destroyed
	if (Target->IsPendingKill() || !IsValid(Target))
	{
		return false;
	}

	// Future implementation will check:
	// 1. Is target an enemy?
	// 2. Is target alive? (check health attribute)
	// 3. Is target on a valid team?
	// 4. Is target visible (line of sight)?

	return true;
}
