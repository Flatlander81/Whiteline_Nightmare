// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestManager.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogTestManager, Log, All);

// Initialize singleton
UTestManager* UTestManager::Instance = nullptr;

UTestManager* UTestManager::Get(UObject* WorldContext)
{
	if (!Instance)
	{
		// Create singleton instance
		Instance = NewObject<UTestManager>();
		Instance->AddToRoot(); // Prevent garbage collection

		UE_LOG(LogTestManager, Log, TEXT("TestManager: Singleton instance created"));
	}

	return Instance;
}

void UTestManager::RegisterTest(const FString& TestName, ETestCategory Category, TestFunction Function)
{
	// Validate inputs
	if (TestName.IsEmpty())
	{
		UE_LOG(LogTestManager, Error, TEXT("RegisterTest: Test name cannot be empty"));
		return;
	}

	if (!Function)
	{
		UE_LOG(LogTestManager, Error, TEXT("RegisterTest: Test function cannot be null for test '%s'"), *TestName);
		return;
	}

	// Check for duplicate test names
	for (const FTestCase& ExistingTest : RegisteredTests)
	{
		if (ExistingTest.TestName.Equals(TestName))
		{
			UE_LOG(LogTestManager, Warning, TEXT("RegisterTest: Test '%s' already registered, skipping"), *TestName);
			return;
		}
	}

	// Register the test
	FTestCase NewTest(TestName, Category, Function);
	RegisteredTests.Add(NewTest);

	UE_LOG(LogTestManager, Log, TEXT("RegisterTest: Registered test '%s' in category %d"), *TestName, (int32)Category);

#if !UE_BUILD_SHIPPING
	// Register a console command for this test (allows direct calling by name)
	// Create command name with Test_ prefix to avoid conflicts
	FString CommandName = FString::Printf(TEXT("Test_%s"), *TestName);
	FString HelpText = FString::Printf(TEXT("Run test: %s"), *TestName);

	// Register console command
	IConsoleCommand* Command = IConsoleManager::Get().RegisterConsoleCommand(
		*CommandName,
		*HelpText,
		FConsoleCommandDelegate::CreateLambda([TestName]()
		{
			UE_LOG(LogTestManager, Log, TEXT("Console: Running test '%s' via direct command"), *TestName);

			// Get TestManager instance
			UTestManager* Manager = UTestManager::Get(nullptr);
			if (Manager)
			{
				Manager->RunTest(TestName);
			}
			else
			{
				UE_LOG(LogTestManager, Error, TEXT("Console: Failed to get TestManager"));
			}
		}),
		ECVF_Default
	);

	// Store command for cleanup
	ConsoleCommands.Add(TestName, Command);

	UE_LOG(LogTestManager, Log, TEXT("RegisterTest: Created console command '%s'"), *CommandName);
#endif
}

bool UTestManager::RunAllTests()
{
	UE_LOG(LogTestManager, Log, TEXT("========================================"));
	UE_LOG(LogTestManager, Log, TEXT("Running All Tests..."));
	UE_LOG(LogTestManager, Log, TEXT("========================================"));

	bool bAllPassed = true;

	for (FTestCase& Test : RegisteredTests)
	{
		const bool bPassed = ExecuteTest(Test);
		if (!bPassed)
		{
			bAllPassed = false;
		}
	}

	LogTestSummary();

	return bAllPassed;
}

bool UTestManager::RunTestCategory(ETestCategory Category)
{
	UE_LOG(LogTestManager, Log, TEXT("========================================"));
	UE_LOG(LogTestManager, Log, TEXT("Running Tests in Category: %d"), (int32)Category);
	UE_LOG(LogTestManager, Log, TEXT("========================================"));

	bool bAllPassed = true;
	int32 TestsRun = 0;

	for (FTestCase& Test : RegisteredTests)
	{
		if (Test.Category == Category || Category == ETestCategory::All)
		{
			const bool bPassed = ExecuteTest(Test);
			if (!bPassed)
			{
				bAllPassed = false;
			}
			TestsRun++;
		}
	}

	if (TestsRun == 0)
	{
		UE_LOG(LogTestManager, Warning, TEXT("No tests found in category %d"), (int32)Category);
	}

	LogTestSummary();

	return bAllPassed;
}

bool UTestManager::RunTest(const FString& TestName)
{
	// Find test by name
	FTestCase* FoundTest = nullptr;
	for (FTestCase& Test : RegisteredTests)
	{
		if (Test.TestName.Equals(TestName))
		{
			FoundTest = &Test;
			break;
		}
	}

	if (!FoundTest)
	{
		UE_LOG(LogTestManager, Error, TEXT("RunTest: Test '%s' not found"), *TestName);
		return false;
	}

	UE_LOG(LogTestManager, Log, TEXT("========================================"));
	UE_LOG(LogTestManager, Log, TEXT("Running Test: %s"), *TestName);
	UE_LOG(LogTestManager, Log, TEXT("========================================"));

	const bool bPassed = ExecuteTest(*FoundTest);

	LogTestSummary();

	return bPassed;
}

void UTestManager::GetTestResults(int32& OutTotalTests, int32& OutPassedTests, int32& OutFailedTests) const
{
	OutTotalTests = 0;
	OutPassedTests = 0;
	OutFailedTests = 0;

	for (const FTestCase& Test : RegisteredTests)
	{
		if (Test.bExecuted)
		{
			OutTotalTests++;
			if (Test.bPassed)
			{
				OutPassedTests++;
			}
			else
			{
				OutFailedTests++;
			}
		}
	}
}

void UTestManager::ClearResults()
{
	for (FTestCase& Test : RegisteredTests)
	{
		Test.bExecuted = false;
		Test.bPassed = false;
	}

	UE_LOG(LogTestManager, Log, TEXT("ClearResults: All test results cleared"));
}

bool UTestManager::ExecuteTest(FTestCase& TestCase)
{
	// Validate test case
	if (!TestCase.Function)
	{
		UE_LOG(LogTestManager, Error, TEXT("ExecuteTest: Test '%s' has null function"), *TestCase.TestName);
		TestCase.bExecuted = true;
		TestCase.bPassed = false;
		return false;
	}

	UE_LOG(LogTestManager, Log, TEXT("Executing: %s"), *TestCase.TestName);

	// Execute the test
	const bool bPassed = TestCase.Function();

	// Update test case
	TestCase.bExecuted = true;
	TestCase.bPassed = bPassed;

	// Log result
	if (bPassed)
	{
		UE_LOG(LogTestManager, Log, TEXT("  [PASS] %s"), *TestCase.TestName);
	}
	else
	{
		UE_LOG(LogTestManager, Error, TEXT("  [FAIL] %s"), *TestCase.TestName);
	}

	return bPassed;
}

void UTestManager::LogTestSummary() const
{
	int32 TotalTests, PassedTests, FailedTests;
	GetTestResults(TotalTests, PassedTests, FailedTests);

	UE_LOG(LogTestManager, Log, TEXT("========================================"));
	UE_LOG(LogTestManager, Log, TEXT("Test Summary:"));
	UE_LOG(LogTestManager, Log, TEXT("  Total:  %d"), TotalTests);
	UE_LOG(LogTestManager, Log, TEXT("  Passed: %d"), PassedTests);
	UE_LOG(LogTestManager, Log, TEXT("  Failed: %d"), FailedTests);

	if (FailedTests == 0 && TotalTests > 0)
	{
		UE_LOG(LogTestManager, Log, TEXT("  Result: ALL TESTS PASSED!"));
	}
	else if (FailedTests > 0)
	{
		UE_LOG(LogTestManager, Error, TEXT("  Result: SOME TESTS FAILED!"));
	}

	UE_LOG(LogTestManager, Log, TEXT("========================================"));
}

void UTestManager::Cleanup()
{
#if !UE_BUILD_SHIPPING
	// Unregister all console commands
	for (const TPair<FString, IConsoleCommand*>& Pair : ConsoleCommands)
	{
		if (Pair.Value)
		{
			FString CommandName = FString::Printf(TEXT("Test_%s"), *Pair.Key);
			IConsoleManager::Get().UnregisterConsoleObject(*CommandName);
		}
	}

	ConsoleCommands.Empty();
	UE_LOG(LogTestManager, Log, TEXT("Cleanup: Unregistered %d console commands"), ConsoleCommands.Num());
#endif
}

UTestManager::~UTestManager()
{
	Cleanup();
}

// Auto-registration helper implementation
FTestAutoRegister::FTestAutoRegister(const FString& TestName, ETestCategory Category, TestFunction Function)
{
	// Register the test with the test manager
	// Note: This happens at static initialization time, so we need to handle the case where
	// the test manager doesn't exist yet
	// For now, tests will need to be registered manually or during game initialization
	UE_LOG(LogTestManager, Log, TEXT("FTestAutoRegister: Test '%s' queued for registration"), *TestName);
}
