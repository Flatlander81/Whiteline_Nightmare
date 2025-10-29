// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestingGameMode.h"
#include "Testing/TestMacros.h"
#include "TimerManager.h"

// Forward declaration of test registration functions
#if !UE_BUILD_SHIPPING
void RegisterObjectPoolTests(class UTestManager* TestManager);
#endif

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

#if !UE_BUILD_SHIPPING
	// Register object pool tests (includes ground tile tests)
	RegisterObjectPoolTests(TestManager);
	UE_LOG(LogTestingGameMode, Log, TEXT("RegisterSampleTests: Registered object pool and ground tile tests"));
#endif
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
#else
	return true;
#endif
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
#else
	return true;
#endif
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
#else
	return true;
#endif
}

bool ATestingGameMode::SampleTest_NullCheck()
{
#if !UE_BUILD_SHIPPING
	UObject* NullObject = nullptr;
	TEST_NULL(NullObject, "Null pointer should be null");

	// Use a concrete class instead of abstract UObject
	UTestManager* ValidObject = NewObject<UTestManager>();
	TEST_NOT_NULL(ValidObject, "Valid object should not be null");

	TEST_SUCCESS("NullCheck");
#else
	return true;
#endif
}

// Console command implementation
#if !UE_BUILD_SHIPPING

static FAutoConsoleCommand RunTestsCommand(
	TEXT("RunTests"),
	TEXT("Run automated tests. Usage: RunTests [Category]\nCategories: All, Movement, Combat, Economy, Spawning, ObjectPool, GAS, UI"),
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
			else if (CategoryStr.Equals(TEXT("UI"), ESearchCase::IgnoreCase))
			{
				Category = ETestCategory::UI;
			}
		}

		// Run tests
		TestManager->RunTestCategory(Category);
	})
);

static FAutoConsoleCommand ListTestsCommand(
	TEXT("ListTests"),
	TEXT("List all registered tests by category"),
	FConsoleCommandDelegate::CreateStatic([]()
	{
		UE_LOG(LogTestingGameMode, Log, TEXT("Console: ListTests command executed"));

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

		// Get all tests
		const TArray<FTestCase>& AllTests = TestManager->GetAllTests();

		if (AllTests.Num() == 0)
		{
			UE_LOG(LogTestingGameMode, Warning, TEXT("No tests registered"));
			return;
		}

		// Log header
		UE_LOG(LogTestingGameMode, Log, TEXT("========================================"));
		UE_LOG(LogTestingGameMode, Log, TEXT("Registered Tests (%d total):"), AllTests.Num());
		UE_LOG(LogTestingGameMode, Log, TEXT("========================================"));

		// Group tests by category
		TMap<ETestCategory, TArray<FString>> TestsByCategory;
		for (const FTestCase& Test : AllTests)
		{
			TestsByCategory.FindOrAdd(Test.Category).Add(Test.TestName);
		}

		// Display tests by category
		auto DisplayCategory = [](ETestCategory Category, const TArray<FString>& Tests)
		{
			FString CategoryName;
			switch (Category)
			{
				case ETestCategory::All: CategoryName = TEXT("All"); break;
				case ETestCategory::Movement: CategoryName = TEXT("Movement"); break;
				case ETestCategory::Combat: CategoryName = TEXT("Combat"); break;
				case ETestCategory::Economy: CategoryName = TEXT("Economy"); break;
				case ETestCategory::Spawning: CategoryName = TEXT("Spawning"); break;
				case ETestCategory::ObjectPool: CategoryName = TEXT("ObjectPool"); break;
				case ETestCategory::GAS: CategoryName = TEXT("GAS"); break;
				case ETestCategory::UI: CategoryName = TEXT("UI"); break;
				default: CategoryName = TEXT("Unknown"); break;
			}

			UE_LOG(LogTestingGameMode, Log, TEXT("\n[%s] - %d tests:"), *CategoryName, Tests.Num());
			for (const FString& TestName : Tests)
			{
				UE_LOG(LogTestingGameMode, Log, TEXT("  - %s"), *TestName);
			}
		};

		// Display in category order
		if (TestsByCategory.Contains(ETestCategory::Movement))
			DisplayCategory(ETestCategory::Movement, TestsByCategory[ETestCategory::Movement]);
		if (TestsByCategory.Contains(ETestCategory::Combat))
			DisplayCategory(ETestCategory::Combat, TestsByCategory[ETestCategory::Combat]);
		if (TestsByCategory.Contains(ETestCategory::Economy))
			DisplayCategory(ETestCategory::Economy, TestsByCategory[ETestCategory::Economy]);
		if (TestsByCategory.Contains(ETestCategory::Spawning))
			DisplayCategory(ETestCategory::Spawning, TestsByCategory[ETestCategory::Spawning]);
		if (TestsByCategory.Contains(ETestCategory::ObjectPool))
			DisplayCategory(ETestCategory::ObjectPool, TestsByCategory[ETestCategory::ObjectPool]);
		if (TestsByCategory.Contains(ETestCategory::GAS))
			DisplayCategory(ETestCategory::GAS, TestsByCategory[ETestCategory::GAS]);
		if (TestsByCategory.Contains(ETestCategory::UI))
			DisplayCategory(ETestCategory::UI, TestsByCategory[ETestCategory::UI]);
		if (TestsByCategory.Contains(ETestCategory::All))
			DisplayCategory(ETestCategory::All, TestsByCategory[ETestCategory::All]);

		UE_LOG(LogTestingGameMode, Log, TEXT("========================================"));
	})
);

#endif // !UE_BUILD_SHIPPING
