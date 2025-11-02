// Copyright Flatlander81. All Rights Reserved.

#include "GAS/Attributes/CombatAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

UCombatAttributeSet::UCombatAttributeSet()
{
	// Initialize default values
	Health = 100.0f;
	MaxHealth = 100.0f;
	Damage = 10.0f;
	FireRate = 1.0f;
	Range = 1000.0f;
}

void UCombatAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, FireRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCombatAttributeSet, Range, COND_None, REPNOTIFY_Always);
}

void UCombatAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp Health to valid range [0, MaxHealth]
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Ensure MaxHealth is never negative
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	// Ensure Damage is never negative
	else if (Attribute == GetDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	// Ensure FireRate is never negative
	else if (Attribute == GetFireRateAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	// Ensure Range is never negative
	else if (Attribute == GetRangeAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UCombatAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Clamp health after any gameplay effect
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		ClampHealth();

		// Handle turret destruction when health reaches zero
		if (GetHealth() <= 0.0f)
		{
			// Log turret destruction (actual destruction will be handled by turret class in future)
			UE_LOG(LogTemp, Warning, TEXT("UCombatAttributeSet: Turret health reached zero - turret should be destroyed"));

			// Future implementation will:
			// 1. Notify owning turret actor
			// 2. Play destruction effects
			// 3. Remove turret from mount point
			// 4. Update UI
		}
	}
	// Handle MaxHealth changes - need to re-clamp current health
	else if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		ClampHealth();
	}
}

void UCombatAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Health, OldHealth);
}

void UCombatAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, MaxHealth, OldMaxHealth);
}

void UCombatAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Damage, OldDamage);
}

void UCombatAttributeSet::OnRep_FireRate(const FGameplayAttributeData& OldFireRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, FireRate, OldFireRate);
}

void UCombatAttributeSet::OnRep_Range(const FGameplayAttributeData& OldRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCombatAttributeSet, Range, OldRange);
}

void UCombatAttributeSet::ClampHealth()
{
	const float CurrentHealth = GetHealth();
	const float CurrentMaxHealth = GetMaxHealth();

	if (CurrentHealth > CurrentMaxHealth)
	{
		SetHealth(CurrentMaxHealth);
	}
	else if (CurrentHealth < 0.0f)
	{
		SetHealth(0.0f);
	}
}
