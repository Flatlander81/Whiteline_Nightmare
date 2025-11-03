// Copyright Flatlander81. All Rights Reserved.

#include "GAS/GameplayAbility_GameOver.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WarRigPlayerController.h"
#include "UI/GameOverWidget.h"
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

	// Disable player input
	DisablePlayerInput();

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

	// Create the game over widget (pure C++, no Blueprint required!)
	// Use UGameOverWidget::StaticClass() directly (same pattern as WarRigHUD with FuelWidget)
	TSubclassOf<UGameOverWidget> WidgetClass = GameOverWidgetClass;
	if (!WidgetClass)
	{
		// Fallback to C++ class if no Blueprint override is set
		WidgetClass = UGameOverWidget::StaticClass();
		UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Using default C++ UGameOverWidget class"));
	}

	GameOverWidget = CreateWidget<UGameOverWidget>(PC, WidgetClass);
	if (GameOverWidget)
	{
		// Cast to UGameOverWidget to set the reason
		UGameOverWidget* GameOverWidgetTyped = Cast<UGameOverWidget>(GameOverWidget);
		if (GameOverWidgetTyped)
		{
			GameOverWidgetTyped->SetGameOverReason(GameOverReason);
		}

		// Add to viewport with high Z-order
		GameOverWidget->AddToViewport(100);

		// CRITICAL: Set visibility (same as WarRigHUDWidget pattern)
		GameOverWidget->SetVisibility(ESlateVisibility::Visible);

		UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Game over widget added to viewport and set to Visible"));

		// Show mouse cursor for UI interaction
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		// Set input mode to Game and UI (allows both mouse and keyboard input)
		// Don't use UIOnly mode because it requires a focusable widget
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UGameplayAbility_GameOver::ShowGameOverUI - Failed to create game over widget"));
	}
}

void UGameplayAbility_GameOver::PlayGameOverSound()
{
	// TODO: Implement game over sound
	// For now, just log
	UE_LOG(LogTemp, Log, TEXT("UGameplayAbility_GameOver::PlayGameOverSound - (Not implemented)"));
}
