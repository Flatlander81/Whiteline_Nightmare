# Enhanced Input Setup Guide - Lane Changing System

This guide provides step-by-step instructions for setting up Enhanced Input in Unreal Engine 5.6 for the War Rig lane-changing system.

## System Overview

- **C++ Pawn Class**: `AWarRigPawn` (inherits from APawn)
- **C++ Controller Class**: `AWarRigPlayerController` (inherits from APlayerController)
- **C++ Function**: `bool RequestLaneChange(int32 Direction)` - already marked `UFUNCTION(BlueprintCallable)`
  - Direction: `-1` for left, `1` for right
- **Keys**: A/Left Arrow for left, D/Right Arrow for right

## Prerequisites

- Ensure your project is using Enhanced Input (UE 5.6 defaults to this)
- Your PlayerController Blueprint should inherit from `AWarRigPlayerController`
- Your Pawn Blueprint should inherit from `AWarRigPawn`

---

## Step 1: Create Input Actions

### Create IA_MoveLeft

1. Open **Content Browser**
2. Navigate to `Content/Input/` (create this folder if it doesn't exist)
3. Right-click in the folder → **Input** → **Input Action**
4. Name it: `IA_MoveLeft`
5. **Double-click** to open it
6. Set **Value Type**: `Digital (bool)` (this is for simple on/off button presses)
7. Leave **Triggers** and **Modifiers** empty (default settings are fine)
8. **Save** and close

### Create IA_MoveRight

1. In the same `Content/Input/` folder
2. Right-click → **Input** → **Input Action**
3. Name it: `IA_MoveRight`
4. **Double-click** to open it
5. Set **Value Type**: `Digital (bool)`
6. Leave **Triggers** and **Modifiers** empty
7. **Save** and close

---

## Step 2: Create Input Mapping Context

### Create IMC_WarRig

1. In **Content Browser**, navigate to `Content/Input/`
2. Right-click → **Input** → **Input Mapping Context**
3. Name it: `IMC_WarRig`
4. **Double-click** to open it

### Add Key Mappings

#### Mapping 1: Move Left

1. Click **+ Mappings** button
2. In the dropdown, select **IA_MoveLeft**
3. Click the **+** button next to IA_MoveLeft to add a key binding
4. Click the key field and press **A** key on your keyboard
5. Click the **+** button again to add another binding
6. Click the key field and press **Left Arrow** key

#### Mapping 2: Move Right

1. Click **+ Mappings** button again
2. In the dropdown, select **IA_MoveRight**
3. Click the **+** button next to IA_MoveRight to add a key binding
4. Click the key field and press **D** key on your keyboard
5. Click the **+** button again to add another binding
6. Click the key field and press **Right Arrow** key

Your IMC should now show:
```
IA_MoveLeft
  - A
  - Left

IA_MoveRight
  - D
  - Right
```

7. **Save** and close

---

## Step 3: Setup Input in PlayerController Blueprint

### Open BP_WarRigPlayerController

1. In **Content Browser**, navigate to where your PlayerController Blueprint is located (likely `Content/Core/`)
2. If it doesn't exist, create it:
   - Right-click → **Blueprint Class**
   - Search for and select **WarRigPlayerController** as parent class
   - Name it: `BP_WarRigPlayerController`
3. **Double-click** to open the Blueprint

### Add Mapping Context in BeginPlay

1. In the **Event Graph**, find the **Event BeginPlay** node (or create it if missing)
2. From Event BeginPlay's execution pin, drag out and search: **"Add Mapping Context"**
3. You'll need to get the Enhanced Input Subsystem first:
   - From Event BeginPlay, drag out and search: **"Get Player Controller"**
   - From Get Player Controller, drag out and search: **"Get Local Player"**
   - From Get Local Player, drag out and search: **"Get Subsystem"** (Enhanced Input Local Player Subsystem)
   - In the Get Subsystem node, select **Class**: `EnhancedInputLocalPlayerSubsystem`
4. Connect the subsystem output to **Target** pin of "Add Mapping Context" node
5. In the **Add Mapping Context** node:
   - **Mapping Context**: Select `IMC_WarRig` from the dropdown
   - **Priority**: Set to `0` (or `1` if you have other contexts)
6. Connect execution pin from Event BeginPlay → Add Mapping Context

### Setup Input Binding Event

Since we need to bind input actions dynamically, we'll use the **ReceiveBeginPlay** or create custom events for each input action that the Enhanced Input system will trigger.

**Alternative Simpler Approach**: Use Input Action events directly in Event Graph:

1. **Right-click** in the Event Graph
2. Search for: **"IA_MoveLeft"**
3. Select **"Input Action IA_MoveLeft"** with event type **"Triggered"**
4. This creates an event node that fires when IA_MoveLeft is triggered

Repeat for IA_MoveRight:
1. **Right-click** in Event Graph
2. Search for: **"IA_MoveRight"**
3. Select **"Input Action IA_MoveRight"** with event type **"Triggered"**

### Wire Up Lane Change Logic

#### For IA_MoveLeft (Triggered) Event:

1. From the **execution pin** of "InputAction IA_MoveLeft (Triggered)", drag out and search: **"Get Pawn"**
   - Note: This might appear as "Get Controlled Pawn" or "Get Pawn" depending on context
2. From the **return value** of Get Pawn, drag out and search: **"Cast to WarRigPawn"**
3. From the **As War Rig Pawn** output pin of the Cast node, drag out and search: **"Request Lane Change"**
4. In the Request Lane Change node:
   - Set **Direction** input to: **-1** (negative for left)
5. Connect the execution flow: InputAction → Cast → Request Lane Change

#### For IA_MoveRight (Triggered) Event:

1. From the **execution pin** of "InputAction IA_MoveRight (Triggered)", drag out and search: **"Get Pawn"**
2. From the **return value** of Get Pawn, drag out and search: **"Cast to WarRigPawn"**
3. From the **As War Rig Pawn** output pin, drag out and search: **"Request Lane Change"**
4. In the Request Lane Change node:
   - Set **Direction** input to: **1** (positive for right)
5. Connect the execution flow: InputAction → Cast → Request Lane Change

### Optional: Add Debug Output

To verify input is working, add **Print String** nodes after each Input Action event:

1. From InputAction IA_MoveLeft execution pin, add a **Print String** node
2. Set text to: `"Move Left Input Received"`
3. Connect execution to the Cast node

Repeat for IA_MoveRight with text: `"Move Right Input Received"`

### Compile and Save

1. Click **Compile** button (top toolbar)
2. Fix any errors if they appear
3. Click **Save**
4. Close Blueprint

---

## Step 4: Configure Game Mode

Ensure your Game Mode uses the correct PlayerController:

1. Open your Game Mode Blueprint (likely `BP_WhitelineNightmareGameMode` in `Content/Core/`)
2. In **Class Defaults** panel:
   - **Player Controller Class** → `BP_WarRigPlayerController`
   - **Default Pawn Class** → Your War Rig pawn Blueprint
3. **Compile** and **Save**

### Set Game Mode in Project Settings

1. **Edit** → **Project Settings**
2. Navigate to **Project** → **Maps & Modes**
3. Set **Default GameMode** → Your Game Mode Blueprint
4. Close Project Settings

---

## Step 5: Test

1. Click **Play** (PIE) in the editor
2. **Click inside the game viewport** to ensure it has focus
3. Press **A** or **Left Arrow** → War Rig should move one lane LEFT
4. Press **D** or **Right Arrow** → War Rig should move one lane RIGHT

### Expected Behavior

- One lane change per key press (not continuous)
- Smooth interpolation between lanes
- If you added Print String nodes, you should see "Move Left/Right Input Received" messages

---

## Troubleshooting

### Input Not Working

**Check 1: Viewport Focus**
- Click in the game viewport during PIE to ensure it has keyboard focus
- Look for a white border around the viewport window

**Check 2: Verify Mapping Context Added**
- Add a Print String after "Add Mapping Context" in BeginPlay
- Check Output Log for the message when game starts

**Check 3: Verify Controller is Possessed**
- In Output Log, look for: `"WarRigPlayerController: Possessed pawn"`
- If missing, check GameMode settings

**Check 4: Verify Input Action Events Fire**
- Add Print String nodes after each Input Action event
- Press keys and check Output Log

**Check 5: Verify Cast Succeeds**
- Right-click on the Cast node → **Add Cast Failed Execution Pin**
- Add a Print String to the Cast Failed pin: "Cast to WarRigPawn FAILED"
- If this fires, your pawn is not a WarRigPawn or is null

**Check 6: Verify Blueprint Class is Used**
- During PIE, open **World Outliner**
- Find the PlayerController
- Verify it's `BP_WarRigPlayerController`, not just `WarRigPlayerController`
- If it's the C++ class, check your GameMode settings

### Input Fires But Lane Doesn't Change

**Check 1: RequestLaneChange Return Value**
- From the Request Lane Change node, drag out the **Return Value** (boolean)
- Add a **Branch** node
- On **False** branch, add Print String: "Lane change blocked"
- This means the pawn is already transitioning or at lane boundary

**Check 2: Check LaneSystemComponent**
- Verify your WarRigPawn has a LaneSystemComponent
- Check that it's initialized properly in BeginPlay

### Multiple Lane Changes Per Press

This shouldn't happen with **Triggered** event type, but if it does:
- Verify you're using **Triggered**, not **Started** or **Ongoing**
- Started fires multiple times (unreliable)
- Triggered should fire once per press when using Digital (bool) value type

---

## Blueprint Graph Visual Reference

Your Event Graph should look like this:

```
[Event BeginPlay]
    → [Get Player Controller]
        → [Get Local Player]
            → [Get Subsystem (EnhancedInputLocalPlayerSubsystem)]
                → [Add Mapping Context]
                    - Mapping Context: IMC_WarRig
                    - Priority: 0

[InputAction IA_MoveLeft (Triggered)]
    → [Get Pawn]
        → [Cast to WarRigPawn]
            → [Request Lane Change]
                - Direction: -1

[InputAction IA_MoveRight (Triggered)]
    → [Get Pawn]
        → [Cast to WarRigPawn]
            → [Request Lane Change]
                - Direction: 1
```

---

## Why This Approach Works

- **Enhanced Input is the standard** in UE5.6
- **Blueprint binding is reliable** - editor won't override it
- **PlayerController handles input** - keeps separation of concerns
- **Get Pawn in Controller context** returns the controlled pawn
- **Triggered event** fires once per key press (perfect for discrete actions)
- **Digital (bool) Value Type** is correct for button presses

---

## Alternative: Handle Input in Pawn Blueprint

If you prefer to handle input in the Pawn instead of Controller:

1. Open `BP_WarRigPawn` Blueprint
2. Add the mapping context in **Event BeginPlay** (same process as above)
3. Add the same **Input Action** events (IA_MoveLeft, IA_MoveRight)
4. From each event, **directly call Request Lane Change** on Self
   - No need to Get Pawn or Cast - you're already the WarRigPawn!
5. Set Direction: -1 for left, 1 for right

**Pros**: Simpler, no casting needed
**Cons**: Input logic tied to specific pawn, less flexible if you want to control multiple pawn types

---

## Summary Checklist

- [ ] Created IA_MoveLeft Input Action (Digital bool)
- [ ] Created IA_MoveRight Input Action (Digital bool)
- [ ] Created IMC_WarRig Input Mapping Context
- [ ] Mapped A and Left Arrow to IA_MoveLeft
- [ ] Mapped D and Right Arrow to IA_MoveRight
- [ ] Created/Opened BP_WarRigPlayerController Blueprint
- [ ] Added mapping context in BeginPlay
- [ ] Added InputAction IA_MoveLeft (Triggered) event
- [ ] Added InputAction IA_MoveRight (Triggered) event
- [ ] Wired up Get Pawn → Cast to WarRigPawn → Request Lane Change
- [ ] Set Direction to -1 for left, 1 for right
- [ ] Compiled and saved Blueprint
- [ ] Set BP_WarRigPlayerController in GameMode
- [ ] Tested in PIE - lane changing works!

---

## Need Help?

- Check the Output Log for error messages
- Use Print String nodes liberally to debug execution flow
- Verify all Blueprint references are set (no "None" values)
- Ensure you clicked Compile after making Blueprint changes
- Check that RequestLaneChange is actually being called by adding a log in the C++ function

Good luck! Once set up correctly, this system should work reliably.
