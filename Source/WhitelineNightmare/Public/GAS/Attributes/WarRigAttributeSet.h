// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WarRigAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Attribute Set for the War Rig
 * Contains all gameplay attributes for the player's war rig
 */
UCLASS()
class WHITELINENIGHTMARE_API UWarRigAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWarRigAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========================================
	// Health Attributes
	// ========================================

	/** Current health - when reduced to 0, the war rig is destroyed */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, Health)

	/** Maximum health */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, MaxHealth)

	// ========================================
	// Fuel Attributes
	// ========================================

	/** Current fuel - when reduced to 0, game over */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Fuel", ReplicatedUsing = OnRep_Fuel)
	FGameplayAttributeData Fuel;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, Fuel)

	/** Maximum fuel capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Fuel", ReplicatedUsing = OnRep_MaxFuel)
	FGameplayAttributeData MaxFuel;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, MaxFuel)

	/** Rate at which fuel drains per second */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Fuel", ReplicatedUsing = OnRep_FuelDrainRate)
	FGameplayAttributeData FuelDrainRate;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, FuelDrainRate)

	// ========================================
	// Resource Attributes
	// ========================================

	/** Current scrap for building turrets */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Resources", ReplicatedUsing = OnRep_Scrap)
	FGameplayAttributeData Scrap;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, Scrap)

	// ========================================
	// Movement Attributes
	// ========================================

	/** World scroll speed multiplier */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement", ReplicatedUsing = OnRep_ScrollSpeedMultiplier)
	FGameplayAttributeData ScrollSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, ScrollSpeedMultiplier)

	// ========================================
	// Meta Attributes (not replicated, used for calculations)
	// ========================================

	/** Incoming damage before mitigation */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, IncomingDamage)

	/** Incoming healing */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingHealing;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, IncomingHealing)

protected:
	// Replication notification handlers
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Fuel(const FGameplayAttributeData& OldFuel);

	UFUNCTION()
	virtual void OnRep_MaxFuel(const FGameplayAttributeData& OldMaxFuel);

	UFUNCTION()
	virtual void OnRep_FuelDrainRate(const FGameplayAttributeData& OldFuelDrainRate);

	UFUNCTION()
	virtual void OnRep_Scrap(const FGameplayAttributeData& OldScrap);

	UFUNCTION()
	virtual void OnRep_ScrollSpeedMultiplier(const FGameplayAttributeData& OldScrollSpeedMultiplier);

private:
	/** Helper function to clamp attribute values */
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
};
