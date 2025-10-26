// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestManager.generated.h"

// Test categories
UENUM(BlueprintType)
enum class ETestCategory : uint8
{
	All			UMETA(DisplayName = "All Tests"),
	Movement	UMETA(DisplayName = "Movement Tests"),
	Combat		UMETA(DisplayName = "Combat Tests"),
	Economy		UMETA(DisplayName = "Economy Tests"),
	Spawning	UMETA(DisplayName = "Spawning Tests"),
	ObjectPool	UMETA(DisplayName = "Object Pool Tests"),
	GAS			UMETA(DisplayName = "Gameplay Ability System Tests"),
	UI			UMETA(DisplayName = "UI Tests")
};

// Forward declarations
class UWorld;

/**
 * Test case function pointer type
 * @return True if test passed, false if failed
 */
typedef bool (*TestFunction)();

/**
 * Test case structure
 */
USTRUCT(BlueprintType)
struct FTestCase
{
	GENERATED_BODY()

	// Name of the test
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
	FString TestName;

	// Category of the test
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
	ETestCategory Category;

	// Test function pointer (not exposed to Blueprint)
	TestFunction Function;

	// Whether the test passed (after execution)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
	bool bPassed;

	// Whether the test has been executed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
	bool bExecuted;

	FTestCase()
		: TestName(TEXT(""))
		, Category(ETestCategory::All)
		, Function(nullptr)
		, bPassed(false)
		, bExecuted(false)
	{
	}

	FTestCase(const FString& InName, ETestCategory InCategory, TestFunction InFunction)
		: TestName(InName)
		, Category(InCategory)
		, Function(InFunction)
		, bPassed(false)
		, bExecuted(false)
	{
	}
};

/**
 * Test Manager - Manages registration and execution of automated tests
 * This is a singleton that can be accessed from anywhere in the game
 */
UCLASS()
class WHITELINENIGHTMARE_API UTestManager : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Get the singleton instance
	 * @param WorldContext - World context object
	 * @return Test manager instance
	 */
	static UTestManager* Get(UObject* WorldContext);

	/**
	 * Register a test case
	 * @param TestName - Name of the test
	 * @param Category - Category of the test
	 * @param Function - Test function pointer
	 */
	void RegisterTest(const FString& TestName, ETestCategory Category, TestFunction Function);

	/**
	 * Run all tests
	 * @return True if all tests passed
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	bool RunAllTests();

	/**
	 * Run tests in a specific category
	 * @param Category - Category to run
	 * @return True if all tests in category passed
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	bool RunTestCategory(ETestCategory Category);

	/**
	 * Run a specific test by name
	 * @param TestName - Name of the test to run
	 * @return True if test passed
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	bool RunTest(const FString& TestName);

	/**
	 * Get test results
	 * @param OutTotalTests - Total number of tests
	 * @param OutPassedTests - Number of tests that passed
	 * @param OutFailedTests - Number of tests that failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void GetTestResults(int32& OutTotalTests, int32& OutPassedTests, int32& OutFailedTests) const;

	/**
	 * Clear all test results
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void ClearResults();

	/**
	 * Get all registered tests
	 * @return Array of test cases
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	const TArray<FTestCase>& GetAllTests() const { return RegisteredTests; }

protected:
	// Array of registered tests
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Testing")
	TArray<FTestCase> RegisteredTests;

private:
	// Singleton instance
	static UTestManager* Instance;

	/**
	 * Execute a single test case
	 * @param TestCase - Test case to execute (modified in place)
	 * @return True if test passed
	 */
	bool ExecuteTest(FTestCase& TestCase);

	/**
	 * Log test summary
	 */
	void LogTestSummary() const;
};

/**
 * Auto-registration helper for tests
 * Usage: static FTestAutoRegister TestName(TEXT("TestName"), ETestCategory::Movement, &TestFunction);
 */
struct FTestAutoRegister
{
	FTestAutoRegister(const FString& TestName, ETestCategory Category, TestFunction Function);
};

// Macro to auto-register a test
#if !UE_BUILD_SHIPPING
#define REGISTER_TEST(TestName, Category, Function) \
	static FTestAutoRegister AutoRegister_##Function(TEXT(TestName), Category, &Function)
#else
#define REGISTER_TEST(TestName, Category, Function)
#endif
