// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestingGameMode.generated.h"

#if !UE_BUILD_SHIPPING

/**
 * Testing game mode
 * Automatically runs tests when the level loads
 */
UCLASS()
class WHITELINENIGHTMARE_API ATestingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATestingGameMode();

	virtual void StartPlay() override;

protected:
	// Register all game tests
	virtual void RegisterTests();

	// Example test functions (to be overridden or extended)
	bool TestExample(FString& OutErrorMessage);
	bool TestDataTableStructDefaults(FString& OutErrorMessage);

	// Category to run on start (empty = run all)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	FString TestCategoryFilter;

	// Auto-run tests on startup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
	bool bAutoRunTests;
};

#endif // !UE_BUILD_SHIPPING
