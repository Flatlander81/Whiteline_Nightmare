// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestingGameMode.h"
#include "Testing/TestMacros.h"
#include "TimerManager.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogTestingGameMode, Log, All);

ATestingGameMode::ATestingGameMode()
	: bAutoRunTests(true)
	, AutoTestCategory(ETestCategory::All)
	, TestStartDelay(1.0f)
	, TestManager(nullptr)
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = false;
}

void ATestingGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTestingGameMode, Log, TEXT("TestingGameMode: Initialized"));

	// Get or create test manager
	TestManager = UTestManager::Get(this);

	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("TestingGameMode: Failed to get TestManager"));
		return;
	}

	// Register sample tests for demonstration
	RegisterSampleTests();

	// Auto-run tests if enabled
	if (bAutoRunTests)
	{
		UE_LOG(LogTestingGameMode, Log, TEXT("TestingGameMode: Scheduling auto-run tests in %.2f seconds"), TestStartDelay);

		// Use a timer to delay test execution
		FTimerHandle TestTimerHandle;
		GetWorldTimerManager().SetTimer(TestTimerHandle, this, &ATestingGameMode::OnTestStartTimer, TestStartDelay, false);
	}
}

void ATestingGameMode::RunTests(ETestCategory Category)
{
	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("RunTests: TestManager is null"));
		return;
	}

	UE_LOG(LogTestingGameMode, Log, TEXT("RunTests: Starting tests for category %d"), (int32)Category);

	const bool bAllPassed = TestManager->RunTestCategory(Category);

	if (bAllPassed)
	{
		UE_LOG(LogTestingGameMode, Log, TEXT("RunTests: All tests passed!"));
	}
	else
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("RunTests: Some tests failed!"));
	}
}

void ATestingGameMode::RunSpecificTest(const FString& TestName)
{
	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("RunSpecificTest: TestManager is null"));
		return;
	}

	UE_LOG(LogTestingGameMode, Log, TEXT("RunSpecificTest: Starting test '%s'"), *TestName);

	const bool bPassed = TestManager->RunTest(TestName);

	if (bPassed)
	{
		UE_LOG(LogTestingGameMode, Log, TEXT("RunSpecificTest: Test passed!"));
	}
	else
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("RunSpecificTest: Test failed!"));
	}
}

void ATestingGameMode::RegisterSampleTests()
{
	if (!TestManager)
	{
		UE_LOG(LogTestingGameMode, Error, TEXT("RegisterSampleTests: TestManager is null"));
		return;
	}

	// Register sample tests
	TestManager->RegisterTest(TEXT("Sample_BasicAssertion"), ETestCategory::All, &ATestingGameMode::SampleTest_BasicAssertion);
	TestManager->RegisterTest(TEXT("Sample_Equality"), ETestCategory::All, &ATestingGameMode::SampleTest_Equality);
	TestManager->RegisterTest(TEXT("Sample_NearlyEqual"), ETestCategory::All, &ATestingGameMode::SampleTest_NearlyEqual);
	TestManager->RegisterTest(TEXT("Sample_NullCheck"), ETestCategory::All, &ATestingGameMode::SampleTest_NullCheck);

	UE_LOG(LogTestingGameMode, Log, TEXT("RegisterSampleTests: Registered %d sample tests"), 4);
}

void ATestingGameMode::OnTestStartTimer()
{
	UE_LOG(LogTestingGameMode, Log, TEXT("OnTestStartTimer: Starting auto-run tests"));
	RunTests(AutoTestCategory);
}

// Sample test implementations
bool ATestingGameMode::SampleTest_BasicAssertion()
{
#if !UE_BUILD_SHIPPING
	TEST_ASSERT(true, "This should always pass");
	TEST_ASSERT(1 + 1 == 2, "Basic math should work");
	TEST_SUCCESS("BasicAssertion");
#endif
	return true;
}

bool ATestingGameMode::SampleTest_Equality()
{
#if !UE_BUILD_SHIPPING
	int32 A = 42;
	int32 B = 42;
	TEST_EQUAL(A, B, "Values should be equal");

	FString StrA = TEXT("Hello");
	FString StrB = TEXT("Hello");
	TEST_EQUAL(StrA, StrB, "Strings should be equal");

	TEST_SUCCESS("Equality");
#endif
	return true;
}

bool ATestingGameMode::SampleTest_NearlyEqual()
{
#if !UE_BUILD_SHIPPING
	float A = 1.0f;
	float B = 1.00001f;
	TEST_NEARLY_EQUAL(A, B, 0.001f, "Floats should be nearly equal");

	float C = 100.0f;
	float D = 100.01f;
	TEST_NEARLY_EQUAL(C, D, 0.1f, "Large floats should be nearly equal");

	TEST_SUCCESS("NearlyEqual");
#endif
	return true;
}

bool ATestingGameMode::SampleTest_NullCheck()
{
#if !UE_BUILD_SHIPPING
	UObject* NullObject = nullptr;
	TEST_NULL(NullObject, "Null pointer should be null");

	UObject* ValidObject = NewObject<UObject>();
	TEST_NOT_NULL(ValidObject, "Valid object should not be null");

	TEST_SUCCESS("NullCheck");
#endif
	return true;
}

// Console command implementation
#if !UE_BUILD_SHIPPING

static FAutoConsoleCommand RunTestsCommand(
	TEXT("RunTests"),
	TEXT("Run automated tests. Usage: RunTests [Category]\nCategories: All, Movement, Combat, Economy, Spawning, ObjectPool, GAS"),
	FConsoleCommandWithArgsDelegate::CreateStatic([](const TArray<FString>& Args)
	{
		UE_LOG(LogTestingGameMode, Log, TEXT("Console: RunTests command executed"));

		// Get the world
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
			UE_LOG(LogTestingGameMode, Error, TEXT("Console: No valid world found"));
			return;
		}

		// Get test manager
		UTestManager* TestManager = UTestManager::Get(World);
		if (!TestManager)
		{
			UE_LOG(LogTestingGameMode, Error, TEXT("Console: Failed to get TestManager"));
			return;
		}

		// Parse category from args
		ETestCategory Category = ETestCategory::All;
		if (Args.Num() > 0)
		{
			const FString& CategoryStr = Args[0];
			if (CategoryStr.Equals(TEXT("Movement"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::Movement;
			}
			else if (CategoryStr.Equals(TEXT("Combat"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::Combat;
			}
			else if (CategoryStr.Equals(TEXT("Economy"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::Economy;
			}
			else if (CategoryStr.Equals(TEXT("Spawning"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::Spawning;
			}
			else if (CategoryStr.Equals(TEXT("ObjectPool"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::ObjectPool;
			}
			else if (CategoryStr.Equals(TEXT("GAS"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::GAS;
			}
		}

		// Run tests
		TestManager->RunTestCategory(Category);
	})
);

#endif // !UE_BUILD_SHIPPING
