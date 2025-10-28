# Whiteline Nightmare - Project Structure & Development Guide

## Overview
Whiteline Nightmare is a Mad Max-inspired tower defense game built in Unreal Engine 5.6. Players command a war rig driving through post-apocalyptic wastelands, defending against raiders while managing resources.

## Core Development Philosophy

### Code-First Approach
- **C++ Primary**: Implement all game logic in C++
- **Blueprints Limited**: Only use for:
  - Data assets (GameplayEffects, data tables)
  - Content (materials, particles, sounds)
  - Level design and placement

### Test-Driven Development
- Write test functions for all major systems
- Use the automated testing framework (`TestManager`, `TestingGameMode`)
- Run tests frequently during development
- All tests must pass before committing

### Defensive Programming
- **Input Validation**: Check all inputs for validity
- **Null Checks**: Always validate pointers before dereferencing
- **Bounds Checking**: Validate array indices
- **Error Logging**: Use `UE_LOG` with appropriate verbosity
- **Graceful Failure**: Don't crash - return safe defaults
- **Assertions**: Use `check()`, `checkSlow()`, `ensure()` for invariants

## Project Structure

### Source Code Organization
All source code follows Unreal's Public/Private convention:
- **Public/**: All header files (.h)
- **Private/**: All implementation files (.cpp)

```
Source/WhitelineNightmare/
├── Public/
│   ├── Core/                      # Core game systems
│   │   ├── GameDataStructs.h      # Data table structures
│   │   ├── WhitelineNightmareGameMode.h
│   │   ├── WarRigPlayerController.h
│   │   ├── WarRigHUD.h
│   │   ├── WarRigPawn.h           # War rig pawn (stationary vehicle)
│   │   └── ObjectPoolComponent.h  # Object pooling system
│   ├── Vehicles/                  # War rig components
│   ├── Turrets/                   # Turret base and types
│   ├── Enemies/                   # Raider base and spawner
│   ├── Obstacles/                 # Obstacle base and spawner
│   ├── Pickups/                   # Fuel and scrap pickups
│   ├── GAS/                       # Gameplay Ability System
│   │   ├── Attributes/            # GAS attributes
│   │   └── Abilities/             # GAS abilities
│   ├── UI/                        # UI widgets (pure C++)
│   ├── World/                     # Scrolling world system
│   ├── Testing/                   # Automated tests
│   │   ├── TestMacros.h           # Test assertion macros
│   │   ├── TestManager.h          # Test registration/execution
│   │   └── TestingGameMode.h      # Auto-run test mode
│   └── ObjectPooling/             # Object pooling system
├── Private/
│   └── [mirrors Public/ structure for .cpp files]
└── WhitelineNightmare.Build.cs
```

### Content Organization
```
Content/
├── Art/                           # Art assets
│   ├── Meshes/                    # 3D mesh assets
│   ├── Textures/                  # Texture assets
│   └── Materials/                 # Material assets and instances
├── Audio/                         # Audio assets
│   ├── Music/                     # Background music and ambient audio
│   └── SFX/                       # Sound effects
├── Blueprints/                    # Blueprint classes (data-only, C++ parents)
│   ├── Vehicles/                  # Vehicle blueprint classes
│   ├── Turrets/                   # Turret blueprint classes
│   ├── Enemies/                   # Enemy blueprint classes
│   ├── Obstacles/                 # Obstacle blueprint classes
│   ├── Pickups/                   # Pickup blueprint classes
│   └── UI/                        # UI widget blueprints
├── Characters/                    # Character-specific assets
├── Data/                          # Data tables and tuning assets
│   ├── DT_GameplayBalance.uasset
│   ├── DT_TurretData.uasset
│   ├── DT_EnemyData.uasset
│   ├── DT_PickupData.uasset
│   ├── DT_WorldScrollData.uasset
│   └── DT_WarRigData.uasset
├── Environment/                   # Environment assets (landscape, props)
├── Maps/                          # Level maps
│   ├── TestMap.umap              # Test map for automated testing
│   └── TestMap_2.umap            # Additional test map
├── UI/                            # UI-specific assets
│   ├── Widgets/                   # UMG widget assets
│   ├── Textures/                  # UI textures (icons, backgrounds)
│   └── Fonts/                     # Font assets
├── VFX/                           # Visual effects
│   ├── Particles/                 # Cascade particle systems
│   └── Niagara/                   # Niagara particle systems
├── Vehicles/                      # Vehicle-specific assets (war rig)
└── Weapons/                       # Weapon and turret assets
```

### Root Level Organization
```
WhitelineNightmare/
├── Config/                        # Configuration files
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   ├── DefaultInput.ini
│   └── DefaultGameplayTags.ini
├── Content/                       # [See Content Organization above]
├── Documentation/                 # Additional documentation and design docs
│   ├── README.md                  # Documentation overview
│   └── WarRigSetup.md            # War rig setup guide
├── Plugins/                       # Project-specific plugins
├── Source/                        # [See Source Code Organization above]
├── .gitignore                     # Git ignore file
├── .vsconfig                      # Visual Studio configuration
├── PROJECT_STRUCTURE.md           # This file
├── README.md                      # Project overview
├── TEST_INSTRUCTIONS.md           # Testing framework guide
└── WhitelineNightmare.uproject    # Unreal project file
```

## Data Table Structures

All tunable gameplay values are stored in data tables. See `GameDataStructs.h` for definitions.

### FGameplayBalanceData
Core gameplay tuning values:
- Fuel drain rate
- Lane change fuel cost
- Win distance
- Scroll speed
- Lane change duration
- Lane width

### FTurretData
Turret definitions:
- Display name and description
- Mesh reference
- Base damage, fire rate, range
- Build and upgrade costs

### FEnemyData
Enemy raider definitions:
- Display name
- Mesh reference
- Health, speed, attack damage
- Spawn weight
- Scrap reward

### FPickupData
Fuel and scrap pickup definitions:
- Display name
- Mesh reference
- Fuel/scrap amounts
- Spawn weight

### FWorldScrollData
World scrolling and pooling settings:
- Tile size and pool size
- Spawn/despawn distances
- Pool sizes for enemies, obstacles, pickups

### FWarRigData
War rig configurations:
- Display name and description
- **Mesh sections** (TArray<UStaticMesh*>) - cab + trailers
- Mount points for turrets
- Base stats (MaxHull, LaneChangeFuelCost, LaneChangeSpeed)
- Visual properties (materials, colors)
- Camera settings (distance, pitch)
- Unlock cost

**CRITICAL**: Supports multiple rig types via data table rows. MVP uses "SemiTruck" configuration.

### FMountPointData
Turret mount point definition:
- Transform (position and rotation)
- Allowed facing directions (0-7 compass)
- Mount tags for restrictions
- Display name for UI

## World Movement System

**CRITICAL**: The war rig is stationary in world space. The illusion of movement is created by scrolling the world backward past the rig.

### Key Concepts:
1. **War Rig**: Remains at a fixed world position
2. **Ground Tiles**: Scroll from ahead of rig toward behind it
3. **Objects**: Enemies, obstacles, pickups spawn ahead and scroll toward rig
4. **Object Pooling**: Reuse objects instead of spawning/destroying
5. **Despawning**: Remove objects that pass behind the camera

### Implementation Notes:
- Scroll speed controlled by `FGameplayBalanceData::ScrollSpeed`
- Object pooling required for performance (see ObjectPooling/)
- Spawn distance controlled by `FWorldScrollData::TileSpawnDistance`
- Despawn distance controlled by `FWorldScrollData::TileDespawnDistance`

## Gameplay Ability System (GAS)

### Enabled Plugins
- GameplayAbilities
- GameplayTags
- GameplayTasks

### Gameplay Tags
Defined in `Config/DefaultGameplayTags.ini`:
- `Ability.LaneChange`: Lane change ability
- `Ability.TurretFire`: Turret firing ability
- `Ability.RaiderAttack`: Raider attack ability
- `Ability.GameOver`: Game over state
- `State.Moving`: War rig moving state
- `State.Dead`: Entity dead state
- `Damage.Direct`: Direct damage type
- `Effect.FuelDrain`: Fuel drain effect
- `Effect.FuelRestore`: Fuel restore effect

### GAS Structure
- **Attributes**: Define in `Public/GAS/Attributes/`
- **Abilities**: Define in `Public/GAS/Abilities/`
- **Effects**: Create as data assets in Content/Data/

## Automated Testing Framework

### Test Macros (TestMacros.h)
```cpp
TEST_ASSERT(Condition, Message)              // Assert condition is true
TEST_EQUAL(A, B, Message)                    // Assert A == B
TEST_NEARLY_EQUAL(A, B, Tolerance, Message)  // Assert A ~= B (floats)
TEST_NOT_NULL(Pointer, Message)              // Assert pointer is not null
TEST_NULL(Pointer, Message)                  // Assert pointer is null
TEST_SUCCESS(TestName)                       // Mark test as passed
```

### Test Manager (TestManager.h)
Singleton that manages test registration and execution:
```cpp
UTestManager* TestManager = UTestManager::Get(this);
TestManager->RegisterTest(TEXT("MyTest"), ETestCategory::Movement, &MyTestFunction);
TestManager->RunAllTests();
TestManager->RunTestCategory(ETestCategory::Combat);
TestManager->RunTest(TEXT("MyTest"));
```

### Testing Game Mode (TestingGameMode.h)
Special game mode that auto-runs tests on level load:
1. Create a test map
2. Set `ATestingGameMode` as the game mode
3. Configure `bAutoRunTests` and `AutoTestCategory`
4. Tests run automatically after `TestStartDelay`

### Console Command
Run tests from console:
```
RunTests                  // Run all tests
RunTests Movement         // Run movement tests
RunTests Combat           // Run combat tests
RunTests Economy          // Run economy tests
RunTests Spawning         // Run spawning tests
RunTests ObjectPool       // Run object pooling tests
RunTests GAS              // Run GAS tests
```

### Writing Tests
```cpp
static bool MyTestFunction()
{
    TEST_ASSERT(true, "This should always pass");

    int32 A = 42;
    int32 B = 42;
    TEST_EQUAL(A, B, "Values should be equal");

    float X = 1.0f;
    float Y = 1.00001f;
    TEST_NEARLY_EQUAL(X, Y, 0.001f, "Floats should be nearly equal");

    UObject* Obj = NewObject<UObject>();
    TEST_NOT_NULL(Obj, "Object should not be null");

    TEST_SUCCESS("MyTestFunction");
}
```

## Core Classes

### AWhitelineNightmareGameMode
Main game mode that manages:
- Distance tracking
- Win/lose conditions
- Game state

Key methods:
- `AddDistanceTraveled(float)`: Track distance progress
- `TriggerGameOver(bool)`: End game (win/lose)
- `HasPlayerWon()`: Check win condition

### AWarRigPlayerController
Player controller that manages:
- Input handling
- Resource management (scrap)
- UI interaction

Key methods:
- `AddScrap(int32)`: Add/remove scrap
- `SpendScrap(int32)`: Attempt purchase
- `CanAfford(int32)`: Check if affordable
- `OnGameOver(bool)`: Handle game over

### AWarRigHUD
HUD that manages UI display:
- Fuel display
- Armor/health display
- Scrap display
- Distance progress
- Game over screen

Key methods:
- `UpdateFuelDisplay(float, float)`: Update fuel UI
- `UpdateArmorDisplay(float, float)`: Update armor UI
- `UpdateScrapDisplay(int32)`: Update scrap UI
- `UpdateDistanceDisplay(float, float)`: Update progress UI
- `ShowGameOverScreen(bool)`: Show game over

### AWarRigPawn
The player's stationary war rig vehicle that stays at world origin (0,0,0).

**CRITICAL DESIGN**: The war rig NEVER moves. The world scrolls past it.

Key features:
- **Data-driven configuration** from DT_WarRigData
- **AbilitySystemComponent** for GAS integration
- **Dynamic mesh spawning** (cab + trailers from data table)
- **Mount point system** for turret attachment
- **Camera setup** (SpringArm + Camera, top-down view)
- **Lane system integration** (to be implemented)

Key methods:
- `LoadWarRigConfiguration(FName)`: Load rig from data table
- `CreateMeshComponents()`: Spawn mesh sections
- `CreateMountPoints()`: Spawn mount point components
- `SetupCamera()`: Configure camera from data
- `ValidateWarRigData()`: Validate configuration data

Testing commands:
- `TestWarRigDataLoading()`: Test data table loading
- `TestWarRigSpawn()`: Test mesh spawning
- `TestMountPointSetup()`: Test mount point creation
- `TestCameraSetup()`: Test camera configuration
- `TestStationaryPosition()`: Verify rig stays at origin

Debug commands:
- `DebugShowWarRigBounds()`: Toggle bounds visualization
- `DebugShowMountPoints()`: Toggle mount point visualization
- `DebugReloadWarRigData()`: Reload configuration

See `Documentation/WarRigSetup.md` for detailed setup instructions.

## Development Workflow

### 1. Planning
- Break features into small, testable units
- Identify potential edge cases
- Plan test cases before implementation

### 2. Implementation
- Write header file with documentation
- Implement in C++ with defensive checks
- Add comprehensive error logging
- Validate all inputs
- Handle failure cases gracefully

### 3. Testing
- Write test functions for the feature
- Register tests with TestManager
- Run tests and verify all pass
- Test edge cases and error conditions

### 4. Code Review
- Check for null pointer dereferences
- Verify bounds checking on arrays
- Ensure error logging is present
- Confirm graceful failure handling
- Review test coverage

### 5. Commit
- All tests must pass
- No compiler warnings
- Code follows style guide
- Commit message describes changes clearly

## Code Style Guidelines

### Naming Conventions
- Classes: `AMyActor`, `UMyObject`, `FMyStruct`
- Interfaces: `IMyInterface`
- Enums: `EMyEnum`
- Member variables: `MyVariable`, `bIsFlag` (bool)
- Functions: `MyFunction()`, `GetValue()`, `SetValue()`
- Constants: `MaxValue`, `DEFAULT_SIZE`

### Header Files
- Always include copyright header
- Use `#pragma once`
- Include guards for compatibility
- Forward declare when possible
- Minimize includes in headers

### Logging
```cpp
DEFINE_LOG_CATEGORY_STATIC(LogMySystem, Log, All);

UE_LOG(LogMySystem, Log, TEXT("Info message"));
UE_LOG(LogMySystem, Warning, TEXT("Warning message"));
UE_LOG(LogMySystem, Error, TEXT("Error message"));
UE_LOG(LogMySystem, Verbose, TEXT("Verbose message"));
```

### Input Validation Example
```cpp
bool AMyActor::ProcessValue(float Value)
{
    // Validate input
    if (Value < 0.0f)
    {
        UE_LOG(LogMySystem, Error, TEXT("ProcessValue: Negative value not allowed: %.2f"), Value);
        return false;
    }

    if (Value > MaxValue)
    {
        UE_LOG(LogMySystem, Warning, TEXT("ProcessValue: Value %.2f exceeds max %.2f, clamping"), Value, MaxValue);
        Value = MaxValue;
    }

    // Process value
    // ...

    return true;
}
```

## Build Configuration

### Dependencies
Listed in `WhitelineNightmare.Build.cs`:
- **PublicDependencyModuleNames**:
  - Core
  - CoreUObject
  - Engine
  - InputCore
  - EnhancedInput
  - GameplayAbilities
  - GameplayTags
  - GameplayTasks

- **PrivateDependencyModuleNames**:
  - Slate
  - SlateCore
  - UMG

## Version Control

### Git Workflow
1. Create feature branch from develop
2. Implement feature with tests
3. Run all tests (must pass)
4. Commit with clear message
5. Push to remote
6. Create pull request

### .gitignore
Properly configured for UE5.6:
- Ignores Binaries/, Intermediate/, Saved/
- Ignores generated files (.sln, .suo, etc.)
- Ignores DerivedDataCache/
- Tracks Config/ and Content/

## Knowledge Gap Protocol

If unfamiliar with any Unreal Engine system:
1. **STOP** implementation immediately
2. Create a detailed prompt asking for clarification
3. Format: "I need information about [SYSTEM] to implement [FEATURE]. Please provide a prompt I can give to the Unreal AI Assistant to learn: [SPECIFIC QUESTIONS]"
4. Wait for educational response
5. Proceed with correct implementation

## Next Steps

This project structure is now ready for feature implementation. Future epics should:

1. Implement war rig vehicle with GAS attributes (fuel, armor)
2. Create lane change ability using GAS
3. Implement world scrolling system with object pooling
4. Create turret base class and types
5. Implement enemy spawning system
6. Create pickup system (fuel and scrap)
7. Implement obstacle spawning
8. Create UI widgets in C++ using UMG
9. Add visual effects and audio
10. Implement save/load system

Each epic should follow the development workflow above and include comprehensive tests.
