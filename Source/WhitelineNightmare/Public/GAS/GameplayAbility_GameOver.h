// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_GameOver.generated.h"

/**
 * UGameplayAbility_GameOver - Handles game over sequence
 *
 * This ability is triggered when the war rig dies from:
 * - Fuel depletion (from PostGameplayEffectExecute)
 * - Hull depletion (future)
 * - Obstacle collision (future)
 *
 * Flow:
 * 1. Ability is activated by death condition
 * 2. Stops world scrolling
 * 3. Disables player input
 * 4. Shows game over UI
 * 5. Waits for player to restart
 */
UCLASS()
class WHITELINENIGHTMARE_API UGameplayAbility_GameOver : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_GameOver();

	// GameplayAbility interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	// Game over reason passed to the UI
	UPROPERTY(BlueprintReadWrite, Category = "Game Over")
	FString GameOverReason;

protected:
	// Game over widget class to spawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Over")
	TSubclassOf<class UUserWidget> GameOverWidgetClass;

	// Cached game over widget instance
	UPROPERTY()
	TObjectPtr<class UUserWidget> GameOverWidget;

	/**
	 * Stop world scrolling
	 */
	void StopWorldScrolling();

	/**
	 * Disable player input
	 */
	void DisablePlayerInput();

	/**
	 * Show game over UI
	 */
	void ShowGameOverUI();

	/**
	 * Play game over sound
	 */
	void PlayGameOverSound();
};
