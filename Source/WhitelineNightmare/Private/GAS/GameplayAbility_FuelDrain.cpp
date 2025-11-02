// Copyright Flatlander81. All Rights Reserved.

#include "GAS/GameplayAbility_FuelDrain.h"
#include "GAS/WarRigAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "TimerManager.h"
#include "Engine/World.h"

UGameplayAbility_FuelDrain::UGameplayAbility_FuelDrain()
{
	// Set to instant activation with no cost or cooldown
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// Default values
	DrainInterval = 1.0f; // Drain every 1 second
	FuelDrainRate = 5.0f; // Default 5 fuel per second
	bFuelDrainPaused = false;

	// This ability should activate on granted and never end
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.FuelDrain")));
}

void UGameplayAbility_FuelDrain::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_FuelDrain::ActivateAbility - Fuel drain started (Rate: %.2f per %.2fs)"),
		FuelDrainRate, DrainInterval);

	// Start the periodic fuel drain timer
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UWorld* World = ActorInfo->OwnerActor->GetWorld();
		if (World)
		{
			World->GetTimerManager().SetTimer(
				DrainTimerHandle,
				this,
				&UGameplayAbility_FuelDrain::ApplyFuelDrain,
				DrainInterval,
				true // Loop
			);
		}
	}
}

void UGameplayAbility_FuelDrain::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// Clear the timer
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UWorld* World = ActorInfo->OwnerActor->GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(DrainTimerHandle);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_FuelDrain::EndAbility - Fuel drain stopped"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_FuelDrain::ApplyFuelDrain()
{
	// Check if fuel drain is paused
	if (bFuelDrainPaused)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UGameplayAbility_FuelDrain::ApplyFuelDrain - Fuel drain paused, skipping"));
		return;
	}

	// Get the ability system component
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_FuelDrain::ApplyFuelDrain - No AbilitySystemComponent found!"));
		return;
	}

	// For now, we'll manually modify the fuel attribute until we have a GameplayEffect asset
	// TODO: Use FuelDrainEffectClass when we have the GameplayEffect asset set up

	// Get all attribute sets
	TArray<UAttributeSet*> AttributeSets;
	ASC->GetAllAttributes(AttributeSets);

	// Find the WarRigAttributeSet
	UWarRigAttributeSet* WarRigAttributeSet = nullptr;
	for (UAttributeSet* AttrSet : AttributeSets)
	{
		WarRigAttributeSet = Cast<UWarRigAttributeSet>(AttrSet);
		if (WarRigAttributeSet)
		{
			break;
		}
	}

	if (!WarRigAttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_FuelDrain::ApplyFuelDrain - WarRigAttributeSet not found!"));
		return;
	}

	// Get the Fuel attribute
	FGameplayAttribute FuelAttribute = WarRigAttributeSet->GetFuelAttribute();
	if (!FuelAttribute.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_FuelDrain::ApplyFuelDrain - Fuel attribute not valid!"));
		return;
	}

	// Apply the fuel drain
	float CurrentFuel = WarRigAttributeSet->GetFuel();
	float NewFuel = FMath::Max(0.0f, CurrentFuel - FuelDrainRate);

	ASC->SetNumericAttributeBase(FuelAttribute, NewFuel);

	UE_LOG(LogTemp, Verbose, TEXT("UGameplayAbility_FuelDrain::ApplyFuelDrain - Drained %.2f fuel (%.2f -> %.2f)"),
		FuelDrainRate, CurrentFuel, NewFuel);
}
