// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameDataStructs.h"
#include "AbilitySystemInterface.h"
#include "WarRigPawn.generated.h"

// Forward declarations
class UAbilitySystemComponent;
class UDataTable;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class ULaneSystemComponent;
class USceneComponent;

/**
 * AWarRigPawn - The player's stationary war rig vehicle
 *
 * CRITICAL DESIGN: The war rig is STATIONARY at world origin (0,0,0). It does NOT move.
 * The world scrolls past the war rig to create the illusion of movement.
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

	// Debug commands
	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugShowWarRigBounds();

	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugShowMountPoints();

	UFUNCTION(Exec, Category = "Debug|War Rig")
	void DebugReloadWarRigData();

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

	/** Root component - always at world origin (0,0,0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig")
	TObjectPtr<USceneComponent> WarRigRoot;

	/** Ability System Component for GAS integration */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

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

	/** Cached war rig data from data table */
	UPROPERTY(BlueprintReadOnly, Category = "War Rig|Configuration")
	FWarRigData CachedRigData;

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
