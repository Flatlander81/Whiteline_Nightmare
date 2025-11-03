// Copyright Flatlander81. All Rights Reserved.

#include "GAS/GameplayAbility_GameOver.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UGameplayAbility_GameOver::UGameplayAbility_GameOver()
{
	// Set to instant activation with no cost or cooldown
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// This ability should only be activated once per game
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.GameOver")));

	GameOverReason = TEXT("Out of Fuel");
}

void UGameplayAbility_GameOver::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════"));
	UE_LOG(LogTemp, Warning, TEXT("    GAME OVER ABILITY ACTIVATED"));
	UE_LOG(LogTemp, Warning, TEXT("    Reason: %s"), *GameOverReason);
	UE_LOG(LogTemp, Warning, TEXT("═══════════════════════════════════════"));

	// Stop world scrolling
	StopWorldScrolling();

	// NOTE: Don't disable input - we need the R key to work for restart
	// The bIsGameOver flag on PlayerController prevents gameplay actions

	// Show game over UI
	ShowGameOverUI();

	// Play game over sound (optional)
	PlayGameOverSound();

	// Trigger game over in GameMode
	if (ActorInfo && ActorInfo->OwnerActor.IsValid())
	{
		UWorld* World = ActorInfo->OwnerActor->GetWorld();
		if (World)
		{
			AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
			if (GameMode)
			{
				GameMode->TriggerGameOver(false); // Player lost
				UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::ActivateAbility - Triggered game over in GameMode"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_GameOver::ActivateAbility - GameMode not found"));
			}
		}
	}
}

void UGameplayAbility_GameOver::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::EndAbility - Game over ability ended"));

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGameplayAbility_GameOver::StopWorldScrolling()
{
	// Get the world
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::StopWorldScrolling - No valid owner actor"));
		return;
	}

	UWorld* World = ActorInfo->OwnerActor->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::StopWorldScrolling - No valid world"));
		return;
	}

	// Get the GameMode
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_GameOver::StopWorldScrolling - GameMode not found"));
		return;
	}

	// Get the WorldScrollComponent
	UWorldScrollComponent* WorldScrollComponent = GameMode->WorldScrollComponent;
	if (!WorldScrollComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_GameOver::StopWorldScrolling - WorldScrollComponent not found"));
		return;
	}

	// Stop scrolling
	WorldScrollComponent->SetScrolling(false);
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::StopWorldScrolling - World scrolling stopped"));
}

void UGameplayAbility_GameOver::DisablePlayerInput()
{
	// Get the owning pawn
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::DisablePlayerInput - No valid owner actor"));
		return;
	}

	APawn* OwningPawn = Cast<APawn>(ActorInfo->OwnerActor.Get());
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::DisablePlayerInput - Owner is not a Pawn"));
		return;
	}

	// Get the player controller
	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_GameOver::DisablePlayerInput - No player controller found"));
		return;
	}

	// Disable input
	OwningPawn->DisableInput(PC);
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::DisablePlayerInput - Player input disabled"));
}

void UGameplayAbility_GameOver::ShowGameOverUI()
{
	// Get the player controller
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (!ActorInfo || !ActorInfo->OwnerActor.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - No valid owner actor"));
		return;
	}

	APawn* OwningPawn = Cast<APawn>(ActorInfo->OwnerActor.Get());
	if (!OwningPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Owner is not a Pawn"));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwningPawn->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - No player controller found"));
		return;
	}

	// Get the HUD and tell it to show game over screen (simple DrawText - placeholder)
	AHUD* HUD = PC->GetHUD();
	if (!HUD)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - No HUD found"));
		return;
	}

	AWarRigHUD* WarRigHUD = Cast<AWarRigHUD>(HUD);
	if (!WarRigHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - HUD is not a WarRigHUD"));
		return;
	}

	// Tell HUD to show game over screen (it will draw it in DrawHUD)
	WarRigHUD->ShowGameOverScreen(false); // Player lost

	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Game over screen activated on HUD"));

	// Set game over flag on PlayerController so restart works
	AWarRigPlayerController* WarRigPC = Cast<AWarRigPlayerController>(PC);
	if (WarRigPC)
	{
		WarRigPC->OnGameOver(false); // Sets bIsGameOver = true
		UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Set bIsGameOver on PlayerController"));
	}

	// Show mouse cursor
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
}

void UGameplayAbility_GameOver::PlayGameOverSound()
{
	// TODO: Implement game over sound
	// For now, just log
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::PlayGameOverSound - (Not implemented)"));
}
