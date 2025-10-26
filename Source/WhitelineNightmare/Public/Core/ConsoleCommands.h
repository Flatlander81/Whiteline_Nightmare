// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

/**
 * Console commands for Whiteline Nightmare
 */
class WHITELINENIGHTMARE_API FWhitelineConsoleCommands
{
public:
	// Register all console commands
	static void RegisterCommands();

	// Unregister all console commands
	static void UnregisterCommands();

private:
	// Command: RunTests [TestCategory]
	static void RunTests(const TArray<FString>& Args);

	// Command: ToggleDebugHUD
	static void ToggleDebugHUD(const TArray<FString>& Args);

	// Console command objects
	static TUniquePtr<FAutoConsoleCommand> RunTestsCommand;
	static TUniquePtr<FAutoConsoleCommand> ToggleDebugHUDCommand;
};

#endif // !UE_BUILD_SHIPPING
