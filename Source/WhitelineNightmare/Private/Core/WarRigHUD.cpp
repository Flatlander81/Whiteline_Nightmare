// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/WarRigHUD.h"
#include "Core/WhitelineNightmareGameMode.h"
#include "Core/WarRigPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogWarRigHUD, Log, All);

AWarRigHUD::AWarRigHUD()
	: GameUIWidget(nullptr)
	, VictoryScreenWidget(nullptr)
	, GameOverScreenWidget(nullptr)
	, bShowDebugInfo(false)
{
	// Enable tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWarRigHUD::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogWarRigHUD, Log, TEXT("WarRigHUD initialized"));

	// Create widgets
	CreateWidgets();

	// Show game UI by default
	ShowGameUI();

	// Validate state
	if (!ValidateHUDState())
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("HUD state validation failed"));
	}
}

void AWarRigHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw debug info if enabled
	if (bShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

void AWarRigHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Periodic validation in non-shipping builds
#if !UE_BUILD_SHIPPING
	static float ValidationTimer = 0.0f;
	ValidationTimer += DeltaSeconds;
	if (ValidationTimer >= 10.0f) // Validate every 10 seconds
	{
		ValidateHUDState();
		ValidationTimer = 0.0f;
	}
#endif
}

void AWarRigHUD::ShowGameUI()
{
	if (!GameUIWidget)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Cannot show game UI: widget not created"));
		return;
	}

	if (!GameUIWidget->IsInViewport())
	{
		GameUIWidget->AddToViewport(0); // Z-order 0 (bottom layer)
		UE_LOG(LogWarRigHUD, Log, TEXT("Game UI shown"));
	}
}

void AWarRigHUD::HideGameUI()
{
	if (!GameUIWidget)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Cannot hide game UI: widget not created"));
		return;
	}

	if (GameUIWidget->IsInViewport())
	{
		GameUIWidget->RemoveFromParent();
		UE_LOG(LogWarRigHUD, Log, TEXT("Game UI hidden"));
	}
}

void AWarRigHUD::ShowVictoryScreen()
{
	if (!VictoryScreenWidget)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Cannot show victory screen: widget not created"));
		return;
	}

	// Hide game UI
	HideGameUI();

	if (!VictoryScreenWidget->IsInViewport())
	{
		VictoryScreenWidget->AddToViewport(100); // High Z-order (top layer)
		UE_LOG(LogWarRigHUD, Log, TEXT("Victory screen shown"));
	}
}

void AWarRigHUD::ShowGameOverScreen()
{
	if (!GameOverScreenWidget)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Cannot show game over screen: widget not created"));
		return;
	}

	// Hide game UI
	HideGameUI();

	if (!GameOverScreenWidget->IsInViewport())
	{
		GameOverScreenWidget->AddToViewport(100); // High Z-order (top layer)
		UE_LOG(LogWarRigHUD, Log, TEXT("Game over screen shown"));
	}
}

void AWarRigHUD::ToggleDebugInfo()
{
	bShowDebugInfo = !bShowDebugInfo;
	UE_LOG(LogWarRigHUD, Log, TEXT("Debug info %s"), bShowDebugInfo ? TEXT("enabled") : TEXT("disabled"));
}

void AWarRigHUD::CreateWidgets()
{
	// Validate player controller
	AWarRigPlayerController* PC = Cast<AWarRigPlayerController>(GetOwningPlayerController());
	if (!PC)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("Cannot create widgets: player controller is not AWarRigPlayerController"));
		return;
	}

	// Create game UI widget
	if (GameUIWidgetClass)
	{
		GameUIWidget = CreateWidget<UUserWidget>(PC, GameUIWidgetClass);
		if (GameUIWidget)
		{
			UE_LOG(LogWarRigHUD, Log, TEXT("Game UI widget created"));
		}
		else
		{
			UE_LOG(LogWarRigHUD, Error, TEXT("Failed to create game UI widget"));
		}
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Game UI widget class not set"));
	}

	// Create victory screen widget
	if (VictoryScreenWidgetClass)
	{
		VictoryScreenWidget = CreateWidget<UUserWidget>(PC, VictoryScreenWidgetClass);
		if (VictoryScreenWidget)
		{
			UE_LOG(LogWarRigHUD, Log, TEXT("Victory screen widget created"));
		}
		else
		{
			UE_LOG(LogWarRigHUD, Error, TEXT("Failed to create victory screen widget"));
		}
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Victory screen widget class not set"));
	}

	// Create game over screen widget
	if (GameOverScreenWidgetClass)
	{
		GameOverScreenWidget = CreateWidget<UUserWidget>(PC, GameOverScreenWidgetClass);
		if (GameOverScreenWidget)
		{
			UE_LOG(LogWarRigHUD, Log, TEXT("Game over screen widget created"));
		}
		else
		{
			UE_LOG(LogWarRigHUD, Error, TEXT("Failed to create game over screen widget"));
		}
	}
	else
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Game over screen widget class not set"));
	}
}

void AWarRigHUD::DrawDebugInfo()
{
	if (!Canvas)
	{
		return;
	}

	// Get game mode
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(World->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	// Get player controller
	AWarRigPlayerController* PC = Cast<AWarRigPlayerController>(GetOwningPlayerController());
	if (!PC)
	{
		return;
	}

	// Setup text rendering
	FLinearColor TextColor = FLinearColor::White;
	float XPos = 50.0f;
	float YPos = 50.0f;
	float YOffset = 20.0f;

	// Draw debug info
	DrawText(TEXT("=== Whiteline Nightmare Debug Info ==="), TextColor, XPos, YPos);
	YPos += YOffset * 1.5f;

	// Game state
	DrawText(FString::Printf(TEXT("Distance: %.2f / %.2f meters"),
		GameMode->GetDistanceTraveled(), GameMode->GetWinDistance()),
		TextColor, XPos, YPos);
	YPos += YOffset;

	DrawText(FString::Printf(TEXT("Score: %d"), GameMode->GetScore()),
		TextColor, XPos, YPos);
	YPos += YOffset;

	DrawText(FString::Printf(TEXT("Scrap: %d"), PC->GetScrap()),
		TextColor, XPos, YPos);
	YPos += YOffset;

	DrawText(FString::Printf(TEXT("Game Won: %s"),
		GameMode->IsGameWon() ? TEXT("Yes") : TEXT("No")),
		TextColor, XPos, YPos);
	YPos += YOffset;

	// Performance info
	YPos += YOffset * 0.5f;
	DrawText(FString::Printf(TEXT("FPS: %.1f"), 1.0f / GetWorld()->GetDeltaSeconds()),
		TextColor, XPos, YPos);
	YPos += YOffset;

	// Instructions
	YPos += YOffset * 0.5f;
	DrawText(TEXT("Press ` to toggle debug info"),
		FLinearColor(0.7f, 0.7f, 0.7f, 1.0f), XPos, YPos);
}

bool AWarRigHUD::ValidateHUDState() const
{
	bool bValid = true;

	// Check world
	if (!GetWorld())
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("Validation failed: World is null"));
		bValid = false;
	}

	// Check player controller
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		UE_LOG(LogWarRigHUD, Error, TEXT("Validation failed: Player controller is null"));
		bValid = false;
	}
	else if (!Cast<AWarRigPlayerController>(PC))
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Validation warning: Player controller is not AWarRigPlayerController"));
	}

	// Check canvas (only during draw)
	// Canvas is null outside of DrawHUD, so we can't validate it here

	// Widget validation (warnings only, not critical)
	if (!GameUIWidget && GameUIWidgetClass)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Validation warning: Game UI widget not created despite class being set"));
	}

	if (!VictoryScreenWidget && VictoryScreenWidgetClass)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Validation warning: Victory screen widget not created despite class being set"));
	}

	if (!GameOverScreenWidget && GameOverScreenWidgetClass)
	{
		UE_LOG(LogWarRigHUD, Warning, TEXT("Validation warning: Game over screen widget not created despite class being set"));
	}

	return bValid;
}
