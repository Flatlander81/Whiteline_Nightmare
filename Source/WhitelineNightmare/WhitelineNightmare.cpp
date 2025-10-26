// Copyright Epic Games, Inc. All Rights Reserved.

#include "WhitelineNightmare.h"
#include "Modules/ModuleManager.h"

#if !UE_BUILD_SHIPPING
#include "Core/ConsoleCommands.h"
#endif

class FWhitelineNightmareModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
#if !UE_BUILD_SHIPPING
		FWhitelineConsoleCommands::RegisterCommands();
#endif
	}

	virtual void ShutdownModule() override
	{
#if !UE_BUILD_SHIPPING
		FWhitelineConsoleCommands::UnregisterCommands();
#endif
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FWhitelineNightmareModule, WhitelineNightmare, "WhitelineNightmare");
