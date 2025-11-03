# Fuel Pickup System Documentation

## Overview
The fuel pickup system allows players to collect fuel pickups in the game world to restore the war rig's fuel. The system uses object pooling for performance and integrates with the Gameplay Ability System (GAS).

## Components

### 1. AFuelPickup Actor
**Location:** `Source/WhitelineNightmare/Public/Pickups/FuelPickup.h`

A poolable actor that represents a fuel pickup in the game world.

**Features:**
- Implements `IPoolableActor` interface for object pooling
- Bright green sphere visual (configurable via data table)
- Sphere collision component for overlap detection
- Scrolls backward with world scroll speed
- Restores fuel on overlap with war rig
- Plays sound and particle effects on collection

**Key Methods:**
- `OnActivated_Implementation()` - Called when pickup is taken from pool
- `OnDeactivated_Implementation()` - Called when pickup is returned to pool
- `ResetState_Implementation()` - Called to reset pickup state
- `InitializeFromDataTable()` - Load configuration from data table

### 2. UPickupPoolComponent
**Location:** `Source/WhitelineNightmare/Public/Pickups/PickupPoolComponent.h`

Specialized object pool component for managing fuel and scrap pickups.

**Features:**
- Extends `UObjectPoolComponent` from Epic 0.2
- Pre-spawns configurable number of pickups (default: 20)
- Automatic despawning when pickups pass behind war rig
- Lane-based spawning system
- Debug visualization and console commands

**Configuration:**
- `SpawnDistanceAhead` - Distance ahead of war rig to spawn pickups (default: 2000 units)
- `DespawnDistanceBehind` - Distance behind war rig to despawn pickups (default: -1000 units)
- `LaneYPositions` - Y-axis positions for each lane (default: -400, -200, 0, 200, 400)

### 3. FPickupData Structure
**Location:** `Source/WhitelineNightmare/Public/Core/GameDataStructs.h`

Data table row structure for configuring pickups.

**Fields:**
- `DisplayName` (FText) - Display name of the pickup
- `PickupMesh` (TSoftObjectPtr<UStaticMesh>) - Mesh to use for the pickup
- `FuelAmount` (float) - Amount of fuel restored (0 if not a fuel pickup)
- `ScrapAmount` (int32) - Amount of scrap given (0 if not a scrap pickup)
- `SpawnWeight` (float) - Spawn weight (higher = more common)
- `PickupSound` (TSoftObjectPtr<USoundBase>) - Sound to play on collection
- `PickupParticle` (TSoftObjectPtr<UNiagaraSystem>) - Particle effect to spawn
- `VisualColor` (FLinearColor) - Color of the pickup sphere (default: Green)
- `PickupRadius` (float) - Collision radius (default: 50.0)

## Setup Instructions

### Step 1: Create Data Table
1. In Unreal Editor, create a new Data Table asset
2. Use `FPickupData` as the row structure
3. Name it `DT_PickupData`
4. Add a row named `FuelPickup` with the following values:
   - FuelAmount: 20.0
   - VisualColor: (0, 1, 0) - Green
   - PickupRadius: 50.0
   - Assign sound and particle assets as desired

### Step 2: Create GameplayEffect (GE_FuelRestore)
1. In Unreal Editor, right-click in Content Browser
2. Create > Gameplay > Gameplay Effect
3. Name it `GE_FuelRestore`
4. Configure the GameplayEffect:
   - **Duration Policy:** Instant
   - **Modifiers:**
     - Add a modifier for the `Fuel` attribute
     - **Modifier Op:** Add
     - **Modifier Magnitude:** Set by Caller
     - **Data Tag:** `Data.Fuel`

### Step 3: Set Up Pickup Pool Component
1. Add `UPickupPoolComponent` to your GameMode or spawner actor
2. In BeginPlay or initialization:
   ```cpp
   // Get references
   AWarRigPawn* WarRig = GetWarRigPawn();
   UWorldScrollComponent* ScrollComponent = GetWorldScrollComponent();

   // Initialize pickup pool
   PickupPoolComponent->InitializePickupPool(
       WarRig,
       ScrollComponent,
       AFuelPickup::StaticClass(),
       20  // Pool size
   );
   ```

### Step 4: Spawn Pickups
```cpp
// Spawn in specific lane (0-4)
AFuelPickup* Pickup = PickupPoolComponent->SpawnPickupInLane(2);

// Or spawn in random lane
AFuelPickup* Pickup = PickupPoolComponent->SpawnPickupInRandomLane();
```

## Console Commands

The following console commands are available for debugging (not available in shipping builds):

### DebugSpawnFuelPickup
**Usage:** `DebugSpawnFuelPickup <lane_index>`
**Description:** Spawns a fuel pickup in the specified lane (0-4) ahead of the war rig.
**Example:** `DebugSpawnFuelPickup 2` - Spawns pickup in center lane

### DebugShowPickups
**Usage:** `DebugShowPickups`
**Description:** Toggles debug visualization for pickups. Shows:
- Green line: Spawn boundary (where pickups spawn)
- Red line: Despawn boundary (where pickups are removed)
- Green spheres: Active pickups
- Cyan lines: Lane markers
- Yellow text: Pool statistics

### DebugShowPickupPool
**Usage:** `DebugShowPickupPool`
**Description:** Displays pool statistics in the log:
- Active pickups count
- Available pickups count
- Total pool size
- Spawn/despawn distances
- Number of lanes

## Pickup Lifecycle

1. **Initialization:**
   - Pool pre-spawns configured number of AFuelPickup actors
   - All pickups start deactivated (hidden, collision disabled)

2. **Spawning:**
   - Spawner calls `GetFromPool()` when pickup needed
   - Pool positions pickup in specified lane ahead of war rig
   - `OnActivated()` called automatically (enables collision, makes visible, starts ticking)

3. **Active State:**
   - Pickup scrolls backward with world scroll velocity
   - Collision sphere detects overlap with war rig

4. **Collection:**
   - On overlap with war rig:
     - Apply fuel restoration via GameplayEffect
     - Play pickup sound
     - Spawn particle effect
     - Call `ReturnToPool()` on pool component

5. **Despawning:**
   - Pool checks active pickups each tick
   - If pickup X position < (War Rig X + DespawnDistanceBehind):
     - Call `ReturnToPool()`
     - `OnDeactivated()` called (disables collision, hides, stops ticking)
     - Pickup returns to available pool

## Testing

The following tests are available (registered in `FuelPickupTests.cpp`):

### Economy Category
- **Fuel Pickup - Collection:** Verifies fuel is restored on overlap
- **Fuel Pickup - Spawn Position:** Validates spawn positioning in lanes
- **Fuel Pickup - Sound Playback:** Checks sound effect structure
- **Fuel Pickup - World Scroll:** Validates scroll integration

### Object Pool Category
- **Fuel Pickup - Pooling:** Verifies pickups are recycled correctly
- **Fuel Pickup - Despawn Logic:** Validates despawn at correct distance
- **Fuel Pickup - IPoolableActor:** Checks interface implementation

### GAS Category
- **Fuel Pickup - GameplayEffect:** Validates GE application structure

Run tests using:
```
RunTests Economy
RunTests ObjectPool
RunTests GAS
```

## Integration with Existing Systems

### World Scroll System
Pickups automatically integrate with `UWorldScrollComponent` to scroll backward, creating the illusion of forward movement.

### Lane System
Pickups spawn in lanes defined by the war rig's `ULaneSystemComponent`, using the same Y-axis positions.

### Gameplay Ability System
Fuel restoration integrates with the war rig's `UAbilitySystemComponent` and `UWarRigAttributeSet`:
- Fuel attribute is modified via GameplayEffect or direct attribute modification
- Automatic clamping via `PreAttributeChange` in AttributeSet
- Respects MaxFuel limit

### Object Pooling
Built on the Epic 0.2 object pooling system (`UObjectPoolComponent`), ensuring efficient memory usage and avoiding garbage collection spikes.

## Performance Considerations

- **Object Pooling:** Eliminates runtime allocation/deallocation overhead
- **Auto-Expand:** Pool can grow dynamically if needed (up to MaxPoolSize)
- **Automatic Cleanup:** Pickups that pass behind war rig are automatically returned to pool
- **Efficient Collision:** Uses simple sphere collision with overlap events

## Future Enhancements

Potential improvements for future iterations:
1. **Scrap Pickups:** Extend system to support scrap collection
2. **Pickup Variety:** Different pickup types with varying values
3. **Visual Mesh:** Replace sphere with actual 3D mesh
4. **Magnetic Pickup:** Auto-attract pickups when war rig is nearby
5. **Combo System:** Bonus fuel for collecting multiple pickups in sequence
6. **Difficulty Scaling:** Adjust spawn frequency based on difficulty

## Troubleshooting

### Pickups not spawning
- Verify `InitializePickupPool()` is called in BeginPlay
- Check that war rig and world scroll component references are valid
- Use `DebugShowPickupPool` to verify pool initialization

### Pickups not moving
- Verify world scroll component is active and scrolling
- Check `SetWorldScrollComponent()` was called on pickups
- Enable `DebugShowPickups` to visualize pickup positions

### Fuel not restoring
- Verify war rig has AbilitySystemComponent with UWarRigAttributeSet
- Check that GameplayEffect is configured correctly
- Verify overlap collision is enabled and war rig collision channel is set correctly

### Pool running out of pickups
- Increase initial pool size
- Enable auto-expand in pool configuration
- Check despawn logic is working (pickups should return to pool)
