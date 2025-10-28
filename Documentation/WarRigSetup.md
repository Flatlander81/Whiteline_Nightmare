# War Rig Pawn Setup Guide

## Overview

The War Rig Pawn is the player's stationary vehicle that stays at the world origin (0, 0, 0). Configuration is data-driven through a Data Table asset.

**CRITICAL DESIGN**: The war rig does NOT move. The world scrolls past it to create the illusion of movement.

## Prerequisites

1. Unreal Engine 5.6+
2. GameplayAbilities plugin enabled (already configured in project)
3. Project compiled successfully

## Creating the War Rig Data Table

### Step 1: Create the Data Table Asset

1. In the Unreal Editor Content Browser, navigate to: `Content/Data/`
2. Right-click → Miscellaneous → Data Table
3. When prompted, select `WarRigData` as the row structure
4. Name the asset: `DT_WarRigData`
5. Save the asset

### Step 2: Configure the "SemiTruck" Row

Open `DT_WarRigData` and add a new row:

**⚡ QUICK SETUP**: Just create a row named `SemiTruck` and you're 90% done! The FWarRigData constructor automatically populates sensible defaults for everything. You only need to:
1. Set the row name to `SemiTruck`
2. Assign 3 mesh references (or use default cubes)
3. Optionally adjust mount point tags in the data table

Here's what gets configured automatically:

#### Row Name
- **Row Name**: `SemiTruck`

#### War Rig Properties

**GOOD NEWS**: Most properties come with sensible defaults! When you create a new row, it will automatically have:
- **Display Name**: `Semi Truck`
- **Description**: `A classic highway semi-truck configured for wasteland combat`
- **3 Mesh Section slots** (ready to assign meshes)
- **10 Mount Points** (pre-configured with transforms and facing directions)
- **Default stats** (MaxHull: 100, Lane Change Speed: 500, etc.)
- **Default camera settings** (Distance: 2000, Pitch: -75)

#### Mesh Sections

The data table row comes with **3 mesh section slots pre-allocated**. You just need to assign the actual meshes:

1. **Mesh Section 0** (Cab):
   - Use: `/Engine/BasicShapes/Cube` (or custom cab mesh)
   - Position: Handled automatically by code (0, 0, 0)

2. **Mesh Section 1** (Trailer 1):
   - Use: `/Engine/BasicShapes/Cube` (or custom trailer mesh)
   - Position: Handled automatically by code (-200, 0, 0)

3. **Mesh Section 2** (Trailer 2):
   - Use: `/Engine/BasicShapes/Cube` (or custom trailer mesh)
   - Position: Handled automatically by code (-400, 0, 0)

#### Stats (Pre-configured)

These are automatically set to sensible defaults:
- **Max Hull**: `100.0` ✓
- **Lane Change Fuel Cost**: `0.0` (free for MVP) ✓
- **Lane Change Speed**: `500.0` (units per second) ✓
- **Max Fuel**: `100.0` (legacy) ✓
- **Max Armor**: `100.0` (legacy) ✓

#### Visual Properties (Pre-configured)

These have automatic defaults:
- **Primary Color**: `Red (1.0, 0.0, 0.0, 1.0)` ✓
- **Secondary Color**: `Gray (0.5, 0.5, 0.5, 1.0)` ✓
- **Primary Material**: Empty (uses default materials) ✓
- **Secondary Material**: Empty ✓

#### Camera Settings (Pre-configured)

Automatically configured for top-down view:
- **Camera Distance**: `2000.0` ✓
- **Camera Pitch**: `-75.0` (negative = looking down) ✓

#### Economy (Pre-configured)

- **Unlock Cost**: `0` (free for MVP) ✓

#### Mount Points

**GOOD NEWS**: Mount points are **automatically populated** with default values when you create a new data table row! The FWarRigData constructor pre-populates all 10 mount points with the configurations below.

You can use them as-is or modify them in the data table editor if needed. Here are the default configurations:

##### Cab Mount Points (2 total)

**Mount Point 0** (Cab - Left Side):
- **Mount Transform**:
  - Location: `(0, -100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[0, 1, 2, 6, 7]` (Forward, Forward-Right, Right, Left, Forward-Left)
- **Display Name**: `Cab Left`
- **Mount Tags**: Create tag `Mount.Cab`

**Mount Point 1** (Cab - Right Side):
- **Mount Transform**:
  - Location: `(0, 100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[0, 1, 2, 3, 4]` (Forward, Forward-Right, Right, Back-Right, Back)
- **Display Name**: `Cab Right`
- **Mount Tags**: `Mount.Cab`

##### Trailer 1 Mount Points (4 total)

**Mount Point 2** (Trailer 1 - Front Left):
- **Mount Transform**:
  - Location: `(-200, -100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[]` (all directions)
- **Display Name**: `Trailer 1 Front Left`
- **Mount Tags**: `Mount.Trailer`

**Mount Point 3** (Trailer 1 - Front Right):
- **Mount Transform**:
  - Location: `(-200, 100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[]` (all directions)
- **Display Name**: `Trailer 1 Front Right`
- **Mount Tags**: `Mount.Trailer`

**Mount Point 4** (Trailer 1 - Rear Left):
- **Mount Transform**:
  - Location: `(-300, -100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[2, 3, 4, 5, 6]` (Right, Back-Right, Back, Back-Left, Left)
- **Display Name**: `Trailer 1 Rear Left`
- **Mount Tags**: `Mount.Trailer`

**Mount Point 5** (Trailer 1 - Rear Right):
- **Mount Transform**:
  - Location: `(-300, 100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[2, 3, 4, 5, 6]` (Right, Back-Right, Back, Back-Left, Left)
- **Display Name**: `Trailer 1 Rear Right`
- **Mount Tags**: `Mount.Trailer`

##### Trailer 2 Mount Points (4 total)

**Mount Point 6** (Trailer 2 - Front Left):
- **Mount Transform**:
  - Location: `(-400, -100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[]` (all directions)
- **Display Name**: `Trailer 2 Front Left`
- **Mount Tags**: `Mount.Trailer`, `Mount.Rear`

**Mount Point 7** (Trailer 2 - Front Right):
- **Mount Transform**:
  - Location: `(-400, 100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[]` (all directions)
- **Display Name**: `Trailer 2 Front Right`
- **Mount Tags**: `Mount.Trailer`, `Mount.Rear`

**Mount Point 8** (Trailer 2 - Rear Left):
- **Mount Transform**:
  - Location: `(-500, -100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[3, 4, 5]` (Back-Right, Back, Back-Left)
- **Display Name**: `Trailer 2 Rear Left`
- **Mount Tags**: `Mount.Trailer`, `Mount.Rear`

**Mount Point 9** (Trailer 2 - Rear Right):
- **Mount Transform**:
  - Location: `(-500, 100, 50)`
  - Rotation: `(0, 0, 0)`
  - Scale: `(1, 1, 1)`
- **Allowed Facing Directions**: `[3, 4, 5]` (Back-Right, Back, Back-Left)
- **Display Name**: `Trailer 2 Rear Right`
- **Mount Tags**: `Mount.Trailer`, `Mount.Rear`

### Step 3: Gameplay Tags (Automatic!)

**GREAT NEWS**: Gameplay tags are now **automatically registered** via native C++ tags! No manual setup required!

The following tags are automatically available:
- ✅ `Mount.Cab` - Auto-assigned to cab mount points
- ✅ `Mount.Trailer` - Auto-assigned to trailer mount points
- ✅ `Mount.Rear` - Auto-assigned to rear trailer mount points
- Plus 30+ other tags for abilities, states, damage types, effects, enemies, pickups, and turrets

All mount points in the default configuration already have their appropriate tags assigned. You can view and modify them in the data table if needed.

**Tags are defined in**: `Source/WhitelineNightmare/Public/Core/WhitelineNightmareGameplayTags.h`

### Step 4: Configure the War Rig Pawn

1. In the Content Browser, create a Blueprint based on `AWarRigPawn`:
   - Right-click → Blueprint Class → Search for "WarRigPawn"
   - Name it: `BP_WarRig_SemiTruck`

2. Open the Blueprint and configure:
   - **War Rig Data Table**: Set to `DT_WarRigData`
   - **Current Rig ID**: Set to `SemiTruck`

3. Save and compile the Blueprint

### Step 5: Test in Game

1. Place `BP_WarRig_SemiTruck` in your level at the world origin (0, 0, 0)
2. Ensure it's set as the default pawn in your Game Mode
3. Play in Editor

#### Testing Commands

Open the console (~ key) and run these commands:

**Data Loading Test**:
```
TestWarRigDataLoading
```

**Mesh Spawn Test**:
```
TestWarRigSpawn
```

**Mount Point Test**:
```
TestMountPointSetup
```

**Camera Test**:
```
TestCameraSetup
```

**Stationary Position Test**:
```
TestStationaryPosition
```

#### Debug Commands

**Toggle Mount Point Visualization**:
```
DebugShowMountPoints
```

**Toggle Bounds Visualization**:
```
DebugShowWarRigBounds
```

**Reload Configuration**:
```
DebugReloadWarRigData
```

## Expected Results

After setup, you should see:
1. A war rig composed of 3 mesh sections (cab + 2 trailers) at world origin
2. Camera positioned above and behind, looking down at ~75-degree angle
3. Mount point visualization when debug command is enabled (cyan spheres)
4. War rig stays perfectly at origin (0, 0, 0) - never moves

## Troubleshooting

### War rig is not visible
- Check that mesh sections are set in the data table
- Verify the data table is assigned to the pawn
- Check the log for errors: `Window → Developer Tools → Output Log`

### Mount points not appearing
- Run `DebugShowMountPoints` to visualize them
- Check that mount point data is configured in the data table
- Verify transforms are reasonable values

### Camera not positioned correctly
- Check Camera Distance and Camera Pitch values in data table
- Typical values: Distance 1500-3000, Pitch -60 to -80
- Use `TestCameraSetup` to print camera configuration

### War rig moving from origin
- This should NEVER happen - it indicates a bug
- Check the log for warnings: "War rig drifted from origin"
- The Tick function will auto-correct this, but investigate the cause

## Architecture Notes

### Extensibility

The system is designed to support multiple rig types:
- Add new rows to `DT_WarRigData` for different vehicles
- Each row can have different mesh counts, mount points, stats, etc.
- Switch rigs at runtime using `LoadWarRigConfiguration(NewRigID)`

### Lane System Integration

The Lane System Component will be implemented in a future task:
- Handles lateral movement between lanes
- Uses the `LaneChangeSpeed` value from the data table
- Consumes `LaneChangeFuelCost` amount of fuel per lane change

### Ability System Integration

The war rig implements `IAbilitySystemInterface`:
- Ready for GAS (Gameplay Ability System) integration
- Can apply gameplay effects (damage, buffs, etc.)
- Supports ability activation

## Next Steps

1. **Implement Lane System Component** - For lateral movement
2. **Add Turret Mounting System** - Use mount points to attach turrets
3. **Implement Hull/Health System** - Use MaxHull value
4. **Add Visual Effects** - Damage states, engine effects, etc.
5. **Create Additional Rig Variants** - Different trucks, configurations

## File References

- Header: `Source/WhitelineNightmare/Public/Core/WarRigPawn.h`
- Implementation: `Source/WhitelineNightmare/Private/Core/WarRigPawn.cpp`
- Data Structures: `Source/WhitelineNightmare/Public/Core/GameDataStructs.h`
- Data Table: `Content/Data/DT_WarRigData.uasset` (to be created)
