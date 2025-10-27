# Troubleshooting Lane Changing Input

## RECOMMENDED SOLUTION: Blueprint Enhanced Input

After extensive troubleshooting, we've determined that **Blueprint Enhanced Input is the correct approach** for UE5.6. The editor automatically forces Enhanced Input on, which causes C++ input bindings to fail.

**→ See [ENHANCED_INPUT_SETUP_GUIDE.md](ENHANCED_INPUT_SETUP_GUIDE.md) for complete step-by-step instructions**

This is the official UE5.6 approach:
- Uses Input Actions (IA_MoveLeft, IA_MoveRight)
- Uses Input Mapping Context (IMC_WarRig)
- Binds Input Action events in PlayerController Blueprint
- Simple, doesn't require recompilation, and won't be overridden by the editor

---

## Legacy C++ Input Troubleshooting (For Reference)

### Issue
Lane changing (A/D or Left/Right arrow keys) not working with C++ input system.

### Root Cause
UE5.6 editor automatically upgrades input classes to Enhanced Input, even when configured for legacy input in DefaultInput.ini. This causes C++ BindAction calls to fail.

## Diagnostic Steps (Legacy C++)

### 1. Check PIE Output Log

When you start Play In Editor (PIE), look for these log messages:

**Expected logs on startup:**
```
WarRigPlayerController: Initialized with X starting scrap
SetupInputComponent: Bound MoveLeft action
SetupInputComponent: Bound MoveRight action
SetupInputComponent: Enhanced Input bindings complete
WarRigPlayerController: Possessed pawn BP_WarRigPawn_C_X
WarRigPlayerController: Set input mode to Game Only
```

**When pressing A or D:**
```
OnMoveLeft: Requesting lane change left
OnMoveLeft: Lane change LEFT successful
```

### 2. If you see "MoveLeftAction is null" or "MoveRightAction is null"

This means the Input Actions aren't assigned in the Blueprint.

**Fix:**
1. Open `Content/Core/BP_WarRigPlayerController` (or your controller blueprint)
2. In the Details panel, find **Input** section
3. Set:
   - **Input Mapping Context** → `IMC_WarRig`
   - **Move Left Action** → `IA_MoveLeft`
   - **Move Right Action** → `IA_MoveRight`
4. Save and test

### 3. If you see "No War Rig pawn possessed"

The controller doesn't have a pawn to control.

**Fix:**
1. Open your **Game Mode Blueprint** (likely `BP_WhitelineNightmareGameMode`)
2. Set **Default Pawn Class** → `BP_WarRigPawn` (or your war rig pawn blueprint)
3. Ensure **Player Controller Class** → `BP_WarRigPlayerController`
4. Save and test

### 4. If no logs appear when pressing keys

The input isn't reaching the game.

**Fix:**
1. Click in the viewport during PIE to ensure it has focus
2. Check Project Settings → Input:
   - **Default Player Input Class** = `EnhancedPlayerInput`
   - **Default Input Component Class** = `EnhancedInputComponent`
3. If changed, **restart the editor**

### 5. Test Direct Input

Type this in the PIE console to test input system:
```
DebugListInputContexts
```

This shows if Enhanced Input is configured correctly.

Then type:
```
DebugShowKeyMappings
```

This shows all key mappings in IMC_WarRig. You should see:
- MoveLeft + A Key: FOUND
- MoveLeft + Left Arrow: FOUND
- MoveRight + D Key: FOUND
- MoveRight + Right Arrow: FOUND

## Quick Setup Checklist

- [ ] Game Mode has Default Pawn Class set to BP_WarRigPawn
- [ ] Game Mode has Player Controller Class set to BP_WarRigPlayerController
- [ ] BP_WarRigPlayerController has IMC_WarRig assigned
- [ ] BP_WarRigPlayerController has IA_MoveLeft assigned
- [ ] BP_WarRigPlayerController has IA_MoveRight assigned
- [ ] Project Settings → Input configured for Enhanced Input
- [ ] Clicked in viewport to give it focus

## Expected Behavior After Fix

- Press **A** or **Left Arrow** → War rig moves one lane left
- Press **D** or **Right Arrow** → War rig moves one lane right
- One lane change per key press (not continuous while held)
- Smooth interpolation between lanes
