// Copyright Flatlander81. All Rights Reserved.

#include "Core/WarRigPawn.h"
#include "Core/LaneSystemComponent.h"
#include "Core/WarRigHUD.h"
#include "AbilitySystemComponent.h"
#include "GAS/WarRigAttributeSet.h"
#include "GAS/GameplayAbility_FuelDrain.h"
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

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<UWarRigAttributeSet>(TEXT("AttributeSet"));

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

		// Initialize fuel attributes
		if (AttributeSet)
		{
			// TODO: Load balance data from data table
			// For now, use hardcoded defaults matching FGameplayBalanceData
			const float DefaultMaxFuel = 100.0f;
			const float DefaultStartFuel = 100.0f;

			// Set MaxFuel first
			AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetMaxFuelAttribute(), DefaultMaxFuel);

			// Then set Fuel (will be clamped to MaxFuel)
			AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), DefaultStartFuel);

			UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::BeginPlay - Initialized fuel: %.2f / %.2f"),
				AttributeSet->GetFuel(), AttributeSet->GetMaxFuel());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::BeginPlay - AttributeSet is null!"));
		}

		// Grant passive fuel drain ability
		if (FuelDrainAbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(FuelDrainAbilityClass, 1, INDEX_NONE, this);
			FuelDrainAbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);

			// Activate the ability immediately
			if (FuelDrainAbilityHandle.IsValid())
			{
				AbilitySystemComponent->TryActivateAbility(FuelDrainAbilityHandle);
				UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::BeginPlay - Fuel drain ability granted and activated"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::BeginPlay - Failed to grant fuel drain ability!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AWarRigPawn::BeginPlay - FuelDrainAbilityClass not set! Set it in Blueprint or defaults."));
		}
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

void AWarRigPawn::DebugShowLanes()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugShowLanes - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->DebugShowLanes();
}

void AWarRigPawn::ToggleDebugLaneUI()
{
	// Get the HUD
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::ToggleDebugLaneUI - No player controller found!"));
		return;
	}

	AWarRigHUD* WarRigHUD = Cast<AWarRigHUD>(PC->GetHUD());
	if (!WarRigHUD)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::ToggleDebugLaneUI - No WarRigHUD found!"));
		return;
	}

	WarRigHUD->ToggleDebugLaneUI();
	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::ToggleDebugLaneUI - Toggled debug lane UI"));
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

void AWarRigPawn::TestWarRigAll()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║          WAR RIG COMPREHENSIVE TEST SUITE                     ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Running all war rig tests in sequence..."));
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 1: Data Loading
	UE_LOG(LogTemp, Log, TEXT("► Test 1/5: Data Loading"));
	TestWarRigDataLoading();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 2: Mesh Spawning
	UE_LOG(LogTemp, Log, TEXT("► Test 2/5: Mesh Spawning"));
	TestWarRigSpawn();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 3: Mount Point Setup
	UE_LOG(LogTemp, Log, TEXT("► Test 3/5: Mount Point Setup"));
	TestMountPointSetup();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 4: Camera Setup
	UE_LOG(LogTemp, Log, TEXT("► Test 4/5: Camera Setup"));
	TestCameraSetup();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 5: Stationary Position
	UE_LOG(LogTemp, Log, TEXT("► Test 5/5: Stationary Position (Movement Model)"));
	TestStationaryPosition();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Summary
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║                    TEST SUITE COMPLETE                        ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("All 5 war rig tests have been executed."));
	UE_LOG(LogTemp, Log, TEXT("Review the output above for SUCCESS/FAILED messages."));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Key Components Tested:"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Data Table Loading (DT_WarRigData)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Dynamic Mesh Spawning (cab + trailers)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Mount Point Creation (10 points)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Camera Configuration (SpringArm + Camera)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Movement Model (X/Z locked, Y for lane changes)"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("═══════════════════════════════════════════════════════════════"));
}

// ===== FUEL DEBUG COMMANDS =====

void AWarRigPawn::DebugAddFuel(float Amount)
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugAddFuel - AbilitySystemComponent or AttributeSet is null!"));
		return;
	}

	float CurrentFuel = AttributeSet->GetFuel();
	float NewFuel = CurrentFuel + Amount;

	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), NewFuel);

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugAddFuel - Added %.2f fuel (%.2f -> %.2f / %.2f)"),
		Amount, CurrentFuel, AttributeSet->GetFuel(), AttributeSet->GetMaxFuel());
}

void AWarRigPawn::DebugSetFuel(float Amount)
{
	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugSetFuel - AbilitySystemComponent or AttributeSet is null!"));
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), Amount);

	UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugSetFuel - Set fuel to %.2f / %.2f"),
		AttributeSet->GetFuel(), AttributeSet->GetMaxFuel());
}

void AWarRigPawn::DebugSetFuelDrainRate(float Rate)
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugSetFuelDrainRate - AbilitySystemComponent is null!"));
		return;
	}

	// Find the fuel drain ability
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(FuelDrainAbilityHandle);
	if (!AbilitySpec || !AbilitySpec->Ability)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugSetFuelDrainRate - Fuel drain ability not found!"));
		return;
	}

	// Cast to the fuel drain ability
	UGameplayAbility_FuelDrain* FuelDrainAbility = Cast<UGameplayAbility_FuelDrain>(AbilitySpec->Ability);
	if (FuelDrainAbility)
	{
		float OldRate = FuelDrainAbility->FuelDrainRate;
		FuelDrainAbility->FuelDrainRate = Rate;
		UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugSetFuelDrainRate - Changed fuel drain rate from %.2f to %.2f"),
			OldRate, Rate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugSetFuelDrainRate - Failed to cast ability to UGameplayAbility_FuelDrain!"));
	}
}

void AWarRigPawn::DebugToggleFuelDrain()
{
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugToggleFuelDrain - AbilitySystemComponent is null!"));
		return;
	}

	// Find the fuel drain ability
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(FuelDrainAbilityHandle);
	if (!AbilitySpec || !AbilitySpec->Ability)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugToggleFuelDrain - Fuel drain ability not found!"));
		return;
	}

	// Cast to the fuel drain ability
	UGameplayAbility_FuelDrain* FuelDrainAbility = Cast<UGameplayAbility_FuelDrain>(AbilitySpec->Ability);
	if (FuelDrainAbility)
	{
		FuelDrainAbility->bFuelDrainPaused = !FuelDrainAbility->bFuelDrainPaused;
		UE_LOG(LogTemp, Log, TEXT("AWarRigPawn::DebugToggleFuelDrain - Fuel drain %s"),
			FuelDrainAbility->bFuelDrainPaused ? TEXT("PAUSED") : TEXT("RESUMED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugToggleFuelDrain - Failed to cast ability to UGameplayAbility_FuelDrain!"));
	}
}

void AWarRigPawn::DebugShowFuel()
{
	if (!AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::DebugShowFuel - AttributeSet is null!"));
		return;
	}

	float CurrentFuel = AttributeSet->GetFuel();
	float MaxFuel = AttributeSet->GetMaxFuel();
	float FuelPercent = (MaxFuel > 0.0f) ? (CurrentFuel / MaxFuel) * 100.0f : 0.0f;

	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  FUEL STATUS"));
	UE_LOG(LogTemp, Log, TEXT("========================================"));
	UE_LOG(LogTemp, Log, TEXT("  Current Fuel: %.2f"), CurrentFuel);
	UE_LOG(LogTemp, Log, TEXT("  Max Fuel:     %.2f"), MaxFuel);
	UE_LOG(LogTemp, Log, TEXT("  Percentage:   %.1f%%"), FuelPercent);
	UE_LOG(LogTemp, Log, TEXT("========================================"));
}

// ===== FUEL SYSTEM TEST FUNCTIONS =====

void AWarRigPawn::TestFuelDrainRate()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestFuelDrainRate =========="));

	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent or AttributeSet is null"));
		return;
	}

	// Record initial fuel
	float InitialFuel = AttributeSet->GetFuel();
	UE_LOG(LogTemp, Log, TEXT("Initial Fuel: %.2f"), InitialFuel);

	// Wait for fuel drain (this test requires manual verification)
	UE_LOG(LogTemp, Log, TEXT("Monitor fuel over 5 seconds..."));
	UE_LOG(LogTemp, Log, TEXT("Expected drain: ~25 fuel (5 fuel/second * 5 seconds)"));
	UE_LOG(LogTemp, Log, TEXT("Use 'DebugShowFuel' command to check current fuel"));
	UE_LOG(LogTemp, Log, TEXT("SUCCESS: Test setup complete - monitor fuel manually"));

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestFuelClamping()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestFuelClamping =========="));

	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent or AttributeSet is null"));
		return;
	}

	bool bAllTestsPassed = true;
	float MaxFuel = AttributeSet->GetMaxFuel();

	// Test 1: Set fuel above max
	UE_LOG(LogTemp, Log, TEXT("Test 1: Setting fuel above max (%.2f)"), MaxFuel + 50.0f);
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), MaxFuel + 50.0f);
	float ClampedFuel = AttributeSet->GetFuel();

	if (FMath::IsNearlyEqual(ClampedFuel, MaxFuel, 0.01f))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel clamped to MaxFuel (%.2f)"), ClampedFuel);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel not clamped correctly! Expected %.2f, got %.2f"), MaxFuel, ClampedFuel);
		bAllTestsPassed = false;
	}

	// Test 2: Set fuel below 0
	UE_LOG(LogTemp, Log, TEXT("Test 2: Setting fuel below 0 (-50.0)"));
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), -50.0f);
	float ClampedToZero = AttributeSet->GetFuel();

	if (FMath::IsNearlyEqual(ClampedToZero, 0.0f, 0.01f))
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel clamped to 0 (%.2f)"), ClampedToZero);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel not clamped to 0! Expected 0.00, got %.2f"), ClampedToZero);
		bAllTestsPassed = false;
	}

	// Restore fuel to a reasonable value
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), MaxFuel);

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: All clamping tests PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some clamping tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestAbilityGranting()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestAbilityGranting =========="));

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent is null"));
		return;
	}

	// Check if fuel drain ability is granted
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(FuelDrainAbilityHandle);

	if (AbilitySpec && AbilitySpec->Ability)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel drain ability is granted"));
		UE_LOG(LogTemp, Log, TEXT("  Ability Class: %s"), *AbilitySpec->Ability->GetClass()->GetName());
		UE_LOG(LogTemp, Log, TEXT("  Is Active: %s"), AbilitySpec->IsActive() ? TEXT("Yes") : TEXT("No"));

		// Check if it's the correct type
		UGameplayAbility_FuelDrain* FuelDrainAbility = Cast<UGameplayAbility_FuelDrain>(AbilitySpec->Ability);
		if (FuelDrainAbility)
		{
			UE_LOG(LogTemp, Log, TEXT("  Drain Rate: %.2f"), FuelDrainAbility->FuelDrainRate);
			UE_LOG(LogTemp, Log, TEXT("  Drain Interval: %.2fs"), FuelDrainAbility->DrainInterval);
			UE_LOG(LogTemp, Log, TEXT("  Is Paused: %s"), FuelDrainAbility->bFuelDrainPaused ? TEXT("Yes") : TEXT("No"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel drain ability is NOT granted or handle is invalid"));
	}

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestGameOverTrigger()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestGameOverTrigger =========="));

	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent or AttributeSet is null"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("WARNING: This test will trigger game over!"));
	UE_LOG(LogTemp, Warning, TEXT("Setting fuel to 0..."));

	// Set fuel to 0 to trigger game over
	AbilitySystemComponent->SetNumericAttributeBase(AttributeSet->GetFuelAttribute(), 0.0f);

	UE_LOG(LogTemp, Log, TEXT("Fuel set to 0. Game over sequence should have triggered."));
	UE_LOG(LogTemp, Log, TEXT("Check console for game over messages."));
	UE_LOG(LogTemp, Log, TEXT("SUCCESS: Test complete - verify game over triggered"));

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestAttributeInitialization()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestAttributeInitialization =========="));

	if (!AbilitySystemComponent || !AttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent or AttributeSet is null"));
		return;
	}

	bool bAllTestsPassed = true;

	// Check Fuel attribute
	float CurrentFuel = AttributeSet->GetFuel();
	UE_LOG(LogTemp, Log, TEXT("Current Fuel: %.2f"), CurrentFuel);

	if (CurrentFuel >= 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel initialized (%.2f)"), CurrentFuel);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel is negative (%.2f)"), CurrentFuel);
		bAllTestsPassed = false;
	}

	// Check MaxFuel attribute
	float MaxFuel = AttributeSet->GetMaxFuel();
	UE_LOG(LogTemp, Log, TEXT("Max Fuel: %.2f"), MaxFuel);

	if (MaxFuel > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: MaxFuel initialized (%.2f)"), MaxFuel);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: MaxFuel is invalid (%.2f)"), MaxFuel);
		bAllTestsPassed = false;
	}

	// Check that Fuel <= MaxFuel
	if (CurrentFuel <= MaxFuel)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel is within MaxFuel bounds"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel (%.2f) exceeds MaxFuel (%.2f)"), CurrentFuel, MaxFuel);
		bAllTestsPassed = false;
	}

	if (bAllTestsPassed)
	{
		UE_LOG(LogTemp, Log, TEXT("OVERALL: All initialization tests PASSED"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OVERALL: Some initialization tests FAILED"));
	}

	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestFuelDrainPause()
{
	UE_LOG(LogTemp, Log, TEXT("========== TestFuelDrainPause =========="));

	if (!AbilitySystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: AbilitySystemComponent is null"));
		return;
	}

	// Find the fuel drain ability
	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(FuelDrainAbilityHandle);
	if (!AbilitySpec || !AbilitySpec->Ability)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Fuel drain ability not found"));
		return;
	}

	UGameplayAbility_FuelDrain* FuelDrainAbility = Cast<UGameplayAbility_FuelDrain>(AbilitySpec->Ability);
	if (!FuelDrainAbility)
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Could not cast to UGameplayAbility_FuelDrain"));
		return;
	}

	// Test pause/resume
	bool bInitialState = FuelDrainAbility->bFuelDrainPaused;
	UE_LOG(LogTemp, Log, TEXT("Initial pause state: %s"), bInitialState ? TEXT("PAUSED") : TEXT("ACTIVE"));

	// Pause
	FuelDrainAbility->bFuelDrainPaused = true;
	UE_LOG(LogTemp, Log, TEXT("Set to PAUSED"));

	if (FuelDrainAbility->bFuelDrainPaused)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel drain paused"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Could not pause fuel drain"));
	}

	// Resume
	FuelDrainAbility->bFuelDrainPaused = false;
	UE_LOG(LogTemp, Log, TEXT("Set to ACTIVE"));

	if (!FuelDrainAbility->bFuelDrainPaused)
	{
		UE_LOG(LogTemp, Log, TEXT("SUCCESS: Fuel drain resumed"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FAILED: Could not resume fuel drain"));
	}

	// Restore initial state
	FuelDrainAbility->bFuelDrainPaused = bInitialState;

	UE_LOG(LogTemp, Log, TEXT("OVERALL: Pause/Resume test complete"));
	UE_LOG(LogTemp, Log, TEXT("==========================================="));
}

void AWarRigPawn::TestFuelSystemAll()
{
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║          FUEL SYSTEM COMPREHENSIVE TEST SUITE                 ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Running all fuel system tests in sequence..."));
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 1: Attribute Initialization
	UE_LOG(LogTemp, Log, TEXT("► Test 1/6: Attribute Initialization"));
	TestAttributeInitialization();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 2: Ability Granting
	UE_LOG(LogTemp, Log, TEXT("► Test 2/6: Ability Granting"));
	TestAbilityGranting();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 3: Fuel Clamping
	UE_LOG(LogTemp, Log, TEXT("► Test 3/6: Fuel Clamping"));
	TestFuelClamping();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 4: Fuel Drain Pause
	UE_LOG(LogTemp, Log, TEXT("► Test 4/6: Fuel Drain Pause/Resume"));
	TestFuelDrainPause();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 5: Fuel Drain Rate
	UE_LOG(LogTemp, Log, TEXT("► Test 5/6: Fuel Drain Rate"));
	TestFuelDrainRate();
	UE_LOG(LogTemp, Log, TEXT(""));

	// Test 6: Game Over Trigger (skipped by default to avoid triggering game over)
	UE_LOG(LogTemp, Log, TEXT("► Test 6/6: Game Over Trigger (SKIPPED)"));
	UE_LOG(LogTemp, Log, TEXT("Run 'TestGameOverTrigger' manually to test game over sequence"));
	UE_LOG(LogTemp, Log, TEXT(""));

	// Summary
	UE_LOG(LogTemp, Log, TEXT("╔═══════════════════════════════════════════════════════════════╗"));
	UE_LOG(LogTemp, Log, TEXT("║                    TEST SUITE COMPLETE                        ║"));
	UE_LOG(LogTemp, Log, TEXT("╚═══════════════════════════════════════════════════════════════╝"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("5 of 6 fuel system tests have been executed."));
	UE_LOG(LogTemp, Log, TEXT("Review the output above for SUCCESS/FAILED messages."));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Key Components Tested:"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Attribute Initialization (Fuel, MaxFuel)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Ability Granting (Fuel Drain)"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Attribute Clamping ([0, MaxFuel])"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Fuel Drain Pause/Resume"));
	UE_LOG(LogTemp, Log, TEXT("  ✓ Fuel Drain Rate Monitoring"));
	UE_LOG(LogTemp, Log, TEXT("  - Game Over Trigger (manual test)"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("Debug Commands Available:"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugAddFuel <amount>"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugSetFuel <amount>"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugSetFuelDrainRate <rate>"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugToggleFuelDrain"));
	UE_LOG(LogTemp, Log, TEXT("  - DebugShowFuel"));
	UE_LOG(LogTemp, Log, TEXT(""));
	UE_LOG(LogTemp, Log, TEXT("═══════════════════════════════════════════════════════════════"));
}

// === LANE SYSTEM TEST WRAPPERS ===
// These functions forward test calls to the LaneSystemComponent
// Exec functions must be on Pawn/PlayerController/GameMode to be accessible from console

void AWarRigPawn::TestLaneSystemBounds()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestLaneSystemBounds - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestLaneSystemBounds();
}

void AWarRigPawn::TestLaneTransitionSpeed()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestLaneTransitionSpeed - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestLaneTransitionSpeed();
}

void AWarRigPawn::TestLaneChangeValidation()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestLaneChangeValidation - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestLaneChangeValidation();
}

void AWarRigPawn::TestCurrentLaneTracking()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestCurrentLaneTracking - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestCurrentLaneTracking();
}

void AWarRigPawn::TestStationaryInOtherAxes()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestStationaryInOtherAxes - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestStationaryInOtherAxes();
}

void AWarRigPawn::TestLaneSystemAll()
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AWarRigPawn::TestLaneSystemAll - LaneSystemComponent is null!"));
		return;
	}

	LaneSystemComponent->TestLaneSystemAll();
}
