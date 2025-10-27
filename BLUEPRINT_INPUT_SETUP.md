# Blueprint Input Setup Guide

## IMPORTANT: Use the Enhanced Input Guide

**For the proper, official Enhanced Input setup, see: [ENHANCED_INPUT_SETUP_GUIDE.md](ENHANCED_INPUT_SETUP_GUIDE.md)**

This file is kept for reference but contains outdated/incorrect information about Blueprint node names.

---

## Why Blueprint Input? (Historical Context)

After extensive troubleshooting with C++ input systems (both Enhanced Input and Legacy Input), we discovered that the Unreal Engine 5.6 editor automatically forces Enhanced Input on, even when configured for legacy input. This causes C++ input bindings to fail.

**Blueprint Enhanced Input is the reliable solution** because:
- The editor doesn't fight or override Blueprint input
- It's the standard approach for UE5.6
- Changes don't require recompilation
- Input events are visual and easy to verify

## Prerequisites

The C++ code is already set up - `RequestLaneChange` is BlueprintCallable in `WarRigPawn.h`. You just need to wire up the Blueprint input events.

**→ Go to [ENHANCED_INPUT_SETUP_GUIDE.md](ENHANCED_INPUT_SETUP_GUIDE.md) for the correct, detailed instructions.**

## Setup Instructions

### Step 1: Open BP_WarRigPlayerController Blueprint

1. In the Content Browser, navigate to `Content/Core/`
2. Open `BP_WarRigPlayerController` (or create it if it doesn't exist yet)
3. If creating new: Parent Class should be `WarRigPlayerController` (the C++ class)

### Step 2: Add Input Action Events

1. In the Event Graph, **right-click** and search for one of these:
   - **Enhanced Input**: Search for "Enhanced Input Action" and select your Input Actions
   - **Legacy Input**: Search for "A" or "D" and add "A Key" and "D Key" events
   - **Action Mappings**: Search for "MoveLeft" and "MoveRight" if you have them configured

**Recommended: Use simple Key Events (most reliable)**

Add these four input events to the graph:
- **A Key** (IE Pressed)
- **D Key** (IE Pressed)
- **Left Arrow Key** (IE Pressed)
- **Right Arrow Key** (IE Pressed)

### Step 3: Wire Up Lane Change Calls

For each LEFT input (A Key, Left Arrow Key):

1. From the input event node, drag out and search for **"Get Controlled Pawn"**
2. From the Get Controlled Pawn output, drag and **Cast to WarRigPawn**
3. From the WarRigPawn cast output, drag and search for **"Request Lane Change"**
4. Set the **Direction** input to **-1** (negative for left)

For each RIGHT input (D Key, Right Arrow Key):

1. From the input event node, drag out and search for **"Get Controlled Pawn"**
2. From the Get Controlled Pawn output, drag and **Cast to WarRigPawn**
3. From the WarRigPawn cast output, drag and search for **"Request Lane Change"**
4. Set the **Direction** input to **1** (positive for right)

### Step 4: Compile and Save

1. Click **Compile** button (top toolbar)
2. Fix any errors if they appear
3. Click **Save**
4. Close the Blueprint

### Visual Example

Your Blueprint graph should look like this:

```
[A Key (Pressed)] --> [Get Controlled Pawn] --> [Cast to WarRigPawn] --> [Request Lane Change (Direction: -1)]

[D Key (Pressed)] --> [Get Controlled Pawn] --> [Cast to WarRigPawn] --> [Request Lane Change (Direction: 1)]

[Left Arrow (Pressed)] --> [Get Controlled Pawn] --> [Cast to WarRigPawn] --> [Request Lane Change (Direction: -1)]

[Right Arrow (Pressed)] --> [Get Controlled Pawn] --> [Cast to WarRigPawn] --> [Request Lane Change (Direction: 1)]
```

### Step 5: Configure Game Mode

Ensure your Game Mode is set up correctly:

1. Open your Game Mode Blueprint (likely `BP_WhitelineNightmareGameMode`)
2. In **Class Defaults**, set:
   - **Player Controller Class** → `BP_WarRigPlayerController`
   - **Default Pawn Class** → `BP_WarRigPawn` (or your war rig blueprint)
3. Compile and Save

### Step 6: Test

1. Click **Play** in the editor (PIE)
2. Click in the game viewport to give it focus
3. Press **A** or **Left Arrow** → War rig should move one lane LEFT
4. Press **D** or **Right Arrow** → War rig should move one lane RIGHT

## Expected Behavior

- Pressing A/D or Arrow keys should trigger immediate lane changes
- One lane change per key press (not continuous)
- Smooth interpolation between lanes
- No logs about "input not bound" or similar errors

## Troubleshooting

### If input still doesn't work:

1. **Verify viewport has focus**: Click in the game viewport during PIE
2. **Check Player Controller is possessed**: Look for "WarRigPlayerController: Possessed pawn" in the Output Log
3. **Verify Blueprint is being used**: In PIE, open World Outliner and check that the Player Controller is your Blueprint class, not the C++ class directly
4. **Check cast is succeeding**: Add a Print String node after the Cast to verify it's working
5. **Verify pawn exists**: Add a Branch after Get Controlled Pawn with an "Is Valid" check

### If you want to use Enhanced Input instead:

Enhanced Input works but requires more setup. If you prefer it:

1. Create Input Actions: `IA_MoveLeft` and `IA_MoveRight`
2. Create Input Mapping Context: `IMC_WarRig`
3. Map keys to actions in the IMC
4. In Blueprint Event BeginPlay, get the Enhanced Input Local Player Subsystem
5. Call "Add Mapping Context" with IMC_WarRig and priority 0
6. Use "Enhanced Input Action" events in the Event Graph instead of Key events
7. Wire them up the same way (Cast to WarRigPawn → Request Lane Change)

## Notes

- This approach completely bypasses C++ input handling
- The C++ controller still handles everything else (scrap, game over, debug commands)
- `RequestLaneChange` is a C++ function - we're just calling it from Blueprint
- You can add visual feedback, sound effects, etc. in the Blueprint before calling RequestLaneChange

## Alternative: Handle Input in BP_WarRigPawn

If you prefer, you can handle input in the **Pawn Blueprint** instead of the Controller:

1. Open `BP_WarRigPawn`
2. Add the same key events (A, D, Left, Right)
3. From each event, directly call **Request Lane Change** (no need to cast since you're already the WarRigPawn)
4. This is actually simpler and more direct!

## Success!

Once set up, you'll have working lane-changing input that won't be overridden by the editor. The Blueprint approach is reliable and easy to modify without recompiling C++.
