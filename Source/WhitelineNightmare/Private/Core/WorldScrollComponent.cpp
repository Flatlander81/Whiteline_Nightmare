// Copyright Flatlander81. All Rights Reserved.

#include "Core/WorldScrollComponent.h"
#include "Core/GameDataStructs.h"
#include "Engine/DataTable.h"

// Define logging category
DEFINE_LOG_CATEGORY_STATIC(LogWorldScroll, Log, All);

UWorldScrollComponent::UWorldScrollComponent()
	: ScrollDataTable(nullptr)
	, DataTableRowName("DefaultScroll")
	, ScrollSpeed(1000.0f)
	, bIsScrolling(true)
	, DistanceTraveled(0.0f)
	, ScrollDirection(-1.0f, 0.0f, 0.0f)
{
	// Enable ticking for distance accumulation
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UWorldScrollComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load configuration from data table
	if (!LoadConfigFromDataTable())
	{
		UE_LOG(LogWorldScroll, Warning, TEXT("WorldScrollComponent: Failed to load config from data table, using defaults"));
		// Use default values from constructor
	}

	// Normalize direction
	ScrollDirection = ValidateScrollDirection(ScrollDirection);

	// Log initial state
	LogScrollState();
}

void UWorldScrollComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Accumulate distance traveled if scrolling is active
	if (bIsScrolling && ScrollSpeed > 0.0f)
	{
		const float DeltaDistance = ScrollSpeed * DeltaTime;
		DistanceTraveled += DeltaDistance;

		// Verbose logging for debugging (can be disabled in shipping builds)
		UE_LOG(LogWorldScroll, VeryVerbose, TEXT("WorldScrollComponent: Distance += %.2f, Total = %.2f"),
			DeltaDistance, DistanceTraveled);
	}
}

FVector UWorldScrollComponent::GetScrollVelocity() const
{
	if (!bIsScrolling)
	{
		return FVector::ZeroVector;
	}

	return ScrollDirection * ScrollSpeed;
}

void UWorldScrollComponent::SetScrollSpeed(float NewSpeed)
{
	const float OldSpeed = ScrollSpeed;
	ScrollSpeed = ValidateScrollSpeed(NewSpeed);

	if (!FMath::IsNearlyEqual(OldSpeed, ScrollSpeed))
	{
		UE_LOG(LogWorldScroll, Log, TEXT("WorldScrollComponent: Scroll speed changed from %.2f to %.2f"),
			OldSpeed, ScrollSpeed);
	}
}

void UWorldScrollComponent::SetScrolling(bool bEnabled)
{
	if (bIsScrolling != bEnabled)
	{
		bIsScrolling = bEnabled;
		UE_LOG(LogWorldScroll, Log, TEXT("WorldScrollComponent: Scrolling %s"),
			bIsScrolling ? TEXT("ENABLED") : TEXT("DISABLED"));
	}
}

void UWorldScrollComponent::ResetDistance()
{
	const float OldDistance = DistanceTraveled;
	DistanceTraveled = 0.0f;

	UE_LOG(LogWorldScroll, Log, TEXT("WorldScrollComponent: Distance reset from %.2f to 0.0"),
		OldDistance);
}

void UWorldScrollComponent::SetScrollDirection(FVector NewDirection)
{
	FVector OldDirection = ScrollDirection;
	ScrollDirection = ValidateScrollDirection(NewDirection);

	if (!OldDirection.Equals(ScrollDirection, 0.01f))
	{
		UE_LOG(LogWorldScroll, Log, TEXT("WorldScrollComponent: Direction changed from %s to %s"),
			*OldDirection.ToString(), *ScrollDirection.ToString());
	}
}

bool UWorldScrollComponent::LoadConfigFromDataTable()
{
	// Check if data table is set
	if (!ScrollDataTable)
	{
		UE_LOG(LogWorldScroll, Warning, TEXT("WorldScrollComponent: No data table assigned"));
		return false;
	}

	// Find the row
	static const FString ContextString(TEXT("WorldScrollComponent::LoadConfigFromDataTable"));
	FWorldScrollData* RowData = ScrollDataTable->FindRow<FWorldScrollData>(DataTableRowName, ContextString);

	if (!RowData)
	{
		UE_LOG(LogWorldScroll, Error, TEXT("WorldScrollComponent: Failed to find row '%s' in data table"),
			*DataTableRowName.ToString());
		return false;
	}

	// Load values from data table
	ScrollSpeed = ValidateScrollSpeed(RowData->ScrollSpeed);
	bIsScrolling = RowData->bScrollEnabled;
	ScrollDirection = ValidateScrollDirection(RowData->ScrollDirection);

	UE_LOG(LogWorldScroll, Log, TEXT("WorldScrollComponent: Loaded config from data table row '%s'"),
		*DataTableRowName.ToString());

	return true;
}

float UWorldScrollComponent::ValidateScrollSpeed(float Speed) const
{
	// Speed cannot be negative
	if (Speed < 0.0f)
	{
		UE_LOG(LogWorldScroll, Warning, TEXT("WorldScrollComponent: Negative speed %.2f clamped to 0.0"),
			Speed);
		return 0.0f;
	}

	// Sanity check for extremely large values
	const float MaxReasonableSpeed = 100000.0f; // 1km/s is probably a bug
	if (Speed > MaxReasonableSpeed)
	{
		UE_LOG(LogWorldScroll, Warning, TEXT("WorldScrollComponent: Speed %.2f exceeds maximum %.2f, clamped"),
			Speed, MaxReasonableSpeed);
		return MaxReasonableSpeed;
	}

	return Speed;
}

FVector UWorldScrollComponent::ValidateScrollDirection(FVector Direction) const
{
	// Check for zero vector
	if (Direction.IsNearlyZero())
	{
		UE_LOG(LogWorldScroll, Warning, TEXT("WorldScrollComponent: Zero direction vector, using default (-1, 0, 0)"));
		return FVector(-1.0f, 0.0f, 0.0f);
	}

	// Normalize the direction
	FVector Normalized = Direction.GetSafeNormal();

	// Verify normalization succeeded
	if (Normalized.IsNearlyZero())
	{
		UE_LOG(LogWorldScroll, Error, TEXT("WorldScrollComponent: Failed to normalize direction %s, using default"),
			*Direction.ToString());
		return FVector(-1.0f, 0.0f, 0.0f);
	}

	return Normalized;
}

void UWorldScrollComponent::LogScrollState() const
{
	UE_LOG(LogWorldScroll, Log, TEXT("=== World Scroll State ==="));
	UE_LOG(LogWorldScroll, Log, TEXT("Speed: %.2f units/second"), ScrollSpeed);
	UE_LOG(LogWorldScroll, Log, TEXT("Direction: %s"), *ScrollDirection.ToString());
	UE_LOG(LogWorldScroll, Log, TEXT("Velocity: %s"), *GetScrollVelocity().ToString());
	UE_LOG(LogWorldScroll, Log, TEXT("Is Scrolling: %s"), bIsScrolling ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogWorldScroll, Log, TEXT("Distance Traveled: %.2f"), DistanceTraveled);
	UE_LOG(LogWorldScroll, Log, TEXT("========================"));
}
