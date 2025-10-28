# Debug Lane UI - Usage Guide

## Overview

The Debug Lane UI provides a simple button-based interface for testing the lane system. It displays two buttons to change lanes left and right, along with the current lane number.

## Quick Start

### Method 1: Use Console Command (Easiest)

1. **Open Unreal Engine 5.6**
   - Open `WhitelineNightmare.uproject`
   - Click **Yes** when prompted to rebuild modules
   - Wait for compilation to complete

2. **Open a Test Map**
   - Navigate to `Content/Maps/`
   - Open `WarRigTest.umap` (or any map with a war rig)

3. **Play in Editor**
   - Click **Play** (or press Alt+P)

4. **Show Debug UI**
   - Press **`** (backtick/tilde) to open console
   - Type: `ToggleDebugLaneUI`
   - Press Enter

5. **Use the UI**
   - Click **<< Lane Left** button to move left
   - Click **Lane Right >>** button to move right
   - Watch the **Lane: X** display update
   - Buttons will be disabled when you reach the edge lanes (0 or 4)

---

## Method 2: Create Blueprint Widget (Optional, for customization)

If you want to customize the appearance of the debug UI:

### Step 1: Create Blueprint Widget

1. **Content Browser → Add → User Interface → Widget Blueprint**
2. Name it `WBP_DebugLaneUI`
3. Set parent class to `DebugLaneUI` (the C++ class)

### Step 2: Design the Widget

1. **Add Canvas Panel** (if not already present)

2. **Add Horizontal Box**
   - Name it anything you like
   - Position it at the bottom of the screen (or wherever you prefer)

3. **Add TextBlock for Lane Info**
   - Name: `LaneInfoText` (IMPORTANT: must match this name)
   - Text: "Lane: --"
   - Font Size: 24
   - Color: White
   - Add to Horizontal Box

4. **Add Button for Left Lane**
   - Name: `LeftButton` (IMPORTANT: must match this name)
   - Add TextBlock child with text: "<< Lane Left"
   - Font Size: 20
   - Add to Horizontal Box

5. **Add Button for Right Lane**
   - Name: `RightButton` (IMPORTANT: must match this name)
   - Add TextBlock child with text: "Lane Right >>"
   - Font Size: 20
   - Add to Horizontal Box

6. **Compile and Save**

### Step 3: Configure HUD to Use Blueprint Widget

1. **Open WarRigHUD Blueprint** (if it exists) or use defaults
2. In the HUD settings, set `Debug Lane UI Class` to your `WBP_DebugLaneUI`
3. Save

### Step 4: Test

1. Play in Editor
2. Run console command: `ToggleDebugLaneUI`
3. Your custom-styled UI should appear

---

## Console Commands

| Command | Description |
|---------|-------------|
| `ToggleDebugLaneUI` | Show/hide the debug lane UI |
| `DebugShowLanes` | Toggle visual lane debug lines |
| `TestLaneSystemAll` | Run all lane system tests |

---

## How It Works

### C++ Implementation

The debug UI is implemented with these components:

1. **UDebugLaneUI** (C++ Widget Class)
   - Location: `Source/WhitelineNightmare/Public/UI/DebugLaneUI.h`
   - Handles button clicks and lane change logic
   - Updates lane display in real-time
   - Enables/disables buttons based on lane boundaries

2. **AWarRigHUD** (HUD Class)
   - Location: `Source/WhitelineNightmare/Public/Core/WarRigHUD.h`
   - Creates and manages the debug lane UI widget
   - Functions: `ShowDebugLaneUI()`, `HideDebugLaneUI()`, `ToggleDebugLaneUI()`

3. **AWarRigPawn** (Pawn Class)
   - Location: `Source/WhitelineNightmare/Public/Core/WarRigPawn.h`
   - Provides console command wrapper: `ToggleDebugLaneUI()`
   - Forwards command to HUD

### Widget Binding

The C++ class uses `BindWidgetOptional` meta tags for:
- `LeftButton` - Button to change lane left
- `RightButton` - Button to change lane right
- `LaneInfoText` - TextBlock to display current lane

If you create a Blueprint widget based on this class, **the widget names must match exactly** for binding to work.

### Button Click Flow

```
User clicks "Lane Left" button
  → UDebugLaneUI::OnLaneLeftClicked()
  → LaneSystemComponent->CanChangeLaneLeft() (check if valid)
  → LaneSystemComponent->ChangeLaneLeft() (perform lane change)
  → UpdateLaneDisplay() (update UI - happens every tick)
```

---

## Features

### Real-Time Updates

- Lane info updates every frame (in NativeTick)
- Buttons enable/disable automatically based on lane boundaries
- Shows current lane index (0-4)

### Safety Checks

- Buttons disabled when at lane boundaries:
  - Left button disabled at lane 0 (leftmost)
  - Right button disabled at lane 4 (rightmost)
- Logs warnings if War Rig or Lane System not found
- Handles missing widget elements gracefully

### Debug Logging

The UI logs all actions to the Output Log:
```
LogTemp: DebugLaneUI: Left button bound
LogTemp: DebugLaneUI: Right button bound
LogTemp: DebugLaneUI: Found LaneSystemComponent
LogTemp: DebugLaneUI: Lane Left button clicked
LogTemp: DebugLaneUI: Changed to left lane
```

---

## Troubleshooting

### "ToggleDebugLaneUI not found"
**Problem:** Console command not recognized

**Solution:**
- Make sure you're in Play mode (PIE)
- The war rig must be spawned and possessed by the player
- Recompile if you just pulled the latest code

### UI doesn't appear
**Problem:** UI is created but not visible

**Solution:**
- Check Output Log for errors
- Make sure WarRigHUD is the active HUD class
- Try running `ToggleDebugLaneUI` again (it's a toggle)

### Buttons don't work
**Problem:** Clicking buttons does nothing

**Solution:**
- If using Blueprint widget, verify widget names match exactly:
  - `LeftButton` (case-sensitive)
  - `RightButton` (case-sensitive)
  - `LaneInfoText` (case-sensitive)
- Check Output Log for binding messages
- Make sure War Rig pawn is spawned (check with `TestLaneSystemAll`)

### "War Rig pawn not found"
**Problem:** UI can't find the War Rig

**Solution:**
- Make sure War Rig is placed in the level
- Make sure War Rig is possessed by Player 0
- Check World Settings → Game Mode → Default Pawn Class

### Buttons always disabled
**Problem:** Both buttons are grayed out

**Solution:**
- Make sure Lane System Component exists on War Rig
- Run `TestLaneSystemAll` to verify lane system is working
- Check Output Log for Lane System errors

---

## Comparison with Console Commands

### Debug UI (Buttons)
✅ Easy to use - just click
✅ Visual feedback (current lane)
✅ Prevents invalid lane changes
✅ No need to remember commands
⚠️ Requires UI setup (but uses default C++ widget)

### Console Commands
✅ No setup required
✅ Can be scripted/automated
⚠️ Need to type commands
⚠️ No visual feedback

**Recommendation:** Use Debug UI for manual testing, console commands for automated testing.

---

## Integration with Lane System

The Debug UI directly calls the Lane System Component:

```cpp
// From DebugLaneUI.cpp
void UDebugLaneUI::OnLaneLeftClicked()
{
    if (LaneSystem->CanChangeLaneLeft())
    {
        LaneSystem->ChangeLaneLeft();  // ← Direct call to lane system
    }
}
```

This means:
- Lane changes are smooth and interpolated (FMath::FInterpTo)
- All lane system rules apply (boundaries, transition state)
- Debug visualization (`DebugShowLanes`) works alongside UI
- All tests (`TestLaneSystemAll`) remain valid

---

## Next Steps

### For Development
1. Test the UI buttons work correctly
2. Verify lane transitions are smooth
3. Check button states update at lane boundaries

### For Gameplay Integration
1. Replace debug UI with proper gameplay UI
2. Add keyboard/gamepad input bindings
3. Add visual feedback (lane indicators, transition effects)
4. Add audio feedback for lane changes
5. Integrate with fuel system (lane changes cost fuel)

### For Polish
1. Customize Blueprint widget appearance
2. Add transition animations
3. Add cooldown indicators
4. Add lane preview visualization

---

## File Locations

**C++ Files:**
- `Source/WhitelineNightmare/Public/UI/DebugLaneUI.h` - Widget class header
- `Source/WhitelineNightmare/Private/UI/DebugLaneUI.cpp` - Widget implementation
- `Source/WhitelineNightmare/Public/Core/WarRigHUD.h` - HUD class (modified)
- `Source/WhitelineNightmare/Private/Core/WarRigHUD.cpp` - HUD implementation (modified)
- `Source/WhitelineNightmare/Public/Core/WarRigPawn.h` - Pawn class (modified)
- `Source/WhitelineNightmare/Private/Core/WarRigPawn.cpp` - Pawn implementation (modified)

**Blueprint (if created):**
- `Content/UI/WBP_DebugLaneUI` - Blueprint widget (optional)

---

## Summary

✅ **Debug Lane UI is ready to use!**

1. Open Unreal Engine
2. Click Play
3. Type `ToggleDebugLaneUI` in console
4. Click buttons to change lanes
5. Watch the War Rig move smoothly between lanes!

The C++ widget will work immediately without any Blueprint setup. You can optionally create a Blueprint widget for custom styling.
