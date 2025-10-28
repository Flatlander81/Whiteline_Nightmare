# World Scroll System Documentation

## Overview

The World Scroll System simulates forward movement by scrolling the world backward past the stationary war rig. This creates the illusion of the war rig driving forward through the wasteland.

**CRITICAL CONCEPT**: The war rig is STATIONARY. The world scrolls BACKWARD past it.

## Components

### UWorldScrollComponent

An ActorComponent that manages world scrolling to simulate forward movement. This component should be attached to the GameMode or a persistent level manager actor (NOT the war rig).

**Location**:
- Header: `Source/WhitelineNightmare/Public/Core/WorldScrollComponent.h`
- Implementation: `Source/WhitelineNightmare/Private/Core/WorldScrollComponent.cpp`

### FWorldScrollData Struct

A data table row struct that configures the world scroll system. This has been extended with scroll velocity properties.

**Location**: `Source/WhitelineNightmare/Public/Core/GameDataStructs.h`

**Properties**:
- **ScrollSpeed** (float): World scroll speed in units per second. Default: 1000
- **bScrollEnabled** (bool): Whether scrolling is enabled at start. Default: true
- **ScrollDirection** (FVector): Normalized direction vector. Default: (-1, 0, 0) for backward along X axis
- **TileSize**, **TilePoolSize**, etc.: Existing tile and object pool configuration

## Setup Instructions

### 1. Create Data Table Asset

1. In Unreal Editor, right-click in Content Browser
2. Select **Miscellaneous → Data Table**
3. Choose **FWorldScrollData** as the row structure
4. Name it `DT_WorldScrollData`
5. Save in `Content/Data/` folder

### 2. Configure Data Table Row

1. Open `DT_WorldScrollData`
2. Add a new row with name: **DefaultScroll**
3. Configure the following properties:
   - **ScrollSpeed**: 1000.0 (units per second)
   - **bScrollEnabled**: true
   - **ScrollDirection**: X=-1.0, Y=0.0, Z=0.0
   - Configure tile and pool properties as needed
4. Save the data table

### 3. Add Component to GameMode

**Option A: In Blueprint (Recommended for quick setup)**

1. Open `BP_WhitelineNightmareGameMode` in Unreal Editor
2. Click **Add Component** → **World Scroll Component**
3. In the Details panel, set:
   - **World Scroll Data Table**: Point to `DT_WorldScrollData`
   - **Data Table Row Name**: `DefaultScroll`
4. Compile and save

**Option B: In C++ (Recommended for production)**

Edit `WhitelineNightmareGameMode.h`:
```cpp
#include "Core/WorldScrollComponent.h"

UCLASS()
class AWhitelineNightmareGameMode : public AGameModeBase
{
    GENERATED_BODY()

protected:
    // World scroll component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Scroll")
    TObjectPtr<UWorldScrollComponent> WorldScrollComponent;
};
```

Edit `WhitelineNightmareGameMode.cpp` constructor:
```cpp
AWhitelineNightmareGameMode::AWhitelineNightmareGameMode()
{
    // Create world scroll component
    WorldScrollComponent = CreateDefaultSubobject<UWorldScrollComponent>(TEXT("WorldScrollComponent"));
}
```

## Usage by Other Systems

### Querying Scroll Velocity

All scrolling actors (ground tiles, enemies, obstacles, pickups) should query the scroll component to get the current scroll velocity.

**Example: Ground Tile Movement**

```cpp
// In your ground tile actor's Tick function
void AGroundTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get the world scroll component from game mode
    AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && GameMode->WorldScrollComponent)
    {
        // Get scroll velocity
        FVector ScrollVelocity = GameMode->WorldScrollComponent->GetScrollVelocity();

        // Move tile backward
        FVector NewLocation = GetActorLocation() + (ScrollVelocity * DeltaTime);
        SetActorLocation(NewLocation);
    }
}
```

**Example: Enemy Movement**

```cpp
// Enemies add scroll velocity to their own movement
void AEnemyRaider::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && GameMode->WorldScrollComponent)
    {
        FVector ScrollVelocity = GameMode->WorldScrollComponent->GetScrollVelocity();

        // Calculate enemy's own movement toward war rig
        FVector EnemyMovement = GetMovementDirection() * EnemySpeed * DeltaTime;

        // Combine with scroll velocity
        FVector TotalMovement = EnemyMovement + (ScrollVelocity * DeltaTime);

        SetActorLocation(GetActorLocation() + TotalMovement);
    }
}
```

### Getting Distance Traveled

The component tracks total virtual distance traveled, which can be used for the win condition:

```cpp
// Check if player has traveled far enough to win
AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
if (GameMode && GameMode->WorldScrollComponent)
{
    float DistanceTraveled = GameMode->WorldScrollComponent->GetDistanceTraveled();
    float WinDistance = 10000.0f; // Example win distance

    if (DistanceTraveled >= WinDistance)
    {
        // Player wins!
        GameMode->TriggerGameOver(true);
    }
}
```

## API Reference

### Query Functions

| Function | Return Type | Description |
|----------|-------------|-------------|
| `GetScrollVelocity()` | FVector | Returns current scroll velocity vector (direction × speed). Returns zero if not scrolling. |
| `GetScrollSpeed()` | float | Returns current scroll speed magnitude in units/second. |
| `GetDistanceTraveled()` | float | Returns accumulated virtual distance in units. |
| `IsScrolling()` | bool | Returns true if scrolling is currently active. |

### Control Functions

| Function | Parameters | Description |
|----------|------------|-------------|
| `SetScrollSpeed()` | float NewSpeed | Change scroll speed at runtime. Negative values are clamped to 0. |
| `SetScrolling()` | bool bEnabled | Start or pause scrolling. |
| `ResetDistance()` | none | Reset distance counter to 0 (for level restarts). |

### Debug Commands

All debug commands can be executed in the console (` key):

| Command | Parameters | Description |
|---------|------------|-------------|
| `DebugSetScrollSpeed` | float | Change scroll speed. Example: `DebugSetScrollSpeed 500` |
| `DebugToggleScroll` | none | Toggle scrolling on/off. |
| `DebugShowScrollInfo` | none | Display current scroll state in log. |
| `DebugResetDistance` | none | Reset distance counter. |

### Test Commands

| Command | Description |
|---------|-------------|
| `TestScrollSpeedConsistency` | Verify scroll speed remains constant. |
| `TestDistanceAccumulation` | Verify distance accumulates correctly. |
| `TestScrollPause` | Verify scrolling can be paused and resumed. |
| `TestScrollVelocity` | Verify velocity calculation is correct. |
| `TestScrollSpeedChange` | Verify runtime speed changes work. |
| `TestWorldScrollAll` | Run all world scroll tests with summary. |

## Testing

### Manual Testing

1. **Test Basic Scrolling**:
   ```
   DebugShowScrollInfo
   ```
   Expected output: Speed = 1000, Scrolling = true, Direction = (-1, 0, 0)

2. **Test Speed Change**:
   ```
   DebugSetScrollSpeed 500
   DebugShowScrollInfo
   ```
   Expected: Speed should be 500

3. **Test Pause/Resume**:
   ```
   DebugToggleScroll
   DebugShowScrollInfo
   DebugToggleScroll
   DebugShowScrollInfo
   ```
   Expected: Scrolling should toggle between true/false

4. **Test Distance Tracking**:
   - Note current distance
   - Wait a few seconds
   - Check distance again
   ```
   DebugShowScrollInfo
   ```
   Expected: Distance should increase over time

### Automated Testing

Run all tests from console:
```
TestWorldScrollAll
```

This will execute all 5 test functions and display a summary:
- TestScrollSpeedConsistency
- TestDistanceAccumulation
- TestScrollPause
- TestScrollVelocity
- TestScrollSpeedChange

Tests are also registered with the test manager in the "Movement" category.

## Common Patterns

### Pattern 1: Single Source of Truth

The WorldScrollComponent is the **single source of truth** for scroll speed. Never hardcode scroll speeds in other systems.

**BAD**:
```cpp
// Don't do this!
FVector Movement = FVector(-1000.0f, 0.0f, 0.0f) * DeltaTime;
```

**GOOD**:
```cpp
// Query the component
FVector ScrollVelocity = WorldScrollComponent->GetScrollVelocity();
FVector Movement = ScrollVelocity * DeltaTime;
```

### Pattern 2: Safe Component Access

Always check for null before accessing the component:

```cpp
AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
if (GameMode && GameMode->WorldScrollComponent)
{
    FVector ScrollVelocity = GameMode->WorldScrollComponent->GetScrollVelocity();
    // Use velocity...
}
```

### Pattern 3: Pause Entire Game

To pause all world movement:

```cpp
// Pause scrolling
WorldScrollComponent->SetScrolling(false);

// This automatically stops:
// - Ground tiles from moving
// - Enemies from advancing
// - Obstacles from scrolling
// - Distance accumulation
```

## Performance Considerations

- **Tick Performance**: The component only updates distance traveled each tick. The calculation is very lightweight (one multiplication per frame).
- **Query Performance**: All getter functions are inline and return cached values. No recalculation happens on query.
- **Memory**: The component has minimal memory footprint (< 100 bytes).

## Troubleshooting

### Problem: Objects not scrolling

**Solution**: Verify objects are querying `GetScrollVelocity()` and applying it to their movement.

### Problem: Scroll speed incorrect

**Solution**:
1. Check data table configuration
2. Run `DebugShowScrollInfo` to verify current speed
3. Ensure no other code is overriding the speed

### Problem: Distance not accumulating

**Solution**:
1. Check `IsScrolling()` returns true
2. Verify `ScrollSpeed > 0`
3. Run `TestDistanceAccumulation` to diagnose

### Problem: Component not found

**Solution**:
1. Verify component is added to GameMode
2. Check GameMode is set correctly in World Settings
3. Ensure proper null checking in code

## Integration Timeline

The World Scroll System should be integrated in this order:

1. **Setup** (Completed): Component and data table created
2. **Ground Tiles**: Make tiles query scroll velocity and move backward
3. **Enemies**: Add scroll velocity to enemy movement
4. **Obstacles**: Make obstacles scroll backward
5. **Pickups**: Make pickups scroll toward player
6. **Win Condition**: Use `GetDistanceTraveled()` for victory check
7. **UI**: Display distance traveled to player
8. **Game Events**: Increase scroll speed over time for difficulty ramping

## Future Enhancements

Possible future additions:

- **Speed Curves**: Use curve assets to define speed changes over time
- **Events**: OnScrollSpeedChanged, OnDistanceMilestone delegates
- **Multipliers**: Temporary speed boosts/slowdowns
- **Zones**: Different scroll speeds in different areas
- **Replay**: Record and playback scroll data

## Example: Complete Integration

Here's a complete example of integrating the scroll system with a ground tile spawner:

```cpp
void AGroundTileSpawner::SpawnTile()
{
    // Spawn tile ahead of war rig
    FVector SpawnLocation = WarRig->GetActorLocation() + FVector(3000.0f, 0.0f, 0.0f);
    AGroundTile* Tile = GetWorld()->SpawnActor<AGroundTile>(TileClass, SpawnLocation, FRotator::ZeroRotator);

    if (Tile)
    {
        // Tile will automatically scroll backward via its Tick function
        ActiveTiles.Add(Tile);
    }
}

void AGroundTile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get scroll velocity from game mode
    AWhitelineNightmareGameMode* GameMode = Cast<AWhitelineNightmareGameMode>(GetWorld()->GetAuthGameMode());
    if (GameMode && GameMode->WorldScrollComponent)
    {
        FVector ScrollVelocity = GameMode->WorldScrollComponent->GetScrollVelocity();
        AddActorWorldOffset(ScrollVelocity * DeltaTime);

        // Despawn if behind war rig
        if (GetActorLocation().X < WarRig->GetActorLocation().X - 1000.0f)
        {
            Destroy();
        }
    }
}
```

## Conclusion

The World Scroll System provides a clean, performant, and testable way to simulate forward movement. By centralizing scroll velocity in a single component, all game systems can coordinate seamlessly to create the illusion of the war rig driving through the wasteland.

For questions or issues, refer to the test functions for expected behavior, or run `TestWorldScrollAll` to verify the system is working correctly.
