// Copyright Flatlander81. All Rights Reserved.

#include "WarRig/WarRigPawn.h"
#include "WarRig/LaneSystemComponent.h"
#include "Core/GameDataStructs.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

AWarRigPawn::AWarRigPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RigRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RigRoot"));
	RootComponent = RigRoot;

	// Create lane system component
	LaneSystemComponent = CreateDefaultSubobject<ULaneSystemComponent>(TEXT("LaneSystemComponent"));

	// Create camera spring arm
	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->SetupAttachment(RigRoot);
	CameraSpringArm->TargetArmLength = 1500.0f; // Default distance
	CameraSpringArm->SetRelativeRotation(FRotator(-75.0f, 0.0f, 0.0f)); // Top-down view
	CameraSpringArm->bDoCollisionTest = false; // No collision needed
	CameraSpringArm->bEnableCameraLag = false; // Camera follows rigidly

	// Create camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);

	// Default configuration
	bIsDataLoaded = false;
	DefaultRowName = "SemiTruck";
	GameplayBalanceDataTable = nullptr;
	BalanceDataRowName = "Default";

	// Auto-possess by player 0
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AWarRigPawn::BeginPlay()
{
	Super::BeginPlay();

	// Load data from default data table if provided
	if (DefaultWarRigDataTable)
	{
		LoadWarRigData(DefaultWarRigDataTable, DefaultRowName);
	}
	else
	{
		// Create default MVP meshes
		UE_LOG(LogTemp, Warning, TEXT("WarRigPawn: No data table configured. Creating default MVP meshes."));
		CreateDefaultMVPMeshes();
	}

	// Position war rig at origin (stationary)
	SetActorLocation(FVector(0.0f, 0.0f, 100.0f)); // Slight Z offset to be above ground
}

void AWarRigPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// War rig stays stationary - world scrolls past it
	// Any movement is handled by LaneSystemComponent (lateral only)
}

void AWarRigPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Input is now handled by the WarRigPlayerController using Enhanced Input System
	// No need to bind actions here - controller will call RequestLaneChange directly
	UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Input component setup (input handled by controller)"));
}

bool AWarRigPawn::LoadWarRigData(UDataTable* WarRigDataTable, FName RowName)
{
	if (!WarRigDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("WarRigPawn: Cannot load data - data table is null."));
		return false;
	}

	// Load data from table
	FWarRigData* Data = WarRigDataTable->FindRow<FWarRigData>(RowName, TEXT("WarRigPawn"));
	if (!Data)
	{
		UE_LOG(LogTemp, Error, TEXT("WarRigPawn: Cannot find row '%s' in data table."), *RowName.ToString());
		return false;
	}

	// Store data
	WarRigData = *Data;
	bIsDataLoaded = true;

	// Create mesh sections
	CreateMeshSections();

	// Setup camera
	SetupCamera();

	// Initialize lane system
	if (LaneSystemComponent)
	{
		// Load lane width from gameplay balance data table
		float LaneWidth = 400.0f; // Default fallback

		if (GameplayBalanceDataTable)
		{
			FGameplayBalanceData* BalanceData = GameplayBalanceDataTable->FindRow<FGameplayBalanceData>(BalanceDataRowName, TEXT("WarRigPawn"));
			if (BalanceData)
			{
				LaneWidth = BalanceData->LaneWidth;
				UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Loaded lane width %.2f from gameplay balance data"), LaneWidth);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WarRigPawn: Failed to load balance data row '%s', using default lane width %.2f"),
					*BalanceDataRowName.ToString(), LaneWidth);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WarRigPawn: No gameplay balance data table set, using default lane width %.2f"), LaneWidth);
		}

		LaneSystemComponent->Initialize(LaneWidth, 5);
		LaneSystemComponent->SetLaneChangeSpeed(WarRigData.LaneChangeSpeed);
	}

	UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Loaded data for '%s'."), *WarRigData.DisplayName.ToString());
	return true;
}

bool AWarRigPawn::RequestLaneChange(int32 Direction)
{
	if (!LaneSystemComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("WarRigPawn: Cannot change lanes - no lane system component."));
		return false;
	}

	return LaneSystemComponent->RequestLaneChange(Direction);
}

int32 AWarRigPawn::GetCurrentLane() const
{
	if (!LaneSystemComponent)
	{
		return 0;
	}

	return LaneSystemComponent->GetCurrentLane();
}

void AWarRigPawn::CreateMeshSections()
{
	// Clear existing mesh sections
	for (UStaticMeshComponent* MeshSection : MeshSections)
	{
		if (MeshSection)
		{
			MeshSection->DestroyComponent();
		}
	}
	MeshSections.Empty();

	// If no sections in data, create default MVP meshes
	if (WarRigData.RigSections.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WarRigPawn: No rig sections in data. Creating default MVP meshes."));
		CreateDefaultMVPMeshes();
		return;
	}

	// Create mesh sections from data
	float CurrentXOffset = 0.0f;

	for (int32 i = 0; i < WarRigData.RigSections.Num(); ++i)
	{
		const FWarRigSectionData& SectionData = WarRigData.RigSections[i];

		// Create mesh component
		FString ComponentName = FString::Printf(TEXT("MeshSection_%d"), i);
		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, FName(*ComponentName));
		MeshComponent->SetupAttachment(RigRoot);
		MeshComponent->RegisterComponent();

		// Position section
		FVector SectionLocation = FVector(CurrentXOffset, 0.0f, 0.0f);
		MeshComponent->SetRelativeLocation(SectionLocation);

		// Set mesh if available
		if (!SectionData.SectionMesh.IsNull())
		{
			UStaticMesh* Mesh = SectionData.SectionMesh.LoadSynchronous();
			if (Mesh)
			{
				MeshComponent->SetStaticMesh(Mesh);
			}
		}
		else
		{
			// Use default cube mesh
			UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
			if (CubeMesh)
			{
				MeshComponent->SetStaticMesh(CubeMesh);

				// Scale cube to match section size
				// Default cube is 100x100x100
				FVector Scale = SectionData.SectionSize / 100.0f;
				MeshComponent->SetRelativeScale3D(Scale);
			}
		}

		// Set material or color
		if (!SectionData.SectionMaterial.IsNull())
		{
			UMaterialInterface* Material = SectionData.SectionMaterial.LoadSynchronous();
			if (Material)
			{
				MeshComponent->SetMaterial(0, Material);
			}
		}
		else
		{
			// TODO: Create dynamic material instance and set color
			// For MVP, color will be applied via material in data table
		}

		// Disable collision
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// Add to array
		MeshSections.Add(MeshComponent);

		// Update offset for next section
		CurrentXOffset += SectionData.SectionSize.X;

		UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Created mesh section %d at X=%.2f"), i, SectionLocation.X);
	}

	UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Created %d mesh sections."), MeshSections.Num());
}

void AWarRigPawn::SetupCamera()
{
	if (!CameraSpringArm || !bIsDataLoaded)
	{
		return;
	}

	// Set camera distance and pitch from data
	CameraSpringArm->TargetArmLength = WarRigData.CameraDistance;
	CameraSpringArm->SetRelativeRotation(FRotator(WarRigData.CameraPitch, 0.0f, 0.0f));

	UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Camera setup - Distance: %.2f, Pitch: %.2f"),
		WarRigData.CameraDistance, WarRigData.CameraPitch);
}

void AWarRigPawn::CreateDefaultMVPMeshes()
{
	// Create 3 sections: cab (red) + 2 trailers (dark grey)

	// Load cube mesh
	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
	if (!CubeMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("WarRigPawn: Failed to load cube mesh."));
		return;
	}

	// Define section data
	struct SectionInfo
	{
		FVector Size;
		FLinearColor Color;
		FString Name;
	};

	TArray<SectionInfo> Sections = {
		{ FVector(200.0f, 150.0f, 100.0f), FLinearColor::Red, TEXT("Cab") },
		{ FVector(200.0f, 150.0f, 80.0f), FLinearColor(0.2f, 0.2f, 0.2f), TEXT("Trailer1") },
		{ FVector(200.0f, 150.0f, 80.0f), FLinearColor(0.2f, 0.2f, 0.2f), TEXT("Trailer2") }
	};

	float CurrentXOffset = 0.0f;

	for (int32 i = 0; i < Sections.Num(); ++i)
	{
		const SectionInfo& Section = Sections[i];

		// Create mesh component
		FString ComponentName = FString::Printf(TEXT("MeshSection_%d_%s"), i, *Section.Name);
		UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(this, FName(*ComponentName));
		MeshComponent->SetupAttachment(RigRoot);
		MeshComponent->RegisterComponent();

		// Set mesh
		MeshComponent->SetStaticMesh(CubeMesh);

		// Position and scale
		FVector SectionLocation = FVector(CurrentXOffset, 0.0f, 0.0f);
		MeshComponent->SetRelativeLocation(SectionLocation);

		// Scale: default cube is 100x100x100
		FVector Scale = Section.Size / 100.0f;
		MeshComponent->SetRelativeScale3D(Scale);

		// Disable collision
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		// TODO: Set color via dynamic material instance
		// For MVP, meshes will use default material

		// Add to array
		MeshSections.Add(MeshComponent);

		// Update offset for next section
		CurrentXOffset += Section.Size.X;

		UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Created default MVP mesh section '%s' at X=%.2f"),
			*Section.Name, SectionLocation.X);
	}

	UE_LOG(LogTemp, Log, TEXT("WarRigPawn: Created %d default MVP mesh sections."), MeshSections.Num());
}
