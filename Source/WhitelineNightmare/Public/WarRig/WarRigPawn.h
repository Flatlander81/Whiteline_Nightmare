// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/GameDataStructs.h"
#include "WarRigPawn.generated.h"

class ULaneSystemComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UDataTable;

/**
 * War Rig Pawn - The player-controlled war rig vehicle
 *
 * Key features:
 * - STATIONARY in world space at origin (world scrolls past it)
 * - Lane-based movement (5 fixed lanes, smooth transitions)
 * - Configurable via data table
 * - Modular mesh sections (cab + trailers)
 * - Top-down camera view
 * - AbilitySystemComponent for future gameplay abilities (TODO)
 */
UCLASS()
class WHITELINENIGHTMARE_API AWarRigPawn : public APawn
{
	GENERATED_BODY()

public:
	AWarRigPawn();

	/**
	 * Load war rig configuration from data table
	 * @param WarRigDataTable - Data table containing war rig configurations
	 * @param RowName - Row name to load
	 * @return True if loading succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "War Rig")
	bool LoadWarRigData(UDataTable* WarRigDataTable, FName RowName);

	/**
	 * Request a lane change
	 * @param Direction - Negative for left, positive for right
	 * @return True if lane change was initiated
	 */
	UFUNCTION(BlueprintCallable, Category = "War Rig")
	bool RequestLaneChange(int32 Direction);

	/**
	 * Get the current lane index
	 * @return Current lane (0-4 for 5 lanes)
	 */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	int32 GetCurrentLane() const;

	/**
	 * Get the lane system component
	 * @return Lane system component
	 */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	ULaneSystemComponent* GetLaneSystemComponent() const { return LaneSystemComponent; }

	/**
	 * Get the loaded war rig data
	 * @return War rig data struct
	 */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	FWarRigData GetWarRigData() const { return WarRigData; }

	/**
	 * Check if war rig data has been loaded
	 * @return True if data is loaded
	 */
	UFUNCTION(BlueprintPure, Category = "War Rig")
	bool IsDataLoaded() const { return bIsDataLoaded; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/**
	 * Create mesh sections from war rig data
	 */
	void CreateMeshSections();

	/**
	 * Setup camera based on war rig data
	 */
	void SetupCamera();

	/**
	 * Create default MVP mesh sections (3 cubes: cab + 2 trailers)
	 */
	void CreateDefaultMVPMeshes();

	// Root component (war rig stays at this position)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig", meta = (AllowPrivateAccess = "true"))
	USceneComponent* RigRoot;

	// Lane system component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Components", meta = (AllowPrivateAccess = "true"))
	ULaneSystemComponent* LaneSystemComponent;

	// Camera spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraSpringArm;

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	// Mesh sections (cab + trailers)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Mesh", meta = (AllowPrivateAccess = "true"))
	TArray<UStaticMeshComponent*> MeshSections;

	// Loaded war rig data
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	FWarRigData WarRigData;

	// Whether data has been loaded
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	bool bIsDataLoaded;

	// Data table reference (for editor configuration)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	UDataTable* DefaultWarRigDataTable;

	// Default row name to load
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	FName DefaultRowName;

	// Gameplay balance data table (for lane width, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	UDataTable* GameplayBalanceDataTable;

	// Balance data row name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "War Rig|Data", meta = (AllowPrivateAccess = "true"))
	FName BalanceDataRowName;
};
