// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/InputConfiguration.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableInputConfig.h"
#include "Engine/World.h"

// Static member initialization
TObjectPtr<UInputMappingContext> UInputConfiguration::CachedMappingContext = nullptr;
TObjectPtr<UInputAction> UInputConfiguration::CachedLaneChangeAction = nullptr;
TObjectPtr<UInputAction> UInputConfiguration::CachedPauseAction = nullptr;

void UInputConfiguration::SetupInputSystem(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return;
	}

	// Create all input objects if they don't exist
	if (!CachedMappingContext)
	{
		CachedMappingContext = CreateDefaultMappingContext(WorldContextObject);
	}

	if (!CachedLaneChangeAction)
	{
		CachedLaneChangeAction = CreateLaneChangeAction(WorldContextObject);
	}

	if (!CachedPauseAction)
	{
		CachedPauseAction = CreatePauseAction(WorldContextObject);
	}

	UE_LOG(LogTemp, Log, TEXT("Input system configured programmatically"));
}

UInputMappingContext* UInputConfiguration::GetDefaultMappingContext(UObject* WorldContextObject)
{
	if (!CachedMappingContext && WorldContextObject)
	{
		SetupInputSystem(WorldContextObject);
	}

	return CachedMappingContext;
}

UInputAction* UInputConfiguration::GetLaneChangeAction(UObject* WorldContextObject)
{
	if (!CachedLaneChangeAction && WorldContextObject)
	{
		SetupInputSystem(WorldContextObject);
	}

	return CachedLaneChangeAction;
}

UInputAction* UInputConfiguration::GetPauseAction(UObject* WorldContextObject)
{
	if (!CachedPauseAction && WorldContextObject)
	{
		SetupInputSystem(WorldContextObject);
	}

	return CachedPauseAction;
}

UInputMappingContext* UInputConfiguration::CreateDefaultMappingContext(UObject* Outer)
{
	UInputMappingContext* MappingContext = NewObject<UInputMappingContext>(Outer, UInputMappingContext::StaticClass(), TEXT("DefaultMappingContext"));

	if (!MappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create InputMappingContext"));
		return nullptr;
	}

	// Get or create input actions
	UInputAction* LaneChangeAction = CachedLaneChangeAction ? CachedLaneChangeAction : CreateLaneChangeAction(Outer);
	UInputAction* PauseAction = CachedPauseAction ? CachedPauseAction : CreatePauseAction(Outer);

	// Map keys to actions
	if (LaneChangeAction)
	{
		// Keyboard: A/D for lane changes
		FEnhancedActionKeyMapping& MappingA = MappingContext->MapKey(LaneChangeAction, EKeys::A);
		MappingA.Player = nullptr;

		FEnhancedActionKeyMapping& MappingD = MappingContext->MapKey(LaneChangeAction, EKeys::D);
		MappingD.Player = nullptr;

		// Arrow keys: Left/Right
		FEnhancedActionKeyMapping& MappingLeft = MappingContext->MapKey(LaneChangeAction, EKeys::Left);
		MappingLeft.Player = nullptr;

		FEnhancedActionKeyMapping& MappingRight = MappingContext->MapKey(LaneChangeAction, EKeys::Right);
		MappingRight.Player = nullptr;

		// Gamepad: Left stick horizontal axis
		FEnhancedActionKeyMapping& MappingGamepad = MappingContext->MapKey(LaneChangeAction, EKeys::Gamepad_LeftX);
		MappingGamepad.Player = nullptr;
	}

	if (PauseAction)
	{
		// Keyboard: Escape key
		FEnhancedActionKeyMapping& MappingEsc = MappingContext->MapKey(PauseAction, EKeys::Escape);
		MappingEsc.Player = nullptr;

		// Keyboard: P key
		FEnhancedActionKeyMapping& MappingP = MappingContext->MapKey(PauseAction, EKeys::P);
		MappingP.Player = nullptr;

		// Gamepad: Start button
		FEnhancedActionKeyMapping& MappingGamepad = MappingContext->MapKey(PauseAction, EKeys::Gamepad_Special_Right);
		MappingGamepad.Player = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Created default mapping context with %d mappings"), MappingContext->GetMappings().Num());

	return MappingContext;
}

UInputAction* UInputConfiguration::CreateLaneChangeAction(UObject* Outer)
{
	UInputAction* InputAction = NewObject<UInputAction>(Outer, UInputAction::StaticClass(), TEXT("IA_LaneChange"));

	if (!InputAction)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create LaneChange InputAction"));
		return nullptr;
	}

	// Configure as 1D axis (left/right)
	InputAction->ValueType = EInputActionValueType::Axis1D;

	UE_LOG(LogTemp, Log, TEXT("Created LaneChange input action"));

	return InputAction;
}

UInputAction* UInputConfiguration::CreatePauseAction(UObject* Outer)
{
	UInputAction* InputAction = NewObject<UInputAction>(Outer, UInputAction::StaticClass(), TEXT("IA_Pause"));

	if (!InputAction)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create Pause InputAction"));
		return nullptr;
	}

	// Configure as digital (pressed/released)
	InputAction->ValueType = EInputActionValueType::Boolean;

	UE_LOG(LogTemp, Log, TEXT("Created Pause input action"));

	return InputAction;
}
