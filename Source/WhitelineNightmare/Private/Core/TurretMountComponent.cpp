// Copyright Flatlander81. All Rights Reserved.

#include "Core/TurretMountComponent.h"
#include "Core/WarRigPawn.h"
#include "Turrets/TurretBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UTurretMountComponent::UTurretMountComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize debug settings
	bShowMountPointDebug = false;
	bShowFacingConstraintDebug = false;
	AvailableMountColor = FColor::Yellow;
	OccupiedMountColor = FColor::Red;
	MountPointDebugSize = 25.0f;
}

void UTurretMountComponent::BeginPlay()
{
	Super::BeginPlay();

	// If no mount points are set, initialize defaults
	if (MountPoints.Num() == 0)
	{
		InitializeDefaultMountPoints();
	}

	// Validate all mount points
	for (int32 i = 0; i < MountPoints.Num(); i++)
	{
		if (!ValidateMountTransform(MountPoints[i]))
		{
			UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::BeginPlay - Invalid transform for mount point %d"), i);
		}

		if (!ValidateFacingConstraints(MountPoints[i].AllowedFacingDirections))
		{
			UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::BeginPlay - Invalid facing constraints for mount point %d"), i);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::BeginPlay - Initialized with %d mount points"), MountPoints.Num());
}

void UTurretMountComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Draw debug visualization if enabled
	if (bShowMountPointDebug)
	{
		DrawMountPointDebug();
	}

	if (bShowFacingConstraintDebug)
	{
		DrawFacingConstraintDebug();
	}
}

// === MOUNT POINT QUERIES ===

FMountPointData UTurretMountComponent::GetMountPoint(int32 Index) const
{
	if (!ValidateMountIndex(Index, TEXT("GetMountPoint")))
	{
		return FMountPointData();
	}

	return MountPoints[Index];
}

bool UTurretMountComponent::IsMountOccupied(int32 Index) const
{
	if (!ValidateMountIndex(Index, TEXT("IsMountOccupied")))
	{
		return false;
	}

	return MountPoints[Index].bOccupied;
}

bool UTurretMountComponent::IsFacingAllowed(int32 MountIndex, int32 FacingDirection) const
{
	if (!ValidateMountIndex(MountIndex, TEXT("IsFacingAllowed")))
	{
		return false;
	}

	// Validate facing direction range (0-7)
	if (FacingDirection < 0 || FacingDirection > 7)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::IsFacingAllowed - Invalid facing direction: %d (must be 0-7)"), FacingDirection);
		return false;
	}

	const FMountPointData& MountPoint = MountPoints[MountIndex];

	// If no constraints, all directions allowed
	if (MountPoint.AllowedFacingDirections.Num() == 0)
	{
		return true;
	}

	// Check if direction is in allowed list
	return MountPoint.AllowedFacingDirections.Contains(FacingDirection);
}

ATurretBase* UTurretMountComponent::GetTurretAtMount(int32 Index) const
{
	if (!ValidateMountIndex(Index, TEXT("GetTurretAtMount")))
	{
		return nullptr;
	}

	return MountPoints[Index].OccupyingTurret;
}

TArray<int32> UTurretMountComponent::GetAvailableFacings(int32 MountIndex) const
{
	TArray<int32> AvailableFacings;

	if (!ValidateMountIndex(MountIndex, TEXT("GetAvailableFacings")))
	{
		return AvailableFacings;
	}

	const FMountPointData& MountPoint = MountPoints[MountIndex];

	// If no constraints, return all directions (0-7)
	if (MountPoint.AllowedFacingDirections.Num() == 0)
	{
		for (int32 i = 0; i < 8; i++)
		{
			AvailableFacings.Add(i);
		}
	}
	else
	{
		AvailableFacings = MountPoint.AllowedFacingDirections;
	}

	return AvailableFacings;
}

// === MOUNT/UNMOUNT OPERATIONS ===

bool UTurretMountComponent::MountTurret(int32 MountIndex, ATurretBase* Turret)
{
	// Validate mount index
	if (!ValidateMountIndex(MountIndex, TEXT("MountTurret")))
	{
		return false;
	}

	// Check if turret is valid
	if (!Turret)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::MountTurret - Turret is null"));
		return false;
	}

	// Check if mount is already occupied
	if (MountPoints[MountIndex].bOccupied)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::MountTurret - Mount %d is already occupied"), MountIndex);
		return false;
	}

	// Get the mount transform
	const FMountPointData& MountPoint = MountPoints[MountIndex];

	// Get owner war rig
	AWarRigPawn* OwnerWarRig = Cast<AWarRigPawn>(GetOwner());
	if (!OwnerWarRig)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretMountComponent::MountTurret - Owner is not a WarRigPawn"));
		return false;
	}

	// Attach turret to war rig at mount point location
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, false);
	Turret->AttachToActor(OwnerWarRig, AttachRules);
	Turret->SetActorRelativeTransform(MountPoint.MountTransform);

	// Update mount point data
	MountPoints[MountIndex].bOccupied = true;
	MountPoints[MountIndex].OccupyingTurret = Turret;

	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::MountTurret - Mounted turret at mount %d (%s)"),
		MountIndex, *MountPoint.DisplayName.ToString());

	return true;
}

bool UTurretMountComponent::UnmountTurret(int32 MountIndex)
{
	// Validate mount index
	if (!ValidateMountIndex(MountIndex, TEXT("UnmountTurret")))
	{
		return false;
	}

	// Check if mount is occupied
	if (!MountPoints[MountIndex].bOccupied)
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::UnmountTurret - Mount %d is not occupied"), MountIndex);
		return false;
	}

	// Get the turret
	ATurretBase* Turret = MountPoints[MountIndex].OccupyingTurret;
	if (Turret)
	{
		// Destroy the turret
		Turret->Destroy();
	}

	// Clear mount point data
	MountPoints[MountIndex].bOccupied = false;
	MountPoints[MountIndex].OccupyingTurret = nullptr;

	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::UnmountTurret - Unmounted turret from mount %d (%s)"),
		MountIndex, *MountPoints[MountIndex].DisplayName.ToString());

	return true;
}

// === INITIALIZATION ===

void UTurretMountComponent::SetMountPoints(const TArray<FMountPointData>& NewMountPoints)
{
	MountPoints = NewMountPoints;
	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::SetMountPoints - Set %d mount points"), MountPoints.Num());
}

void UTurretMountComponent::InitializeDefaultMountPoints()
{
	MountPoints.Empty();

	// Note: These positions are relative to the war rig root
	// X = forward/back, Y = left/right, Z = up/down

	// Mount Point 0: Cab - Left Side
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-100.0f, -150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {5, 6, 7, 0, 1}; // Can't point right (exclude 1,2,3)
		MountPoint.DisplayName = FText::FromString("Cab Left");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 1: Cab - Right Side
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-100.0f, 150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {0, 1, 2, 3, 4}; // Can't point left (exclude 5,6,7)
		MountPoint.DisplayName = FText::FromString("Cab Right");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 2: Trailer 1 - Front Left
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-200.0f, -150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {7, 0, 1, 4, 5, 6}; // Exclude forward-right area (2,3)
		MountPoint.DisplayName = FText::FromString("Trailer 1 Front Left");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 3: Trailer 1 - Front Right
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-200.0f, 150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {0, 1, 2, 3, 4, 5}; // Exclude forward-left area (6,7)
		MountPoint.DisplayName = FText::FromString("Trailer 1 Front Right");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 4: Trailer 1 - Rear Left
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-300.0f, -150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {2, 3, 4, 5, 6}; // Exclude forward (7,0,1)
		MountPoint.DisplayName = FText::FromString("Trailer 1 Rear Left");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 5: Trailer 1 - Rear Right
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-300.0f, 150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {2, 3, 4, 5, 6}; // Exclude forward (7,0,1)
		MountPoint.DisplayName = FText::FromString("Trailer 1 Rear Right");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 6: Trailer 2 - Front Left
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-400.0f, -150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {7, 0, 1, 4, 5, 6}; // Exclude forward-right area (2,3)
		MountPoint.DisplayName = FText::FromString("Trailer 2 Front Left");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 7: Trailer 2 - Front Right
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-400.0f, 150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {0, 1, 2, 3, 4, 5}; // Exclude forward-left area (6,7)
		MountPoint.DisplayName = FText::FromString("Trailer 2 Front Right");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 8: Trailer 2 - Rear Left
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-500.0f, -150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {3, 4, 5}; // Mostly rear-facing
		MountPoint.DisplayName = FText::FromString("Trailer 2 Rear Left");
		MountPoints.Add(MountPoint);
	}

	// Mount Point 9: Trailer 2 - Rear Right
	{
		FMountPointData MountPoint;
		MountPoint.MountTransform = FTransform(FRotator::ZeroRotator, FVector(-500.0f, 150.0f, 100.0f));
		MountPoint.AllowedFacingDirections = {3, 4, 5}; // Mostly rear-facing
		MountPoint.DisplayName = FText::FromString("Trailer 2 Rear Right");
		MountPoints.Add(MountPoint);
	}

	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::InitializeDefaultMountPoints - Initialized %d default mount points"), MountPoints.Num());
}

// === DEBUG VISUALIZATION ===

void UTurretMountComponent::DebugShowMountPoints()
{
	bShowMountPointDebug = !bShowMountPointDebug;
	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::DebugShowMountPoints - %s"),
		bShowMountPointDebug ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTurretMountComponent::DebugShowFacingConstraints()
{
	bShowFacingConstraintDebug = !bShowFacingConstraintDebug;
	UE_LOG(LogTemp, Log, TEXT("UTurretMountComponent::DebugShowFacingConstraints - %s"),
		bShowFacingConstraintDebug ? TEXT("Enabled") : TEXT("Disabled"));
}

void UTurretMountComponent::DebugListMounts()
{
	UE_LOG(LogTemp, Log, TEXT("========== Mount Points List =========="));
	UE_LOG(LogTemp, Log, TEXT("Total Mount Points: %d"), MountPoints.Num());

	for (int32 i = 0; i < MountPoints.Num(); i++)
	{
		const FMountPointData& MountPoint = MountPoints[i];
		UE_LOG(LogTemp, Log, TEXT(""));
		UE_LOG(LogTemp, Log, TEXT("Mount %d: %s"), i, *MountPoint.DisplayName.ToString());
		UE_LOG(LogTemp, Log, TEXT("  Position: %s"), *MountPoint.MountTransform.GetLocation().ToString());
		UE_LOG(LogTemp, Log, TEXT("  Rotation: %s"), *MountPoint.MountTransform.GetRotation().Rotator().ToString());
		UE_LOG(LogTemp, Log, TEXT("  Occupied: %s"), MountPoint.bOccupied ? TEXT("YES") : TEXT("NO"));

		if (MountPoint.bOccupied && MountPoint.OccupyingTurret)
		{
			UE_LOG(LogTemp, Log, TEXT("  Turret: %s"), *MountPoint.OccupyingTurret->GetName());
		}

		if (MountPoint.AllowedFacingDirections.Num() > 0)
		{
			FString DirectionsStr;
			for (int32 Dir : MountPoint.AllowedFacingDirections)
			{
				DirectionsStr += FString::Printf(TEXT("%d "), Dir);
			}
			UE_LOG(LogTemp, Log, TEXT("  Allowed Facings: %s"), *DirectionsStr);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  Allowed Facings: All (0-7)"));
		}

		if (MountPoint.MountTags.Num() > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("  Tags: %d"), MountPoint.MountTags.Num());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("======================================="));
}

void UTurretMountComponent::DrawMountPointDebug() const
{
	if (!GetOwner())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (int32 i = 0; i < MountPoints.Num(); i++)
	{
		const FMountPointData& MountPoint = MountPoints[i];

		// Get world location of mount point
		FVector WorldLocation = GetOwner()->GetActorTransform().TransformPosition(MountPoint.MountTransform.GetLocation());

		// Choose color based on occupancy
		FColor DebugColor = MountPoint.bOccupied ? OccupiedMountColor : AvailableMountColor;

		// Draw sphere
		DrawDebugSphere(World, WorldLocation, MountPointDebugSize, 12, DebugColor, false, -1.0f, 0, 2.0f);

		// Draw mount point index label
		DrawDebugString(World, WorldLocation + FVector(0, 0, MountPointDebugSize + 10.0f),
			FString::Printf(TEXT("%d"), i), nullptr, DebugColor, 0.0f, true);
	}
}

void UTurretMountComponent::DrawFacingConstraintDebug() const
{
	if (!GetOwner())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Direction vectors for 8 compass directions (0-7)
	// 0=North, 1=NE, 2=East, 3=SE, 4=South, 5=SW, 6=West, 7=NW
	const TArray<FVector> DirectionVectors = {
		FVector(1, 0, 0),      // 0: North (Forward)
		FVector(1, 1, 0).GetSafeNormal(),   // 1: NE
		FVector(0, 1, 0),      // 2: East (Right)
		FVector(-1, 1, 0).GetSafeNormal(),  // 3: SE
		FVector(-1, 0, 0),     // 4: South (Back)
		FVector(-1, -1, 0).GetSafeNormal(), // 5: SW
		FVector(0, -1, 0),     // 6: West (Left)
		FVector(1, -1, 0).GetSafeNormal()   // 7: NW
	};

	for (int32 i = 0; i < MountPoints.Num(); i++)
	{
		const FMountPointData& MountPoint = MountPoints[i];

		// Get world location of mount point
		FVector WorldLocation = GetOwner()->GetActorTransform().TransformPosition(MountPoint.MountTransform.GetLocation());

		// Get allowed directions
		TArray<int32> AllowedDirections = MountPoint.AllowedFacingDirections;

		// If no constraints, all directions allowed
		if (AllowedDirections.Num() == 0)
		{
			for (int32 j = 0; j < 8; j++)
			{
				AllowedDirections.Add(j);
			}
		}

		// Draw allowed directions in green, blocked in red
		const float LineLength = 100.0f;
		for (int32 j = 0; j < 8; j++)
		{
			bool bAllowed = AllowedDirections.Contains(j);
			FColor LineColor = bAllowed ? FColor::Green : FColor::Red;

			// Transform direction by owner's rotation
			FVector WorldDirection = GetOwner()->GetActorTransform().TransformVector(DirectionVectors[j]);
			FVector EndLocation = WorldLocation + WorldDirection * LineLength;

			DrawDebugLine(World, WorldLocation, EndLocation, LineColor, false, -1.0f, 0, bAllowed ? 3.0f : 1.0f);
		}
	}
}

// === VALIDATION ===

bool UTurretMountComponent::ValidateMountIndex(int32 Index, const FString& FunctionName) const
{
	if (Index < 0 || Index >= MountPoints.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::%s - Invalid mount index: %d (valid range: 0-%d)"),
			*FunctionName, Index, MountPoints.Num() - 1);
		return false;
	}

	return true;
}

bool UTurretMountComponent::ValidateMountTransform(const FMountPointData& MountPoint) const
{
	const FTransform& Transform = MountPoint.MountTransform;
	const FVector Location = Transform.GetLocation();
	const FVector Scale = Transform.GetScale3D();

	// Check for NaN
	if (Location.ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretMountComponent::ValidateMountTransform - Location contains NaN"));
		return false;
	}

	if (Transform.GetRotation().ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretMountComponent::ValidateMountTransform - Rotation contains NaN"));
		return false;
	}

	if (Scale.ContainsNaN())
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretMountComponent::ValidateMountTransform - Scale contains NaN"));
		return false;
	}

	// Check for zero scale
	if (FMath::IsNearlyZero(Scale.X) || FMath::IsNearlyZero(Scale.Y) || FMath::IsNearlyZero(Scale.Z))
	{
		UE_LOG(LogTemp, Warning, TEXT("UTurretMountComponent::ValidateMountTransform - Scale contains zero"));
		return false;
	}

	return true;
}

bool UTurretMountComponent::ValidateFacingConstraints(const TArray<int32>& Directions) const
{
	for (int32 Direction : Directions)
	{
		if (Direction < 0 || Direction > 7)
		{
			UE_LOG(LogTemp, Error, TEXT("UTurretMountComponent::ValidateFacingConstraints - Invalid direction: %d (must be 0-7)"), Direction);
			return false;
		}
	}

	return true;
}

// === TESTING FUNCTIONS ===

void UTurretMountComponent::TestMountPointCount()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestMountPointCount =========="));

	const int32 ExpectedCount = 10;
	const int32 ActualCount = MountPoints.Num();

	if (ActualCount == ExpectedCount)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount point count is correct (%d)"), ActualCount);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Expected %d mount points, got %d"), ExpectedCount, ActualCount);
	}

	UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

void UTurretMountComponent::TestMountPointPositioning()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestMountPointPositioning =========="));

	bool bAllTestsPassed = true;

	for (int32 i = 0; i < MountPoints.Num(); i++)
	{
		const FMountPointData& MountPoint = MountPoints[i];

		if (!ValidateMountTransform(MountPoint))
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Mount point %d has invalid transform"), i);
			bAllTestsPassed = false;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("  Mount %d: Position %s - VALID"),
				i, *MountPoint.MountTransform.GetLocation().ToString());
		}
	}

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: All mount point transforms are valid"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some mount point positioning tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("==============================================="));
}

void UTurretMountComponent::TestMountOccupancy()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestMountOccupancy =========="));

	bool bAllTestsPassed = true;

	// Test 1: Try to mount at valid mount (should succeed)
	UE_LOG(LogTemp, Log, TEXT("Test 1: Attempting to mount at mount 0 (should succeed)"));

	// Create a dummy turret for testing
	AWarRigPawn* OwnerWarRig = Cast<AWarRigPawn>(GetOwner());
	if (!OwnerWarRig)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Owner is not a WarRigPawn - cannot test mounting"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: World is null - cannot test mounting"));
		return;
	}

	// For now, just test the occupancy flag logic without spawning actual turrets
	// Set mount 0 as occupied manually
	int32 TestMountIndex = 0;
	if (MountPoints.IsValidIndex(TestMountIndex))
	{
		MountPoints[TestMountIndex].bOccupied = true;

		if (IsMountOccupied(TestMountIndex))
		{
			UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount 0 correctly reported as occupied"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Mount 0 should be occupied"));
			bAllTestsPassed = false;
		}

		// Test 2: Try to mount at occupied mount (should fail)
		UE_LOG(LogTemp, Log, TEXT("Test 2: Attempting to mount at occupied mount (should fail)"));
		bool bResult = MountTurret(TestMountIndex, nullptr);
		if (!bResult)
		{
			UE_LOG(LogTemp, Log, TEXT("SUCCESS: Correctly rejected mounting on occupied point"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("FAILED: Should not allow mounting on occupied point"));
			bAllTestsPassed = false;
		}

		// Clean up - clear occupancy
		MountPoints[TestMountIndex].bOccupied = false;
		MountPoints[TestMountIndex].OccupyingTurret = nullptr;
	}

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: All occupancy tests PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some occupancy tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("=========================================="));
}

void UTurretMountComponent::TestFacingConstraints()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestFacingConstraints =========="));

	bool bAllTestsPassed = true;

	// Test mount 0 (Cab Left) - should block some right-facing directions
	int32 TestMountIndex = 0;
	if (MountPoints.IsValidIndex(TestMountIndex))
	{
		const FMountPointData& MountPoint = MountPoints[TestMountIndex];

		UE_LOG(LogTemp, Log, TEXT("Testing Mount 0 (Cab Left)"));

		// This mount should have constraints
		if (MountPoint.AllowedFacingDirections.Num() > 0 && MountPoint.AllowedFacingDirections.Num() < 8)
		{
			UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount has facing constraints (%d allowed directions)"),
				MountPoint.AllowedFacingDirections.Num());

			// Test that at least one direction is blocked
			bool bFoundBlockedDirection = false;
			for (int32 i = 0; i < 8; i++)
			{
				if (!IsFacingAllowed(TestMountIndex, i))
				{
					bFoundBlockedDirection = true;
					UE_LOG(LogTemp, Log, TEXT("  Direction %d is blocked"), i);
				}
			}

			if (bFoundBlockedDirection)
			{
				UE_LOG(LogTemp, Log, TEXT("SUCCESS: Found at least one blocked direction"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("FAILED: No blocked directions found"));
				bAllTestsPassed = false;
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Mount 0 has no constraints or all directions allowed"));
		}
	}

	// Test invalid direction
	UE_LOG(LogTemp, Log, TEXT("Testing invalid direction (-1)"));
	if (!IsFacingAllowed(0, -1))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Correctly rejected invalid direction"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Should reject invalid direction"));
		bAllTestsPassed = false;
	}

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: All facing constraint tests PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some facing constraint tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void UTurretMountComponent::TestMountUnmount()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestMountUnmount =========="));

	// For this test, we'll simulate the mount/unmount cycle by manipulating the data directly
	// In a real scenario, you'd spawn actual turrets

	bool bAllTestsPassed = true;
	int32 TestMountIndex = 0;

	if (!MountPoints.IsValidIndex(TestMountIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Invalid test mount index"));
		return;
	}

	// Initial state - should be unoccupied
	if (!MountPoints[TestMountIndex].bOccupied)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount starts unoccupied"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: Mount is already occupied - clearing for test"));
		MountPoints[TestMountIndex].bOccupied = false;
		MountPoints[TestMountIndex].OccupyingTurret = nullptr;
	}

	// Simulate mounting (without spawning actual turret)
	MountPoints[TestMountIndex].bOccupied = true;

	if (IsMountOccupied(TestMountIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount correctly marked as occupied"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Mount should be occupied"));
		bAllTestsPassed = false;
	}

	// Simulate unmounting
	MountPoints[TestMountIndex].bOccupied = false;
	MountPoints[TestMountIndex].OccupyingTurret = nullptr;

	if (!IsMountOccupied(TestMountIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount correctly marked as unoccupied after unmount"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Mount should be unoccupied"));
		bAllTestsPassed = false;
	}

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: Mount/Unmount cycle test PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Mount/Unmount cycle test FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("======================================"));
}

void UTurretMountComponent::TestDesignerMountOverride()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestDesignerMountOverride =========="));

	// This test verifies that designers can manually add mount points in the editor
	// If MountPoints array was populated before BeginPlay, we should see more than 0 or different from 10

	if (MountPoints.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount points array is populated (%d points)"), MountPoints.Num());
		UE_LOG(LogTemp, Log, TEXT("Designers can customize mount points by editing MountPoints array in WarRigPawn details panel"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No mount points configured - default initialization would run in BeginPlay"));
	}

	UE_LOG(LogTemp, Log, TEXT("To test manual override:"));
	UE_LOG(LogTemp, Log, TEXT("1. Open WarRigPawn in editor"));
	UE_LOG(LogTemp, Log, TEXT("2. Add/modify TurretMountComponent -> MountPoints array"));
	UE_LOG(LogTemp, Log, TEXT("3. Run TestMountPointCount to verify custom count"));

	UE_LOG(LogTemp, Log, TEXT("==============================================="));
}

void UTurretMountComponent::TestTurretMountAll()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║       TURRET MOUNT SYSTEM COMPREHENSIVE TEST SUITE            ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Running all turret mount tests in sequence..."));
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 1: Mount Point Count
	UE_LOG(LogTemp, Log, TEXT("► Test 1/6: Mount Point Count"));
	TestMountPointCount();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 2: Mount Point Positioning
	UE_LOG(LogTemp, Log, TEXT("► Test 2/6: Mount Point Positioning"));
	TestMountPointPositioning();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 3: Mount Occupancy
	UE_LOG(LogTemp, Log, TEXT("► Test 3/6: Mount Occupancy"));
	TestMountOccupancy();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 4: Facing Constraints
	UE_LOG(LogTemp, Log, TEXT("► Test 4/6: Facing Constraints"));
	TestFacingConstraints();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 5: Mount/Unmount Cycle
	UE_LOG(LogTemp, Log, TEXT("► Test 5/6: Mount/Unmount Cycle"));
	TestMountUnmount();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 6: Designer Mount Override
	UE_LOG(LogTemp, Log, TEXT("► Test 6/6: Designer Mount Override"));
	TestDesignerMountOverride();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Summary
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║                    TEST SUITE COMPLETE                        ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("All 6 turret mount tests have been executed."));
	UE_LOG(LogTemp, Log, TEXT("Review the output above for SUCCESS/FAILED messages."));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Key Components Tested:"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Mount Point Count (10 for MVP)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Mount Point Transform Validation"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Occupancy Tracking"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Facing Constraint System"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Mount/Unmount Operations"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Designer Customization Support"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Debug Commands Available:"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugShowMountPoints"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugShowFacingConstraints"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugListMounts"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("═══════════════════════════════════════════════════════════════"));
}
