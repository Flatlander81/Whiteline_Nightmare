// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WarRigPlayerController.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogWarRigController, Log, All);

AWarRigPlayerController::AWarRigPlayerController()
	: CachedGameMode(nullptr)
	, CurrentScrap(0)
{
	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWarRigPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWarRigController, Log, TEXT("WarRigPlayerController initialized"));

	// Initialize references
	InitializeReferences();

	// Validate state
	if (!ValidatePlayerState())
	{
		UE_LOG(LogWarRigController, Error, TEXT("Player state validation failed"));
	}

	// Show mouse cursor for UI interaction
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AWarRigPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!InputComponent)
	{
		UE_LOG(LogWarRigController, Error, TEXT("InputComponent is null"));
		return;
	}

	// Note: Enhanced Input System setup will be added later when implementing input
	// For now, just log that setup was called
	UE_LOG(LogWarRigController, Log, TEXT("Input component setup complete"));
}

void AWarRigPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Periodic validation in non-shipping builds
#if !UE_BUILD_SHIPPING
	static float ValidationTimer = 0.0f;
	ValidationTimer += DeltaSeconds;
	if (ValidationTimer >= 5.0f) // Validate every 5 seconds
	{
		ValidatePlayerState();
		ValidationTimer = 0.0f;
	}
#endif
}

AWhitelineNightmareGameMode* AWarRigPlayerController::GetWhitelineGameMode() const
{
	// Return cached game mode if available
	if (CachedGameMode)
	{
		return CachedGameMode;
	}

	// Try to get and cache game mode
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogWarRigController, Error, TEXT("World is null"));
		return nullptr;
	}

	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Game mode is not AWhitelineNightmareGameMode"));
		return nullptr;
	}

	// Cache for future calls (const_cast is safe here as we're only caching a reference)
	const_cast<AWarRigPlayerController*>(this)->CachedGameMode = GameMode;
	return GameMode;
}

void AWarRigPlayerController::NotifyGameOver(bool bVictory)
{
	UE_LOG(LogWarRigController, Display, TEXT("Game Over notification: %s"), bVictory ? TEXT("Victory") : TEXT("Defeat"));

	// Disable input
	DisableInput(this);

	// Hide mouse cursor
	bShowMouseCursor = false;

	// Show game over UI (to be implemented)
	// For now, just log
	if (bVictory)
	{
		UE_LOG(LogWarRigController, Display, TEXT("Player achieved victory!"));
	}
	else
	{
		UE_LOG(LogWarRigController, Display, TEXT("Player was defeated."));
	}
}

void AWarRigPlayerController::AddScrap(int32 Amount)
{
	if (Amount < 0)
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Attempted to add negative scrap: %d"), Amount);
		return;
	}

	CurrentScrap += Amount;
	UE_LOG(LogWarRigController, Log, TEXT("Scrap added: %d, Total: %d"), Amount, CurrentScrap);
}

bool AWarRigPlayerController::SpendScrap(int32 Amount)
{
	if (Amount < 0)
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Attempted to spend negative scrap: %d"), Amount);
		return false;
	}

	if (CurrentScrap < Amount)
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Insufficient scrap. Required: %d, Available: %d"), Amount, CurrentScrap);
		return false;
	}

	CurrentScrap -= Amount;
	UE_LOG(LogWarRigController, Log, TEXT("Scrap spent: %d, Remaining: %d"), Amount, CurrentScrap);
	return true;
}

void AWarRigPlayerController::OnLaneChangeLeft()
{
	// To be implemented with lane change system
	UE_LOG(LogWarRigController, Log, TEXT("Lane change left requested"));
}

void AWarRigPlayerController::OnLaneChangeRight()
{
	// To be implemented with lane change system
	UE_LOG(LogWarRigController, Log, TEXT("Lane change right requested"));
}

void AWarRigPlayerController::OnPause()
{
	UE_LOG(LogWarRigController, Log, TEXT("Pause requested"));

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

void AWarRigPlayerController::InitializeReferences()
{
	// Cache game mode reference
	GetWhitelineGameMode();

	if (CachedGameMode)
	{
		UE_LOG(LogWarRigController, Log, TEXT("Game mode reference cached successfully"));
	}
	else
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Failed to cache game mode reference"));
	}
}

bool AWarRigPlayerController::ValidatePlayerState() const
{
	bool bValid = true;

	// Check world
	if (!GetWorld())
	{
		UE_LOG(LogWarRigController, Error, TEXT("Validation failed: World is null"));
		bValid = false;
	}

	// Check input component
	if (!InputComponent)
	{
		UE_LOG(LogWarRigController, Error, TEXT("Validation failed: InputComponent is null"));
		bValid = false;
	}

	// Check game mode
	if (!CachedGameMode && !GetWhitelineGameMode())
	{
		UE_LOG(LogWarRigController, Warning, TEXT("Validation warning: Game mode is not available"));
		// Not a critical failure, so don't set bValid to false
	}

	// Validate scrap is non-negative
	if (CurrentScrap < 0)
	{
		UE_LOG(LogWarRigController, Error, TEXT("Validation failed: Scrap is negative (%d)"), CurrentScrap);
		bValid = false;
	}

	return bValid;
}
