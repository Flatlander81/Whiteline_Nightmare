# Fuel HUD Widget Debugging Guide

## Quick Diagnostic Commands

Run these commands in the Unreal console (press ~ or ` key in-game):

### 1. Check if the widget exists and is bound:
```
DebugShowFuelBindings
```
Expected output: `Fuel widget binding status: SUCCESS`

### 2. Test visibility toggle:
```
DebugToggleFuelUI
```
Try this twice - once to hide, once to show. See if it appears.

### 3. Test color cycling (will also update the display):
```
DebugTestFuelColors
```
This should cycle through Green -> Yellow -> Red and update the display.

### 4. Check what Game Mode is active:
Look in the Output Log for: `WhitelineNightmareGameMode: Game started`

### 5. Verify War Rig spawned:
Look for log messages containing `WarRigPawn` in the Output Log.

## Common Issues and Solutions

### Issue 1: Widget created but not visible
**Symptoms:** You see "Created fuel HUD widget" in logs but no UI
**Solutions:**
- Widget might be off-screen or very small
- Try running `DebugTestFuelColors` - this will force an update
- Check Z-order - another widget might be covering it

### Issue 2: Widget not created
**Symptoms:** No "Created fuel HUD widget" message in logs
**Solutions:**
- Wrong Game Mode - check your map's World Settings
- HUD class not set correctly

### Issue 3: Widget created but not bound to GAS
**Symptoms:** Widget exists but shows "Fuel: -- / --" or default values
**Solutions:**
- War Rig might not have spawned
- AbilitySystemComponent might not be initialized
- Run `DebugShowFuelBindings` to check

## Manual Test

If the widget isn't visible, try forcing it to appear with test values:

1. Press ~ to open console
2. Type: `DebugTestFuelColors`
3. The widget should appear at top-left with a progress bar
4. Keep pressing the command to cycle colors

## Expected Appearance

When working correctly, you should see at TOP-LEFT of screen:
```
Fuel: 100 / 100
[=================>] (green progress bar)
```

Position: 50 pixels from left edge, 10 pixels from top
Progress Bar: 300x30 pixels
Text: 18pt font, white color

## What to Report

If still not visible, please check Output Log and report:
1. Any errors related to "WarRigHUD" or "WarRigHUDWidget"
2. Output from `DebugShowFuelBindings`
3. Whether you're using WhitelineNightmareGameMode or TestingGameMode
4. Screenshot of your World Settings -> GameMode Override
