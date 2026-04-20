# WebView UI Testing Guide

## Quick Start

### Option 1: Direct Browser Testing

1. Open `WebUI/test.html` in a web browser
2. The UI will load in an iframe with a mock JUCE interface
3. Interact with knobs and controls
4. Watch console output at the bottom

### Option 2: Direct UI Testing

1. Open `WebUI/index.html` directly in a web browser
2. Open browser console (F12 or Cmd+Option+I)
3. Interact with controls
4. Watch console.log output for parameter messages

## Testing Checklist

### Visual Appearance

- [ ] Window is 900x500 pixels
- [ ] Background is deep navy (#0E0E1A)
- [ ] Delay section has teal border and glow
- [ ] Blend section has gradient colors
- [ ] Reverb section has amber border and glow
- [ ] All text is readable
- [ ] Layout has no overflow or scrollbars

### Knob Interactions

For each knob, test:

- [ ] **Vertical drag**: Click and drag up/down
  - Up should increase value
  - Down should decrease value
  - Value display should update in real-time
  - Knob arc should grow/shrink smoothly
  - Pointer should rotate smoothly
  
- [ ] **Double-click**: Double-click knob
  - Text input should appear
  - Current value should be pre-filled
  - Enter key should apply value
  - Escape key should cancel
  
- [ ] **Right-click**: Right-click knob
  - Context menu should appear
  - "Reset to default" option should be visible
  - Clicking reset should restore default value
  - Clicking outside should close menu
  
- [ ] **Hover**: Hover over knob
  - Brightness should increase
  - Cursor should change to pointer

### Button Controls

#### Shimmer Button
- [ ] Click to toggle on
  - Button should show amber background
  - Button should have amber glow and halo
  - Console should show: `shimmerEnabled: 1.0`
- [ ] Click to toggle off
  - Button should return to normal appearance
  - Console should show: `shimmerEnabled: 0.0`

#### Freeze Button
- [ ] Click to toggle on
  - Button should show blue background
  - Button should have pulsing glow animation
  - Animation should alternate between blue and white
  - Console should show: `freezeEnabled: 1.0`
- [ ] Click to toggle off
  - Button should return to normal appearance
  - Animation should stop
  - Console should show: `freezeEnabled: 0.0`

### Reverse Mode Selector

- [ ] Click "OFF"
  - All options should be inactive
  - Console should show: `reverseMode: 0.0`
  
- [ ] Click "REV"
  - REV button should have amber background
  - Console should show: `reverseMode: 0.333...`
  
- [ ] Click "DLY"
  - DLY button should have teal background
  - Console should show: `reverseMode: 0.666...`
  
- [ ] Click "BOTH"
  - BOTH button should have gradient background
  - Console should show: `reverseMode: 1.0`

### Value Display Formatting

Check that values display correctly:

- [ ] **delayTime**: "500ms" (integer + ms)
- [ ] **delayFeedback**: "50%" (integer + %)
- [ ] **reverbDecay**: "2.0s" (1 decimal + s)
- [ ] **modRate**: "0.50Hz" (2 decimals + Hz)
- [ ] **shimmerPitch**: "+12st" (signed integer + st)
- [ ] **outputGain**: "0.0dB" (1 decimal + dB, signed)
- [ ] **stereoWidth**: "100%" (integer + %)

### Special Features

#### BPM Display (Manual Test)
Since BPM sync is controlled by C++, test the API manually:

1. Open browser console
2. Run: `window.flowstate.updateBPMDisplay(120, 9)`
3. Check that "1/4 · 120bpm" appears below delay time knob
4. Run: `window.flowstate.hideBPMDisplay()`
5. Check that BPM display disappears

#### Parameter Updates (Manual Test)
Test the C++ → JS update API:

1. Open browser console
2. Run: `window.flowstate.updateParam('blend', 0.75, false)`
3. Check that blend knob updates to 75%
4. Run: `window.flowstate.updateParam('mix', 0.25, false)`
5. Check that mix knob updates to 25%

### Automation Simulation (test.html only)

1. Open `test.html` in browser
2. Press 'a' key to start automation
3. Watch blend knob sweep from 0% to 100%
4. Check that animation is smooth
5. Press 's' key to stop automation

### Console Output Testing

When interacting with controls, console should show:

```
Flowstate UI initialized
Sent to plugin: blend 0.75
Sent to plugin: mix 0.5
Sent to plugin: shimmerEnabled 1
Sent to plugin: freezeEnabled 1
Sent to plugin: reverseMode 0.6666666666666666
```

### Performance Testing

- [ ] Drag knobs rapidly
  - UI should remain responsive
  - No lag or stuttering
  - Smooth animations
  
- [ ] Interact with multiple controls quickly
  - No visual glitches
  - All updates apply correctly
  
- [ ] Leave page open for extended period
  - No memory leaks
  - No performance degradation

### Cross-Browser Testing

Test in multiple browsers:

- [ ] Chrome/Chromium
- [ ] Firefox
- [ ] Safari (macOS)
- [ ] Edge (Windows)

Check for:
- Visual consistency
- Interaction behavior
- Console output
- Animation smoothness

## Known Issues / Expected Behavior

### Font Fallback
If Inter fonts are not installed, the UI will use system fonts:
- macOS: -apple-system
- Windows: BlinkMacSystemFont

This is expected and the UI should still be readable.

### JUCE Interface
When testing in browser (not in plugin):
- `window.__juce__` is undefined or mocked
- Parameter messages won't reach audio engine
- This is expected for browser testing

### Value Precision
Normalized values (0-1) may show many decimal places in console.
This is expected and will be handled by C++ parameter system.

## Debugging Tips

### Knob Not Responding
1. Check browser console for errors
2. Verify `data-param` attribute is set
3. Check that `window.flowstate.init()` was called
4. Verify SVG was created inside knob element

### Visual Issues
1. Check browser zoom is 100%
2. Verify window size is 900x500
3. Check for CSS conflicts
4. Inspect element styles in dev tools

### Value Not Updating
1. Check console for "Sent to plugin" messages
2. Verify parameter ID matches Parameters.h
3. Check value is being normalized correctly
4. Verify `updateParam()` is being called

### Animation Not Smooth
1. Check browser hardware acceleration is enabled
2. Verify CSS transitions are not disabled
3. Check for high CPU usage
4. Try different browser

## Integration Testing (Task 6)

Once integrated with JUCE:

1. Load plugin in DAW
2. Verify UI renders correctly
3. Test all knob interactions
4. Test automation from DAW
5. Verify parameter sync (UI ↔ Audio)
6. Test preset save/load
7. Test project save/load

## Reporting Issues

When reporting issues, include:

1. Browser name and version
2. Operating system
3. Steps to reproduce
4. Expected behavior
5. Actual behavior
6. Console errors (if any)
7. Screenshots (if visual issue)

## Success Criteria

Task 5 testing is successful when:

- ✅ All 21 parameters are interactive
- ✅ All visual states display correctly
- ✅ All interactions work as expected
- ✅ Console shows correct parameter messages
- ✅ No JavaScript errors in console
- ✅ UI is responsive and smooth
- ✅ Layout is correct (900x500, no overflow)
- ✅ Colors match specification
- ✅ Animations are smooth

## Next Steps

After successful browser testing:

1. Proceed to Task 6: Parameter Bridge
2. Integrate with JUCE WebBrowserComponent
3. Test bidirectional communication
4. Verify automation updates
5. Test in actual DAW environment
