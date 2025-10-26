// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigPlayerController, Log, All);

AWarRigPlayerController::AWarRigPlayerController()
	: CurrentScrap(0)
	, StartingScrap(100)
{
	// Enable ticking if needed
	PrimaryActorTick.bCanEverTick = false;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resources
	CurrentScrap = StartingScrap;

	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Initialized with %d starting scrap"), StartingScrap);
	LogPlayerState();
}

void AWarRigPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Possessed pawn %s"), *InPawn->GetName());
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("WarRigPlayerController: Possessed null pawn"));
	}
}

void AWarRigPlayerController::OnUnPossess()
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Unpossessing pawn"));
	Super::OnUnPossess();
}

void AWarRigPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Input will be set up using Enhanced Input in the future
	// For now, this is just a placeholder
	if (!InputComponent)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("SetupInputComponent: InputComponent is null"));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Input component ready"));
}

bool AWarRigPlayerController::AddScrap(int32 Amount)
{
	// Validate amount (can be negative for subtraction)
	if (Amount == 0)
	{
		UE_LOG(LogWarRigPlayerController, Verbose, TEXT("AddScrap: Amount is zero, ignoring"));
		return false;
	}

	const int32 NewAmount = CurrentScrap + Amount;

	// Validate new amount
	if (!ValidateScrapAmount(NewAmount))
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("AddScrap: Invalid new amount %d (current: %d, delta: %d)"),
			NewAmount, CurrentScrap, Amount);
		return false;
	}

	// Update scrap
	const int32 OldScrap = CurrentScrap;
	CurrentScrap = NewAmount;

	UE_LOG(LogWarRigPlayerController, Log, TEXT("AddScrap: %d -> %d (delta: %d)"),
		OldScrap, CurrentScrap, Amount);

	// TODO: Notify HUD of scrap change
	// TODO: Trigger audio/visual feedback

	return true;
}

bool AWarRigPlayerController::CanAfford(int32 Cost) const
{
	// Validate cost
	if (Cost < 0)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("CanAfford: Negative cost %d"), Cost);
		return false;
	}

	return CurrentScrap >= Cost;
}

bool AWarRigPlayerController::SpendScrap(int32 Cost)
{
	// Validate cost
	if (Cost < 0)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("SpendScrap: Negative cost not allowed: %d"), Cost);
		return false;
	}

	// Check if player can afford
	if (!CanAfford(Cost))
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("SpendScrap: Cannot afford cost %d (current: %d)"),
			Cost, CurrentScrap);
		return false;
	}

	// Spend scrap (use negative amount for AddScrap)
	return AddScrap(-Cost);
}

void AWarRigPlayerController::OnGameOver(bool bPlayerWon)
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("OnGameOver: Player %s"), bPlayerWon ? TEXT("WON") : TEXT("LOST"));

	LogPlayerState();

	// TODO: Show game over UI
	// TODO: Disable input
	// TODO: Save statistics
}

bool AWarRigPlayerController::ValidateScrapAmount(int32 NewAmount) const
{
	// Scrap cannot be negative
	if (NewAmount < 0)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("ValidateScrapAmount: Negative scrap not allowed: %d"), NewAmount);
		return false;
	}

	// Sanity check for extremely large values (probably a bug or exploit)
	const int32 MaxReasonableScrap = 1000000; // One million should be plenty
	if (NewAmount > MaxReasonableScrap)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("ValidateScrapAmount: Scrap too large, possible bug: %d"), NewAmount);
		return false;
	}

	return true;
}

void AWarRigPlayerController::LogPlayerState() const
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("=== Player State ==="));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("Current Scrap: %d"), CurrentScrap);
	UE_LOG(LogWarRigPlayerController, Log, TEXT("==================="));
}
