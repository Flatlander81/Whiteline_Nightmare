// Copyright Flatlander81. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Turrets/TurretBase.h"
#include "TestTurret.generated.h"

/**
 * ATestTurret - Concrete turret class for unit testing
 *
 * This is a minimal implementation of ATurretBase used exclusively for testing.
 * It provides no additional functionality beyond the base class - just makes
 * the abstract class instantiable for test purposes.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class WHITELINENIGHTMARE_API ATestTurret : public ATurretBase
{
	GENERATED_BODY()

public:
	ATestTurret();

	// Override Fire() for testing purposes (just call base implementation)
	virtual void Fire() override;
};
