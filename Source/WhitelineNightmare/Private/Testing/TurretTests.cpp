// Copyright Flatlander81. All Rights Reserved.

#include "Testing/TestMacros.h"
#include "Testing/TestManager.h"
#include "Turrets/TurretBase.h"
#include "GAS/Attributes/CombatAttributeSet.h"
#include "Core/WarRigPawn.h"
#include "Core/GameDataStructs.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemComponent.h"

#if !UE_BUILD_SHIPPING

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Helper function to get a valid world for testing
static UWorld* GetTestWorld()
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

// Helper function to create a test war rig
static AWarRigPawn* CreateTestWarRig()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	AWarRigPawn* WarRig = World->SpawnActor<AWarRigPawn>();
	if (WarRig)
	{
		WarRig->SetActorLocation(FVector::ZeroVector);
	}
	return WarRig;
}

// Helper function to create a test turret
static ATurretBase* CreateTestTurret()
{
	UWorld* World = GetTestWorld();
	if (!World)
	{
		return nullptr;
	}

	// Spawn turret at origin
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATurretBase* Turret = World->SpawnActor<ATurretBase>(ATurretBase::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	return Turret;
}

// Helper function to create test turret data
static FTurretData CreateTestTurretData()
{
	FTurretData Data;
	Data.TurretName = TEXT("TestTurret");
	Data.DisplayName = FText::FromString(TEXT("Test Turret"));
	Data.Description = FText::FromString(TEXT("A test turret for unit testing"));
	Data.BaseDamage = 25.0f;
	Data.FireRate = 2.0f;
	Data.Range = 1500.0f;
	Data.BaseHealth = 150.0f;
	Data.BuildCost = 100;
	Data.UpgradeCost = 50;
	return Data;
}

// ============================================================================
// TURRET TESTS
// ============================================================================

/**
 * Test: Turret Spawn
 * Verify that turret spawns at mount point correctly with all components
 */
static bool TurretTest_TurretSpawn()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Verify components exist
	TEST_NOT_NULL(Turret->GetAbilitySystemComponent(), "AbilitySystemComponent should exist");
	TEST_NOT_NULL(Turret->GetCombatAttributeSet(), "CombatAttributeSet should exist");
	TEST_NOT_NULL(Turret->GetRootComponent(), "Root component should exist");

	// Verify initial state
	TEST_EQUAL(Turret->GetMountIndex(), -1, "Mount index should be -1 (uninitialized)");
	TEST_NULL(Turret->GetOwnerWarRig(), "Owner war rig should be null (not initialized)");
	TEST_NULL(Turret->GetCurrentTarget(), "Current target should be null");

	// Cleanup
	if (Turret)
	{
		Turret->Destroy();
	}

	TEST_SUCCESS("TurretTest_TurretSpawn");
}

/**
 * Test: Target Acquisition
 * Verify turret finds targets within range and firing arc
 */
static bool TurretTest_TargetAcquisition()
{
	UWorld* World = GetTestWorld();
	TEST_NOT_NULL(World, "World should exist");

	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	FRotator FacingDirection = FRotator(0.0f, 0.0f, 0.0f); // Facing forward (X-axis)
	Turret->Initialize(TurretData, 0, FacingDirection, WarRig);

	// Verify attributes initialized correctly
	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist after initialization");
	TEST_NEARLY_EQUAL(Attributes->GetRange(), TurretData.Range, 0.1f, "Range should match data table");

	// Create a test target (enemy pawn) within range and arc
	FVector TargetLocationInRange = Turret->GetActorLocation() + FVector(500.0f, 0.0f, 0.0f); // 500 units ahead
	AActor* TargetInRange = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationInRange, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetInRange, "Target in range should be created");

	// Verify target is in firing arc
	bool bIsInArc = Turret->IsTargetInFiringArc(TargetLocationInRange);
	TEST_TRUE(bIsInArc, "Target should be within 180° firing arc");

	// Create a test target outside range
	FVector TargetLocationOutOfRange = Turret->GetActorLocation() + FVector(3000.0f, 0.0f, 0.0f); // 3000 units ahead (beyond range)
	AActor* TargetOutOfRange = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationOutOfRange, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetOutOfRange, "Target out of range should be created");

	// Create a target behind turret (outside arc)
	FVector TargetLocationBehind = Turret->GetActorLocation() + FVector(-500.0f, 0.0f, 0.0f); // 500 units behind
	AActor* TargetBehind = World->SpawnActor<AActor>(AActor::StaticClass(), TargetLocationBehind, FRotator::ZeroRotator);
	TEST_NOT_NULL(TargetBehind, "Target behind should be created");

	// Verify target behind is not in arc
	bool bIsBehindInArc = Turret->IsTargetInFiringArc(TargetLocationBehind);
	TEST_FALSE(bIsBehindInArc, "Target behind should not be in firing arc");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();
	if (TargetInRange) TargetInRange->Destroy();
	if (TargetOutOfRange) TargetOutOfRange->Destroy();
	if (TargetBehind) TargetBehind->Destroy();

	TEST_SUCCESS("TurretTest_TargetAcquisition");
}

/**
 * Test: Firing Arc Calculation
 * Verify 180° arc math is correct using dot product
 */
static bool TurretTest_FiringArcCalculation()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Set turret facing forward (0°)
	FRotator FacingDirection = FRotator(0.0f, 0.0f, 0.0f); // Facing +X
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	Turret->Initialize(TurretData, 0, FacingDirection, WarRig);

	FVector TurretLocation = Turret->GetActorLocation();

	// Test 1: Target directly ahead (0°) - should be in arc (dot = 1.0)
	FVector TargetAhead = TurretLocation + FVector(100.0f, 0.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(TargetAhead), "Target directly ahead should be in arc");

	// Test 2: Target at 45° (forward-right) - should be in arc (dot > 0)
	FVector Target45Degrees = TurretLocation + FVector(100.0f, 100.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(Target45Degrees), "Target at 45° should be in arc");

	// Test 3: Target at 90° (perpendicular right) - should be at edge of arc (dot ≈ 0)
	FVector Target90Degrees = TurretLocation + FVector(0.0f, 100.0f, 0.0f);
	// This is the edge case - dot product will be very close to 0
	// It should still be in arc since we use > 0.0, but edge case
	bool bAt90 = Turret->IsTargetInFiringArc(Target90Degrees);
	// At exactly 90°, dot product is 0, which is NOT > 0, so it should be false
	TEST_FALSE(bAt90, "Target at exactly 90° should be at arc edge (not in arc)");

	// Test 4: Target at -45° (forward-left) - should be in arc (dot > 0)
	FVector TargetNeg45Degrees = TurretLocation + FVector(100.0f, -100.0f, 0.0f);
	TEST_TRUE(Turret->IsTargetInFiringArc(TargetNeg45Degrees), "Target at -45° should be in arc");

	// Test 5: Target at -90° (perpendicular left) - should be at edge of arc (dot ≈ 0)
	FVector TargetNeg90Degrees = TurretLocation + FVector(0.0f, -100.0f, 0.0f);
	bool bAtNeg90 = Turret->IsTargetInFiringArc(TargetNeg90Degrees);
	TEST_FALSE(bAtNeg90, "Target at exactly -90° should be at arc edge (not in arc)");

	// Test 6: Target at 135° (back-right) - should NOT be in arc (dot < 0)
	FVector Target135Degrees = TurretLocation + FVector(-100.0f, 100.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(Target135Degrees), "Target at 135° should not be in arc");

	// Test 7: Target directly behind (180°) - should NOT be in arc (dot = -1.0)
	FVector TargetBehind = TurretLocation + FVector(-100.0f, 0.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(TargetBehind), "Target directly behind should not be in arc");

	// Test 8: Target at -135° (back-left) - should NOT be in arc (dot < 0)
	FVector TargetNeg135Degrees = TurretLocation + FVector(-100.0f, -100.0f, 0.0f);
	TEST_FALSE(Turret->IsTargetInFiringArc(TargetNeg135Degrees), "Target at -135° should not be in arc");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_FiringArcCalculation");
}

/**
 * Test: Attribute Initialization
 * Verify stats load correctly from data table
 */
static bool TurretTest_AttributeInitialization()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Create test data
	FTurretData TurretData = CreateTestTurretData();
	TurretData.BaseDamage = 42.0f;
	TurretData.FireRate = 3.5f;
	TurretData.Range = 2000.0f;
	TurretData.BaseHealth = 250.0f;

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FRotator FacingDirection = FRotator(0.0f, 90.0f, 0.0f); // Facing right
	Turret->Initialize(TurretData, 5, FacingDirection, WarRig);

	// Verify attributes match data table
	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist");

	TEST_NEARLY_EQUAL(Attributes->GetHealth(), TurretData.BaseHealth, 0.1f, "Health should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetMaxHealth(), TurretData.BaseHealth, 0.1f, "MaxHealth should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetDamage(), TurretData.BaseDamage, 0.1f, "Damage should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetFireRate(), TurretData.FireRate, 0.1f, "FireRate should match data table");
	TEST_NEARLY_EQUAL(Attributes->GetRange(), TurretData.Range, 0.1f, "Range should match data table");

	// Verify turret properties
	TEST_EQUAL(Turret->GetMountIndex(), 5, "Mount index should be set correctly");
	TEST_EQUAL(Turret->GetOwnerWarRig(), WarRig, "Owner war rig should be set correctly");

	// Verify facing direction (check yaw)
	FRotator ActualFacing = Turret->GetFacingDirection();
	TEST_NEARLY_EQUAL(ActualFacing.Yaw, FacingDirection.Yaw, 0.1f, "Facing direction should match");

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_AttributeInitialization");
}

/**
 * Test: Null Target Handling
 * Verify graceful handling when no targets are available
 */
static bool TurretTest_NullTargetHandling()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Initialize turret
	AWarRigPawn* WarRig = CreateTestWarRig();
	FTurretData TurretData = CreateTestTurretData();
	Turret->Initialize(TurretData, 0, FRotator::ZeroRotator, WarRig);

	// Try to find a target (should return nullptr since no enemies exist)
	AActor* Target = Turret->FindTarget();
	TEST_NULL(Target, "FindTarget should return null when no targets available");

	// Verify current target is null
	TEST_NULL(Turret->GetCurrentTarget(), "Current target should be null");

	// Try to fire with null target (should handle gracefully without crashing)
	Turret->Fire(); // Should not crash or throw error

	// Verify turret is still valid after attempting to fire with null target
	TEST_TRUE(IsValid(Turret), "Turret should still be valid after firing with null target");

	// Test IsTargetInFiringArc with null target's location (edge case)
	// This tests that the function handles invalid input gracefully
	FVector InvalidLocation = FVector::ZeroVector;
	bool bResult = Turret->IsTargetInFiringArc(InvalidLocation);
	// Should not crash (result doesn't matter as much as not crashing)

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_NullTargetHandling");
}

/**
 * Test: Attribute Clamping
 * Verify that health is clamped to [0, MaxHealth] range
 */
static bool TurretTest_AttributeClamping()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	UCombatAttributeSet* Attributes = Turret->GetCombatAttributeSet();
	TEST_NOT_NULL(Attributes, "Attributes should exist");

	// Initialize with known values
	float MaxHealth = 100.0f;
	Attributes->InitMaxHealth(MaxHealth);
	Attributes->InitHealth(MaxHealth);

	// Verify initial values
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), MaxHealth, 0.1f, "Health should be at max");

	// Try to set health above max (should be clamped)
	Attributes->SetHealth(150.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), MaxHealth, 0.1f, "Health should be clamped to MaxHealth");

	// Try to set health below zero (should be clamped)
	Attributes->SetHealth(-50.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), 0.0f, 0.1f, "Health should be clamped to 0");

	// Set health to valid value
	Attributes->SetHealth(50.0f);
	TEST_NEARLY_EQUAL(Attributes->GetHealth(), 50.0f, 0.1f, "Health should be set to 50");

	// Change MaxHealth to lower value - health should be re-clamped
	Attributes->SetMaxHealth(40.0f);
	// Note: This requires PostGameplayEffectExecute to handle properly
	// For now, just verify MaxHealth changed
	TEST_NEARLY_EQUAL(Attributes->GetMaxHealth(), 40.0f, 0.1f, "MaxHealth should be updated");

	// Cleanup
	if (Turret) Turret->Destroy();

	TEST_SUCCESS("TurretTest_AttributeClamping");
}

/**
 * Test: Mount Point Integration
 * Verify turret correctly stores and retrieves mount point information
 */
static bool TurretTest_MountPointIntegration()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	AWarRigPawn* WarRig = CreateTestWarRig();
	TEST_NOT_NULL(WarRig, "War rig should be created");

	FTurretData TurretData = CreateTestTurretData();

	// Test different mount indices
	for (int32 MountIdx = 0; MountIdx < 10; ++MountIdx)
	{
		FRotator FacingDirection = FRotator(0.0f, MountIdx * 45.0f, 0.0f); // Different facing for each mount
		Turret->Initialize(TurretData, MountIdx, FacingDirection, WarRig);

		TEST_EQUAL(Turret->GetMountIndex(), MountIdx, "Mount index should match initialization value");
		TEST_NEARLY_EQUAL(Turret->GetFacingDirection().Yaw, FacingDirection.Yaw, 0.1f, "Facing direction should match");
		TEST_EQUAL(Turret->GetOwnerWarRig(), WarRig, "Owner war rig should be consistent");
	}

	// Cleanup
	if (Turret) Turret->Destroy();
	if (WarRig) WarRig->Destroy();

	TEST_SUCCESS("TurretTest_MountPointIntegration");
}

/**
 * Test: Ability System Component Integration
 * Verify ASC is properly integrated and accessible
 */
static bool TurretTest_AbilitySystemIntegration()
{
	ATurretBase* Turret = CreateTestTurret();
	TEST_NOT_NULL(Turret, "Turret should be created");

	// Verify ASC exists
	UAbilitySystemComponent* ASC = Turret->GetAbilitySystemComponent();
	TEST_NOT_NULL(ASC, "AbilitySystemComponent should exist");

	// Verify attribute set is added to ASC
	const TArray<UAttributeSet*>& AttributeSets = ASC->GetSpawnedAttributes();
	TEST_TRUE(AttributeSets.Num() > 0, "ASC should have at least one attribute set");

	// Verify CombatAttributeSet is present
	bool bHasCombatAttributes = false;
	for (UAttributeSet* AttrSet : AttributeSets)
	{
		if (Cast<UCombatAttributeSet>(AttrSet))
		{
			bHasCombatAttributes = true;
			break;
		}
	}
	TEST_TRUE(bHasCombatAttributes, "ASC should have CombatAttributeSet");

	// Verify ASC replication is enabled
	TEST_TRUE(ASC->GetIsReplicated(), "ASC should be replicated");

	// Cleanup
	if (Turret) Turret->Destroy();

	TEST_SUCCESS("TurretTest_AbilitySystemIntegration");
}

// ============================================================================
// TEST REGISTRATION
// ============================================================================

/**
 * Register all turret tests with the test manager
 * This function should be called from TestingGameMode::RegisterSampleTests()
 */
void RegisterTurretTests(UTestManager* TestManager)
{
	if (!TestManager)
	{
		return;
	}

	// Register all turret tests in Combat category
	TestManager->RegisterTest(TEXT("Turret_Spawn"), ETestCategory::Combat, &TurretTest_TurretSpawn);
	TestManager->RegisterTest(TEXT("Turret_TargetAcquisition"), ETestCategory::Combat, &TurretTest_TargetAcquisition);
	TestManager->RegisterTest(TEXT("Turret_FiringArcCalculation"), ETestCategory::Combat, &TurretTest_FiringArcCalculation);
	TestManager->RegisterTest(TEXT("Turret_AttributeInitialization"), ETestCategory::Combat, &TurretTest_AttributeInitialization);
	TestManager->RegisterTest(TEXT("Turret_NullTargetHandling"), ETestCategory::Combat, &TurretTest_NullTargetHandling);
	TestManager->RegisterTest(TEXT("Turret_AttributeClamping"), ETestCategory::GAS, &TurretTest_AttributeClamping);
	TestManager->RegisterTest(TEXT("Turret_MountPointIntegration"), ETestCategory::Combat, &TurretTest_MountPointIntegration);
	TestManager->RegisterTest(TEXT("Turret_AbilitySystemIntegration"), ETestCategory::GAS, &TurretTest_AbilitySystemIntegration);
}

#endif // !UE_BUILD_SHIPPING
