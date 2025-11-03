// Copyright Flatlander81. All Rights Reserved.

#include "GAS/WarRigAttributeSet.h"
#include "GAS/GameplayAbility_GameOver.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "AbilitySystemComponent.h"

UWarRigAttributeSet::UWarRigAttributeSet()
{
	// Default values will be set by the gameplay effect or initialization
}

void UWarRigAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, Fuel, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UWarRigAttributeSet, MaxFuel, COND_None, REPNOTIFY_Always);
}

void UWarRigAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp Fuel to [0, MaxFuel]
	if (Attribute == GetFuelAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxFuel());
		UE_LOG(LogTemp, Verbose, TEXT("UWarRigAttributeSet::PreAttributeChange - Fuel clamped to %.2f (MaxFuel: %.2f)"),
			NewValue, GetMaxFuel());
	}
}

void UWarRigAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// Check if Fuel attribute changed
	if (Data.EvaluatedData.Attribute == GetFuelAttribute())
	{
		// Clamp Fuel to [0, MaxFuel] (defensive, should already be clamped by PreAttributeChange)
		SetFuel(FMath::Clamp(GetFuel(), 0.0f, GetMaxFuel()));

		UE_LOG(LogTemp, Log, TEXT("UWarRigAttributeSet::PostGameplayEffectExecute - Fuel changed to %.2f / %.2f"),
			GetFuel(), GetMaxFuel());

		// Check if fuel depleted
		if (GetFuel() <= 0.0f)
		{
			HandleFuelDepleted(Data);
		}
	}
}

void UWarRigAttributeSet::OnRep_Fuel(const FGameplayAttributeData& OldFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, Fuel, OldFuel);
}

void UWarRigAttributeSet::OnRep_MaxFuel(const FGameplayAttributeData& OldMaxFuel)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UWarRigAttributeSet, MaxFuel, OldMaxFuel);
}

void UWarRigAttributeSet::HandleFuelDepleted(const FGameplayEffectModCallbackData& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("UWarRigAttributeSet::HandleFuelDepleted - FUEL DEPLETED! Triggering game over..."));

	// Get the owning actor
	AActor* OwningActor = GetOwningActor();
	if (!OwningActor)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarRigAttributeSet::HandleFuelDepleted - No owning actor found!"));
		return;
	}

	// Get the ability system component
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarRigAttributeSet::HandleFuelDepleted - No AbilitySystemComponent found!"));
		return;
	}

	// Find the game over ability by class
	FGameplayAbilitySpec* GameOverSpec = ASC->FindAbilitySpecFromClass(UGameplayAbility_GameOver::StaticClass());
	if (!GameOverSpec)
	{
		UE_LOG(LogTemp, Error, TEXT("UWarRigAttributeSet::HandleFuelDepleted - Game over ability not found!"));
		UE_LOG(LogTemp, Error, TEXT("Make sure GameOverAbilityClass is set in BP_WarRig Blueprint!"));
		return;
	}

	// Try to activate the game over ability
	if (!ASC->TryActivateAbility(GameOverSpec->Handle))
	{
		UE_LOG(LogTemp, Error, TEXT("UWarRigAttributeSet::HandleFuelDepleted - Failed to activate game over ability!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("    GAME OVER - OUT OF FUEL"));
	UE_LOG(LogTemp, Warning, TEXT("    Game over ability activated"));
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════"));
}

void UWarRigAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}
