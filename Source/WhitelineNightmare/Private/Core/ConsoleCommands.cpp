// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/ConsoleCommands.h"
#include "Testing/TestManager.h"
#include "Core/WarRigHUD.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"

#if !UE_BUILD_SHIPPING

DEFINE_LOG_CATEGORY_STATIC(LogConsoleCommands, Log, All);

TUniquePtr<FAutoConsoleCommand> FWhitelineConsoleCommands::RunTestsCommand;
TUniquePtr<FAutoConsoleCommand> FWhitelineConsoleCommands::ToggleDebugHUDCommand;

void FWhitelineConsoleCommands::RegisterCommands()
{
	UE_LOG(LogConsoleCommands, Log, TEXT("Registering console commands"));

	// Register RunTests command
	RunTestsCommand = MakeUnique<FAutoConsoleCommand>(
		TEXT("RunTests"),
		TEXT("Run automated tests. Usage: RunTests [Category]. If no category specified, runs all tests."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FWhitelineConsoleCommands::RunTests)
	);

	// Register ToggleDebugHUD command
	ToggleDebugHUDCommand = MakeUnique<FAutoConsoleCommand>(
		TEXT("ToggleDebugHUD"),
		TEXT("Toggle debug HUD display."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&FWhitelineConsoleCommands::ToggleDebugHUD)
	);

	UE_LOG(LogConsoleCommands, Log, TEXT("Console commands registered"));
}

void FWhitelineConsoleCommands::UnregisterCommands()
{
	UE_LOG(LogConsoleCommands, Log, TEXT("Unregistering console commands"));

	RunTestsCommand.Reset();
	ToggleDebugHUDCommand.Reset();

	UE_LOG(LogConsoleCommands, Log, TEXT("Console commands unregistered"));
}

void FWhitelineConsoleCommands::RunTests(const TArray<FString>& Args)
{
	UE_LOG(LogConsoleCommands, Display, TEXT("RunTests command executed"));

	// Get world context
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}

	if (!World)
	{
		UE_LOG(LogConsoleCommands, Error, TEXT("Cannot run tests: No valid world found"));
		return;
	}

	// Get test manager
	UTestManager* TestManager = UTestManager::Get(World);
	if (!TestManager)
	{
		UE_LOG(LogConsoleCommands, Error, TEXT("Cannot run tests: TestManager not available"));
		return;
	}

	// Check if category was specified
	if (Args.Num() > 0)
	{
		FString Category = Args[0];
		UE_LOG(LogConsoleCommands, Display, TEXT("Running tests in category: %s"), *Category);
		TestManager->RunTestsByCategory(Category);
	}
	else
	{
		UE_LOG(LogConsoleCommands, Display, TEXT("Running all tests"));
		TestManager->RunAllTests();
	}
}

void FWhitelineConsoleCommands::ToggleDebugHUD(const TArray<FString>& Args)
{
	UE_LOG(LogConsoleCommands, Display, TEXT("ToggleDebugHUD command executed"));

	// Get world context
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
		{
			World = Context.World();
			break;
		}
	}

	if (!World)
	{
		UE_LOG(LogConsoleCommands, Error, TEXT("Cannot toggle debug HUD: No valid world found"));
		return;
	}

	// Get player controller
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogConsoleCommands, Error, TEXT("Cannot toggle debug HUD: No player controller found"));
		return;
	}

	// Get HUD
	AWarRigHUD* HUD = Cast<AWarRigHUD>(PC->GetHUD());
	if (!HUD)
	{
		UE_LOG(LogConsoleCommands, Error, TEXT("Cannot toggle debug HUD: HUD is not AWarRigHUD"));
		return;
	}

	// Toggle debug info
	HUD->ToggleDebugInfo();
}

#endif // !UE_BUILD_SHIPPING
