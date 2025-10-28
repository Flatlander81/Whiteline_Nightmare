# World Scroll System - Setup and Testing Guide

## Overview

The world scrolling system simulates forward movement by scrolling the world backward past a **stationary war rig**. This guide provides step-by-step instructions for setting up and testing the system.

---

## Part 1: Initial Project Setup

### Step 1: Open Project and Compile

1. **Open Unreal Editor**
   - Launch Unreal Engine 5.5+
   - Open `WhitelineNightmare.uproject`

2. **Compile C++ Code**
   - Wait for Unreal Editor to fully load
   - If prompted to rebuild modules, click **Yes**
   - Alternatively, compile from your IDE (Visual Studio/Rider)
   - Wait for compilation to complete successfully

3. **Verify Compilation**
   - Check Output Log (Window > Developer Tools > Output Log)
   - Look for: `LogCompile: Success - 0 error(s), 0 warning(s)`
   - If errors occur, check the error messages and verify all files are present

---

## Part 2: Create Data Table Asset

### Step 2: Create World Scroll Data Table

1. **Navigate to Content Browser**
   - In Unreal Editor, open Content Browser (Ctrl+Space)
   - Navigate to: `Content/Data/`

2. **Create New Data Table**
   - Right-click in empty space
   - Select: **Miscellaneous > Data Table**
   - In the "Pick Structure" dialog, search for: `WorldScrollData`
   - Select: **WorldScrollData** (should show "Struct" type)
   - Click **Select**

3. **Name the Data Table**
   - Name it: `DT_WorldScrollData`
   - Press Enter to confirm

4. **Configure the Data Table**
   - Double-click `DT_WorldScrollData` to open it
   - Click **Add** button (top toolbar) to add a new row
   - Set Row Name to: `DefaultScroll`
   - Configure the values:
     - **ScrollSpeed**: `1000.0` (units per second)
     - **bScrollEnabled**: ✓ (checked)
     - **ScrollDirection**: X=-1.0, Y=0.0, Z=0.0

5. **Save the Data Table**
   - Click **Save** button (top toolbar)
   - Close the Data Table editor

### Step 3: Verify Data Table Structure

The data table should look like this:

| Row Name | ScrollSpeed | bScrollEnabled | ScrollDirection |
|----------|-------------|----------------|-----------------|
| DefaultScroll | 1000.0 | true | X=-1.0, Y=0.0, Z=0.0 |

---

## Part 3: Configure GameMode

### Step 4: Create GameMode Blueprint (If Needed)

If you don't already have a GameMode Blueprint:

1. **Create Blueprint Class**
   - Content Browser > Right-click
   - Select: **Blueprint Class**
   - Search for parent class: `WhitelineNightmareGameMode`
   - Name it: `BP_WhitelineNightmareGameMode`
   - Save in: `Content/Blueprints/Core/`

2. **Configure World Scroll Component**
   - Open `BP_WhitelineNightmareGameMode`
   - In the Components panel, find: **World Scroll Component**
   - Select it to view Details panel

3. **Assign Data Table**
   - In Details panel, find: **World Scroll > Config**
   - Set **Scroll Data Table** to: `DT_WorldScrollData`
   - Set **Data Table Row Name** to: `DefaultScroll`
   - Click **Compile** and **Save**

### Step 5: Set as Default Game Mode

1. **Project Settings**
   - Edit > Project Settings
   - Navigate to: **Project > Maps & Modes**

2. **Set Default GameMode**
   - Under **Default Modes**:
   - Set **Default GameMode** to: `BP_WhitelineNightmareGameMode`
   - Close Project Settings

3. **Level Settings (Per-Level Override)**
   - Alternatively, set per level:
   - Window > World Settings
   - Under **Game Mode > GameMode Override**:
   - Select: `BP_WhitelineNightmareGameMode`

---

## Part 4: Testing with Console Commands

### Step 6: Enable Console and Test Basic Functionality

1. **Start PIE (Play In Editor)**
   - Click Play button or press Alt+P
   - Wait for game to start

2. **Open Console**
   - Press ` (backtick/tilde key)
   - Console input box should appear at bottom

3. **Test: Show Scroll Info**
   ```
   DebugShowScrollInfo
   ```

   **Expected Output in Log:**
   ```
   LogWhitelineNightmare: === World Scroll Info ===
   LogWhitelineNightmare: Scroll Speed: 1000.00 units/second
   LogWhitelineNightmare: Scroll Direction: X=-1.000 Y=0.000 Z=0.000
   LogWhitelineNightmare: Scroll Velocity: X=-1000.000 Y=0.000 Z=0.000
   LogWhitelineNightmare: Is Scrolling: Yes
   LogWhitelineNightmare: Distance Traveled: [some value] units
   LogWhitelineNightmare: ========================
   ```

4. **Test: Change Scroll Speed**
   ```
   DebugSetScrollSpeed 2000
   ```

   **Expected Output:**
   ```
   LogWhitelineNightmare: DebugSetScrollSpeed: Set scroll speed to 2000.00
   ```

   Then verify:
   ```
   DebugShowScrollInfo
   ```

   Should now show: `Scroll Speed: 2000.00 units/second`

5. **Test: Toggle Scrolling**
   ```
   DebugToggleScroll
   ```

   **Expected Output:**
   ```
   LogWhitelineNightmare: DebugToggleScroll: Scrolling is now DISABLED
   ```

   Verify:
   ```
   DebugShowScrollInfo
   ```

   Should show: `Is Scrolling: No`

   Toggle again:
   ```
   DebugToggleScroll
   ```

   Should show: `Is Scrolling: ENABLED`

6. **Test: Reset Distance**
   ```
   DebugResetDistance
   ```

   **Expected Output:**
   ```
   LogWhitelineNightmare: DebugResetDistance: Reset distance from [value] to 0.0
   ```

   Verify:
   ```
   DebugShowScrollInfo
   ```

   Should show: `Distance Traveled: 0.00 units` (or very small)

### Step 7: Observe Distance Accumulation

1. **While game is running** (PIE active)
2. **Run command multiple times:**
   ```
   DebugShowScrollInfo
   ```

3. **Observe Distance Traveled increasing:**
   - First check: `Distance Traveled: 100.00 units`
   - Second check (1 sec later): `Distance Traveled: 1100.00 units`
   - Third check (1 sec later): `Distance Traveled: 2100.00 units`

4. **Verify calculation:**
   - Distance increases by ~1000 units per second (at default speed)
   - Formula: `Distance += ScrollSpeed * DeltaTime`

---

## Part 5: Automated Testing

### Step 8: Run Automated Tests

1. **Create Test Level** (if not already exists)
   - File > New Level
   - Select: Empty Level
   - Save as: `TestMap` in `Content/Maps/`

2. **Set Testing Game Mode**
   - Window > World Settings
   - Under **Game Mode Override**:
   - Select: `TestingGameMode` (or `ATestingGameMode` in C++)

3. **Play in Editor**
   - Click Play or Alt+P

4. **Open Console** (` key)

5. **Run World Scroll Tests:**
   ```
   RunTests Movement
   ```

   **Expected Output:**
   ```
   LogTestManager: ====================================
   LogTestManager: Running tests for category: Movement
   LogTestManager: ====================================
   LogTestManager: [PASS] WorldScroll_ScrollSpeedConsistency
   LogTestManager: [PASS] WorldScroll_DistanceAccumulation
   LogTestManager: [PASS] WorldScroll_ScrollPause
   LogTestManager: [PASS] WorldScroll_ScrollVelocity
   LogTestManager: [PASS] WorldScroll_ScrollSpeedChange
   LogTestManager: [PASS] WorldScroll_DirectionNormalization
   LogTestManager: [PASS] WorldScroll_DistanceReset
   LogTestManager: ====================================
   LogTestManager: Test Summary: 7/7 passed (100.0%)
   LogTestManager: ====================================
   ```

6. **Run Individual Tests** (optional):
   - These tests verify specific functionality
   - All 7 tests should PASS

### Step 9: Interpret Test Results

**If all tests pass:**
- ✅ World scroll component is working correctly
- ✅ Distance accumulation is accurate
- ✅ Pause/resume functionality works
- ✅ Speed changes apply correctly
- ✅ Direction normalization works

**If tests fail:**
- ❌ Check Output Log for detailed error messages
- ❌ Verify data table is configured correctly
- ❌ Verify component is attached to GameMode
- ❌ Check for compilation errors

---

## Part 6: Visual Verification (Advanced)

### Step 10: Create Debug Visualization

To visually see the scrolling system working:

1. **Create a test level with visible objects:**
   - Place some static mesh actors (cubes, spheres) in the level
   - Position them at various X coordinates (e.g., X=0, X=1000, X=2000)

2. **Create a simple Blueprint to visualize scrolling:**

**Blueprint: BP_ScrollingTestObject**

```
Event Tick
  ├─ Get Game Mode
  │  └─ Cast to WhitelineNightmareGameMode
  │     └─ Get World Scroll Component
  │        └─ Get Scroll Velocity
  │           └─ (Break Vector)
  │              └─ Make Vector
  │                 ├─ X = Scroll Velocity X * Delta Seconds
  │                 ├─ Y = 0
  │                 └─ Z = 0
  │                    └─ Add Actor Local Offset
  │                       └─ Delta Location = [Vector from above]
```

**In Kismet (visual representation):**
- Event Tick
- Get GameMode → Cast to WhitelineNightmareGameMode
- Get WorldScrollComponent
- Get ScrollVelocity (returns FVector)
- Multiply by Delta Seconds
- Add Actor Local Offset (move object by velocity)

3. **Place several instances** of `BP_ScrollingTestObject` in level

4. **Play** and observe:
   - Objects should move backward (negative X direction)
   - All objects move at same speed (1000 units/sec by default)
   - Objects should stop when you run `DebugToggleScroll`

### Step 11: Verify with Print Strings

Add debug output to verify system is working:

1. **In BP_WhitelineNightmareGameMode Event BeginPlay:**
   ```
   Event BeginPlay
     └─ Get World Scroll Component
        ├─ Get Scroll Speed
        │  └─ Print String ("Scroll Speed: " + [value])
        └─ Get Scroll Velocity
           └─ Print String ("Scroll Velocity: " + [value])
   ```

2. **Expected output on screen** (when game starts):
   ```
   Scroll Speed: 1000.0
   Scroll Velocity: X=-1000.000 Y=0.000 Z=0.000
   ```

---

## Part 7: Integration Testing

### Step 12: Test with War Rig

Once you have the war rig set up:

1. **Verify war rig is stationary:**
   - Place war rig at world origin (0, 0, 0)
   - Start game
   - War rig should NOT move in X direction
   - War rig should only move in Y direction (lane changes)

2. **Verify world scrolls past war rig:**
   - Ground tiles should move backward
   - Enemies should move backward (plus their own movement)
   - Obstacles should move backward
   - Pickups should move backward

3. **Test distance tracking:**
   - Play for 10 seconds
   - Run `DebugShowScrollInfo`
   - Should show ~10,000 units traveled (at 1000 u/s)

---

## Part 8: Common Issues and Troubleshooting

### Issue 1: "WorldScrollComponent is null" error

**Cause:** Component not created or GameMode not set correctly

**Fix:**
1. Verify `BP_WhitelineNightmareGameMode` is set as GameMode
2. Open Blueprint, verify WorldScrollComponent exists in Components panel
3. If missing, parent class may not be compiled correctly - recompile C++

### Issue 2: Distance not accumulating

**Cause:** Component not ticking or scrolling disabled

**Fix:**
1. Run: `DebugToggleScroll` (ensure scrolling is enabled)
2. Verify component tick is enabled:
   - In C++: `PrimaryComponentTick.bCanEverTick = true`
   - Check Output Log for tick-related errors

### Issue 3: Scroll speed seems wrong

**Cause:** Data table not loaded or wrong values

**Fix:**
1. Verify data table exists: `Content/Data/DT_WorldScrollData`
2. Verify row name matches: `DefaultScroll`
3. Check Output Log for data table load errors
4. Manually set speed: `DebugSetScrollSpeed 1000`

### Issue 4: Tests failing

**Cause:** Various - check specific test failure

**Fix:**
1. Read test failure message in Output Log
2. Each test message indicates what failed
3. Common issues:
   - Component not created: Verify GameMode setup
   - Values not matching: Check floating point precision
   - Timing issues: Tests use frame-based simulation

### Issue 5: Console commands not working

**Cause:** GameMode not set or Exec functions not registered

**Fix:**
1. Verify you're using `WhitelineNightmareGameMode` (or child)
2. Ensure you compiled C++ code successfully
3. Check you're typing command correctly (case-sensitive)
4. Try in PIE mode, not in Editor mode

---

## Part 9: Performance Verification

### Step 13: Check Performance Impact

1. **Open Stat Commands:**
   ```
   stat fps
   stat unit
   ```

2. **Verify performance:**
   - FPS should remain stable (60fps target)
   - Game thread time should be minimal
   - WorldScrollComponent adds negligible overhead

3. **Profile if needed:**
   ```
   stat startfile
   [play for 30 seconds]
   stat stopfile
   ```

---

## Part 10: Next Steps

### After successful setup and testing:

1. **Integrate with ground tiles:**
   - Query `WorldScrollComponent->GetScrollVelocity()`
   - Move tiles backward each tick
   - Recycle tiles using object pool

2. **Integrate with enemies:**
   - Add scroll velocity to enemy movement
   - Enemies move backward relative to world
   - Plus their own forward/backward movement

3. **Integrate with obstacles:**
   - Move backward at scroll speed
   - Despawn when pass war rig

4. **Integrate with pickups:**
   - Move backward at scroll speed
   - Collect when overlap war rig

5. **Use for win condition:**
   - Query `GetDistanceTraveled()` in GameMode
   - When distance >= WinDistance, trigger victory

---

## Quick Reference: Console Commands

| Command | Description |
|---------|-------------|
| `DebugShowScrollInfo` | Display current scroll state in log |
| `DebugSetScrollSpeed <speed>` | Set scroll speed (e.g., 2000) |
| `DebugToggleScroll` | Pause/resume scrolling |
| `DebugResetDistance` | Reset distance counter to 0 |
| `RunTests Movement` | Run all movement tests (includes scroll tests) |

---

## Quick Reference: Expected Values

| Property | Default Value | Notes |
|----------|---------------|-------|
| ScrollSpeed | 1000.0 | Units per second |
| ScrollDirection | (-1, 0, 0) | Backward along X axis |
| bScrollEnabled | true | Scrolling active by default |
| Distance Traveled | 0.0 at start | Accumulates over time |
| Scroll Velocity | (-1000, 0, 0) | Direction × Speed |

---

## Success Criteria

✅ **Setup Complete When:**
- Data table `DT_WorldScrollData` exists with `DefaultScroll` row
- GameMode has WorldScrollComponent with data table assigned
- Compilation succeeds with no errors
- Console commands work in PIE

✅ **Testing Complete When:**
- All 7 automated tests pass (100%)
- Console commands show expected output
- Distance accumulates correctly (~1000 units/sec)
- Pause/resume works correctly
- Speed changes apply immediately

✅ **Ready for Integration When:**
- Setup and testing complete
- Performance acceptable (60fps)
- Other systems can query `GetScrollVelocity()`
- Win condition can use `GetDistanceTraveled()`

---

## Support

If you encounter issues not covered here:

1. Check **Output Log** for detailed error messages
2. Verify all files compiled successfully
3. Check that data table is properly configured
4. Try running automated tests for specific failures
5. Review `WorldScrollComponent.cpp` for debug logging

For questions specific to Unreal Engine:
- Check [Unreal Engine Documentation](https://docs.unrealengine.com/)
- Review ActorComponent lifecycle documentation

---

## File Reference

**C++ Files Created:**
- `Source/WhitelineNightmare/Public/Core/WorldScrollComponent.h`
- `Source/WhitelineNightmare/Private/Core/WorldScrollComponent.cpp`
- `Source/WhitelineNightmare/Private/Testing/WorldScrollTests.cpp`

**Modified Files:**
- `Source/WhitelineNightmare/Public/Core/GameDataStructs.h` (FWorldScrollData struct)
- `Source/WhitelineNightmare/Public/Core/WhitelineNightmareGameMode.h` (debug commands)
- `Source/WhitelineNightmare/Private/Core/WhitelineNightmareGameMode.cpp` (component integration)

**Assets to Create:**
- `Content/Data/DT_WorldScrollData.uasset` (data table)
- `Content/Blueprints/Core/BP_WhitelineNightmareGameMode.uasset` (optional, if using BP)

---

**End of Setup Guide**
