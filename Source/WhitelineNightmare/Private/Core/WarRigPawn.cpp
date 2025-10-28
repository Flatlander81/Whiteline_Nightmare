// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPawn.h"
#include "Core/LaneSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AWarRigPawn::AWarRigPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component at world origin
	WarRigRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WarRigRoot"));
	RootComponent = WarRigRoot;
	WarRigRoot->SetWorldLocation(FVector::ZeroVector);
	WarRigRoot->SetMobility(EComponentMobility::Movable); // Movable to allow lane changes (lateral Y-axis movement)

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create Lane System Component
	LaneSystemComponent = CreateDefaultSubobject<ULaneSystemComponent>(TEXT("LaneSystemComponent"));

	// Create Spring Arm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(WarRigRoot);
	SpringArmComponent->bDoCollisionTest = false; // No collision testing needed for top-down view
	SpringArmComponent->bInheritPitch = false;
	SpringArmComponent->bInheritYaw = false;
	SpringArmComponent->bInheritRoll = false;

	// Create Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	// Initialize debug settings
	bDebugShowMountPoints = false;
	bDebugShowBounds = false;
	MountPointDebugColor = FColor::Cyan;
	MountPointDebugSize = 50.0f;

	// Default rig ID
	CurrentRigID = FName("SemiTruck");
}

void AWarRigPawn::BeginPlay()
{
	Super::BeginPlay();

	// Initialize Ability System Component
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::BeginPlay - AbilitySystemComponent is null!"));
	}

	// Load war rig configuration from data table
	LoadWarRigConfiguration(CurrentRigID);

	// Ensure we're at world origin (defensive check)
	SetActorLocation(FVector::ZeroVector);
}

void AWarRigPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// CRITICAL: Ensure war rig stays at origin in X and Z (defensive programming)
	// Y-axis is allowed to change for lane changes
	FVector CurrentLocation = GetActorLocation();
	const float Tolerance = 0.1f;

	bool bNeedsCorrection = false;
	FVector CorrectedLocation = CurrentLocation;

	// Check X-axis (forward/backward) - should always be 0
	if (!FMath::IsNearlyZero(CurrentLocation.X, Tolerance))
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::Tick - War rig drifted in X! Resetting X to 0. Was at: %.2f"), CurrentLocation.X);
		CorrectedLocation.X = 0.0f;
		bNeedsCorrection = true;
	}

	// Check Z-axis (vertical) - should always be 0
	if (!FMath::IsNearlyZero(CurrentLocation.Z, Tolerance))
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::Tick - War rig drifted in Z! Resetting Z to 0. Was at: %.2f"), CurrentLocation.Z);
		CorrectedLocation.Z = 0.0f;
		bNeedsCorrection = true;
	}

	// Y-axis (lateral/lane movement) is allowed - don't check it

	if (bNeedsCorrection)
	{
		SetActorLocation(CorrectedLocation);
	}

	// Debug visualization
	if (bDebugShowMountPoints)
	{
		for (const TObjectPtr<USceneComponent>& MountPoint : MountPointComponents)
		{
			if (MountPoint)
			{
				FVector Location = MountPoint->GetComponentLocation();
				DrawDebugSphere(GetWorld(), Location, MountPointDebugSize, 12, MountPointDebugColor, false, -1.0f, 0, 2.0f);
			}
		}
	}

	if (bDebugShowBounds)
	{
		FVector Origin, BoxExtent;
		GetActorBounds(false, Origin, BoxExtent);
		DrawDebugBox(GetWorld(), Origin, BoxExtent, FColor::Green, false, -1.0f, 0, 3.0f);
	}
}

void AWarRigPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Input will be handled by the player controller
}

UAbilitySystemComponent* AWarRigPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AWarRigPawn::LoadWarRigConfiguration(const FName& RigID)
{
	if (!WarRigDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::LoadWarRigConfiguration - WarRigDataTable is null! Cannot load configuration."));
		return;
	}

	// Find the row in the data table
	FString ContextString(TEXT("LoadWarRigConfiguration"));
	FWarRigData* RigData = WarRigDataTable->FindRow<FWarRigData>(RigID, ContextString);

	if (!RigData)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::LoadWarRigConfiguration - Failed to find rig data for ID: %s"), *RigID.ToString());
		return;
	}

	// Validate the data
	if (!ValidateWarRigData(*RigData))
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::LoadWarRigConfiguration - Invalid rig data for ID: %s"), *RigID.ToString());
		return;
	}

	// Cache the data
	CachedRigData = *RigData;
	CurrentRigID = RigID;

	// Clear existing components
	ClearMeshComponents();
	ClearMountPoints();

	// Create new components from data
	CreateMeshComponents(*RigData);
	CreateMountPoints(*RigData);
	SetupCamera(*RigData);
	ApplyVisualProperties(*RigData);

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::LoadWarRigConfiguration - Successfully loaded configuration for: %s (%s)"),
		*RigID.ToString(), *RigData->DisplayName.ToString());
}

void AWarRigPawn::CreateMeshComponents(const FWarRigData& RigData)
{
	// For MVP, we'll use simple cube meshes positioned linearly
	// Cab at origin, trailers behind it
	const float SectionLength = 200.0f; // Length of each section
	const float SectionWidth = 150.0f;
	const float SectionHeight = 100.0f;

	for (int32 i = 0; i < RigData.MeshSections.Num(); i++)
	{
		// Create mesh component
		FString ComponentName = FString::Printf(TEXT("MeshSection_%d"), i);
		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), *ComponentName);

		if (MeshComponent)
		{
			MeshComponent->RegisterComponent();
			MeshComponent->AttachToComponent(WarRigRoot, FAttachmentTransformRules::KeepRelativeTransform);

			// Position sections linearly (X-axis is forward)
			// Cab at origin, trailers behind (negative X)
			FVector Position = FVector(-i * SectionLength, 0.0f, 0.0f);
			MeshComponent->SetRelativeLocation(Position);

			// Try to load the mesh from the data table
			if (RigData.MeshSections[i].ToSoftObjectPath().IsValid())
			{
				UStaticMesh* LoadedMesh = RigData.MeshSections[i].LoadSynchronous();
				if (LoadedMesh)
				{
					MeshComponent->SetStaticMesh(LoadedMesh);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::CreateMeshComponents - Failed to load mesh for section %d"), i);
				}
			}

			// For MVP: Use engine primitive cube as fallback
			// This will be replaced with actual meshes in the data table
			if (!MeshComponent->GetStaticMesh())
			{
				// Note: In a real project, you'd use a default cube mesh from engine content
				// For now, we'll just leave it without a mesh and rely on data table setup
				UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::CreateMeshComponents - Section %d has no mesh. Set up meshes in data table."), i);
			}

			MeshComponents.Add(MeshComponent);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::CreateMeshComponents - Created %d mesh sections"), MeshComponents.Num());
}

void AWarRigPawn::CreateMountPoints(const FWarRigData& RigData)
{
	for (int32 i = 0; i < RigData.MountPoints.Num(); i++)
	{
		const FMountPointData& MountData = RigData.MountPoints[i];

		// Create scene component for mount point
		FString ComponentName = FString::Printf(TEXT("MountPoint_%d"), i);
		USceneComponent* MountComponent = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), *ComponentName);

		if (MountComponent)
		{
			MountComponent->RegisterComponent();
			MountComponent->AttachToComponent(WarRigRoot, FAttachmentTransformRules::KeepRelativeTransform);
			MountComponent->SetRelativeTransform(MountData.MountTransform);

			// Tag the component for easy lookup
			MountComponent->ComponentTags.Add(FName("MountPoint"));
			MountComponent->ComponentTags.Add(FName(*FString::Printf(TEXT("MountPoint_%d"), i)));

			// Add gameplay tags if any
			// Note: Scene components don't have native gameplay tag support,
			// so we'll store this information in the cached data

			MountPointComponents.Add(MountComponent);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::CreateMountPoints - Created %d mount points"), MountPointComponents.Num());
}

void AWarRigPawn::SetupCamera(const FWarRigData& RigData)
{
	if (SpringArmComponent && CameraComponent)
	{
		// Set spring arm length (camera distance)
		SpringArmComponent->TargetArmLength = RigData.CameraDistance;

		// Set rotation (pitch angle for top-down view)
		FRotator CameraRotation(RigData.CameraPitch, 0.0f, 0.0f);
		SpringArmComponent->SetRelativeRotation(CameraRotation);

		// Position spring arm slightly forward to center on the war rig
		SpringArmComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

		UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::SetupCamera - Distance: %.1f, Pitch: %.1f"),
			RigData.CameraDistance, RigData.CameraPitch);
	}
}

void AWarRigPawn::ApplyVisualProperties(const FWarRigData& RigData)
{
	// Apply materials and colors to mesh components
	for (UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent)
		{
			// Load primary material if set
			if (RigData.PrimaryMaterial.ToSoftObjectPath().IsValid())
			{
				UMaterialInterface* Material = RigData.PrimaryMaterial.LoadSynchronous();
				if (Material)
				{
					MeshComponent->SetMaterial(0, Material);
				}
			}

			// In MVP, we'll use simple colored materials
			// The actual implementation will depend on material setup in Unreal
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::ApplyVisualProperties - Applied visual properties"));
}

bool AWarRigPawn::ValidateWarRigData(const FWarRigData& RigData) const
{
	bool bValid = true;

	if (RigData.MeshSections.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::ValidateWarRigData - No mesh sections defined"));
		bValid = false;
	}

	if (RigData.MaxHull <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::ValidateWarRigData - Invalid MaxHull value: %.1f"), RigData.MaxHull);
		bValid = false;
	}

	if (RigData.LaneChangeSpeed <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::ValidateWarRigData - Invalid LaneChangeSpeed value: %.1f"), RigData.LaneChangeSpeed);
		bValid = false;
	}

	if (RigData.CameraDistance <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::ValidateWarRigData - Invalid CameraDistance value: %.1f"), RigData.CameraDistance);
		bValid = false;
	}

	return bValid;
}

void AWarRigPawn::ClearMeshComponents()
{
	for (UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent)
		{
			MeshComponent->DestroyComponent();
		}
	}
	MeshComponents.Empty();
}

void AWarRigPawn::ClearMountPoints()
{
	for (USceneComponent* MountComponent : MountPointComponents)
	{
		if (MountComponent)
		{
			MountComponent->DestroyComponent();
		}
	}
	MountPointComponents.Empty();
}

// ===== DEBUG COMMANDS =====

void AWarRigPawn::DebugShowWarRigBounds()
{
	bDebugShowBounds = !bDebugShowBounds;
	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugShowWarRigBounds - %s"), bDebugShowBounds ? TEXT("Enabled") : TEXT("Disabled"));
}

void AWarRigPawn::DebugShowMountPoints()
{
	bDebugShowMountPoints = !bDebugShowMountPoints;
	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugShowMountPoints - %s"), bDebugShowMountPoints ? TEXT("Enabled") : TEXT("Disabled"));
}

void AWarRigPawn::DebugReloadWarRigData()
{
	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugReloadWarRigData - Reloading configuration for: %s"), *CurrentRigID.ToString());
	LoadWarRigConfiguration(CurrentRigID);
}

// ===== TESTING FUNCTIONS =====

void AWarRigPawn::TestWarRigDataLoading()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestWarRigDataLoading =========="));

	if (!WarRigDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: WarRigDataTable is null"));
		return;
	}

	FString ContextString(TEXT("TestWarRigDataLoading"));
	FWarRigData* RigData = WarRigDataTable->FindRow<FWarRigData>(CurrentRigID, ContextString);

	if (RigData)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Found rig data for ID: %s"), *CurrentRigID.ToString());
		UE_LOG(LogTemp, Log, TEXT("  Display Name: %s"), *RigData->DisplayName.ToString());
		UE_LOG(LogTemp, Log, TEXT("  Max Hull: %.1f"), RigData->MaxHull);
		UE_LOG(LogTemp, Log, TEXT("  Lane Change Speed: %.1f"), RigData->LaneChangeSpeed);
		UE_LOG(LogTemp, Log, TEXT("  Mesh Sections: %d"), RigData->MeshSections.Num());
		UE_LOG(LogTemp, Log, TEXT("  Mount Points: %d"), RigData->MountPoints.Num());
		UE_LOG(LogTemp, Log, TEXT("  Camera Distance: %.1f"), RigData->CameraDistance);
		UE_LOG(LogTemp, Log, TEXT("  Camera Pitch: %.1f"), RigData->CameraPitch);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Could not find rig data for ID: %s"), *CurrentRigID.ToString());
	}

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestWarRigSpawn()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestWarRigSpawn =========="));

	UE_LOG(LogTemp, Log, TEXT("Mesh Components: %d"), MeshComponents.Num());
	for (int32 i = 0; i < MeshComponents.Num(); i++)
	{
		if (MeshComponents[i])
		{
			FVector Location = MeshComponents[i]->GetComponentLocation();
			UE_LOG(LogTemp, Log, TEXT("  Mesh %d: Location = %s, HasMesh = %s"),
				i, *Location.ToString(), MeshComponents[i]->GetStaticMesh() ? TEXT("Yes") : TEXT("No"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  Mesh %d: NULL"), i);
		}
	}

	if (MeshComponents.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mesh components created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: No mesh components created"));
	}

	UE_LOG(LogTemp, Log, TEXT("====================================="));
}

void AWarRigPawn::TestMountPointSetup()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestMountPointSetup =========="));

	UE_LOG(LogTemp, Log, TEXT("Mount Point Components: %d"), MountPointComponents.Num());
	for (int32 i = 0; i < MountPointComponents.Num(); i++)
	{
		if (MountPointComponents[i])
		{
			FVector Location = MountPointComponents[i]->GetComponentLocation();
			FRotator Rotation = MountPointComponents[i]->GetComponentRotation();
			UE_LOG(LogTemp, Log, TEXT("  Mount %d: Location = %s, Rotation = %s"),
				i, *Location.ToString(), *Rotation.ToString());

			// Check cached data
			if (i < CachedRigData.MountPoints.Num())
			{
				const FMountPointData& MountData = CachedRigData.MountPoints[i];
				UE_LOG(LogTemp, Log, TEXT("    Display Name: %s"), *MountData.DisplayName.ToString());
				UE_LOG(LogTemp, Log, TEXT("    Facing Constraints: %d"), MountData.AllowedFacingDirections.Num());
				UE_LOG(LogTemp, Log, TEXT("    Tags: %d"), MountData.MountTags.Num());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("  Mount %d: NULL"), i);
		}
	}

	if (MountPointComponents.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Mount points created"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: No mount points created"));
	}

	UE_LOG(LogTemp, Log, TEXT("========================================="));
}

void AWarRigPawn::TestCameraSetup()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestCameraSetup =========="));

	if (SpringArmComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Spring Arm:"));
		UE_LOG(LogTemp, Log, TEXT("  Target Arm Length: %.1f"), SpringArmComponent->TargetArmLength);
		UE_LOG(LogTemp, Log, TEXT("  Rotation: %s"), *SpringArmComponent->GetComponentRotation().ToString());
		UE_LOG(LogTemp, Log, TEXT("  Location: %s"), *SpringArmComponent->GetComponentLocation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: SpringArmComponent is null"));
	}

	if (CameraComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("Camera:"));
		UE_LOG(LogTemp, Log, TEXT("  Location: %s"), *CameraComponent->GetComponentLocation().ToString());
		UE_LOG(LogTemp, Log, TEXT("  Rotation: %s"), *CameraComponent->GetComponentRotation().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: CameraComponent is null"));
	}

	if (SpringArmComponent && CameraComponent)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Camera setup complete"));
	}

	UE_LOG(LogTemp, Log, TEXT("===================================="));
}

void AWarRigPawn::TestStationaryPosition()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestStationaryPosition =========="));

	FVector ActorLocation = GetActorLocation();
	FVector RootLocation = WarRigRoot ? WarRigRoot->GetComponentLocation() : FVector::ZeroVector;

	UE_LOG(LogTemp, Log, TEXT("Actor Location: %s"), *ActorLocation.ToString());
	UE_LOG(LogTemp, Log, TEXT("Root Component Location: %s"), *RootLocation.ToString());

	const float Tolerance = 0.1f;
	bool bPassedTests = true;

	// Check X-axis (forward/backward) - should always be 0
	if (FMath::IsNearlyZero(ActorLocation.X, Tolerance))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: War rig X position is at origin (%.2f)"), ActorLocation.X);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: War rig X position is NOT at origin! X = %.2f"), ActorLocation.X);
		bPassedTests = false;
	}

	// Y-axis (lateral/lane) - can be any value (lane changes)
	UE_LOG(LogTemp, Log, TEXT("INFO: War rig Y position (lateral/lane): %.2f (allowed to vary)"), ActorLocation.Y);

	// Check Z-axis (vertical) - should always be 0
	if (FMath::IsNearlyZero(ActorLocation.Z, Tolerance))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: War rig Z position is at origin (%.2f)"), ActorLocation.Z);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: War rig Z position is NOT at origin! Z = %.2f"), ActorLocation.Z);
		bPassedTests = false;
	}

	// Check mobility - should be Movable to allow lane changes
	if (WarRigRoot && WarRigRoot->Mobility == EComponentMobility::Movable)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Root component mobility is Movable (allows lane changes)"));
	}
	else if (WarRigRoot)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Root component mobility is not Movable! Mobility: %d"), (int32)WarRigRoot->Mobility);
		bPassedTests = false;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: WarRigRoot is null!"));
		bPassedTests = false;
	}

	if (bPassedTests)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: All stationary position tests PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some stationary position tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("============================================"));
}
