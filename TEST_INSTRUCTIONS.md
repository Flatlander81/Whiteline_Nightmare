# Object Pool System - Testing Instructions

## Quick Test (Automated Tests Only) - **RECOMMENDED**

This is the fastest and easiest way to verify the object pooling system works.

### Method 1: Any Map (Easiest)
1. Open `WhitelineNightmare.uproject` in Unreal Engine 5.6
2. Click **Yes** when prompted to rebuild modules
3. Wait for compilation to complete
4. Open ANY map (or create a new empty level)
5. Click **Play** button in toolbar (or press Alt+P)
6. Press **`** (backtick/tilde key, usually above Tab) to open console
7. Type: `RunTests ObjectPool`
8. Press **Enter**
9. Open **Output Log** if not visible: **Window → Developer Tools → Output Log**
10. Look for test results

**Expected Result:** All 8 tests should pass
```
TEST PASSED: ObjectPoolTest_Initialization
TEST PASSED: ObjectPoolTest_GetFromPool
TEST PASSED: ObjectPoolTest_ReturnToPool
TEST PASSED: ObjectPoolTest_PoolExhaustion
TEST PASSED: ObjectPoolTest_PoolReuse
TEST PASSED: ObjectPoolTest_ActiveCount
TEST PASSED: ObjectPoolTest_AutoExpand
TEST PASSED: ObjectPoolTest_ResetPool
```

---

## Comprehensive Test (Visual Verification)

### 1. Create Test Map (First Time)
1. **File → New Level → Empty Level**
2. Save as `Content/Maps/PoolTestMap`

**Note:** The TestingGameMode will auto-run tests by default when you press Play. To change which tests run, use the console command method instead (see Quick Test section).

### 2. Create Visual Test Actor (Optional)

#### A. Create Blueprint Actor
1. **Content Browser → Add → Blueprint Class**
2. Choose **Actor** as parent
3. Name it `BP_PoolTestActor`
4. Open the blueprint

#### B. Add Visual Components
1. Add a **Static Mesh Component**
2. Set mesh to `Engine Content → BasicShapes → Cube`
3. Scale to (0.5, 0.5, 0.5)
4. Set material to a bright color

#### C. Implement IPoolableActor Interface
1. **Class Settings → Interfaces → Add → PoolableActor**
2. Implement interface events:
   - **OnActivated**: Print "Actor Activated"
   - **OnDeactivated**: Print "Actor Deactivated"
   - **ResetState**: Print "Actor Reset"

### 3. Create Pool Manager Actor

#### A. Create Blueprint Actor
1. **Content Browser → Add → Blueprint Class → Actor**
2. Name it `BP_PoolManager`
3. Open the blueprint

#### B. Add Pool Component
1. **Add Component → Object Pool Component**
2. Name it `TestPool`

#### C. Setup Pool in BeginPlay
1. Open **Event Graph**
2. From **BeginPlay** event:
3. Call **Initialize** on `TestPool`:
   - **Actor Class**: `BP_PoolTestActor`
   - **Pool Config**:
     - Pool Size: 5
     - Auto Expand: false
     - Spawn Distance Ahead: 2000
     - Despawn Distance Behind: 1000

#### D. Test Pool Operations
Add these test sequences:

**Sequence 1: Get 3 Actors**
```
BeginPlay
  → Delay 1.0s
  → Get From Pool (Location: 0,0,100) → Store as Actor1
  → Delay 0.5s
  → Get From Pool (Location: 0,200,100) → Store as Actor2
  → Delay 0.5s
  → Get From Pool (Location: 0,400,100) → Store as Actor3
  → Print String: "Active: " + Get Active Count
  → Print String: "Available: " + Get Available Count
```

**Sequence 2: Return Actors**
```
  → Delay 2.0s
  → Return To Pool (Actor2)
  → Print String: "Returned Actor2"
  → Print String: "Active: " + Get Active Count
  → Delay 1.0s
  → Return To Pool (Actor1)
  → Return To Pool (Actor3)
  → Print String: "Returned all actors"
```

### 4. Run Visual Test
1. Drag `BP_PoolManager` into the level
2. Click **Play**
3. Enable debug visualization:
   - Press ` (tilde)
   - Type: `DebugShowPools`
   - You should see green/red spheres

**Expected Behavior:**
- Green spheres appear at origin (5 total) - available objects
- After 1s: First actor appears at (0,0,100) - turns red
- After 1.5s: Second actor appears at (0,200,100) - turns red
- After 2s: Third actor appears at (0,400,100) - turns red
- Output shows: "Active: 3, Available: 2"
- After 4s: Actor2 disappears (returned to pool) - turns green, moves to origin
- Output shows: "Active: 2, Available: 3"
- After 5s: All actors returned - all green at origin
- Output shows: "Active: 0, Available: 5"

### 5. Verify Console Commands

#### List Tests
```
> ListTests
```
Should show ObjectPool category with 8 tests

#### Run Specific Test
```
> RunTests ObjectPool
```
Should run all ObjectPool tests and show results

#### Debug Visualization
```
> DebugShowPools
```
Toggles sphere visualization (green = available, red = active)

---

## Troubleshooting

### Compilation Errors
**Problem:** "Cannot find ObjectPoolComponent.h"
**Solution:**
- Close Unreal Editor
- Delete `Intermediate/`, `Binaries/`, and `Saved/` folders
- Right-click `.uproject → Generate Visual Studio project files`
- Recompile

### Tests Not Found
**Problem:** "No tests registered"
**Solution:**
- Ensure you're running a Development build (not Shipping)
- Tests are only compiled in `!UE_BUILD_SHIPPING`
- Restart editor

### Debug Visualization Not Showing
**Problem:** Spheres not visible
**Solution:**
- Make sure pool is initialized
- Verify actors are spawned (check Active/Available counts)
- Try `Show Collision` in viewport to see debug shapes
- Enable `DebugShowPools` console command

### Interface Not Working
**Problem:** "Does not implement IPoolableActor"
**Solution:**
- In Blueprint, check **Class Settings → Interfaces**
- Must have `PoolableActor` in the list
- Implement all three functions: OnActivated, OnDeactivated, ResetState

---

## Success Criteria

✅ **All 8 automated tests pass**
✅ **Pool initializes with correct size** (5 available, 0 active)
✅ **GetFromPool returns valid actors** (decrements available, increments active)
✅ **ReturnToPool deactivates actors** (increments available, decrements active)
✅ **Pool exhaustion handled** (returns null when empty, no auto-expand)
✅ **Same actor instances reused** (pool reuse test passes)
✅ **Auto-expand works** (pool grows when enabled)
✅ **Debug visualization works** (green/red spheres appear correctly)

---

## Console Commands Reference

| Command | Description |
|---------|-------------|
| `RunTests` | Run all tests |
| `RunTests ObjectPool` | Run only ObjectPool tests |
| `ListTests` | Show all registered tests |
| `DebugShowPools` | Toggle pool visualization |

---

## File Locations

- **Headers**: `Source/WhitelineNightmare/Public/Core/`
  - `ObjectPoolTypes.h` - Config struct and interface
  - `ObjectPoolComponent.h` - Pool component

- **Implementation**: `Source/WhitelineNightmare/Private/Core/`
  - `ObjectPoolComponent.cpp` - Pool logic

- **Tests**: `Source/WhitelineNightmare/Private/Testing/`
  - `ObjectPoolTests.cpp` - All 8 automated tests

---

## Next Steps

After verifying the object pool works:
1. Use it for enemy pooling
2. Use it for obstacle pooling
3. Use it for pickup pooling
4. Use it for projectile pooling
5. Use it for ground tile pooling

**Integration Example:**
```cpp
// In your spawn manager
UObjectPoolComponent* EnemyPool;

void ASpawnManager::BeginPlay()
{
    // Create pool
    EnemyPool = CreateDefaultSubobject<UObjectPoolComponent>(TEXT("EnemyPool"));

    // Configure
    FObjectPoolConfig Config;
    Config.PoolSize = 50;
    Config.bAutoExpand = true;
    Config.MaxPoolSize = 100;

    // Initialize
    EnemyPool->Initialize(AEnemyRaider::StaticClass(), Config);
}

void ASpawnManager::SpawnEnemy(FVector Location)
{
    AActor* Enemy = EnemyPool->GetFromPool(Location, FRotator::ZeroRotator);
    if (Enemy)
    {
        // Enemy is ready to use!
    }
}

void ASpawnManager::DespawnEnemy(AActor* Enemy)
{
    EnemyPool->ReturnToPool(Enemy);
}
```
