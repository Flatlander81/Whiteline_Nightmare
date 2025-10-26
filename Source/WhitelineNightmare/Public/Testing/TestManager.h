// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestFramework.h"
#include "TestManager.generated.h"

#if !UE_BUILD_SHIPPING

/**
 * Test manager singleton
 * Manages test registration and execution
 */
UCLASS()
class WHITELINENIGHTMARE_API UTestManager : public UObject
{
	GENERATED_BODY()

public:
	UTestManager();

	// Get singleton instance
	static UTestManager* Get(UWorld* World);

	// Register a test
	void RegisterTest(const FString& TestName, const FString& Category, FTestFunction TestFunction);

	// Run all tests
	FTestStatistics RunAllTests();

	// Run tests in a specific category
	FTestStatistics RunTestsByCategory(const FString& Category);

	// Run a specific test by name
	bool RunTest(const FString& TestName, FString& OutErrorMessage);

	// Get all registered tests
	const TArray<FTestCase>& GetAllTests() const { return RegisteredTests; }

	// Get tests by category
	TArray<FTestCase> GetTestsByCategory(const FString& Category) const;

	// Clear all registered tests
	void ClearTests();

	// Print test results to log
	void PrintTestResults(const FTestStatistics& Stats) const;

private:
	// Execute a single test case
	bool ExecuteTest(FTestCase& TestCase);

	// Registered tests
	UPROPERTY()
	TArray<FTestCase> RegisteredTests;

	// Singleton instance per world
	static TMap<UWorld*, UTestManager*> Instances;
};

#endif // !UE_BUILD_SHIPPING
