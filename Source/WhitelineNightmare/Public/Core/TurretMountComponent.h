// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameDataStructs.h"
#include "TurretMountComponent.generated.h"

// Forward declarations
class ATurretBase;
class AWarRigPawn;

/**
 * UTurretMountComponent - Manages turret mounting system for the war rig
 *
 * DESIGN:
 * - Attached to AWarRigPawn
 * - Manages array of mount points (TArray<FMountPointData>)
 * - Handles turret placement and removal
 * - Tracks which mounts are occupied
 * - Provides mount point queries
 *
 * MOUNT POINT CONFIGURATION:
 * - 10 default mounting points for MVP:
 *   - 2 on cab (left/right sides)
 *   - 4 on first trailer (front-left, front-right, rear-left, rear-right)
 *   - 4 on second trailer (same layout)
 *
 * FACING CONSTRAINTS:
 * - 0=North, 1=NE, 2=East, 3=SE, 4=South, 5=SW, 6=West, 7=NW (45° increments)
 * - Left-side mounts: constrain to not point right (exclude directions 1,2,3)
 * - Right-side mounts: constrain to not point left (exclude directions 5,6,7)
 * - Front mounts: less restrictive (maybe exclude backward 3,4,5)
 * - Rear mounts: less restrictive (maybe exclude forward 7,0,1)
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API UTurretMountComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurretMountComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// === MOUNT POINT QUERIES ===

	/**
	 * Get mount point data by index
	 * @param Index - Mount point index (0 to NumMounts-1)
	 * @return Mount point data, or default if index out of range
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	FMountPointData GetMountPoint(int32 Index) const;

	/**
	 * Check if a mount point is occupied
	 * @param Index - Mount point index
	 * @return true if occupied, false if available or index out of range
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	bool IsMountOccupied(int32 Index) const;

	/**
	 * Check if a facing direction is allowed for a mount point
	 * @param MountIndex - Mount point index
	 * @param FacingDirection - Direction to check (0-7)
	 * @return true if allowed, false if not allowed or index out of range
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	bool IsFacingAllowed(int32 MountIndex, int32 FacingDirection) const;

	/**
	 * Get the turret currently mounted at a mount point
	 * @param Index - Mount point index
	 * @return Turret reference, or nullptr if no turret or index out of range
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	ATurretBase* GetTurretAtMount(int32 Index) const;

	/**
	 * Get number of mount points
	 * @return Total number of mount points
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	int32 GetNumMountPoints() const { return MountPoints.Num(); }

	/**
	 * Get all available facing directions for a mount point
	 * @param MountIndex - Mount point index
	 * @return Array of allowed directions (0-7), or empty if index out of range
	 */
	UFUNCTION(BlueprintPure, Category = "Turret Mount")
	TArray<int32> GetAvailableFacings(int32 MountIndex) const;

	// === MOUNT/UNMOUNT OPERATIONS ===

	/**
	 * Mount a turret at a specific mount point
	 * @param MountIndex - Index of mount point
	 * @param Turret - Turret to mount
	 * @return true if successfully mounted, false if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret Mount")
	bool MountTurret(int32 MountIndex, ATurretBase* Turret);

	/**
	 * Unmount a turret from a mount point
	 * @param MountIndex - Index of mount point
	 * @return true if successfully unmounted, false if failed
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret Mount")
	bool UnmountTurret(int32 MountIndex);

	// === INITIALIZATION ===

	/**
	 * Set mount points (called by WarRigPawn during initialization)
	 * @param NewMountPoints - Array of mount point data
	 */
	UFUNCTION(BlueprintCallable, Category = "Turret Mount")
	void SetMountPoints(const TArray<FMountPointData>& NewMountPoints);

	/**
	 * Initialize default mount points for MVP (10 points)
	 * Only called if no mount points are set
	 */
	void InitializeDefaultMountPoints();

	// === DEBUG VISUALIZATION ===

	/** Toggle debug visualization for mount points */
	UFUNCTION(Exec, Category = "Debug|Turret Mount")
	void DebugShowMountPoints();

	/** Toggle facing constraint visualization */
	UFUNCTION(Exec, Category = "Debug|Turret Mount")
	void DebugShowFacingConstraints();

	/** Log all mount points to console with status */
	UFUNCTION(Exec, Category = "Debug|Turret Mount")
	void DebugListMounts();

	// === TESTING FUNCTIONS ===

	/** Test: Verify correct number of mount points (10 for MVP) */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestMountPointCount();

	/** Test: Verify mount point transforms are reasonable */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestMountPointPositioning();

	/** Test: Verify can't place two turrets on same mount */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestMountOccupancy();

	/** Test: Verify constraints block invalid directions */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestFacingConstraints();

	/** Test: Verify mount/unmount cycle works correctly */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestMountUnmount();

	/** Test: Verify can manually add mounts in editor */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestDesignerMountOverride();

	/** Run all turret mount tests */
	UFUNCTION(Exec, Category = "Testing|Combat")
	void TestTurretMountAll();

protected:
	// === VALIDATION ===

	/**
	 * Validate mount index is in valid range
	 * @param Index - Mount index to validate
	 * @param FunctionName - Name of calling function (for logging)
	 * @return true if valid, false if out of range
	 */
	bool ValidateMountIndex(int32 Index, const FString& FunctionName) const;

	/**
	 * Validate mount point transform is valid (no NaN, no zero scale)
	 * @param MountPoint - Mount point to validate
	 * @return true if valid, false if invalid
	 */
	bool ValidateMountTransform(const FMountPointData& MountPoint) const;

	/**
	 * Validate facing constraint values (0-7 range)
	 * @param Directions - Array of directions to validate
	 * @return true if all valid, false if any invalid
	 */
	bool ValidateFacingConstraints(const TArray<int32>& Directions) const;

	/**
	 * Draw debug visualization for mount points
	 */
	void DrawMountPointDebug() const;

	/**
	 * Draw debug visualization for facing constraints
	 */
	void DrawFacingConstraintDebug() const;

protected:
	// === MOUNT POINTS ===

	/** Array of mount point data - designer can customize in editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount")
	TArray<FMountPointData> MountPoints;

	// === DEBUG ===

	/** Show mount point debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount|Debug")
	bool bShowMountPointDebug;

	/** Show facing constraint debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount|Debug")
	bool bShowFacingConstraintDebug;

	/** Color for available mount points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount|Debug")
	FColor AvailableMountColor;

	/** Color for occupied mount points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount|Debug")
	FColor OccupiedMountColor;

	/** Size of mount point debug spheres */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret Mount|Debug")
	float MountPointDebugSize;
};
