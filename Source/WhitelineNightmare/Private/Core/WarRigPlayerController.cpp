// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "WarRig/WarRigPawn.h"
#include "WarRig/LaneSystemComponent.h"
#include "World/WorldScrollComponent.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigPlayerController, Log, All);

AWarRigPlayerController::AWarRigPlayerController()
	: CurrentScrap(0)
	, StartingScrap(100)
	, InputMappingContext(nullptr)
	, MoveLeftAction(nullptr)
	, MoveRightAction(nullptr)
{
	// No ticking needed
	PrimaryActorTick.bCanEverTick = false;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resources
	CurrentScrap = StartingScrap;

	// Note: SetupEnhancedInput() is now called in SetupInputComponent() to fix timing issue
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

	// Setup Enhanced Input BEFORE binding (fixes timing issue)
	SetupEnhancedInput();

	// Bind Enhanced Input actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveLeftAction)
		{
			// Use ETriggerEvent::Started to fire once per key press (not every frame)
			EnhancedInputComponent->BindAction(MoveLeftAction, ETriggerEvent::Started, this, &AWarRigPlayerController::OnMoveLeft);
			UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Bound MoveLeft action"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Warning, TEXT("SetupInputComponent: MoveLeftAction is null"));
		}

		if (MoveRightAction)
		{
			// Use ETriggerEvent::Started to fire once per key press (not every frame)
			EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Started, this, &AWarRigPlayerController::OnMoveRight);
			UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Bound MoveRight action"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Warning, TEXT("SetupInputComponent: MoveRightAction is null"));
		}

		UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Enhanced Input bindings complete"));
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("SetupInputComponent: Failed to cast to EnhancedInputComponent"));
	}
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

// Enhanced Input Setup

void AWarRigPlayerController::SetupEnhancedInput()
{
	// Get the Enhanced Input Local Player Subsystem
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("SetupEnhancedInput: Failed to get Enhanced Input Subsystem - check Project Settings -> Input"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  Make sure Default Player Input Class = EnhancedPlayerInput"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  Make sure Default Input Component Class = EnhancedInputComponent"));
		return;
	}

	// Check if assets are assigned from editor (manual setup)
	bool bUsingEditorAssets = (InputMappingContext != nullptr && MoveLeftAction != nullptr && MoveRightAction != nullptr);

	if (bUsingEditorAssets)
	{
		// Using editor-assigned assets
		UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupEnhancedInput: Using editor-assigned Input Assets"));
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - Mapping Context: %s"), *InputMappingContext->GetName());
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - Move Left Action: %s"), *MoveLeftAction->GetName());
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - Move Right Action: %s"), *MoveRightAction->GetName());
	}
	else
	{
		// Create programmatically
		UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupEnhancedInput: Creating Input Assets programmatically"));

		// Create Input Mapping Context
		InputMappingContext = NewObject<UInputMappingContext>(this, TEXT("WarRigInputMappingContext"));

		// Create Input Actions
		MoveLeftAction = NewObject<UInputAction>(this, TEXT("MoveLeftAction"));
		MoveLeftAction->ValueType = EInputActionValueType::Boolean;

		MoveRightAction = NewObject<UInputAction>(this, TEXT("MoveRightAction"));
		MoveRightAction->ValueType = EInputActionValueType::Boolean;

		// Add mappings to context
		// Move Left: A key and Left Arrow
		InputMappingContext->MapKey(MoveLeftAction, EKeys::A);
		InputMappingContext->MapKey(MoveLeftAction, EKeys::Left);

		// Move Right: D key and Right Arrow
		InputMappingContext->MapKey(MoveRightAction, EKeys::D);
		InputMappingContext->MapKey(MoveRightAction, EKeys::Right);

		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - Move Left: A or Left Arrow"));
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - Move Right: D or Right Arrow"));
	}

	// Add mapping context to subsystem (priority 0)
	Subsystem->AddMappingContext(InputMappingContext, 0);
	UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupEnhancedInput: Added mapping context to Enhanced Input Subsystem with priority 0"));
}

void AWarRigPlayerController::OnMoveLeft()
{
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetPawn());
	if (WarRig)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveLeft: Requesting lane change left"));
		bool bSuccess = WarRig->RequestLaneChange(-1);
		if (bSuccess)
		{
			UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveLeft: Lane change LEFT successful"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Warning, TEXT("OnMoveLeft: Lane change LEFT failed (already at leftmost lane or transitioning)"));
		}
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("OnMoveLeft: No War Rig pawn possessed"));
	}
}

void AWarRigPlayerController::OnMoveRight()
{
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(GetPawn());
	if (WarRig)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveRight: Requesting lane change right"));
		bool bSuccess = WarRig->RequestLaneChange(1);
		if (bSuccess)
		{
			UE_LOG(LogWarRigPlayerController, Log, TEXT("OnMoveRight: Lane change RIGHT successful"));
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Warning, TEXT("OnMoveRight: Lane change RIGHT failed (already at rightmost lane or transitioning)"));
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
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("=== ENHANCED INPUT DIAGNOSTIC ==="));

	// Get the Enhanced Input Local Player Subsystem
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("DebugListInputContexts: Enhanced Input Subsystem NOT FOUND!"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  >>> This means Project Settings -> Input is NOT configured for Enhanced Input! <<<"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  Required settings:"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("    - Default Player Input Class = EnhancedPlayerInput"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("    - Default Input Component Class = EnhancedInputComponent"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  After changing, you MUST restart the editor!"));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("Enhanced Input Subsystem: FOUND (OK)"));

	// Get all active mapping contexts
	TArray<FEnhancedActionKeyMapping> Mappings = Subsystem->GetAllPlayerMappableActionKeyMappings();
	UE_LOG(LogWarRigPlayerController, Log, TEXT("Total Player Mappable Key Mappings: %d"), Mappings.Num());

	// Check if our specific mapping context is present
	bool bFoundWarRigContext = false;
	if (InputMappingContext)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("Checking for our IMC_WarRig context: %s"), *InputMappingContext->GetName());

		// Check if it's in the subsystem
		if (Subsystem->HasMappingContext(InputMappingContext))
		{
			UE_LOG(LogWarRigPlayerController, Log, TEXT("  >>> IMC_WarRig IS ACTIVE in subsystem (OK) <<<"));
			bFoundWarRigContext = true;

			// Get priority
			int32 Priority = 0; // Default priority
			UE_LOG(LogWarRigPlayerController, Log, TEXT("  Priority: %d"), Priority);
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Error, TEXT("  >>> IMC_WarRig is NOT ACTIVE in subsystem! <<<"));
			UE_LOG(LogWarRigPlayerController, Error, TEXT("  This means AddMappingContext failed or was never called!"));
		}
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("InputMappingContext is NULL - not assigned!"));
	}

	// Check input actions
	UE_LOG(LogWarRigPlayerController, Log, TEXT("Input Actions:"));
	if (MoveLeftAction)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - MoveLeftAction: %s (OK)"), *MoveLeftAction->GetName());
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  - MoveLeftAction: NULL!"));
	}

	if (MoveRightAction)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("  - MoveRightAction: %s (OK)"), *MoveRightAction->GetName());
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  - MoveRightAction: NULL!"));
	}

	// Check if input component is Enhanced
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT("Input Component: EnhancedInputComponent (OK)"));
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("Input Component: NOT EnhancedInputComponent!"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("  >>> Check Project Settings -> Input -> Default Input Component Class <<<"));
	}

	UE_LOG(LogWarRigPlayerController, Warning, TEXT("================================="));
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("Run this command in PIE console to diagnose input issues"));
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("Then press A or D and check if callback logs appear"));
}

void AWarRigPlayerController::DebugShowKeyMappings()
{
	UE_LOG(LogWarRigPlayerController, Warning, TEXT("=== KEY MAPPINGS DIAGNOSTIC ==="));

	if (!InputMappingContext)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("InputMappingContext is NULL - no mappings to show!"));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("Input Mapping Context: %s"), *InputMappingContext->GetName());

	// Get all mappings from the context
	const TArray<FEnhancedActionKeyMapping>& Mappings = InputMappingContext->GetMappings();

	if (Mappings.Num() == 0)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT(">>> IMC_WarRig HAS ZERO KEY MAPPINGS! <<<"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT(">>> This is why input doesn't work! <<<"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT(""));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("You need to open IMC_WarRig in the editor and add mappings:"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("1. In Content Browser, navigate to Content/Input/"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("2. Double-click IMC_WarRig to open it"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("3. In the Mappings section, add:"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("   - IA_MoveLeft mapped to A and Left Arrow"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("   - IA_MoveRight mapped to D and Right Arrow"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT("4. Save the asset"));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("Total Mappings: %d"), Mappings.Num());
	UE_LOG(LogWarRigPlayerController, Log, TEXT(""));

	// List all mappings
	for (int32 i = 0; i < Mappings.Num(); i++)
	{
		const FEnhancedActionKeyMapping& Mapping = Mappings[i];

		if (Mapping.Action)
		{
			FString ActionName = Mapping.Action->GetName();
			FString KeyName = Mapping.Key.GetDisplayName().ToString();

			UE_LOG(LogWarRigPlayerController, Log, TEXT("  [%d] Action: %s -> Key: %s"), i, *ActionName, *KeyName);

			// Check if it's one of our expected actions
			if (Mapping.Action == MoveLeftAction)
			{
				UE_LOG(LogWarRigPlayerController, Log, TEXT("       ^ This is MoveLeftAction (CORRECT)"));
			}
			else if (Mapping.Action == MoveRightAction)
			{
				UE_LOG(LogWarRigPlayerController, Log, TEXT("       ^ This is MoveRightAction (CORRECT)"));
			}
		}
		else
		{
			UE_LOG(LogWarRigPlayerController, Warning, TEXT("  [%d] Invalid mapping - Action is NULL"), i);
		}
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT(""));

	// Check if we have the expected mappings
	bool bFoundMoveLeftA = false;
	bool bFoundMoveLeftArrow = false;
	bool bFoundMoveRightD = false;
	bool bFoundMoveRightArrow = false;

	for (const FEnhancedActionKeyMapping& Mapping : Mappings)
	{
		if (Mapping.Action == MoveLeftAction)
		{
			if (Mapping.Key == EKeys::A) bFoundMoveLeftA = true;
			if (Mapping.Key == EKeys::Left) bFoundMoveLeftArrow = true;
		}
		else if (Mapping.Action == MoveRightAction)
		{
			if (Mapping.Key == EKeys::D) bFoundMoveRightD = true;
			if (Mapping.Key == EKeys::Right) bFoundMoveRightArrow = true;
		}
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("Expected Mappings Check:"));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("  MoveLeft + A Key:         %s"), bFoundMoveLeftA ? TEXT("FOUND") : TEXT("MISSING"));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("  MoveLeft + Left Arrow:    %s"), bFoundMoveLeftArrow ? TEXT("FOUND") : TEXT("MISSING"));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("  MoveRight + D Key:        %s"), bFoundMoveRightD ? TEXT("FOUND") : TEXT("MISSING"));
	UE_LOG(LogWarRigPlayerController, Log, TEXT("  MoveRight + Right Arrow:  %s"), bFoundMoveRightArrow ? TEXT("FOUND") : TEXT("MISSING"));

	if (bFoundMoveLeftA && bFoundMoveLeftArrow && bFoundMoveRightD && bFoundMoveRightArrow)
	{
		UE_LOG(LogWarRigPlayerController, Log, TEXT(""));
		UE_LOG(LogWarRigPlayerController, Warning, TEXT(">>> ALL MAPPINGS ARE CORRECT! <<<"));
		UE_LOG(LogWarRigPlayerController, Warning, TEXT(">>> Input should be working! <<<"));
		UE_LOG(LogWarRigPlayerController, Warning, TEXT(">>> If input still doesn't work, click in the viewport and try again <<<"));
	}
	else
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT(""));
		UE_LOG(LogWarRigPlayerController, Error, TEXT(">>> SOME MAPPINGS ARE MISSING! <<<"));
		UE_LOG(LogWarRigPlayerController, Error, TEXT(">>> This is why input doesn't work! <<<"));
	}

	UE_LOG(LogWarRigPlayerController, Warning, TEXT("================================="));
}
