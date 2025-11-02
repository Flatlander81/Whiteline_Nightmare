// Copyright Flatlander81. All Rights Reserved.

#include "Pickups/FuelPickup.h"
#include "Pickups/PickupPoolComponent.h"
#include "Components/SphereComponent.h"
#include "Core/WorldScrollComponent.h"
#include "Core/WarRigPawn.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"

AFuelPickup::AFuelPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create sphere component as root
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	RootComponent = SphereComponent;

	// Default collision settings
	SphereComponent->SetSphereRadius(50.0f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);

	// Bind overlap event
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AFuelPickup::OnSphereBeginOverlap);

	// Default pickup data row name
	PickupDataRowName = FName("FuelPickup");
}

void AFuelPickup::BeginPlay()
{
	Super::BeginPlay();

	// Load pickup data from data table if available
	if (PickupDataTable && !PickupDataRowName.IsNone())
	{
		InitializeFromDataTable(PickupDataRowName, PickupDataTable);
	}

	// Initially deactivated until pooling system activates it
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
}

void AFuelPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Scroll backward with the world
	if (WorldScrollComponent && WorldScrollComponent->IsScrolling())
	{
		const FVector ScrollVelocity = WorldScrollComponent->GetScrollVelocity();
		AddActorWorldOffset(ScrollVelocity * DeltaTime, false);
	}
}

void AFuelPickup::OnActivated_Implementation()
{
	// Enable collision
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// Make visible
	SetActorHiddenInGame(false);

	// Enable ticking for movement
	SetActorTickEnabled(true);
}

void AFuelPickup::OnDeactivated_Implementation()
{
	// Disable collision
	if (SphereComponent)
	{
		SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Make invisible
	SetActorHiddenInGame(true);

	// Disable ticking
	SetActorTickEnabled(false);

	// Clean up any active particle effects
	if (ActiveParticleComponent)
	{
		ActiveParticleComponent->DestroyComponent();
		ActiveParticleComponent = nullptr;
	}
}

void AFuelPickup::ResetState_Implementation()
{
	// Clear any cached state
	// Currently no state to clear beyond what OnDeactivated handles
}

void AFuelPickup::InitializeFromDataTable(FName RowName, UDataTable* DataTable)
{
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFuelPickup::InitializeFromDataTable - DataTable is null"));
		return;
	}

	// Load pickup data from data table
	FPickupData* PickupDataRow = DataTable->FindRow<FPickupData>(RowName, TEXT("FuelPickup"));
	if (PickupDataRow)
	{
		PickupData = *PickupDataRow;
		PickupDataRowName = RowName;
		PickupDataTable = DataTable;

		// Update visual appearance based on loaded data
		UpdateVisualAppearance();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AFuelPickup::InitializeFromDataTable - Failed to find row '%s' in DataTable"),
			*RowName.ToString());
	}
}

void AFuelPickup::SetWorldScrollComponent(UWorldScrollComponent* ScrollComponent)
{
	WorldScrollComponent = ScrollComponent;
}

void AFuelPickup::SetPoolComponent(UPickupPoolComponent* InPoolComponent)
{
	PoolComponent = InPoolComponent;
}

void AFuelPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if overlapping with war rig
	AWarRigPawn* WarRig = Cast<AWarRigPawn>(OtherActor);
	if (!WarRig)
	{
		return;
	}

	// Apply fuel restoration
	ApplyFuelRestore(WarRig);

	// Play pickup effects
	PlayPickupEffects();

	// Return to pool
	if (PoolComponent)
	{
		// The pool component will handle deactivation via OnDeactivated
		PoolComponent->ReturnToPool(this);
	}
	else
	{
		// If no pool, just deactivate and hide
		IPoolableActor::Execute_OnDeactivated(this);
	}
}

void AFuelPickup::ApplyFuelRestore(AWarRigPawn* WarRig)
{
	if (!WarRig)
	{
		return;
	}

	// Get the war rig's ability system component
	UAbilitySystemComponent* ASC = WarRig->GetAbilitySystemComponent();
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AFuelPickup::ApplyFuelRestore - War rig has no AbilitySystemComponent"));
		return;
	}

	// Apply the fuel restore gameplay effect if available
	if (FuelRestoreEffectClass)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(FuelRestoreEffectClass, 1.0f, EffectContext);
		if (SpecHandle.IsValid())
		{
			// Set the fuel amount magnitude
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Fuel")), PickupData.FuelAmount);

			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
	else
	{
		// Fallback: Directly modify fuel attribute if no gameplay effect is set
		// This is similar to how FuelDrain currently works
		UAttributeSet* AttributeSet = const_cast<UAttributeSet*>(ASC->GetAttributeSet(UAttributeSet::StaticClass()));
		if (AttributeSet)
		{
			// Find the Fuel attribute
			FGameplayAttribute FuelAttribute = UAbilitySystemBlueprintLibrary::GetAttributeFromName(FName("Fuel"));
			if (FuelAttribute.IsValid())
			{
				float CurrentFuel = ASC->GetNumericAttribute(FuelAttribute);
				float NewFuel = CurrentFuel + PickupData.FuelAmount;
				ASC->SetNumericAttributeBase(FuelAttribute, NewFuel);
			}
		}
	}
}

void AFuelPickup::PlayPickupEffects()
{
	// Play pickup sound
	if (!PickupData.PickupSound.IsNull())
	{
		USoundBase* Sound = PickupData.PickupSound.LoadSynchronous();
		if (Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
	}

	// Spawn pickup particle effect
	if (!PickupData.PickupParticle.IsNull())
	{
		UNiagaraSystem* ParticleSystem = PickupData.PickupParticle.LoadSynchronous();
		if (ParticleSystem)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				ParticleSystem,
				GetActorLocation(),
				GetActorRotation()
			);
		}
	}
}

void AFuelPickup::UpdateVisualAppearance()
{
	if (!SphereComponent)
	{
		return;
	}

	// Update sphere radius
	SphereComponent->SetSphereRadius(PickupData.PickupRadius);

	// Create a dynamic material instance to set the color
	// For now, we'll use a simple colored material
	// In a full implementation, you'd create a proper material in the editor
	// and set its color parameter here

	// Note: Visual appearance will be more complete when actual materials are set up
	// For testing, the sphere collision visualization will show the pickup
}
