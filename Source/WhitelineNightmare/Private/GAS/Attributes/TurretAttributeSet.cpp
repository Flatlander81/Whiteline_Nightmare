// Copyright Epic Games, Inc. All Rights Reserved.

#include "GAS/Attributes/TurretAttributeSet.h"
#include "Net/UnrealNetwork.h"

UTurretAttributeSet::UTurretAttributeSet()
{
	// Initialize default values
	InitDamage(10.0f);
	InitFireRate(1.0f);
	InitRange(1000.0f);
	InitFuelCostPerShot(0.5f);
}

void UTurretAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UTurretAttributeSet, Damage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTurretAttributeSet, FireRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTurretAttributeSet, Range, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UTurretAttributeSet, FuelCostPerShot, COND_None, REPNOTIFY_Always);
}

void UTurretAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UTurretAttributeSet::OnRep_Damage(const FGameplayAttributeData& OldDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTurretAttributeSet, Damage, OldDamage);
}

void UTurretAttributeSet::OnRep_FireRate(const FGameplayAttributeData& OldFireRate)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTurretAttributeSet, FireRate, OldFireRate);
}

void UTurretAttributeSet::OnRep_Range(const FGameplayAttributeData& OldRange)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTurretAttributeSet, Range, OldRange);
}

void UTurretAttributeSet::OnRep_FuelCostPerShot(const FGameplayAttributeData& OldFuelCostPerShot)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UTurretAttributeSet, FuelCostPerShot, OldFuelCostPerShot);
}

void UTurretAttributeSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetDamageAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetFireRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.1f, 100.0f);
	}
	else if (Attribute == GetRangeAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetFuelCostPerShotAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}
