# Lane System Testing Guide

## Quick Test Setup (Easiest Method)

The lane system tests are **Exec functions** attached to the component, so you just need a War Rig in the level to test.

### Method 1: Use Existing Test Map

1. **Open Unreal Engine 5.6**
   - Open `WhitelineNightmare.uproject`
   - Click **Yes** when prompted to rebuild modules
   - Wait for compilation to complete

2. **Open a Test Map**
   - Navigate to `Content/Maps/`
   - Open `WarRigTest.umap` (or `TestMap.umap`)
   - If these don't have a war rig, continue to Method 2

3. **Play in Editor**
   - Click **Play** (or press Alt+P)
   - Press **`** (backtick/tilde) to open console

4. **Run Tests**
   ```
   TestLaneSystemAll
   ```
   - This runs all 5 tests with a comprehensive report

5. **View Results**
   - Open **Output Log**: Window → Developer Tools → Output Log
   - Look for the test report

---

### Method 2: Create New Test Map

If the existing maps don't have a war rig, create one:

#### Step 1: Create Test Map
1. **File → New Level → Empty Level**
2. Save as `Content/Maps/LaneSystemTest`

#### Step 2: Add War Rig to Level
You have two options:

**Option A: Use Blueprint War Rig** (if it exists)
1. Open Content Browser
2. Navigate to `Content/Blueprints/Vehicles/`
3. Find `BP_WarRig_SemiTruck`
4. Drag into viewport at location (0, 0, 0)

**Option B: Place C++ War Rig**
1. Open **Place Actors** panel
2. Search for "WarRigPawn"
3. Drag `AWarRigPawn` into viewport
4. Set location to (0, 0, 0) in Details panel
5. **IMPORTANT:** Set as Default Pawn:
   - World Settings → Game Mode → Default Pawn Class → WarRigPawn

#### Step 3: Add Player Start (Optional)
1. Place Actors → Player Start
2. Position near (0, 0, 100) so camera can see war rig

#### Step 4: Play and Test
1. Click **Play**
2. Open Console (` key)
3. Run: `TestLaneSystemAll`

---

## Individual Test Commands

You can run tests individually:

```
TestLaneSystemBounds          - Test boundary limits (can't go beyond lanes 0-4)
TestLaneTransitionSpeed       - Test interpolation speed is reasonable
TestLaneChangeValidation      - Test invalid changes are rejected
TestCurrentLaneTracking       - Test lane index updates correctly
TestStationaryInOtherAxes     - Test only Y changes, not X or Z
```

Or run all at once:
```
TestLaneSystemAll             - Run all 5 tests with detailed report
```

---

## Visual Testing (Debug Visualization)

To see the lanes visually:

1. **Enable Debug Visualization**
   ```
   DebugShowLanes
   ```

2. **Test Lane Changes** (requires Blueprint or C++ setup)
   - The component has `ChangeLaneLeft()` and `ChangeLaneRight()` functions
   - You'll need to bind these to input or call from Blueprint

**What You'll See:**
- **White lines** - Other lanes
- **Green line** - Current lane (idle)
- **Yellow line** - Current lane (transitioning away)
- **Cyan line** - Target lane (transitioning toward)
- **Orange sphere** - Current Y position during transition

---

## Expected Test Output

When you run `TestLaneSystemAll`, you should see:

```
================================================================================
                    LANE SYSTEM COMPREHENSIVE TEST SUITE
================================================================================

Component: ULaneSystemComponent
Owner: WarRigPawn_0
Configuration:
  - Num Lanes: 5
  - Lane Spacing: 200.00 units
  - Center Lane Index: 2
  - Lane Change Speed: 500.00 units/sec
  - Current Lane: 2

Running Test Suite...
--------------------------------------------------------------------------------

=== TestLaneSystemBounds START ===
=== TestLaneSystemBounds PASSED ===

=== TestLaneTransitionSpeed START ===
Lane spacing: 200.00, Speed: 500.00, Expected transition time: ~0.40 seconds
=== TestLaneTransitionSpeed PASSED ===

=== TestLaneChangeValidation START ===
=== TestLaneChangeValidation PASSED ===

=== TestCurrentLaneTracking START ===
=== TestCurrentLaneTracking PASSED ===

=== TestStationaryInOtherAxes START ===
Initial position - X: 0.00, Y: 0.00, Z: 0.00
=== TestStationaryInOtherAxes PASSED ===

================================================================================
                           TEST SUMMARY REPORT
================================================================================

  [PASS]  TestLaneSystemBounds              5.23ms
  [PASS]  TestLaneTransitionSpeed           2.41ms
  [PASS]  TestLaneChangeValidation          3.87ms
  [PASS]  TestCurrentLaneTracking           4.12ms
  [PASS]  TestStationaryInOtherAxes         6.34ms

--------------------------------------------------------------------------------
Total Tests:    5
Passed:         5
Failed:         0
Success Rate:   100.0%
Total Duration: 21.97ms

  *** ALL TESTS PASSED ***
  Lane System is functioning correctly!

================================================================================
```

---

## Troubleshooting

### "TestLaneSystemAll not found"
**Problem:** Console command not recognized

**Solution:**
- Make sure you're in Play mode (PIE)
- The war rig must be spawned in the level
- You must be running a Development build (not Shipping)
- Try the full object path: Get the war rig reference and call the function

### "Owner must exist for position test"
**Problem:** Lane system component has no owner

**Solution:**
- Make sure the War Rig Pawn is placed in the level
- Make sure it's spawned/possessed when you run tests
- The component is created in the pawn's constructor, so this should work automatically

### No Output in Console
**Problem:** Tests run but no visible output

**Solution:**
- Open **Output Log** window (Window → Developer Tools → Output Log)
- Filter by "LogTemp" to see test output
- Make sure you're not in a shipping build

### Tests Fail
**Problem:** One or more tests show [FAIL]

**Solution:**
- Check the Output Log for detailed error messages
- Each test shows exactly what failed and why
- Common issues:
  - Lane change speed is 0 or negative (check data table)
  - Num lanes is invalid (< 1)
  - Owner pawn is null (war rig not spawned)

---

## Advanced: Testing from Blueprint

You can also call these tests from Blueprint:

1. **Get Lane System Component**
   - Get Player Pawn → Cast to WarRigPawn
   - Get Component by Class → LaneSystemComponent

2. **Call Test Function**
   - Call `TestLaneSystemAll` node
   - Connect to BeginPlay or a button press event

---

## Testing with Input

To test lane changes with keyboard input (requires setup):

### Blueprint Setup:
1. Open the War Rig Blueprint
2. In Event Graph:
   ```
   Event: InputAction Left
     → Get Lane System Component
     → Call ChangeLaneLeft

   Event: InputAction Right
     → Get Lane System Component
     → Call ChangeLaneRight
   ```

3. Enable Debug: Run `DebugShowLanes` in console
4. Press Left/Right to see smooth lane transitions

---

## Component Architecture

**ULaneSystemComponent** is automatically created on **AWarRigPawn**:
- Location: `WarRigPawn.cpp:30`
- Component is always present on the war rig
- Initialized in BeginPlay with default 5 lanes
- Loads configuration from data table (optional)

**Default Configuration:**
- 5 lanes (indices 0-4)
- Center lane: index 2
- Lane spacing: 200 units
- Lane positions: [-400, -200, 0, 200, 400]
- Lane change speed: 500 units/sec

**Lane Indexing:**
- Lane 0 = Leftmost (Y = -400)
- Lane 1 = Left (Y = -200)
- Lane 2 = Center (Y = 0) ← **Starting position**
- Lane 3 = Right (Y = 200)
- Lane 4 = Rightmost (Y = 400)

---

## Why Not Registered with TestManager?

Unlike the Object Pool tests, the lane system tests are **Exec functions** on the component itself, not registered with the central TestManager. This is because:

1. They require a specific component instance
2. They test component-level functionality
3. They need the owner actor for position testing
4. They follow the same pattern as `TestWarRigAll()`

This means you **cannot** run them via `RunTests Movement`. You must run them directly by name.

---

## Summary Checklist

✅ Open Unreal Engine 5.6
✅ Let code compile (auto-prompt)
✅ Open or create test map
✅ Add War Rig to level at (0,0,0)
✅ Click Play
✅ Open console with ` key
✅ Type: `TestLaneSystemAll`
✅ View results in Output Log

**Expected:** All 5 tests pass, 100% success rate

---

## Next Steps After Testing

Once tests pass, you'll want to:
1. Bind lane changes to input (A/D or Left/Right arrows)
2. Test visual lane transitions with `DebugShowLanes`
3. Integrate with gameplay (fuel cost, collision detection)
4. Add audio feedback for lane changes
5. Connect to UI to show current lane

The lane system is fully functional and ready for game integration!
