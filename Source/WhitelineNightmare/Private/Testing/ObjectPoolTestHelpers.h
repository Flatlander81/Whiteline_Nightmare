// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ObjectPoolTypes.h"
#include "ObjectPoolTestHelpers.generated.h"

/**
 * Test actor that implements IPoolableActor for testing purposes
 */
UCLASS()
class ATestPoolableActor : public AActor, public IPoolableActor
{
	GENERATED_BODY()

public:
	ATestPoolableActor()
	{
		ActivationCount = 0;
		DeactivationCount = 0;
		ResetCount = 0;
	}

	// IPoolableActor interface
	virtual void OnActivated_Implementation() override
	{
		ActivationCount++;
	}

	virtual void OnDeactivated_Implementation() override
	{
		DeactivationCount++;
	}

	virtual void ResetState_Implementation() override
	{
		ResetCount++;
		ActivationCount = 0;
		DeactivationCount = 0;
	}

	int32 ActivationCount;
	int32 DeactivationCount;
	int32 ResetCount;
};
