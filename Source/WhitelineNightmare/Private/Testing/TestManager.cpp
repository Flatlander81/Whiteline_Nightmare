// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestManager.h"
#include "Engine/World.h"

#if !UE_BUILD_SHIPPING

DEFINE_LOG_CATEGORY_STATIC(LogTestManager, Log, All);

TMap<UWorld*, UTestManager*> UTestManager::Instances;

UTestManager::UTestManager()
{
}

UTestManager* UTestManager::Get(UWorld* World)
{
	if (!World)
	{
		UE_LOG(LogTestManager, Error, TEXT("Cannot get TestManager: World is null"));
		return nullptr;
	}

	// Check if instance exists for this world
	UTestManager** FoundInstance = Instances.Find(World);
	if (FoundInstance && *FoundInstance)
	{
		return *FoundInstance;
	}

	// Create new instance
	UTestManager* NewInstance = NewObject<UTestManager>();
	if (NewInstance)
	{
		NewInstance->AddToRoot(); // Prevent garbage collection
		Instances.Add(World, NewInstance);
		UE_LOG(LogTestManager, Log, TEXT("Created new TestManager instance for world: %s"), *World->GetName());
	}
	else
	{
		UE_LOG(LogTestManager, Error, TEXT("Failed to create TestManager instance"));
	}

	return NewInstance;
}

void UTestManager::RegisterTest(const FString& TestName, const FString& Category, FTestFunction TestFunction)
{
	// Validate inputs
	if (TestName.IsEmpty())
	{
		UE_LOG(LogTestManager, Warning, TEXT("Cannot register test: TestName is empty"));
		return;
	}

	if (Category.IsEmpty())
	{
		UE_LOG(LogTestManager, Warning, TEXT("Cannot register test '%s': Category is empty"), *TestName);
		return;
	}

	if (!TestFunction.IsBound())
	{
		UE_LOG(LogTestManager, Warning, TEXT("Cannot register test '%s': TestFunction is not bound"), *TestName);
		return;
	}

	// Check for duplicate test names
	for (const FTestCase& ExistingTest : RegisteredTests)
	{
		if (ExistingTest.TestName == TestName)
		{
			UE_LOG(LogTestManager, Warning, TEXT("Test '%s' is already registered. Skipping duplicate registration."), *TestName);
			return;
		}
	}

	FTestCase NewTest(TestName, Category, TestFunction);
	RegisteredTests.Add(NewTest);

	UE_LOG(LogTestManager, Log, TEXT("Registered test: %s [%s]"), *TestName, *Category);
}

FTestStatistics UTestManager::RunAllTests()
{
	FTestStatistics Stats;
	Stats.TotalTests = RegisteredTests.Num();

	UE_LOG(LogTestManager, Display, TEXT("========================================"));
	UE_LOG(LogTestManager, Display, TEXT("Running all tests (%d total)"), Stats.TotalTests);
	UE_LOG(LogTestManager, Display, TEXT("========================================"));

	for (FTestCase& TestCase : RegisteredTests)
	{
		bool bTestPassed = ExecuteTest(TestCase);

		Stats.TotalExecutionTimeMs += TestCase.ExecutionTimeMs;

		if (bTestPassed)
		{
			Stats.PassedTests++;
			UE_LOG(LogTestManager, Display, TEXT("[PASS] %s (%.2f ms)"), *TestCase.TestName, TestCase.ExecutionTimeMs);
		}
		else
		{
			Stats.FailedTests++;
			UE_LOG(LogTestManager, Error, TEXT("[FAIL] %s (%.2f ms): %s"),
				*TestCase.TestName, TestCase.ExecutionTimeMs, *TestCase.ErrorMessage);
		}
	}

	PrintTestResults(Stats);
	return Stats;
}

FTestStatistics UTestManager::RunTestsByCategory(const FString& Category)
{
	FTestStatistics Stats;

	UE_LOG(LogTestManager, Display, TEXT("========================================"));
	UE_LOG(LogTestManager, Display, TEXT("Running tests in category: %s"), *Category);
	UE_LOG(LogTestManager, Display, TEXT("========================================"));

	for (FTestCase& TestCase : RegisteredTests)
	{
		if (TestCase.Category != Category)
		{
			continue;
		}

		Stats.TotalTests++;
		bool bTestPassed = ExecuteTest(TestCase);

		Stats.TotalExecutionTimeMs += TestCase.ExecutionTimeMs;

		if (bTestPassed)
		{
			Stats.PassedTests++;
			UE_LOG(LogTestManager, Display, TEXT("[PASS] %s (%.2f ms)"), *TestCase.TestName, TestCase.ExecutionTimeMs);
		}
		else
		{
			Stats.FailedTests++;
			UE_LOG(LogTestManager, Error, TEXT("[FAIL] %s (%.2f ms): %s"),
				*TestCase.TestName, TestCase.ExecutionTimeMs, *TestCase.ErrorMessage);
		}
	}

	PrintTestResults(Stats);
	return Stats;
}

bool UTestManager::RunTest(const FString& TestName, FString& OutErrorMessage)
{
	for (FTestCase& TestCase : RegisteredTests)
	{
		if (TestCase.TestName == TestName)
		{
			bool bPassed = ExecuteTest(TestCase);
			OutErrorMessage = TestCase.ErrorMessage;
			return bPassed;
		}
	}

	OutErrorMessage = FString::Printf(TEXT("Test '%s' not found"), *TestName);
	UE_LOG(LogTestManager, Warning, TEXT("%s"), *OutErrorMessage);
	return false;
}

TArray<FTestCase> UTestManager::GetTestsByCategory(const FString& Category) const
{
	TArray<FTestCase> CategoryTests;

	for (const FTestCase& TestCase : RegisteredTests)
	{
		if (TestCase.Category == Category)
		{
			CategoryTests.Add(TestCase);
		}
	}

	return CategoryTests;
}

void UTestManager::ClearTests()
{
	RegisteredTests.Empty();
	UE_LOG(LogTestManager, Log, TEXT("Cleared all registered tests"));
}

void UTestManager::PrintTestResults(const FTestStatistics& Stats) const
{
	UE_LOG(LogTestManager, Display, TEXT("========================================"));
	UE_LOG(LogTestManager, Display, TEXT("Test Results:"));
	UE_LOG(LogTestManager, Display, TEXT("  Total:  %d"), Stats.TotalTests);
	UE_LOG(LogTestManager, Display, TEXT("  Passed: %d"), Stats.PassedTests);
	UE_LOG(LogTestManager, Display, TEXT("  Failed: %d"), Stats.FailedTests);

	if (Stats.TotalTests > 0)
	{
		float PassRate = (float)Stats.PassedTests / (float)Stats.TotalTests * 100.0f;
		UE_LOG(LogTestManager, Display, TEXT("  Pass Rate: %.1f%%"), PassRate);
	}

	UE_LOG(LogTestManager, Display, TEXT("  Total Execution Time: %.2f ms"), Stats.TotalExecutionTimeMs);
	UE_LOG(LogTestManager, Display, TEXT("========================================"));
}

bool UTestManager::ExecuteTest(FTestCase& TestCase)
{
	if (!TestCase.TestFunction.IsBound())
	{
		TestCase.Result = ETestResult::Failed;
		TestCase.ErrorMessage = TEXT("Test function is not bound");
		UE_LOG(LogTestManager, Error, TEXT("Cannot execute test '%s': function not bound"), *TestCase.TestName);
		return false;
	}

	// Measure execution time
	double StartTime = FPlatformTime::Seconds();

	FString ErrorMessage;
	bool bPassed = TestCase.TestFunction.Execute(ErrorMessage);

	double EndTime = FPlatformTime::Seconds();
	TestCase.ExecutionTimeMs = static_cast<float>((EndTime - StartTime) * 1000.0);

	// Update test case
	TestCase.Result = bPassed ? ETestResult::Passed : ETestResult::Failed;
	TestCase.ErrorMessage = ErrorMessage;

	return bPassed;
}

#endif // !UE_BUILD_SHIPPING
