// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Attributes/WarRigAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

UWarRigAttributeSet::UWarRigAttributeSet()
{
	// Initialize default values
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitFuel(100.0f);
	InitMaxFuel(100.0f);
	InitFuelDrainRate(1.0f);
	InitScrap(0.0f);
	InitScrollSpeedMultiplier(1.0f);
	InitIncomingDamage(0.0f);
	InitIncomingHealing(0.0f);
}

void UWarRigAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, Fuel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, MaxFuel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, FuelDrainRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, Scrap, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, ScrollSpeedMultiplier, COND_None, REPNOTIFY_Always);
}

void UWarRigAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UWarRigAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* SourceASC = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	// Get the target actor
	AActor* TargetActor = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
	}

	// Handle incoming damage
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.0f);

		if (LocalIncomingDamage > 0.0f)
		{
			// Apply damage to health
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			// TODO: Broadcast damage event for UI/audio feedback
		}
	}

	// Handle incoming healing
	else if (Data.EvaluatedData.Attribute == GetIncomingHealingAttribute())
	{
		const float LocalIncomingHealing = GetIncomingHealing();
		SetIncomingHealing(0.0f);

		if (LocalIncomingHealing > 0.0f)
		{
			// Apply healing to health
			const float NewHealth = GetHealth() + LocalIncomingHealing;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
		}
	}

	// Clamp Health
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));

		// Check for death
		if (GetHealth() <= 0.0f)
		{
			// TODO: Trigger game over
			UE_LOG(LogTemp, Warning, TEXT("War Rig destroyed!"));
		}
	}

	// Clamp Fuel
	else if (Data.EvaluatedData.Attribute == GetFuelAttribute())
	{
		SetFuel(FMath::Clamp(GetFuel(), 0.0f, GetMaxFuel()));

		// Check for fuel depletion
		if (GetFuel() <= 0.0f)
		{
			// TODO: Trigger game over (out of fuel)
			UE_LOG(LogTemp, Warning, TEXT("Out of fuel!"));
		}
	}

	// Clamp Scrap (can't go negative)
	else if (Data.EvaluatedData.Attribute == GetScrapAttribute())
	{
		SetScrap(FMath::Max(GetScrap(), 0.0f));
	}

	// Clamp scroll speed multiplier to reasonable range
	else if (Data.EvaluatedData.Attribute == GetScrollSpeedMultiplierAttribute())
	{
		SetScrollSpeedMultiplier(FMath::Clamp(GetScrollSpeedMultiplier(), 0.1f, 5.0f));
	}
}

void UWarRigAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, Health, OldHealth);
}

void UWarRigAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, MaxHealth, OldMaxHealth);
}

void UWarRigAttributeSet::OnRep_Fuel(const FGameplayAttributeData& OldFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, Fuel, OldFuel);
}

void UWarRigAttributeSet::OnRep_MaxFuel(const FGameplayAttributeData& OldMaxFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, MaxFuel, OldMaxFuel);
}

void UWarRigAttributeSet::OnRep_FuelDrainRate(const FGameplayAttributeData& OldFuelDrainRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, FuelDrainRate, OldFuelDrainRate);
}

void UWarRigAttributeSet::OnRep_Scrap(const FGameplayAttributeData& OldScrap)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, Scrap, OldScrap);
}

void UWarRigAttributeSet::OnRep_ScrollSpeedMultiplier(const FGameplayAttributeData& OldScrollSpeedMultiplier)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, ScrollSpeedMultiplier, OldScrollSpeedMultiplier);
}

void UWarRigAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetFuelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxFuel());
	}
	else if (Attribute == GetMaxFuelAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetScrapAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetScrollSpeedMultiplierAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 5.0f);
	}
}
