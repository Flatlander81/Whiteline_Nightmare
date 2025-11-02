// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_FuelDrain.generated.h"

/**
 * UGameplayAbility_FuelDrain - Passive ability that drains fuel over time
 *
 * This ability is granted on BeginPlay and activates automatically.
 * It loops indefinitely, applying a fuel drain effect every second.
 *
 * The fuel drain rate is configurable and can be modified via debug commands.
 *
 * Flow:
 * 1. Ability is granted on BeginPlay
 * 2. Activates immediately
 * 3. Every DrainInterval seconds, applies fuel drain effect
 * 4. Repeats until ability is ended or owner is destroyed
 */
UCLASS()
class WHITELINENIGHTMARE_API UGameplayAbility_FuelDrain : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_FuelDrain();

	// GameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// How often to drain fuel (in seconds)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fuel Drain")
	float DrainInterval;

	// How much fuel to drain per interval
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fuel Drain")
	float FuelDrainRate;

	// Gameplay Effect class to use for draining fuel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fuel Drain")
	TSubclassOf<class UGameplayEffect> FuelDrainEffectClass;

	// Toggle to pause/resume fuel drain
	UPROPERTY(BlueprintReadWrite, Category = "Fuel Drain")
	bool bFuelDrainPaused;

protected:
	// Timer handle for periodic fuel drain
	FTimerHandle DrainTimerHandle;

	// Function called by timer to drain fuel
	void ApplyFuelDrain();
};
