// Copyright Flatlander81. All Rights Reserved.

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
 * UWarRigAttributeSet - Attribute set for the War Rig's fuel system
 *
 * Manages fuel and max fuel attributes using the Gameplay Ability System.
 *
 * Attributes:
 * - Fuel: Current fuel amount (clamped to [0, MaxFuel])
 * - MaxFuel: Maximum fuel capacity
 *
 * Key Functions:
 * - PreAttributeChange: Clamps attribute values before they are changed
 * - PostGameplayEffectExecute: Handles post-effect logic (e.g., game over when fuel reaches 0)
 */
UCLASS()
class WHITELINENIGHTMARE_API UWarRigAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWarRigAttributeSet();

	// AttributeSet overrides
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// Fuel attribute - current fuel amount
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Fuel", ReplicatedUsing = OnRep_Fuel)
	FGameplayAttributeData Fuel;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, Fuel)

	// MaxFuel attribute - maximum fuel capacity
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Fuel", ReplicatedUsing = OnRep_MaxFuel)
	FGameplayAttributeData MaxFuel;
	ATTRIBUTE_ACCESSORS(UWarRigAttributeSet, MaxFuel)

	// Replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when Fuel is replicated
	UFUNCTION()
	virtual void OnRep_Fuel(const FGameplayAttributeData& OldFuel);

	// Called when MaxFuel is replicated
	UFUNCTION()
	virtual void OnRep_MaxFuel(const FGameplayAttributeData& OldMaxFuel);

	// Helper function to handle game over when fuel reaches 0
	void HandleFuelDepleted();

	// Helper function to clamp an attribute
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute,
		float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);
};
