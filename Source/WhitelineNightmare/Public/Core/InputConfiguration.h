// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputConfiguration.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * Input Configuration
 * Manages programmatic creation and setup of Enhanced Input actions and mapping contexts
 * This allows input to be configured in C++ without needing editor assets
 */
UCLASS()
class WHITELINENIGHTMARE_API UInputConfiguration : public UObject
{
	GENERATED_BODY()

public:
	/** Create and configure all input actions and mapping contexts */
	static void SetupInputSystem(UObject* WorldContextObject);

	/** Get the default input mapping context */
	UFUNCTION(BlueprintPure, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static UInputMappingContext* GetDefaultMappingContext(UObject* WorldContextObject);

	/** Get lane change input action */
	UFUNCTION(BlueprintPure, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static UInputAction* GetLaneChangeAction(UObject* WorldContextObject);

	/** Get pause input action */
	UFUNCTION(BlueprintPure, Category = "Input", meta = (WorldContext = "WorldContextObject"))
	static UInputAction* GetPauseAction(UObject* WorldContextObject);

private:
	/** Cached input mapping context */
	static TObjectPtr<UInputMappingContext> CachedMappingContext;

	/** Cached input actions */
	static TObjectPtr<UInputAction> CachedLaneChangeAction;
	static TObjectPtr<UInputAction> CachedPauseAction;

	/** Create the default mapping context */
	static UInputMappingContext* CreateDefaultMappingContext(UObject* Outer);

	/** Create input actions */
	static UInputAction* CreateLaneChangeAction(UObject* Outer);
	static UInputAction* CreatePauseAction(UObject* Outer);
};
