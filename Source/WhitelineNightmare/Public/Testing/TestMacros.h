// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Testing is only available in non-shipping builds
#if !UE_BUILD_SHIPPING

/**
 * Test assertion macro - fails test if condition is false
 * @param Condition - Condition to check
 * @param Message - Error message if assertion fails
 */
#define TEST_ASSERT(Condition, Message) \
	do { \
		if (!(Condition)) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s - %s (Line %d)"), TEXT(#Condition), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test equality macro - fails test if values are not equal
 * @param A - First value
 * @param B - Second value
 * @param Message - Error message if assertion fails
 */
#define TEST_EQUAL(A, B, Message) \
	do { \
		if ((A) != (B)) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s != %s - %s (Line %d)"), TEXT(#A), TEXT(#B), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test near equality macro for floats - fails test if values are not nearly equal
 * @param A - First value
 * @param B - Second value
 * @param Tolerance - Acceptable difference
 * @param Message - Error message if assertion fails
 */
#define TEST_NEARLY_EQUAL(A, B, Tolerance, Message) \
	do { \
		if (!FMath::IsNearlyEqual((A), (B), (Tolerance))) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s (~= %s) - %s (Line %d)"), TEXT(#A), TEXT(#B), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test near equality macro for floats (alias for TEST_NEARLY_EQUAL)
 * @param A - First value
 * @param B - Second value
 * @param Tolerance - Acceptable difference
 * @param Message - Error message if assertion fails
 */
#define TEST_NEAR(A, B, Tolerance, Message) TEST_NEARLY_EQUAL(A, B, Tolerance, Message)

/**
 * Test null check macro - fails test if pointer is null
 * @param Pointer - Pointer to check
 * @param Message - Error message if assertion fails
 */
#define TEST_NOT_NULL(Pointer, Message) \
	do { \
		if ((Pointer) == nullptr) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s is null - %s (Line %d)"), TEXT(#Pointer), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test null check macro - fails test if pointer is not null
 * @param Pointer - Pointer to check
 * @param Message - Error message if assertion fails
 */
#define TEST_NULL(Pointer, Message) \
	do { \
		if ((Pointer) != nullptr) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s is not null - %s (Line %d)"), TEXT(#Pointer), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test true macro - fails test if condition is false (alias for TEST_ASSERT)
 * @param Condition - Condition to check
 * @param Message - Error message if assertion fails
 */
#define TEST_TRUE(Condition, Message) TEST_ASSERT(Condition, Message)

/**
 * Test false macro - fails test if condition is true
 * @param Condition - Condition to check
 * @param Message - Error message if assertion fails
 */
#define TEST_FALSE(Condition, Message) \
	do { \
		if (Condition) \
		{ \
			UE_LOG(LogTemp, Error, TEXT("TEST FAILED: %s is true (expected false) - %s (Line %d)"), TEXT(#Condition), TEXT(Message), __LINE__); \
			return false; \
		} \
	} while(0)

/**
 * Test success macro - logs success message
 * @param TestName - Name of the test that passed
 */
#define TEST_SUCCESS(TestName) \
	do { \
		UE_LOG(LogTemp, Log, TEXT("TEST PASSED: %s"), TEXT(TestName)); \
		return true; \
	} while(0)

#else

// In shipping builds, tests are disabled
#define TEST_ASSERT(Condition, Message) do {} while(0)
#define TEST_EQUAL(A, B, Message) do {} while(0)
#define TEST_NEARLY_EQUAL(A, B, Tolerance, Message) do {} while(0)
#define TEST_NEAR(A, B, Tolerance, Message) do {} while(0)
#define TEST_NOT_NULL(Pointer, Message) do {} while(0)
#define TEST_NULL(Pointer, Message) do {} while(0)
#define TEST_TRUE(Condition, Message) do {} while(0)
#define TEST_FALSE(Condition, Message) do {} while(0)
#define TEST_SUCCESS(TestName) do { return true; } while(0)

#endif // !UE_BUILD_SHIPPING
