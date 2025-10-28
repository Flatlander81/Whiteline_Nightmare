// Copyright Flatlander81. All Rights Reserved.

#include "Core/LaneSystemComponent.h"

ULaneSystemComponent::ULaneSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Default values
	CurrentLane = 0;
	TargetLane = 0;
	bIsChangingLanes = false;
	TotalLanes = 5; // Center + 2 left + 2 right
	LaneWidth = 400.0f;
	LaneChangeSpeed = 500.0f;
}

void ULaneSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Start in the center lane
	CurrentLane = 0;
	TargetLane = 0;
	bIsChangingLanes = false;

	UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::BeginPlay - Initialized at center lane"));
}

void ULaneSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Lane change movement logic will be implemented in future task
	if (bIsChangingLanes)
	{
		// TODO: Implement smooth lane transition
	}
}

bool ULaneSystemComponent::ChangeLane(int32 TargetLaneIndex)
{
	// Basic validation
	int32 MaxLaneOffset = TotalLanes / 2;
	if (TargetLaneIndex < -MaxLaneOffset || TargetLaneIndex > MaxLaneOffset)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLane - Invalid lane index: %d"), TargetLaneIndex);
		return false;
	}

	if (bIsChangingLanes)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULaneSystemComponent::ChangeLane - Already changing lanes"));
		return false;
	}

	// TODO: Implement lane change logic
	UE_LOG(LogTemp, Log, TEXT("ULaneSystemComponent::ChangeLane - Lane change requested from %d to %d (NOT IMPLEMENTED)"),
		CurrentLane, TargetLaneIndex);

	return false;
}

bool ULaneSystemComponent::MoveLaneLeft()
{
	return ChangeLane(CurrentLane - 1);
}

bool ULaneSystemComponent::MoveLaneRight()
{
	return ChangeLane(CurrentLane + 1);
}
