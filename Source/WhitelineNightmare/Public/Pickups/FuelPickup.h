// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ObjectPoolTypes.h"
#include "Core/GameDataStructs.h"
#include "FuelPickup.generated.h"

// Forward declarations
class USphereComponent;
class UWorldScrollComponent;
class AWarRigPawn;
class UGameplayEffect;
class USoundBase;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * AFuelPickup - Poolable actor that restores fuel to the war rig
 *
 * Implements IPoolableActor for object pooling
 * Visual: Bright green sphere (configurable via data table)
 * Scrolls backward with world scroll speed
 * Restores fuel on overlap with war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API AFuelPickup : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	AFuelPickup();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// IPoolableActor interface implementation
	virtual void OnActivated_Implementation() override;
	virtual void OnDeactivated_Implementation() override;
	virtual void ResetState_Implementation() override;

	/**
	 * Initialize the pickup with data from data table
	 * @param RowName - Name of the row in the pickup data table
	 * @param DataTable - The pickup data table to use
	 */
	void InitializeFromDataTable(FName RowName, UDataTable* DataTable);

	/**
	 * Set the world scroll component reference for movement
	 * @param ScrollComponent - The world scroll component to use
	 */
	void SetWorldScrollComponent(UWorldScrollComponent* ScrollComponent);

	/**
	 * Set the owning pool component
	 * @param InPoolComponent - The pool component that manages this pickup
	 */
	void SetPoolComponent(class UPickupPoolComponent* InPoolComponent);

protected:
	/** Sphere component for collision and visual representation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComponent;

	/** Reference to world scroll component for movement */
	UPROPERTY()
	TObjectPtr<UWorldScrollComponent> WorldScrollComponent;

	/** Reference to the pool component that manages this pickup */
	UPROPERTY()
	TObjectPtr<class UPickupPoolComponent> PoolComponent;

	/** Pickup data loaded from data table */
	UPROPERTY(BlueprintReadOnly, Category = "Pickup")
	FPickupData PickupData;

	/** GameplayEffect class to apply fuel restoration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup|Gameplay Effect")
	TSubclassOf<UGameplayEffect> FuelRestoreEffectClass;

	/** Data table containing pickup configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UDataTable> PickupDataTable;

	/** Row name in the pickup data table */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	FName PickupDataRowName;

	/** Current Niagara component for particle effect (if any) */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveParticleComponent;

	/** Handle sphere overlap events */
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Apply fuel restoration to the war rig */
	void ApplyFuelRestore(AWarRigPawn* WarRig);

	/** Play pickup effects (sound and particle) */
	void PlayPickupEffects();

	/** Update visual appearance based on pickup data */
	void UpdateVisualAppearance();
};
