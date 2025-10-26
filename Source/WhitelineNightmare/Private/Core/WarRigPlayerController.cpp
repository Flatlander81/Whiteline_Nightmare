// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/InputConfiguration.h"
#include "Data/GameplayDataStructs.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AWarRigPlayerController::AWarRigPlayerController()
{
	// Initialize values
	CurrentLane = 1; // Start in middle lane (0-indexed, so lane 1 is middle of 3 lanes)
	TotalLanes = 3;
	bIsChangingLanes = false;

	// Input will be set in Blueprints or editor
	DefaultMappingContext = nullptr;
	LaneChangeAction = nullptr;
	PauseAction = nullptr;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Get lane count from game mode
	if (AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode()))
	{
		FGameplayBalanceData BalanceData;
		if (GameMode->GetGameplayBalanceData(BalanceData))
		{
			InitializeLaneData(BalanceData.NumberOfLanes);
		}
	}

	// Setup programmatic input if not configured in editor
	if (!DefaultMappingContext)
	{
		UInputConfiguration::SetupInputSystem(this);
		DefaultMappingContext = UInputConfiguration::GetDefaultMappingContext(this);
		LaneChangeAction = UInputConfiguration::GetLaneChangeAction(this);
		PauseAction = UInputConfiguration::GetPauseAction(this);
	}

	// Setup Enhanced Input
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			UE_LOG(LogTemp, Log, TEXT("Added input mapping context to player controller"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to get input mapping context!"));
		}
	}
}

void AWarRigPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Setup Enhanced Input bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (LaneChangeAction)
		{
			EnhancedInputComponent->BindAction(LaneChangeAction, ETriggerEvent::Triggered, this, &AWarRigPlayerController::HandleLaneChange);
		}

		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &AWarRigPlayerController::HandlePause);
		}
	}
}

void AWarRigPlayerController::HandleLaneChange(const FInputActionValue& Value)
{
	// Get the axis value from input (will be -1 for left, +1 for right)
	const float DirectionValue = Value.Get<float>();

	if (FMath::Abs(DirectionValue) > 0.5f) // Threshold to avoid analog stick drift
	{
		const int32 Direction = DirectionValue > 0.0f ? 1 : -1;
		RequestLaneChange(Direction);
	}
}

void AWarRigPlayerController::HandlePause(const FInputActionValue& Value)
{
	// Toggle pause
	if (IsPaused())
	{
		SetPause(false);
	}
	else
	{
		SetPause(true);
	}
}

void AWarRigPlayerController::RequestLaneChange(int32 Direction)
{
	// Don't allow lane change if already changing or if at the edge
	if (bIsChangingLanes)
	{
		return;
	}

	const int32 NewLane = CurrentLane + Direction;

	// Check if new lane is valid
	if (NewLane < 0 || NewLane >= TotalLanes)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change to lane %d - out of bounds"), NewLane);
		return;
	}

	// Update current lane
	CurrentLane = NewLane;
	bIsChangingLanes = true;

	UE_LOG(LogTemp, Log, TEXT("Changing to lane %d"), CurrentLane);

	// TODO: Trigger lane change ability via GAS
	// This will be implemented when we create the GAS abilities
	// For now, we just update the lane index
	// The actual movement will be handled by the war rig pawn or via a gameplay ability

	// Note: SetIsChangingLanes(false) should be called by the ability or pawn when the lane change completes
}

void AWarRigPlayerController::InitializeLaneData(int32 NumLanes, int32 StartingLane)
{
	TotalLanes = FMath::Max(1, NumLanes);
	CurrentLane = FMath::Clamp(StartingLane, 0, TotalLanes - 1);

	UE_LOG(LogTemp, Log, TEXT("Initialized lane data: %d total lanes, starting in lane %d"), TotalLanes, CurrentLane);
}
