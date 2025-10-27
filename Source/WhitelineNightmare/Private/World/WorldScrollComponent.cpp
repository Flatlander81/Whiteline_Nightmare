// Copyright Flatlander81. All Rights Reserved.

#include "World/WorldScrollComponent.h"
#include "Core/GameDataStructs.h"
#include "Engine/DataTable.h"

UWorldScrollComponent::UWorldScrollComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// Default configuration (editor-configurable)
	GameplayBalanceDataTable = nullptr;
	BalanceDataRowName = "Default";
	FallbackScrollSpeed = 1000.0f;

	// Runtime state
	ScrollSpeed = 0.0f; // Will be set from data table or fallback in BeginPlay
	DistanceTraveled = 0.0f;
	ScrollDirection = FVector(-1.0f, 0.0f, 0.0f); // Backward (negative X)
	bIsScrollEnabled = true;
	bIsInitialized = false;
}

void UWorldScrollComponent::BeginPlay()
{
	Super::BeginPlay();

	// Auto-initialize with configuration
	if (!bIsInitialized)
	{
		float ScrollSpeedToUse = FallbackScrollSpeed;

		// Try to load from gameplay balance data table (PRIMARY SOURCE)
		if (GameplayBalanceDataTable)
		{
			FGameplayBalanceData* BalanceData = GameplayBalanceDataTable->FindRow<FGameplayBalanceData>(BalanceDataRowName, TEXT("WorldScrollComponent"));
			if (BalanceData)
			{
				ScrollSpeedToUse = BalanceData->ScrollSpeed;
				UE_LOG(LogTemp, Log, TEXT("WorldScrollComponent: Loaded scroll speed %.2f from gameplay balance data table '%s'"),
					ScrollSpeedToUse, *BalanceDataRowName.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("WorldScrollComponent: Failed to load row '%s' from gameplay balance data table, using fallback speed %.2f"),
					*BalanceDataRowName.ToString(), FallbackScrollSpeed);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WorldScrollComponent: No gameplay balance data table set, using fallback scroll speed %.2f"), FallbackScrollSpeed);
		}

		// Initialize with the determined speed
		InitializeWithSpeed(ScrollSpeedToUse);
	}
}

void UWorldScrollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized || !bIsScrollEnabled)
	{
		return;
	}

	// Update distance traveled
	UpdateDistanceTraveled(DeltaTime);
}

bool UWorldScrollComponent::Initialize(UDataTable* WorldScrollDataTable, FName RowName)
{
	if (!WorldScrollDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("WorldScrollComponent: Cannot initialize - data table is null."));
		return false;
	}

	// Load data from table
	FWorldScrollData* ScrollData = WorldScrollDataTable->FindRow<FWorldScrollData>(RowName, TEXT("WorldScrollComponent"));
	if (!ScrollData)
	{
		UE_LOG(LogTemp, Error, TEXT("WorldScrollComponent: Cannot find row '%s' in data table."), *RowName.ToString());
		return false;
	}

	// Note: ScrollSpeed is now loaded from FGameplayBalanceData, not FWorldScrollData
	// For now, use a default or allow it to be set separately
	ScrollSpeed = 1000.0f; // Default, should be set via SetScrollSpeed or gameplay balance data

	DistanceTraveled = 0.0f;
	bIsScrollEnabled = true;
	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("WorldScrollComponent: Initialized from data table with speed %.2f."), ScrollSpeed);
	return true;
}

void UWorldScrollComponent::InitializeWithSpeed(float InScrollSpeed)
{
	if (InScrollSpeed < 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("WorldScrollComponent: Invalid scroll speed (%.2f). Must be non-negative."), InScrollSpeed);
		return;
	}

	ScrollSpeed = InScrollSpeed;
	DistanceTraveled = 0.0f;
	bIsScrollEnabled = true;
	bIsInitialized = true;

	UE_LOG(LogTemp, Log, TEXT("WorldScrollComponent: Initialized with speed %.2f."), ScrollSpeed);
}

void UWorldScrollComponent::SetScrollSpeed(float NewSpeed)
{
	if (NewSpeed < 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldScrollComponent: Invalid scroll speed (%.2f). Must be non-negative."), NewSpeed);
		return;
	}

	ScrollSpeed = NewSpeed;
	UE_LOG(LogTemp, Log, TEXT("WorldScrollComponent: Scroll speed set to %.2f."), ScrollSpeed);
}

FVector UWorldScrollComponent::GetScrollVelocity() const
{
	return ScrollDirection * ScrollSpeed;
}

void UWorldScrollComponent::SetScrollDirection(FVector NewDirection)
{
	if (NewDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldScrollComponent: Cannot set scroll direction to zero vector."));
		return;
	}

	ScrollDirection = NewDirection.GetSafeNormal();
	UE_LOG(LogTemp, Log, TEXT("WorldScrollComponent: Scroll direction set to (%.2f, %.2f, %.2f)."),
		ScrollDirection.X, ScrollDirection.Y, ScrollDirection.Z);
}

void UWorldScrollComponent::UpdateDistanceTraveled(float DeltaTime)
{
	float DeltaDistance = ScrollSpeed * DeltaTime;
	DistanceTraveled += DeltaDistance;

	// Optional: Log milestone distances
	static float LastLoggedDistance = 0.0f;
	if (DistanceTraveled - LastLoggedDistance >= 1000.0f)
	{
		UE_LOG(LogTemp, Verbose, TEXT("WorldScrollComponent: Distance traveled: %.2f"), DistanceTraveled);
		LastLoggedDistance = DistanceTraveled;
	}
}
