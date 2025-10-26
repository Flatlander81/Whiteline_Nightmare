// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Testing/TestManager.h"
#include "TestingGameMode.generated.h"

/**
 * Testing Game Mode
 * Special game mode that runs automated tests on level load
 * Usage: Create a test map and set this as the game mode
 */
UCLASS()
class WHITELINENIGHTMARE_API ATestingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATestingGameMode();

	// Called when the game starts
	virtual void BeginPlay() override;

	/**
	 * Run tests for a specific category
	 * @param Category - Category to test (default: All)
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunTests(ETestCategory Category = ETestCategory::All);

	/**
	 * Run a specific test by name
	 * @param TestName - Name of the test to run
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunSpecificTest(const FString& TestName);

	/**
	 * Register sample tests for demonstration
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RegisterSampleTests();

protected:
	// Whether to run tests automatically on begin play
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Testing")
	bool bAutoRunTests;

	// Which category to run automatically (if bAutoRunTests is true)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Testing")
	ETestCategory AutoTestCategory;

	// Delay before running tests (to allow level to fully load)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Testing")
	float TestStartDelay;

	// Reference to the test manager
	UPROPERTY()
	UTestManager* TestManager;

private:
	/**
	 * Timer callback to start tests
	 */
	void OnTestStartTimer();

	// Sample test functions
	static bool SampleTest_BasicAssertion();
	static bool SampleTest_Equality();
	static bool SampleTest_NearlyEqual();
	static bool SampleTest_NullCheck();
};

// Console command to run tests
// Usage: RunTests [Category]
// Example: RunTests Movement
