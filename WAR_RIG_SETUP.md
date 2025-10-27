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
2. Configure:
   - **Default War Rig Data Table**: Select `DT_WarRigData`
   - **Default Row Name**: "SemiTruck"
3. Position at origin: `Location=(0, 0, 100)` (slight Z offset to be above ground)

### 3. Setup World Scrolling

**Option A: Simple Setup (Recommended for MVP)**

Create a Blueprint actor or add to Game Mode:
1. Add `UWorldScrollComponent`
2. In the **Details Panel**, under **World Scroll | Config**:
   - **Default Scroll Speed**: `1000.0` (units per second)
   - **Scroll Direction**: `(-1, 0, 0)` (already set by default)
   - **Is Scroll Enabled**: ✓ (checked)

That's it! The component will auto-initialize with these values on BeginPlay.

**Option B: Data Table Configuration (Advanced)**

If you want to use a data table for configuration:
1. Add `UWorldScrollComponent`
2. In the **Details Panel**, under **World Scroll | Config**:
   - **World Scroll Data Table**: Select `DT_WorldScrollData`
   - **Data Table Row Name**: "Default"
   - **Default Scroll Speed**: `1000.0` (fallback if data table fails)

The component will prioritize data table settings, falling back to Default Scroll Speed if the table is unavailable.

**Runtime Properties** (visible during Play in Editor):
- Under **World Scroll | Runtime**, you'll see:
  - **Scroll Speed**: Current active scroll speed
  - **Distance Traveled**: Total distance traveled (for win condition tracking)
  - **Is Initialized**: Whether the component has been initialized

### 4. Setup Ground Tile Pool

Create a Blueprint actor or add to Game Mode:
1. Add `UGroundTilePoolComponent`
2. Configure in BeginPlay:
   ```cpp
   // Initialize tile pool
   GroundTilePool->InitializeTilePool(
       AGroundTile::StaticClass(),
       FVector2D(2000.0f, 2000.0f),  // Tile size
       5,                              // Pool size
       3000.0f,                        // Spawn distance ahead
       1000.0f,                        // Despawn distance behind
       WorldScrollComponent            // Reference to scroll component
   );

   // Spawn initial tiles
   GroundTilePool->SpawnInitialTiles();
   ```

## Input Setup

### Automatic Input Configuration

Input is configured **programmatically** using Enhanced Input System - no manual setup required!

The `AWarRigPlayerController` automatically creates and binds input actions in `BeginPlay()`:

**Configured Bindings:**
- **Move Left**: `A` key or `Left Arrow`
- **Move Right**: `D` key or `Right Arrow`

**Implementation Details:**
- Creates `UInputAction` objects at runtime
- Creates `UInputMappingContext` and adds key mappings
- Registers context with Enhanced Input Subsystem
- Binds actions to controller methods

**No editor configuration needed!** Just play and the input will work automatically.

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
- Check that `ULaneSystemComponent` is initialized
- Verify input actions are bound correctly
- Check lane change speed is > 0

### World Not Scrolling
- Verify `UWorldScrollComponent` is initialized and enabled
- Check scroll speed is > 0
- Ensure scroll direction is set (default: -X)

### Ground Tiles Not Appearing
- Verify `UGroundTilePoolComponent` is initialized
- Check that `SpawnInitialTiles()` was called
- Verify tile pool size > 0
- Check spawn/despawn distances are reasonable

### Camera Not Following War Rig
- Verify `USpringArmComponent` and `UCameraComponent` are attached
- Check camera distance and pitch values in data table
- Ensure camera lag is disabled for rigid following

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
