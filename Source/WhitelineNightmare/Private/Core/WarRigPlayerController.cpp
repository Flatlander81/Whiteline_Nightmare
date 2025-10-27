// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "WarRig/WarRigPawn.h"
#include "WarRig/LaneSystemComponent.h"
#include "World/WorldScrollComponent.h"
#include "Core/WhitelineNightmareGameMode.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigPlayerController, Log, All);

AWarRigPlayerController::AWarRigPlayerController()
	: CurrentScrap(0)
	, StartingScrap(100)
{
	// Ticking not needed - using legacy input system
	PrimaryActorTick.bCanEverTick = false;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resources
	CurrentScrap = StartingScrap;

	// Note: SetInputMode() is now called in OnPossess() to ensure it persists through re-possession

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

	// CRITICAL: Set input mode to Game Only so that keyboard input works
	// This must be in OnPossess (not BeginPlay) because the pawn can be unpossessed/re-possessed
	// during initialization, which would reset the input mode
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false; // Hide cursor in game mode

	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Set input mode to Game Only"));
}

void AWarRigPlayerController::OnUnPossess()
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Unpossessing pawn"));
	Super::OnUnPossess();
}

void AWarRigPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("SetupInputComponent: InputComponent is null"));
		return;
	}

	// Bind legacy input actions (defined in DefaultInput.ini)
	InputComponent->BindAction("MoveLeft", IE_Pressed, this, &AWarRigPlayerController::OnMoveLeft);
	InputComponent->BindAction("MoveRight", IE_Pressed, this, &AWarRigPlayerController::OnMoveRight);

	UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Bound legacy input actions (MoveLeft, MoveRight)"));
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

// Legacy Input Callbacks

void AWarRigPlayerController::OnMoveLeft()
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveLeft: Input received"));

	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetPawn());
	if (WarRig)
	{
		bool bSuccess = WarRig->RequestLaneChange(-1);
		if (bSuccess)
		{
			UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveLeft: Lane change LEFT successful"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Verbose, TEXT("OnMoveLeft: Lane change LEFT failed (already transitioning)"));
		}
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("OnMoveLeft: No War Rig pawn possessed"));
	}
}

void AWarRigPlayerController::OnMoveRight()
{
	UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveRight: Input received"));

	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetPawn());
	if (WarRig)
	{
		bool bSuccess = WarRig->RequestLaneChange(1);
		if (bSuccess)
		{
			UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveRight: Lane change RIGHT successful"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Verbose, TEXT("OnMoveRight: Lane change RIGHT failed (already transitioning)"));
		}
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("OnMoveRight: No War Rig pawn possessed"));
	}
}

// Debug Console Commands

void AWarRigPlayerController::DebugShowLanes()
{
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetPawn());
	if (!WarRig)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugShowLanes: No War Rig pawn possessed."));
		return;
	}

	ULaneSystemComponent* LaneSystem = WarRig->GetLaneSystemComponent();
	if (!LaneSystem)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugShowLanes: War Rig has no Lane System Component."));
		return;
	}

	bool bNewState = !LaneSystem->IsDebugVisualizationEnabled();
	LaneSystem->SetDebugVisualization(bNewState);

	UE_LOG(LogWarRigPlayerController, Log, TEXT("DebugShowLanes: Lane visualization %s"),
		bNewState ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void AWarRigPlayerController::DebugShowTileBounds()
{
	// TODO: Implement tile bounds visualization
	// This will require access to the GroundTilePoolComponent, which should be on the game mode
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugShowTileBounds: No game mode found."));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("DebugShowTileBounds: Not yet implemented - needs GroundTilePoolComponent on GameMode."));
}

void AWarRigPlayerController::DebugSetScrollSpeed(float Speed)
{
	if (Speed < 0.0f)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugSetScrollSpeed: Invalid speed %.2f (must be non-negative)."), Speed);
		return;
	}

	// TODO: Get WorldScrollComponent from game mode
	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugSetScrollSpeed: No game mode found."));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("DebugSetScrollSpeed: Not yet implemented - needs WorldScrollComponent on GameMode."));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("DebugSetScrollSpeed: Requested speed: %.2f"), Speed);
}

void AWarRigPlayerController::DebugListInputContexts()
{
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugListInputContexts: Not available with legacy input system"));
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("  This command is for Enhanced Input diagnostics only"));
}

void AWarRigPlayerController::DebugShowKeyMappings()
{
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("DebugShowKeyMappings: Not available with legacy input system"));
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("  This command is for Enhanced Input diagnostics only"));
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("  Check Config/DefaultInput.ini for legacy action mappings"));
}
