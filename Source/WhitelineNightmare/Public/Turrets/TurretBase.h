// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "TurretBase.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class USceneComponent;
class UStaticMeshComponent;
class UCombatAttributeSet;
class AWarRigPawn;
struct FTurretData;

/**
 * ATurretBase - Abstract base class for all turret types
 *
 * DESIGN:
 * - All turrets inherit from this base class
 * - Uses Gameplay Ability System for attributes and abilities
 * - Spawned at mount points on the war rig
 * - 180° firing arc from facing direction
 * - Target acquisition via sphere overlap + arc filtering
 *
 * LIFECYCLE:
 * 1. Spawned at mount point during placement
 * 2. Initialize() called to setup stats from data table
 * 3. Abilities granted (auto-fire ability)
 * 4. Begins targeting and firing
 * 5. Destroyed when war rig destroyed or player sells
 *
 * TARGETING:
 * - Sphere overlap query for potential targets (radius = Range attribute)
 * - Filters targets within 180° firing arc using dot product
 * - Priority: closest enemy in range + arc
 * - Returns nullptr if no valid targets
 */
UCLASS(Abstract)
class WHITELINENIGHTMARE_API ATurretBase : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATurretBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// === INITIALIZATION ===

	/**
	 * Initialize turret with data from data table
	 * Called after spawn to setup attributes, mesh, etc.
	 * @param TurretData - Data from turret data table
	 * @param InMountIndex - Index of mount point this turret occupies
	 * @param InFacingDirection - Forward facing direction for firing arc
	 * @param InOwnerWarRig - Reference to parent war rig
	 */
	virtual void Initialize(const FTurretData& TurretData, int32 InMountIndex, const FRotator& InFacingDirection, AWarRigPawn* InOwnerWarRig);

	// === VIRTUAL FUNCTIONS FOR SUBCLASSES ===

	/**
	 * Fire the turret's weapon
	 * Override in subclasses to implement specific firing behavior
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret|Combat")
	virtual void Fire();

	/**
	 * Find a valid target within range and firing arc
	 * @return Target actor, or nullptr if no valid targets
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret|Combat")
	virtual AActor* FindTarget();

	// === TARGET ACQUISITION ===

	/**
	 * Check if a potential target is within the turret's 180° firing arc
	 * Uses dot product: if dot > 0.0, target is within arc
	 * @param TargetLocation - World location of potential target
	 * @return true if target is within firing arc
	 */
	UFUNCTION(BlueprintPure, Category = "Turret|Combat")
	bool IsTargetInFiringArc(const FVector& TargetLocation) const;

	/**
	 * Get all potential targets within range (sphere overlap query)
	 * @return Array of actors within range sphere
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret|Combat")
	TArray<AActor*> GetPotentialTargets() const;

	// === GETTERS ===

	UFUNCTION(BlueprintPure, Category = "Turret")
	FORCEINLINE int32 GetMountIndex() const { return MountIndex; }

	UFUNCTION(BlueprintPure, Category = "Turret")
	FORCEINLINE FRotator GetFacingDirection() const { return FacingDirection; }

	UFUNCTION(BlueprintPure, Category = "Turret")
	FORCEINLINE AWarRigPawn* GetOwnerWarRig() const { return OwnerWarRig; }

	UFUNCTION(BlueprintPure, Category = "Turret")
	FORCEINLINE AActor* GetCurrentTarget() const { return CurrentTarget; }

	UFUNCTION(BlueprintPure, Category = "Turret")
	UCombatAttributeSet* GetCombatAttributeSet() const { return CombatAttributes; }

	// === DEBUG VISUALIZATION ===

	/** Draw debug visualization for firing arc, range, and current target */
	UFUNCTION(BlueprintCallable, Category = "Turret|Debug")
	void DrawDebugVisualization() const;

	/** Toggle debug visualization on/off */
	UFUNCTION(BlueprintCallable, Category = "Turret|Debug")
	void ToggleDebugVisualization();

	/** Console command: Show detailed turret info */
	UFUNCTION(Exec, Category = "Debug|Turret")
	void DebugShowTurretInfo();

protected:
	// === COMPONENTS ===

	/** Root scene component for positioning at mount points */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret|Components")
	TObjectPtr<USceneComponent> TurretRoot;

	/** Visual mesh component (varies by turret type) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret|Components")
	TObjectPtr<UStaticMeshComponent> TurretMesh;

	/** Ability System Component for GAS integration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Combat attribute set (Health, Damage, FireRate, Range) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turret|Attributes")
	TObjectPtr<UCombatAttributeSet> CombatAttributes;

	// === TURRET PROPERTIES ===

	/** Facing direction (forward vector) - set during placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Properties")
	FRotator FacingDirection;

	/** Which mount point this turret occupies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Properties")
	int32 MountIndex;

	/** Reference to parent war rig */
	UPROPERTY(BlueprintReadOnly, Category = "Turret|Properties")
	TObjectPtr<AWarRigPawn> OwnerWarRig;

	/** Current target being tracked */
	UPROPERTY(BlueprintReadOnly, Category = "Turret|Combat")
	TObjectPtr<AActor> CurrentTarget;

	/** Time since last fire (for fire rate timing) */
	UPROPERTY(BlueprintReadOnly, Category = "Turret|Combat")
	float TimeSinceLastFire;

	// === DEBUG ===

	/** Show debug visualization (firing arc, range, target line) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Debug")
	bool bShowDebugVisualization;

	/** Color for firing arc visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Debug")
	FColor FiringArcDebugColor;

	/** Color for range sphere visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Debug")
	FColor RangeDebugColor;

	/** Color for target line visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Debug")
	FColor TargetLineDebugColor;

	// === VALIDATION ===

	/** Validate that all required components and references are valid */
	bool ValidateTurretSetup() const;

	/** Validate that target is valid and alive */
	bool IsTargetValid(AActor* Target) const;
};
