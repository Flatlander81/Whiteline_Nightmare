// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WarRigHUD.h"
#include "Kismet/GameplayStatics.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWarRigPlayerController, Log, All);

AWarRigPlayerController::AWarRigPlayerController()
	: CurrentScrap(0)
	, StartingScrap(100)
	, bIsGameOver(false)
{
	// Enable ticking if needed
	PrimaryActorTick.bCanEverTick = false;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Initialize resources
	CurrentScrap = StartingScrap;

	// Enable mouse cursor for UI interaction
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Set input mode to allow both game and UI input
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Initialized with %d starting scrap"), StartingScrap);
	UE_LOG(LogWarRigPlayerController, Log, TEXT("WarRigPlayerController: Mouse cursor enabled for UI interaction"));
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

	// Bind "R" key to RestartGame (for game over restart)
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AWarRigPlayerController::RestartGame);

	UE_LOG(LogWarRigPlayerController, Log, TEXT("SetupInputComponent: Input component ready (Restart bound to 'R' key)"));
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

	// Set game over flag
	bIsGameOver = true;

	LogPlayerState();

	// Game over sequence is now handled by UGameplayAbility_GameOver
	// This function is kept for Blueprint compatibility and additional logic if needed
}

void AWarRigPlayerController::RestartGame()
{
	// Only allow restart if game is over
	if (!bIsGameOver)
	{
		UE_LOG(LogWarRigPlayerController, Warning, TEXT("RestartGame: Game is not over, ignoring restart request"));
		return;
	}

	UE_LOG(LogWarRigPlayerController, Log, TEXT("RestartGame: Restarting game..."));

	// Get the current level name
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogWarRigPlayerController, Error, TEXT("RestartGame: World is null!"));
		return;
	}

	FString CurrentLevelName = World->GetName();

	// Remove "UEDPIE_0_" prefix if present (editor play-in-editor)
	CurrentLevelName.RemoveFromStart(TEXT("UEDPIE_0_"));

	UE_LOG(LogWarRigPlayerController, Log, TEXT("RestartGame: Reloading level '%s'"), *CurrentLevelName);

	// Reload the current level
	UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
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
