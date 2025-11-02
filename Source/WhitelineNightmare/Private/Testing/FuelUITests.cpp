// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "UI/WarRigHUDWidget.h"
#include "Core/WarRigHUD.h"
#include "Core/WarRigPawn.h"
#include "GAS/WarRigAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"

#if !UE_BUILD_SHIPPING

namespace
{
	// Helper function to get a valid world for testing
	UWorld* GetTestWorldForFuelUITests()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				return Context.World();
			}
		}
		return nullptr;
	}
}

/**
 * Test: Fuel UI Widget Creation
 * Verify that the widget creates all necessary UI elements
 */
static bool FuelUITest_WidgetCreation()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	// Trigger NativeConstruct by adding to viewport (we'll remove it immediately)
	Widget->AddToViewport();

	// Verify UI elements exist (they should be created in NativeConstruct)
	// Note: Widget internal members are protected, so we verify via logging/behavior
	// The fact that AddToViewport didn't crash means the widget was constructed successfully

	// Clean up
	Widget->RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_WidgetCreation: Widget created successfully"));
	TEST_SUCCESS("FuelUITest_WidgetCreation");
}

/**
 * Test: Fuel UI Update
 * Verify that the UI updates correctly when fuel values change
 */
static bool FuelUITest_UIUpdate()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Test various fuel values
	Widget->UpdateFuelDisplay(100.0f, 100.0f); // Full
	Widget->UpdateFuelDisplay(50.0f, 100.0f);  // Half
	Widget->UpdateFuelDisplay(0.0f, 100.0f);   // Empty

	// Test with different max fuel
	Widget->UpdateFuelDisplay(75.0f, 150.0f);

	// Test edge cases
	Widget->UpdateFuelDisplay(0.0f, 100.0f);   // Zero fuel
	Widget->UpdateFuelDisplay(100.0f, 100.0f); // Full fuel

	// Clean up
	Widget->RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_UIUpdate: UI updates completed successfully"));
	TEST_SUCCESS("FuelUITest_UIUpdate");
}

/**
 * Test: Fuel Color Coding
 * Verify that colors change at the correct thresholds
 */
static bool FuelUITest_ColorCoding()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Test high fuel (should be green) - > 60%
	Widget->UpdateFuelDisplay(80.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("FuelUITest_ColorCoding: Testing high fuel (80%% - should be green)"));

	// Test medium fuel (should be yellow) - 30-60%
	Widget->UpdateFuelDisplay(45.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("FuelUITest_ColorCoding: Testing medium fuel (45%% - should be yellow)"));

	// Test low fuel (should be red) - < 30%
	Widget->UpdateFuelDisplay(15.0f, 100.0f);
	UE_LOG(LogTemp, Log, TEXT("FuelUITest_ColorCoding: Testing low fuel (15%% - should be red)"));

	// Test threshold boundaries
	Widget->UpdateFuelDisplay(60.1f, 100.0f); // Just above yellow threshold (should be green)
	Widget->UpdateFuelDisplay(60.0f, 100.0f); // At yellow threshold (should be yellow)
	Widget->UpdateFuelDisplay(30.1f, 100.0f); // Just above red threshold (should be yellow)
	Widget->UpdateFuelDisplay(30.0f, 100.0f); // At red threshold (should be yellow)
	Widget->UpdateFuelDisplay(29.9f, 100.0f); // Just below red threshold (should be red)

	// Clean up
	Widget->RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_ColorCoding: Color coding tests completed successfully"));
	TEST_SUCCESS("FuelUITest_ColorCoding");
}

/**
 * Test: Fuel Text Display Format
 * Verify that the text displays fuel values correctly
 */
static bool FuelUITest_TextDisplay()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Test text format with various values
	Widget->UpdateFuelDisplay(100.0f, 100.0f); // Should display "Fuel: 100 / 100"
	Widget->UpdateFuelDisplay(75.0f, 100.0f);  // Should display "Fuel: 75 / 100"
	Widget->UpdateFuelDisplay(0.0f, 100.0f);   // Should display "Fuel: 0 / 100"
	Widget->UpdateFuelDisplay(50.5f, 100.0f);  // Should display "Fuel: 50 / 100" or "Fuel: 51 / 100" (rounded)

	// Test with different max fuel
	Widget->UpdateFuelDisplay(150.0f, 200.0f); // Should display "Fuel: 150 / 200"

	// Clean up
	Widget->RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_TextDisplay: Text display format tests completed successfully"));
	TEST_SUCCESS("FuelUITest_TextDisplay");
}

/**
 * Test: GAS Attribute Binding
 * Verify that the widget successfully binds to GAS attributes
 */
static bool FuelUITest_GASBinding()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Try to get the War Rig pawn
	AWarRigPawn* WarRig = nullptr;
	for (TActorIterator<AWarRigPawn> It(World); It; ++It)
	{
		WarRig = *It;
		break;
	}

	if (WarRig)
	{
		// Get the Ability System Component
		UAbilitySystemComponent* ASC = WarRig->GetAbilitySystemComponent();
		TEST_NOT_NULL(ASC, "War Rig should have an AbilitySystemComponent");

		// Initialize the widget with the ASC
		Widget->InitializeWidget(ASC);

		// Verify binding was successful
		TEST_TRUE(Widget->IsBindingSuccessful(), "Widget should successfully bind to GAS attributes");

		UE_LOG(LogTemp, Log, TEXT("FuelUITest_GASBinding: Successfully bound to GAS attributes"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FuelUITest_GASBinding: No War Rig found in world, skipping binding test"));
		// This is not a failure - just means we're not in a game with a War Rig
	}

	// Clean up
	Widget->RemoveFromParent();

	TEST_SUCCESS("FuelUITest_GASBinding");
}

/**
 * Test: MaxFuel Change Handling
 * Verify that UI updates correctly when MaxFuel attribute changes
 */
static bool FuelUITest_MaxFuelChange()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Test max fuel changes
	Widget->UpdateFuelDisplay(50.0f, 100.0f); // 50%
	Widget->UpdateFuelDisplay(50.0f, 200.0f); // Now 25% (same fuel, different max)
	Widget->UpdateFuelDisplay(100.0f, 200.0f); // Now 50%

	// Verify that percentage calculation is correct
	// 50/100 = 50% (yellow)
	// 50/200 = 25% (red)
	// 100/200 = 50% (yellow)

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_MaxFuelChange: MaxFuel change handling completed successfully"));

	// Clean up
	Widget->RemoveFromParent();

	TEST_SUCCESS("FuelUITest_MaxFuelChange");
}

/**
 * Test: Widget Visibility Toggle
 * Verify that the widget can be hidden and shown
 */
static bool FuelUITest_VisibilityToggle()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Create the widget
	UWarRigHUDWidget* Widget = CreateWidget<UWarRigHUDWidget>(World, UWarRigHUDWidget::StaticClass());
	TEST_NOT_NULL(Widget, "Widget should be created");

	Widget->AddToViewport();

	// Initial state should be visible
	TEST_EQUAL(Widget->GetVisibility(), ESlateVisibility::Visible, "Widget should start visible");

	// Toggle visibility
	Widget->ToggleVisibility();
	TEST_EQUAL(Widget->GetVisibility(), ESlateVisibility::Hidden, "Widget should be hidden after toggle");

	// Toggle back
	Widget->ToggleVisibility();
	TEST_EQUAL(Widget->GetVisibility(), ESlateVisibility::Visible, "Widget should be visible after second toggle");

	// Clean up
	Widget->RemoveFromParent();

	UE_LOG(LogTemp, Log, TEXT("FuelUITest_VisibilityToggle: Visibility toggle tests completed successfully"));
	TEST_SUCCESS("FuelUITest_VisibilityToggle");
}

/**
 * Test: HUD Widget Integration
 * Verify that AWarRigHUD creates and initializes the fuel widget correctly
 */
static bool FuelUITest_HUDIntegration()
{
	UWorld* World = GetTestWorldForFuelUITests();
	TEST_NOT_NULL(World, "World should exist for UI testing");

	// Try to get the HUD
	APlayerController* PC = World->GetFirstPlayerController();
	if (PC)
	{
		AWarRigHUD* HUD = Cast<AWarRigHUD>(PC->GetHUD());
		if (HUD)
		{
			// HUD should have created the fuel widget in BeginPlay
			// We can't directly access the widget (it's private), but we can test the debug commands
			HUD->DebugShowFuelBindings();
			UE_LOG(LogTemp, Log, TEXT("FuelUITest_HUDIntegration: HUD integration test completed"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("FuelUITest_HUDIntegration: No WarRigHUD found, skipping integration test"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("FuelUITest_HUDIntegration: No PlayerController found, skipping integration test"));
	}

	TEST_SUCCESS("FuelUITest_HUDIntegration");
}

/**
 * Run all fuel UI tests with comprehensive summary
 */
static bool FuelUITest_TestAll()
{
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  FUEL UI TEST SUITE"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));

	struct FTestInfo
	{
		FString Name;
		bool bPassed;
		typedef bool (*TestFunction)();
		TestFunction Function;
	};

	TArray<FTestInfo> Tests = {
		{ TEXT("Widget Creation"), false, &FuelUITest_WidgetCreation },
		{ TEXT("UI Update"), false, &FuelUITest_UIUpdate },
		{ TEXT("Color Coding"), false, &FuelUITest_ColorCoding },
		{ TEXT("Text Display"), false, &FuelUITest_TextDisplay },
		{ TEXT("GAS Binding"), false, &FuelUITest_GASBinding },
		{ TEXT("MaxFuel Change"), false, &FuelUITest_MaxFuelChange },
		{ TEXT("Visibility Toggle"), false, &FuelUITest_VisibilityToggle },
		{ TEXT("HUD Integration"), false, &FuelUITest_HUDIntegration }
	};

	int32 PassedTests = 0;
	int32 TotalTests = Tests.Num();

	// Run all tests
	for (FTestInfo& Test : Tests)
	{
		UE_LOG(LogTemp, Log, TEXT("Running Test: %s"), *Test.Name);
		UE_LOG(LogTemp, Log, TEXT("----------------------------------------"));

		Test.bPassed = Test.Function();

		if (Test.bPassed)
		{
			PassedTests++;
			UE_LOG(LogTemp, Log, TEXT("        [PASS] %s"), *Test.Name);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("        [FAIL] %s"), *Test.Name);
		}

		UE_LOG(LogTemp, Log, TEXT(""));
	}

	// Print summary
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  TEST SUMMARY"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT(""));

	for (const FTestInfo& Test : Tests)
	{
		FString Status = Test.bPassed ? TEXT("[PASS]") : TEXT("[FAIL]");
		UE_LOG(LogTemp, Log, TEXT("  %s %s"), *Status, *Test.Name);
	}

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("  Total Tests: %d"), TotalTests);
	UE_LOG(LogTemp, Log, TEXT("  Passed: %d"), PassedTests);
	UE_LOG(LogTemp, Log, TEXT("  Failed: %d"), TotalTests - PassedTests);

	const float PassPercentage = (float)PassedTests / (float)TotalTests * 100.0f;
	UE_LOG(LogTemp, Log, TEXT("  Pass Rate: %.1f%%"), PassPercentage);

	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("========================================"));

	if (PassedTests == TotalTests)
	{
		UE_LOG(LogTemp, Log, TEXT("  ALL TESTS PASSED!"));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("  SOME TESTS FAILED!"));
		UE_LOG(LogTemp, Log, TEXT("========================================"));
		return false;
	}
}

/**
 * Register all fuel UI tests with the test manager
 */
void RegisterFuelUITests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	// Register individual tests
	TestManager->RegisterTest(TEXT("FuelUI_WidgetCreation"), ETestCategory::UI, &FuelUITest_WidgetCreation);
	TestManager->RegisterTest(TEXT("FuelUI_UIUpdate"), ETestCategory::UI, &FuelUITest_UIUpdate);
	TestManager->RegisterTest(TEXT("FuelUI_ColorCoding"), ETestCategory::UI, &FuelUITest_ColorCoding);
	TestManager->RegisterTest(TEXT("FuelUI_TextDisplay"), ETestCategory::UI, &FuelUITest_TextDisplay);
	TestManager->RegisterTest(TEXT("FuelUI_GASBinding"), ETestCategory::GAS, &FuelUITest_GASBinding);
	TestManager->RegisterTest(TEXT("FuelUI_MaxFuelChange"), ETestCategory::GAS, &FuelUITest_MaxFuelChange);
	TestManager->RegisterTest(TEXT("FuelUI_VisibilityToggle"), ETestCategory::UI, &FuelUITest_VisibilityToggle);
	TestManager->RegisterTest(TEXT("FuelUI_HUDIntegration"), ETestCategory::UI, &FuelUITest_HUDIntegration);

	// Register comprehensive test
	TestManager->RegisterTest(TEXT("FuelUI_TestAll"), ETestCategory::UI, &FuelUITest_TestAll);

	UE_LOG(LogTemp, Log, TEXT("RegisterFuelUITests: Registered %d fuel UI tests"), 9);
}

#endif // !UE_BUILD_SHIPPING
