// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "CombatAttributeSet.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * UCombatAttributeSet - Attribute set for turret combat stats
 *
 * Defines the core attributes used by all turrets:
 * - Health: Current turret health (can be damaged in future)
 * - MaxHealth: Maximum health capacity
 * - Damage: Base damage per shot
 * - FireRate: Shots per second
 * - Range: Maximum target acquisition distance
 *
 * Implements GAS lifecycle functions:
 * - PreAttributeChange: Clamps values before changes
 * - PostGameplayEffectExecute: Handles post-effect logic (turret destruction, etc.)
 */
UCLASS()
class WHITELINENIGHTMARE_API UCombatAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UCombatAttributeSet();

	// AttributeSet overrides
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ===== HEALTH =====

	/** Current health of the turret */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Health", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Health)

	/** Maximum health capacity */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Health", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, MaxHealth)

	// ===== DAMAGE =====

	/** Damage dealt per shot */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Damage", ReplicatedUsing = OnRep_Damage)
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Damage)

	// ===== FIRE RATE =====

	/** Shots per second */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|FireRate", ReplicatedUsing = OnRep_FireRate)
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, FireRate)

	// ===== RANGE =====

	/** Maximum target acquisition distance */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Range", ReplicatedUsing = OnRep_Range)
	FGameplayAttributeData Range;
	ATTRIBUTE_ACCESSORS(UCombatAttributeSet, Range)

protected:
	// Replication callbacks
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	virtual void OnRep_Damage(const FGameplayAttributeData& OldDamage);

	UFUNCTION()
	virtual void OnRep_FireRate(const FGameplayAttributeData& OldFireRate);

	UFUNCTION()
	virtual void OnRep_Range(const FGameplayAttributeData& OldRange);

	// Helper function to clamp health
	void ClampHealth();
};
