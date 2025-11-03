// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameDataStructs.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "WarRigPawn.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class UWarRigAttributeSet;
class UDataTable;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class ULaneSystemComponent;
class USceneComponent;
class ATurretBase;

/**
 * AWarRigPawn - The player's war rig vehicle
 *
 * CRITICAL DESIGN: The war rig is STATIONARY in forward/backward movement.
 * - X-axis (forward/backward): Always at 0 - does NOT move forward or backward
 * - Y-axis (lateral): Moves between lanes for lane changes
 * - Z-axis (vertical): Always at 0 - does NOT move up or down
 *
 * The world scrolls past the war rig to create the illusion of forward movement.
 * Lane changes are the ONLY player-controlled movement (lateral Y-axis).
 *
 * Configuration is loaded from a data table (DT_WarRigData) which supports multiple rig types.
 * MVP uses only the "SemiTruck" configuration.
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigPawn : public APawn, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AWarRigPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Data table loading
	void LoadWarRigConfiguration(const FName& RigID);

	// Testing functions
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestWarRigDataLoading();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestWarRigSpawn();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestMountPointSetup();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestCameraSetup();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestStationaryPosition();

	/** Run all war rig tests in sequence with comprehensive summary */
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestWarRigAll();

	// Lane System Testing Functions (wrappers for LaneSystemComponent)
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestLaneSystemBounds();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestLaneTransitionSpeed();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestLaneChangeValidation();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestCurrentLaneTracking();

	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestStationaryInOtherAxes();

	/** Run all lane system tests with comprehensive report */
	UFUNCTION(Exec, Category = "Testing|Movement")
	void TestLaneSystemAll();

	// Fuel System Testing Functions
	UFUNCTION(Exec, Category = "Testing|Economy")
	void TestFuelDrainRate();

	UFUNCTION(Exec, Category = "Testing|Economy")
	void TestFuelClamping();

	UFUNCTION(Exec, Category = "Testing|GAS")
	void TestAbilityGranting();

	UFUNCTION(Exec, Category = "Testing|Economy")
	void TestGameOverTrigger();

	UFUNCTION(Exec, Category = "Testing|GAS")
	void TestAttributeInitialization();

	UFUNCTION(Exec, Category = "Testing|Economy")
	void TestFuelDrainPause();

	/** Run all fuel system tests */
	UFUNCTION(Exec, Category = "Testing|Economy")
	void TestFuelSystemAll();

	// Debug commands
	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugShowWarRigBounds();

	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugShowMountPoints();

	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugReloadWarRigData();

	/** Toggle lane debug visualization (wrapper for LaneSystemComponent) */
	UFUNCTION(Exec, Category = "Debug|Lane System")
	void DebugShowLanes();

	/** Toggle debug lane UI widget (shows lane change buttons) */
	UFUNCTION(Exec, Category = "Debug|Lane System")
	void ToggleDebugLaneUI();

	// Fuel Debug Commands
	UFUNCTION(Exec, Category = "Debug|Fuel")
	void DebugAddFuel(float Amount);

	UFUNCTION(Exec, Category = "Debug|Fuel")
	void DebugSetFuel(float Amount);

	UFUNCTION(Exec, Category = "Debug|Fuel")
	void DebugSetFuelDrainRate(float Rate);

	UFUNCTION(Exec, Category = "Debug|Fuel")
	void DebugToggleFuelDrain();

	UFUNCTION(Exec, Category = "Debug|Fuel")
	void DebugShowFuel();

protected:
	// Component creation and setup
	void CreateMeshComponents(const FWarRigData& RigData);
	void CreateMountPoints(const FWarRigData& RigData);
	void SetupCamera(const FWarRigData& RigData);
	void ApplyVisualProperties(const FWarRigData& RigData);

	// Validation
	bool ValidateWarRigData(const FWarRigData& RigData) const;

	// Clean up existing components
	void ClearMeshComponents();
	void ClearMountPoints();

protected:
	// === COMPONENTS ===

	/** Root component - always at X=0, Z=0 (Y varies for lane changes) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig")
	TObjectPtr<USceneComponent> WarRigRoot;

	/** Ability System Component for GAS integration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Attribute Set for fuel system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Abilities")
	TObjectPtr<UWarRigAttributeSet> AttributeSet;

	/** Lane system component for lateral movement between lanes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Movement")
	TObjectPtr<ULaneSystemComponent> LaneSystemComponent;

	/** Spring arm for camera positioning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	/** Camera component for player view */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	/** Dynamically spawned mesh components (cab + trailers) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Mesh")
	TArray<TObjectPtr<UStaticMeshComponent>> MeshComponents;

	/** Dynamically spawned mount point scene components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Mount Points")
	TArray<TObjectPtr<USceneComponent>> MountPointComponents;

	// === CONFIGURATION ===

	/** Reference to the war rig data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Configuration")
	TObjectPtr<UDataTable> WarRigDataTable;

	/** Current rig configuration ID (e.g., "SemiTruck") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Configuration")
	FName CurrentRigID;

	/** Reference to the turret data table (DT_TurretData) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Turrets")
	TObjectPtr<UDataTable> TurretDataTable;

	/** Currently spawned turrets on this war rig */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Turrets")
	TArray<TObjectPtr<ATurretBase>> SpawnedTurrets;

	/** Cached war rig data from data table */
	UPROPERTY(BlueprintReadOnly, Category = "War Rig|Configuration")
	FWarRigData CachedRigData;

	/** Fuel drain ability class to grant */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Rig|Abilities")
	TSubclassOf<class UGameplayAbility> FuelDrainAbilityClass;

	/** Fuel drain ability spec handle */
	FGameplayAbilitySpecHandle FuelDrainAbilityHandle;

	/** Game over ability class to grant */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "War Rig|Abilities")
	TSubclassOf<class UGameplayAbility> GameOverAbilityClass;

	/** Game over ability spec handle */
	FGameplayAbilitySpecHandle GameOverAbilityHandle;

	// === DEBUG ===

	/** Show mount point debug spheres */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Debug")
	bool bDebugShowMountPoints;

	/** Show war rig collision bounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Debug")
	bool bDebugShowBounds;

	/** Color for mount point debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Debug")
	FColor MountPointDebugColor;

	/** Size of mount point debug spheres */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Debug")
	float MountPointDebugSize;
};
