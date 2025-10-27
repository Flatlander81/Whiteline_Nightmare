# War Rig and Scrolling World System Setup

This document provides instructions for setting up the War Rig pawn and scrolling world system in Unreal Engine.

## Overview

The war rig system implements:
- **Stationary war rig** - The rig stays at the origin while the world scrolls backward
- **Lane-based movement** - 5 fixed lanes with smooth transitions
- **Scrolling world** - Ground tiles pool and recycle for infinite scrolling
- **Data-driven configuration** - All war rig properties defined in data tables

## Architecture

### Core Components

1. **AWarRigPawn** - The player-controlled pawn
   - `ULaneSystemComponent` - Manages lane changes
   - `USpringArmComponent` + `UCameraComponent` - Top-down camera view
   - Mesh sections array - Visual representation (cab + trailers)

2. **UWorldScrollComponent** - Manages world scrolling
   - Tracks scroll speed and distance traveled
   - Provides velocity to other systems

3. **AGroundTile** - Poolable ground tile actor
   - Implements `IPoolableActor`
   - Scrolls backward at world scroll speed

4. **UGroundTilePoolComponent** - Specialized pool for ground tiles
   - Extends `UObjectPoolComponent`
   - Automatically recycles tiles

## Data Table Setup

### 1. Create WarRigData Data Table

**In the Unreal Editor:**

1. Navigate to `Content/DataTables/` (create folder if needed)
2. Right-click → Miscellaneous → Data Table
3. Select `FWarRigData` as the row structure
4. Name it `DT_WarRigData`

### 2. Configure "SemiTruck" Row

Add a row with the name **"SemiTruck"** and configure:

#### Basic Info
- **Display Name**: "Semi Truck"
- **Description**: "Classic 18-wheeler war rig with 3 sections"

#### Rig Sections
Add 3 sections to the `RigSections` array:

**Section 0 (Cab):**
- Section Mesh: `/Engine/BasicShapes/Cube` (or leave empty to use default)
- Section Size: `X=200, Y=150, Z=100`
- Section Color: `Red (R=1.0, G=0.0, B=0.0)`

**Section 1 (Trailer 1):**
- Section Mesh: `/Engine/BasicShapes/Cube`
- Section Size: `X=200, Y=150, Z=80`
- Section Color: `Dark Grey (R=0.2, G=0.2, B=0.2)`

**Section 2 (Trailer 2):**
- Section Mesh: `/Engine/BasicShapes/Cube`
- Section Size: `X=200, Y=150, Z=80`
- Section Color: `Dark Grey (R=0.2, G=0.2, B=0.2)`

#### Mount Points
Add 10 mount points to the `MountPoints` array (default empty transforms are fine for MVP):
- 2 on cab (indices 0-1)
- 4 on trailer 1 (indices 2-5)
- 4 on trailer 2 (indices 6-9)

#### Stats
- **Max Hull**: `100.0`
- **Max Fuel**: `100.0`
- **Lane Change Fuel Cost**: `0.0` (free for MVP)
- **Lane Change Speed**: `500.0` (units per second)

#### Camera
- **Camera Distance**: `1500.0`
- **Camera Pitch**: `-75.0` (negative for top-down view)

#### Economy
- **Unlock Cost**: `0` (unlocked by default)

### 3. Create WorldScrollData Data Table

**In the Unreal Editor:**

1. Navigate to `Content/DataTables/`
2. Right-click → Miscellaneous → Data Table
3. Select `FWorldScrollData` as the row structure
4. Name it `DT_WorldScrollData`

Add a row with the name **"Default"**:
- **Tile Size**: `2000.0` (length of each ground tile)
- **Tile Pool Size**: `5` (number of tiles in the pool)
- **Tile Spawn Distance**: `3000.0` (distance ahead to spawn)
- **Tile Despawn Distance**: `1000.0` (distance behind to recycle)
- **Enemy Pool Size**: `50` (for future use)
- **Obstacle Pool Size**: `30` (for future use)
- **Pickup Pool Size**: `20` (for future use)

### 4. Create GameplayBalanceData Data Table

**In the Unreal Editor:**

1. Navigate to `Content/DataTables/`
2. Right-click → Miscellaneous → Data Table
3. Select `FGameplayBalanceData` as the row structure
4. Name it `DT_GameplayBalance`

Add a row with the name **"Default"**:
- **Scroll Speed**: `1000.0` (units per second)
- **Lane Width**: `400.0` (distance between lanes)
- **Lane Change Duration**: `1.0` (seconds)
- **Fuel Drain Rate**: `1.0` (per second)
- **Lane Change Fuel Cost**: `5.0`
- **Win Distance**: `10000.0`
- **Obstacle Spawn Distance**: `2000.0`

## Level Setup

### 1. Create Game Mode Blueprint

1. Create a Blueprint based on `AWhitelineNightmareGameMode`
2. Name it `BP_WarRigGameMode`

### 2. Spawn War Rig

In your level:
1. Add a `BP_WarRigPawn` (or create Blueprint based on `AWarRigPawn`)
2. In the **Details Panel**, under **War Rig | Data**:
   - **Default War Rig Data Table**: Select `DT_WarRigData`
   - **Default Row Name**: `SemiTruck`
   - **Gameplay Balance Data Table**: Select `DT_GameplayBalance`
   - **Balance Data Row Name**: `Default`
3. Position at origin: `Location=(0, 0, 100)` (slight Z offset to be above ground)

### 3. Setup World Scrolling

Create a Blueprint actor or add to Game Mode:
1. Add `UWorldScrollComponent`
2. In the **Details Panel**, under **World Scroll | Config**:
   - **Gameplay Balance Data Table**: Select `DT_GameplayBalance`
   - **Balance Data Row Name**: `Default`
   - **Fallback Scroll Speed**: `1000.0` (used if data table unavailable)
   - **Scroll Direction**: `(-1, 0, 0)` (already set by default)
   - **Is Scroll Enabled**: ✓ (checked)

**That's it!** The component auto-initializes on BeginPlay and loads scroll speed from the data table.

**Runtime Properties** (visible during Play in Editor):
- Under **World Scroll | Runtime**, you'll see:
  - **Scroll Speed**: Current active scroll speed (loaded from data table)
  - **Distance Traveled**: Total distance traveled (for win condition tracking)
  - **Is Initialized**: Whether the component has been initialized

### 4. Setup Ground Tile Pool

Create a Blueprint actor or add to Game Mode:
1. Add `UGroundTilePoolComponent`
2. In the **Details Panel**, under **Ground Tile Pool | Config**:
   - **Tile Class**: Select `AGroundTile` (or your custom tile class)
   - **Default Tile Size**: `(2000, 2000)` (X=length, Y=width)
   - **Default Pool Size**: `5`
   - **Spawn Distance Ahead**: `3000.0`
   - **Despawn Distance Behind**: `1000.0`
   - **World Scroll Component**: Leave empty if both components are on the same actor (auto-discovered)
   - **World Scroll Data Table**: (Optional) Select `DT_WorldScrollData`
   - **Data Table Row Name**: `Default`
   - **Auto Initialize**: ✓ (checked)

**That's it!** The component auto-initializes on BeginPlay, loads config from data tables, and spawns tiles automatically.

**Important**: If both `GroundTilePoolComponent` and `WorldScrollComponent` are on the **same actor** (like BP_WarRigGameMode), you can leave **World Scroll Component** empty - it will automatically find the WorldScrollComponent on BeginPlay. If they're on different actors, you'll need to set the reference in Blueprint's Event Graph.

**Advanced Option**: If you set the **World Scroll Data Table**, it will load tile size, pool size, and spawn/despawn distances from there instead of the default values.

**Runtime Properties** (visible during Play in Editor):
- Under **Ground Tile Pool | Runtime**, you'll see:
  - **Tile Size**: Current tile size being used
  - **War Rig Location**: Reference point for spawning/despawning
  - **Furthest Tile Position**: Position of the furthest tile ahead
  - **Is Tile Pool Initialized**: Whether the pool has been initialized

## Input Setup

### Automatic Input Configuration

Input is configured **programmatically** using Enhanced Input System - no manual setup required!

The `AWarRigPlayerController` automatically creates and binds input actions in `SetupInputComponent()`:

**Configured Bindings:**
- **Move Left**: `A` key or `Left Arrow`
- **Move Right**: `D` key or `Right Arrow`

**Implementation Details:**
- Creates `UInputAction` objects at runtime
- Creates `UInputMappingContext` and adds key mappings
- Registers context with Enhanced Input Subsystem
- Binds actions to controller methods

**No editor configuration needed!** Just ensure Enhanced Input is enabled in Project Settings:
- Edit → Project Settings → Input
- **Default Player Input Class** = `EnhancedPlayerInput`
- **Default Input Component Class** = `EnhancedInputComponent`

Then play and the input will work automatically.

## Debug Commands

Available console commands:

- **DebugShowLanes** - Toggle lane line visualization
- **DebugShowTileBounds** - Toggle tile spawn/despawn boundaries (TODO)
- **DebugSetScrollSpeed [speed]** - Change scroll speed (e.g., `DebugSetScrollSpeed 500`)

Access console with `~` key and type commands.

## Testing

Run automated tests using the testing framework:

1. Create a test map with `ATestingGameMode`
2. Open console and run:
   - `RunTests Movement` - Run movement tests
   - `RunTests All` - Run all tests

Available tests:
- **TestLaneSystemBounds** - Verifies lane boundaries
- **TestLaneTransitionSpeed** - Verifies smooth lane transitions
- **TestTilePoolRecycling** - Verifies tile recycling (TODO)
- **TestScrollSpeedConsistency** - Verifies constant scroll speed
- **TestWarRigDataLoading** - Verifies data table loading (TODO)

## Troubleshooting

### War Rig Not Moving Between Lanes

**Symptom**: Pressing A/D or Left/Right Arrow keys does nothing.

**How to Check:**

1. **Verify LaneSystemComponent is initialized**
   - Play the game (PIE - Play in Editor)
   - Select your War Rig pawn in the World Outliner (should be selected automatically)
   - In the Details panel, find **LaneSystemComponent**
   - Under **Lane System**, check **Is Initialized** - should be ✓ (true)
   - Check **Output Log** (Window → Developer Tools → Output Log) for:
     ```
     LogTemp: LaneSystemComponent: Initialized with 5 lanes, width 400.00. Starting in lane 2.
     ```

2. **Verify input actions are bound correctly**
   - Check **Output Log** for these messages in order:
     ```
     LogWarRigPlayerController: SetupEnhancedInput: Enhanced Input configured programmatically
     LogWarRigPlayerController:   - Move Left: A or Left Arrow
     LogWarRigPlayerController:   - Move Right: D or Right Arrow
     LogWarRigPlayerController: SetupInputComponent: Bound MoveLeft action
     LogWarRigPlayerController: SetupInputComponent: Bound MoveRight action
     LogWarRigPlayerController: SetupInputComponent: Enhanced Input bindings complete
     ```
   - If you see "MoveLeftAction is null" or "MoveRightAction is null", the input system failed to initialize
   - If you see "Failed to get Enhanced Input Subsystem", check Project Settings (see step 5 below)
   - Try pressing A or D and look for: `LogWarRigPlayerController: OnMoveLeft: Lane change requested` (enable Verbose logging)

3. **Check lane change speed**
   - In Details panel, find **LaneSystemComponent**
   - Check **Lane Change Speed** - should be `500.0` or higher
   - If it's 0 or very low, lane changes will be invisible/very slow
   - Also check in **War Rig Data Table** (`DT_WarRigData` → SemiTruck row) that **Lane Change Speed** = `500.0`

4. **Use debug visualization**
   - Open console (~ key) and type: `DebugShowLanes`
   - You should see 5 colored lines extending forward from the war rig
   - Green line = current lane, White lines = other lanes
   - If lines appear but war rig doesn't move, check the Output Log for lane change rejections

5. **Verify Enhanced Input is enabled in Project Settings**
   - Go to **Edit → Project Settings → Input**
   - Under **Default Classes**, verify:
     - **Default Player Input Class** = `EnhancedPlayerInput`
     - **Default Input Component Class** = `EnhancedInputComponent`
   - If these are set to "PlayerInput" or "InputComponent", change them and restart the editor
   - This is required for the programmatic Enhanced Input setup to work

### Enhanced Input Not Working - Advanced Diagnostics

**If input still doesn't work after following all steps above**, use these advanced diagnostic tools:

#### Step 1: Run the Input Diagnostic Command

1. **Play the game (PIE)**
2. **Open console** (~ key)
3. **Type**: `DebugListInputContexts`
4. **Press Enter**

This command will show detailed information about the Enhanced Input system state.

**What to look for:**

✅ **Expected output** (everything working):
```
LogWarRigPlayerController: === ENHANCED INPUT DIAGNOSTIC ===
LogWarRigPlayerController: Enhanced Input Subsystem: FOUND (OK)
LogWarRigPlayerController: Checking for our IMC_WarRig context: IMC_WarRig
LogWarRigPlayerController:   >>> IMC_WarRig IS ACTIVE in subsystem (OK) <<<
LogWarRigPlayerController:   Priority: 0
LogWarRigPlayerController: Input Actions:
LogWarRigPlayerController:   - MoveLeftAction: IA_MoveLeft (OK)
LogWarRigPlayerController:   - MoveRightAction: IA_MoveRight (OK)
LogWarRigPlayerController: Input Component: EnhancedInputComponent (OK)
LogWarRigPlayerController: =================================
```

❌ **Problem 1: Enhanced Input Subsystem NOT FOUND**
```
LogWarRigPlayerController: Enhanced Input Subsystem NOT FOUND!
LogWarRigPlayerController:   >>> This means Project Settings -> Input is NOT configured for Enhanced Input! <<<
```

**FIX:**
1. Close Unreal Editor completely
2. Go to **Edit → Project Settings → Input**
3. Under **Default Classes**, set:
   - **Default Player Input Class** = `EnhancedPlayerInput`
   - **Default Input Component Class** = `EnhancedInputComponent`
4. Click **Set as Default** for both
5. **CRITICAL**: Close and restart the editor (required for input system changes)
6. Test again

❌ **Problem 2: IMC_WarRig is NOT ACTIVE in subsystem**
```
LogWarRigPlayerController:   >>> IMC_WarRig is NOT ACTIVE in subsystem! <<<
```

**FIX:**
- This means the Input Mapping Context failed to register
- Check that BP_WarRigPlayerController is actually being used as the Player Controller
- Verify Game Mode's **Player Controller Class** = `BP_WarRigPlayerController`
- Check Output Log for "SetupEnhancedInput: Added mapping context" message during startup

❌ **Problem 3: Input Component NOT EnhancedInputComponent**
```
LogWarRigPlayerController: Input Component: NOT EnhancedInputComponent!
```

**FIX:**
- Same as Problem 1 - Project Settings not configured correctly
- Must restart editor after changing settings

#### Step 2: Test if Callbacks are Being Invoked

After running `DebugListInputContexts` and seeing "OK" for everything:

1. **Keep the Output Log visible**
2. **Press A key**
3. **Look for this LOUD warning log**:
   ```
   LogWarRigPlayerController: Warning: >>> OnMoveLeft() CALLED - Enhanced Input callback triggered! <<<
   ```

**Results:**

✅ **If you see the log**:
- Enhanced Input is working correctly!
- The problem is NOT with input, but with lane changing logic
- Check LaneSystemComponent initialization
- Run `DebugShowLanes` to visualize lanes

❌ **If you DON'T see the log**:
- Enhanced Input system is NOT triggering the callback
- Even though everything appears configured correctly
- **Most likely causes:**
  1. **Viewport doesn't have focus** - Click in the game viewport before pressing keys
  2. **Another Input Mapping Context is blocking** - Check for conflicting IMCs with higher priority
  3. **Key mappings not set in IMC_WarRig** - See "Manual Editor Setup" below

#### Step 3: Manual Editor Setup (Alternative to Programmatic)

If the programmatic setup continues to fail, you can set up Enhanced Input manually in the editor:

**A. Create Input Actions:**

1. Navigate to `Content/Input/` (create folder if needed)
2. Right-click → Input → Input Action
3. Create two actions:
   - `IA_MoveLeft` (Value Type: Digital (bool))
   - `IA_MoveRight` (Value Type: Digital (bool))

**B. Create Input Mapping Context:**

1. In `Content/Input/`, right-click → Input → Input Mapping Context
2. Name it `IMC_WarRig`
3. Open `IMC_WarRig`
4. In the **Mappings** section:

   **Add IA_MoveLeft:**
   - Click **"+ Add Mapping"**
   - Select `IA_MoveLeft` from dropdown
   - Click the **"+"** icon next to it
   - Press **A** key (it will record "A : Keyboard")
   - Click **"+"** again
   - Press **Left Arrow** key (it will record "Left : Keyboard")

   **Add IA_MoveRight:**
   - Click **"+ Add Mapping"**
   - Select `IA_MoveRight` from dropdown
   - Click the **"+"** icon
   - Press **D** key
   - Click **"+"** again
   - Press **Right Arrow** key

5. **Save** the asset

**C. Assign to Player Controller Blueprint:**

1. Open `Content/Blueprints/Player/BP_WarRigPlayerController`
2. In **Class Defaults** (toolbar button), find **Whiteline Nightmare | Input** category:
   - **Input Mapping Context** = `IMC_WarRig`
   - **Move Left Action** = `IA_MoveLeft`
   - **Move Right Action** = `IA_MoveRight`
3. **Compile** and **Save**

**D. Verify in Output Log:**

When you play, you should now see:
```
LogWarRigPlayerController: SetupEnhancedInput: Using editor-assigned Input Assets
LogWarRigPlayerController:   - Mapping Context: IMC_WarRig
LogWarRigPlayerController:   - Move Left Action: IA_MoveLeft
LogWarRigPlayerController:   - Move Right Action: IA_MoveRight
```

Instead of:
```
LogWarRigPlayerController: SetupEnhancedInput: Creating Input Assets programmatically
```

#### Common Issues and Final Checks

**Issue**: "Everything shows OK but input still doesn't work"

**Try these:**
1. **Click in the viewport** before pressing keys (viewport must have focus)
2. **Check for conflicting input** - Disable any other input systems or plugins
3. **Verify War Rig is possessed** - Output Log should show: `LogWarRigPlayerController: Possessed pawn BP_WarRigPawn`
4. **Check Game Mode setup**:
   - World Settings → Game Mode = `BP_WarRigGameMode`
   - BP_WarRigGameMode → Player Controller Class = `BP_WarRigPlayerController`
   - BP_WarRigGameMode → Default Pawn Class = `BP_WarRigPawn`

**Issue**: "Logs show OnMoveLeft() called but war rig doesn't move"

**This is NOT an input problem!** The input system is working. Check:
1. LaneSystemComponent initialization (see "War Rig Not Moving Between Lanes" section above)
2. Lane change speed > 0
3. Not already at leftmost/rightmost lane
4. Enable `DebugShowLanes` to visualize

**Issue**: "Project Settings changes don't take effect"

**Solution:**
1. Close Unreal Editor completely (don't just stop PIE)
2. Delete `Saved/` and `Intermediate/` folders from project root
3. Reopen editor
4. Test again

### World Not Scrolling

**Symptom**: Ground tiles aren't moving backward, world appears frozen.

**How to Check:**

1. **Verify WorldScrollComponent is initialized and enabled**
   - During Play in Editor, select the actor with WorldScrollComponent (BP_WarRigGameMode)
   - In Details panel, find **WorldScrollComponent**
   - Under **World Scroll | Runtime**, check:
     - **Is Initialized**: Should be ✓ (true)
     - **Scroll Speed**: Should show a value like `1000.0` (not 0)
     - **Distance Traveled**: Should be increasing every frame (watch it change)
   - Check **Output Log** for:
     ```
     LogTemp: WorldScrollComponent: Loaded scroll speed 1000.00 from gameplay balance data table 'Default'
     ```

2. **Check scroll speed is greater than 0**
   - In Details panel: **World Scroll | Runtime → Scroll Speed** should be > 0
   - Check data table: Open `DT_GameplayBalance` → Default row → **Scroll Speed** = `1000.0`
   - If Scroll Speed is 0 in runtime, check:
     - **Gameplay Balance Data Table** is set in Config section
     - **Balance Data Row Name** = "Default"
     - **Fallback Scroll Speed** = `1000.0` (in case table fails)

3. **Ensure scroll direction is set**
   - In Details panel: **World Scroll | Config → Scroll Direction** = `(-1, 0, 0)`
   - X should be **negative** for backward scroll
   - Check **Output Log** for: `LogTemp: WorldScrollComponent: Initialized with speed 1000.00`

4. **Check scrolling is enabled**
   - In Details panel: **World Scroll | Config → Is Scroll Enabled** should be ✓ (checked)
   - If unchecked, scrolling is paused

### Ground Tiles Not Appearing

**Symptom**: No ground visible, or tiles aren't recycling.

**How to Check:**

1. **Verify GroundTilePoolComponent is initialized**
   - During Play in Editor, select the actor with GroundTilePoolComponent
   - In Details panel, find **GroundTilePoolComponent**
   - Under **Ground Tile Pool | Runtime**, check:
     - **Is Tile Pool Initialized**: Should be ✓ (true)
     - **Tile Size**: Should show `(2000, 2000)` or your configured size
   - Check **Output Log** for:
     ```
     LogTemp: GroundTilePoolComponent: Auto-discovered WorldScrollComponent on owner actor
     LogTemp: GroundTilePoolComponent: Initialized with 5 tiles of size (2000.00, 2000.00).
     LogTemp: GroundTilePoolComponent: Spawning 5 initial tiles.
     ```

2. **Check that SpawnInitialTiles was called**
   - Look in **Output Log** for: `LogTemp: GroundTilePoolComponent: Initial tiles spawned. Furthest position: XXXX`
   - If you see "Auto-initialization failed", check:
     - **Tile Class** is set to `AGroundTile`
     - **WorldScrollComponent** was found (should see "Auto-discovered" message)
     - **Auto Initialize** is ✓ (checked)

3. **Verify tile pool size > 0**
   - In Details panel: **Ground Tile Pool | Config → Default Pool Size** should be `5` or higher
   - Check base class: **Object Pool Component** section shows:
     - **Total Pool Size**: Should match your Default Pool Size
     - **Available Count** + **Active Count** = Total Pool Size

4. **Check spawn/despawn distances**
   - In Details panel:
     - **Spawn Distance Ahead**: `3000.0` (should be > tile size)
     - **Despawn Distance Behind**: `1000.0`
   - If tiles spawn too far ahead or despawn too late, adjust these values
   - For 2000-unit tiles, spawn distance should be at least 3000

5. **Verify WorldScrollComponent reference**
   - Check **Output Log** for: `LogTemp: GroundTilePoolComponent: Auto-discovered WorldScrollComponent on owner actor`
   - If you see "WorldScrollComponent not found", both components aren't on the same actor
   - In that case, manually set **World Scroll Component** reference

6. **Check tile pool status during play**
   - Select the GroundTilePoolComponent actor
   - Watch **Active Count** and **Available Count** change as tiles recycle
   - **Active Count** should be around 3-5 (depending on distances)
   - **Available Count** should decrease/increase as tiles are used/returned

### Camera Not Following War Rig

**Symptom**: Camera is at wrong angle, too far, or not attached.

**How to Check:**

1. **Verify SpringArm and Camera are attached**
   - Select your War Rig pawn in World Outliner
   - In the Details panel, look at the component hierarchy (top left)
   - Should see:
     ```
     RigRoot (SceneComponent)
     ├─ CameraSpringArm (SpringArmComponent)
     │  └─ Camera (CameraComponent)
     └─ LaneSystemComponent
     ```
   - If Camera or SpringArm is missing, check War Rig blueprint/C++ class

2. **Check camera distance and pitch**
   - During Play, select War Rig pawn
   - Find **CameraSpringArm** in Details panel:
     - **Target Arm Length**: Should be `1500.0` (or value from data table)
     - **Rotation**: Pitch should be around `-75.0` (negative for top-down)
   - Check **War Rig Data Table** (`DT_WarRigData` → SemiTruck row):
     - **Camera Distance** = `1500.0`
     - **Camera Pitch** = `-75.0`
   - Check **Output Log** for:
     ```
     LogTemp: WarRigPawn: Camera setup - Distance: 1500.00, Pitch: -75.00
     ```

3. **Ensure camera lag is disabled**
   - Select War Rig pawn
   - Find **CameraSpringArm** in Details panel
   - **Camera Settings**:
     - **Enable Camera Lag**: Should be ☐ (unchecked) for rigid following
     - **Enable Camera Rotation Lag**: Should be ☐ (unchecked)
   - If enabled, camera will smoothly follow instead of staying locked

4. **Check camera is set as view target**
   - War Rig pawn should auto-possess Player 0
   - Check **Output Log** for: `LogWarRigPlayerController: Possessed pawn BP_WarRigPawn`
   - If camera view is wrong, check:
     - **Auto Possess Player** = `Player 0` in War Rig pawn defaults
     - Game Mode's **Default Pawn Class** points to your War Rig

### General Debugging Tips

**Enable Verbose Logging:**
- Open console (~) and type: `Log LogTemp Verbose`
- This shows more detailed messages including lane change requests

**Check All Components During Play:**
- Select actor in World Outliner during PIE
- Details panel shows RUNTIME values (different from edit-time)
- Watch values change in real-time

**Output Log Categories:**
- `LogTemp`: General system messages
- `LogWarRigPlayerController`: Input and player actions
- Filter Output Log by typing category name in search box

**Common Data Table Issues:**
- Ensure all data tables are saved and compiled
- Row names are case-sensitive ("Default" ≠ "default")
- Check for typos in row names in component configs

**Reset to Defaults:**
- If something is completely broken:
  1. Delete the component
  2. Re-add it (gets fresh default values)
  3. Set only the essential properties (data tables)
  4. Test incrementally

## Future Enhancements

- Add Gameplay Ability System integration for lane changes
- Implement fuel consumption for lane changes
- Add mounting points for turrets
- Create visual effects for lane changes
- Add sound effects
- Implement multiple war rig types

## File Reference

### Core Classes
- `Source/WhitelineNightmare/Public/WarRig/WarRigPawn.h`
- `Source/WhitelineNightmare/Public/WarRig/LaneSystemComponent.h`
- `Source/WhitelineNightmare/Public/World/WorldScrollComponent.h`
- `Source/WhitelineNightmare/Public/World/GroundTile.h`
- `Source/WhitelineNightmare/Public/World/GroundTilePoolComponent.h`

### Data Structures
- `Source/WhitelineNightmare/Public/Core/GameDataStructs.h`
  - `FWarRigData`
  - `FWarRigSectionData`
  - `FMountPointData`
  - `FWorldScrollData`
  - `FGameplayBalanceData`

### Testing
- `Source/WhitelineNightmare/Private/Testing/MovementTests.cpp`
