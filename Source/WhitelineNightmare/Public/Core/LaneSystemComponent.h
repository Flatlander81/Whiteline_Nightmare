// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LaneSystemComponent.generated.h"

/**
 * ULaneSystemComponent - Handles lateral lane-based movement for the war rig
 *
 * The war rig is stationary in forward/backward movement but can move laterally
 * between discrete lanes. This component manages lane changes, lane positions,
 * and lane change costs/speeds.
 *
 * NOTE: This is a stub implementation. Full functionality will be implemented
 * in a future task.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WHITELINENIGHTMARE_API ULaneSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULaneSystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Lane management functions (to be implemented)

	/** Get the current lane index (0 = center) */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	int32 GetCurrentLane() const { return CurrentLane; }

	/** Attempt to change to a specific lane */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool ChangeLane(int32 TargetLane);

	/** Move one lane to the left */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool MoveLaneLeft();

	/** Move one lane to the right */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool MoveLaneRight();

	/** Check if currently changing lanes */
	UFUNCTION(BlueprintCallable, Category = "Lane System")
	bool IsChangingLanes() const { return bIsChangingLanes; }

protected:
	/** Current lane index (0 = center, negative = left, positive = right) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System")
	int32 CurrentLane;

	/** Target lane when changing lanes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System")
	int32 TargetLane;

	/** Whether currently changing lanes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lane System")
	bool bIsChangingLanes;

	/** Total number of lanes available (center + left/right) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	int32 TotalLanes;

	/** Distance between lanes (units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	float LaneWidth;

	/** Speed of lane change movement (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lane System")
	float LaneChangeSpeed;
};
