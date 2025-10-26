// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TestFramework.generated.h"

// Disable testing framework in shipping builds
#if !UE_BUILD_SHIPPING

// Forward declarations
class UTestManager;

/**
 * Test result enum
 */
UENUM(BlueprintType)
enum class ETestResult : uint8
{
	NotRun,
	Passed,
	Failed
};

/**
 * Test function signature
 */
DECLARE_DELEGATE_RetVal_OneParam(bool, FTestFunction, FString& /* OutErrorMessage */);

/**
 * Individual test case
 */
USTRUCT(BlueprintType)
struct FTestCase
{
	GENERATED_BODY()

	// Test name for display
	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	FString TestName;

	// Test category (Movement, Combat, Economy, Spawning, ObjectPool, GAS)
	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	FString Category;

	// Test function to execute
	FTestFunction TestFunction;

	// Test result
	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	ETestResult Result;

	// Error message if failed
	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	FString ErrorMessage;

	// Execution time in milliseconds
	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	float ExecutionTimeMs;

	FTestCase()
		: Result(ETestResult::NotRun)
		, ExecutionTimeMs(0.0f)
	{
	}

	FTestCase(const FString& InName, const FString& InCategory, FTestFunction InFunction)
		: TestName(InName)
		, Category(InCategory)
		, TestFunction(InFunction)
		, Result(ETestResult::NotRun)
		, ExecutionTimeMs(0.0f)
	{
	}
};

/**
 * Test statistics
 */
USTRUCT(BlueprintType)
struct FTestStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	int32 TotalTests;

	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	int32 PassedTests;

	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	int32 FailedTests;

	UPROPERTY(BlueprintReadOnly, Category = "Testing")
	float TotalExecutionTimeMs;

	FTestStatistics()
		: TotalTests(0)
		, PassedTests(0)
		, FailedTests(0)
		, TotalExecutionTimeMs(0.0f)
	{
	}
};

// Test assertion macros
#define TEST_ASSERT(Condition, Message) \
	if (!(Condition)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Assertion failed: %s (%s:%d)"), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_EQUAL(A, B, Message) \
	if ((A) != (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Equality check failed: %s != %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_NOT_EQUAL(A, B, Message) \
	if ((A) == (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Inequality check failed: %s == %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_NULL(Pointer, Message) \
	if ((Pointer) != nullptr) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Null check failed: pointer is not null - %s (%s:%d)"), \
			TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_NOT_NULL(Pointer, Message) \
	if ((Pointer) == nullptr) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Not null check failed: pointer is null - %s (%s:%d)"), \
			TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_TRUE(Condition, Message) TEST_ASSERT((Condition), Message)
#define TEST_FALSE(Condition, Message) TEST_ASSERT(!(Condition), Message)

#define TEST_GREATER(A, B, Message) \
	if ((A) <= (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Greater than check failed: %s <= %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_LESS(A, B, Message) \
	if ((A) >= (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Less than check failed: %s >= %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_GREATER_EQUAL(A, B, Message) \
	if ((A) < (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Greater or equal check failed: %s < %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#define TEST_LESS_EQUAL(A, B, Message) \
	if ((A) > (B)) \
	{ \
		OutErrorMessage = FString::Printf(TEXT("Less or equal check failed: %s > %s - %s (%s:%d)"), \
			*FString::SanitizeFloat(A), *FString::SanitizeFloat(B), TEXT(Message), TEXT(__FILE__), __LINE__); \
		return false; \
	}

#else // UE_BUILD_SHIPPING

// Empty macros for shipping builds
#define TEST_ASSERT(Condition, Message)
#define TEST_EQUAL(A, B, Message)
#define TEST_NOT_EQUAL(A, B, Message)
#define TEST_NULL(Pointer, Message)
#define TEST_NOT_NULL(Pointer, Message)
#define TEST_TRUE(Condition, Message)
#define TEST_FALSE(Condition, Message)
#define TEST_GREATER(A, B, Message)
#define TEST_LESS(A, B, Message)
#define TEST_GREATER_EQUAL(A, B, Message)
#define TEST_LESS_EQUAL(A, B, Message)

#endif // !UE_BUILD_SHIPPING
