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

Open `DT_WarRigData` and add a new row with the following configuration:

#### Row Name
- **Row Name**: `SemiTruck`

#### War Rig Properties

**Display Name**: `Semi Truck`
**Description**: `A classic highway semi-truck configured for wasteland combat`

#### Mesh Sections

For MVP, add 3 mesh sections (you can use Engine primitive cubes or custom meshes):

1. **Mesh Section 0** (Cab):
   - Use: `/Engine/BasicShapes/Cube` (or custom cab mesh)
   - Position: Handled automatically by code (0, 0, 0)

2. **Mesh Section 1** (Trailer 1):
   - Use: `/Engine/BasicShapes/Cube` (or custom trailer mesh)
   - Position: Handled automatically by code (-200, 0, 0)

3. **Mesh Section 2** (Trailer 2):
   - Use: `/Engine/BasicShapes/Cube` (or custom trailer mesh)
   - Position: Handled automatically by code (-400, 0, 0)

#### Stats

- **Max Hull**: `100.0`
- **Lane Change Fuel Cost**: `0.0` (free for MVP)
- **Lane Change Speed**: `500.0` (units per second)
- **Max Fuel**: `100.0` (legacy)
- **Max Armor**: `100.0` (legacy)

#### Visual Properties

- **Primary Color**: `Red (1.0, 0.0, 0.0, 1.0)`
- **Secondary Color**: `Dark Gray (0.2, 0.2, 0.2, 1.0)`
- **Primary Material**: Leave empty for MVP (uses default materials)
- **Secondary Material**: Leave empty for MVP

#### Camera Settings

- **Camera Distance**: `2000.0`
- **Camera Pitch**: `-75.0` (negative = looking down)

#### Economy

- **Unlock Cost**: `0` (free for MVP)

#### Mount Points

Add 10 mount points with the following configurations:

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

### Step 3: Set Up Gameplay Tags

Create the following gameplay tags in your project:
- `Mount.Cab`
- `Mount.Trailer`
- `Mount.Rear`

1. Go to: Project Settings → Project → Gameplay Tags
2. Add new tag sources or add directly:
   - `Mount.Cab`
   - `Mount.Trailer`
   - `Mount.Rear`

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
