# Gameplay Tags - Native Tag System

## Overview

Whiteline Nightmare uses **Native Gameplay Tags** defined in C++. This means:
- ✅ **No manual setup required** - Tags are automatically registered
- ✅ **Type-safe** - Use tag constants instead of strings
- ✅ **Compile-time checking** - Typos caught at compile time
- ✅ **Intellisense support** - Auto-completion in your IDE
- ✅ **Version controlled** - Tags live in source code
- ✅ **Performance** - Tags are created once at startup

## How to Use

### In C++ Code

```cpp
#include "Core/WhitelineNightmareGameplayTags.h"

// Add a tag to a container
MyTagContainer.AddTag(WhitelineNightmareGameplayTags::Mount_Cab);

// Check if a container has a tag
if (MyTagContainer.HasTag(WhitelineNightmareGameplayTags::State_Dead))
{
    // Handle dead state
}

// Remove a tag
MyTagContainer.RemoveTag(WhitelineNightmareGameplayTags::State_Moving);

// Use with gameplay effects
GameplayEffectSpec.AddDynamicAssetTag(WhitelineNightmareGameplayTags::Effect_FuelDrain);
```

### In Blueprints

The native tags are automatically visible in the Blueprint editor:
1. In any Gameplay Tag selector, search for the tag name
2. Example: Search "Mount.Cab" to find the cab mount tag
3. The tags appear under their hierarchy (Mount, Ability, State, etc.)

### Adding New Tags

To add new gameplay tags to the project:

1. **Open the header file**:
   `Source/WhitelineNightmare/Public/Core/WhitelineNightmareGameplayTags.h`

2. **Add the declaration** in the appropriate section:
   ```cpp
   /** My new tag description */
   UE_DECLARE_GAMEPLAY_TAG_EXTERN(MyCategory_MyNewTag);
   ```

3. **Open the implementation file**:
   `Source/WhitelineNightmare/Private/Core/WhitelineNightmareGameplayTags.cpp`

4. **Add the definition**:
   ```cpp
   UE_DEFINE_GAMEPLAY_TAG_COMMENT(MyCategory_MyNewTag, "MyCategory.MyNewTag", "My new tag description");
   ```

5. **Recompile** the project

The new tag is now available throughout your project!

## Tag Categories

### Ability Tags (`Ability.*`)
Used to identify and filter gameplay abilities:
- `Ability.LaneChange` - Lane change ability
- `Ability.TurretFire` - Turret firing ability
- `Ability.RaiderAttack` - Raider attack ability
- `Ability.GameOver` - Game over state

**Usage**: Grant to AbilitySystemComponents to activate abilities

### State Tags (`State.*`)
Used to track entity states:
- `State.Moving` - Entity is actively moving
- `State.Dead` - Entity has been destroyed/killed
- `State.LaneChanging` - War rig is currently changing lanes

**Usage**: Add/remove from tag containers to track state changes

### Damage Tags (`Damage.*`)
Used to categorize damage types:
- `Damage.Direct` - Standard damage from weapons/collisions
- `Damage.Explosive` - Area of effect damage
- `Damage.Collision` - Damage from physical collisions

**Usage**: Apply to gameplay effects for damage type filtering

### Effect Tags (`Effect.*`)
Used to identify gameplay effects:
- `Effect.FuelDrain` - Reduces fuel over time
- `Effect.FuelRestore` - Restores fuel (from pickups)
- `Effect.ArmorRestore` - Restores hull/armor
- `Effect.SpeedBoost` - Increases lane change speed

**Usage**: Tag gameplay effects for filtering and identification

### Mount Point Tags (`Mount.*`)
Used to filter which turrets can be mounted where:
- `Mount.Cab` - Mount point on the cab
- `Mount.Trailer` - Mount point on a trailer
- `Mount.Rear` - Mount point at the rear
- `Mount.Heavy` - Can support heavy turrets
- `Mount.AntiAir` - Suitable for anti-air turrets

**Usage**: Automatically assigned to mount points, used for turret placement filtering

**Auto-Assignment**:
- Cab mount points: `Mount.Cab`
- Trailer 1 mount points: `Mount.Trailer`
- Trailer 2 mount points: `Mount.Trailer` + `Mount.Rear`

### Enemy Tags (`Enemy.*`)
Used to categorize enemy types:
- `Enemy.Ground` - Approaches on the road
- `Enemy.Air` - Flies above the war rig
- `Enemy.Boss` - Larger, tougher enemies

**Usage**: Tag enemy actors for filtering and behavior logic

### Pickup Tags (`Pickup.*`)
Used to identify pickup types:
- `Pickup.Fuel` - Fuel pickup
- `Pickup.Scrap` - Scrap/currency pickup
- `Pickup.Armor` - Armor repair pickup

**Usage**: Tag pickup actors for collection logic

### Turret Tags (`Turret.*`)
Used to categorize turret types:
- `Turret.Ballistic` - Standard projectile weapons
- `Turret.Energy` - Laser/energy weapons
- `Turret.Explosive` - Area damage weapons
- `Turret.Support` - Utility/defensive turrets

**Usage**: Tag turret actors, filter by mount point compatibility

## Best Practices

### Do's
- ✅ Use the namespace constants: `WhitelineNightmareGameplayTags::Mount_Cab`
- ✅ Add descriptive comments when defining new tags
- ✅ Group related tags with consistent naming (e.g., `Mount.*`, `Ability.*`)
- ✅ Use tags for filtering and categorization
- ✅ Document tag usage in code comments

### Don'ts
- ❌ Don't use string literals: `FGameplayTag::RequestGameplayTag("Mount.Cab")` - Use the constant instead!
- ❌ Don't create duplicate tags with different names
- ❌ Don't use tags as data storage (use attributes or properties instead)
- ❌ Don't create tags at runtime (define them in the native tag file)

## Tag Hierarchy

Tags use dot notation for hierarchy:
```
Mount
├── Cab
├── Trailer
├── Rear
├── Heavy
└── AntiAir

Ability
├── LaneChange
├── TurretFire
├── RaiderAttack
└── GameOver

State
├── Moving
├── Dead
└── LaneChanging

...etc
```

This allows for parent/child tag queries:
```cpp
// Matches Mount.Cab, Mount.Trailer, Mount.Rear, etc.
if (MyTagContainer.HasTag(FGameplayTag::RequestGameplayTag("Mount")))
{
    // Any mount-related tag
}
```

## Migration from Manual Tags

If you previously set up tags manually in Project Settings:
1. Delete the manual tag definitions
2. The native tags will automatically replace them
3. Update any string-based tag requests to use the constants
4. Recompile and test

## Technical Details

### Registration
Tags are registered automatically during engine startup via the `UE_DEFINE_GAMEPLAY_TAG` macro.

### Memory
Each tag is stored as a singleton `FNativeGameplayTag` and is extremely lightweight.

### Performance
- Tag comparisons are fast (integer comparison)
- Tag registration happens once at startup
- No runtime overhead for tag lookups

## File References

- **Header**: `Source/WhitelineNightmare/Public/Core/WhitelineNightmareGameplayTags.h`
- **Implementation**: `Source/WhitelineNightmare/Private/Core/WhitelineNightmareGameplayTags.cpp`
- **Usage Example**: See `GameDataStructs.h` constructor (mount point tag assignment)

## Support

For questions or to propose new tags, see the project documentation or reach out to the development team.
