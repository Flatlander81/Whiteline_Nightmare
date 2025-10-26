// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "TurretAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set for Turrets
 * Contains gameplay attributes for turrets mounted on the war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API UTurretAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTurretAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// ========================================
	// Combat Attributes
	// ========================================

	/** Damage per shot */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UTurretAttributeSet, Damage)

	/** Fire rate (shots per second) */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_FireRate)
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UTurretAttributeSet, FireRate)

	/** Maximum range */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_Range)
	FGameplayAttributeData Range;
	ATTRIBUTE_ACCESSORS(UTurretAttributeSet, Range)

	/** Fuel cost per shot */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat", ReplicatedUsing = OnRep_FuelCostPerShot)
	FGameplayAttributeData FuelCostPerShot;
	ATTRIBUTE_ACCESSORS(UTurretAttributeSet, FuelCostPerShot)

protected:
	UFUNCTION()
	virtual void OnRep_Damage(const FGameplayAttributeData& OldDamage);

	UFUNCTION()
	virtual void OnRep_FireRate(const FGameplayAttributeData& OldFireRate);

	UFUNCTION()
	virtual void OnRep_Range(const FGameplayAttributeData& OldRange);

	UFUNCTION()
	virtual void OnRep_FuelCostPerShot(const FGameplayAttributeData& OldFuelCostPerShot);

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
