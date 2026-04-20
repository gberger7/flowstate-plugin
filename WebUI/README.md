# Flowstate Plugin WebView UI

This directory contains the complete web-based user interface for the Flowstate Audio Plugin, rendered through JUCE's WebBrowserComponent.

## Files

- **index.html** - Main UI file with embedded CSS and JavaScript
- **test.html** - Standalone test page for browser-based UI development
- **fonts/** - Directory for Inter font files (see fonts/README.md)

## Architecture

The UI is built as a single HTML file with embedded CSS and JavaScript, following the requirements:

### Layout Structure

```
┌─────────────────────────────────────────────────────────┐
│  DELAY          │    BLEND     │       REVERB           │
│  (Teal)         │  (Gradient)  │      (Amber)           │
│                 │              │                        │
│  Time           │   Blend      │   Size                 │
│  Feedback       │   Mix        │   Decay                │
│  Diffusion      │              │   Damping              │
├─────────────────┴──────────────┴────────────────────────┤
│ MODULATION    │  CHARACTER    │     SPECIAL            │
│               │               │                        │
│ Rate  Depth   │ Drive  Tone   │ Shimmer  Reverse       │
│               │ Duck          │ Freeze   Gain  Width   │
└───────────────────────────────────────────────────────────┘
```

### Parameters (21 Total)

#### Delay Section
- `delayTime` (1-2000ms)
- `delayFeedback` (0-100%)
- `delayDiffusion` (0-100%)

#### Blend Section
- `blend` (0-100%) - Delay ↔ Reverb crossfade
- `mix` (0-100%) - Wet/Dry balance

#### Reverb Section
- `reverbSize` (0-100%)
- `reverbDecay` (0.1-20s)
- `reverbDamping` (0-100%)

#### Modulation Section
- `modRate` (0.01-5Hz)
- `modDepth` (0-100%)

#### Character Section
- `drive` (0-100%)
- `tone` (0-100%)
- `duckSensitivity` (0-100%)

#### Special Section
- `shimmerEnabled` (bool)
- `shimmerPitch` (-24 to +24 semitones)
- `reverseMode` (0=OFF, 1=REVERB, 2=DELAY, 3=BOTH)
- `freezeEnabled` (bool)
- `outputGain` (-60 to +6dB)
- `stereoWidth` (0-150%)

## Features

### Interactive Knob Controls
- **Vertical drag**: Up increases value, down decreases
- **Double-click**: Opens text input for precise value entry
- **Right-click**: Context menu with "Reset to default"
- **Hover**: Shows tooltip with current value and unit
- **Visual feedback**: Smooth CSS transitions for rotation and arc

### Active State Visualizations
- **Freeze button**: Pulsing blue-white glow when active
- **Shimmer toggle**: Amber glow with outer halo when enabled
- **Reverse mode**: Section color highlighting (teal for delay, amber for reverb, gradient for both)
- **BPM sync**: Displays division and tempo next to time knob (e.g., "1/4 · 120bpm")

### Color Scheme
- Background: `#0E0E1A` (deep navy)
- Delay accent: `#3ECFCF` (teal/cyan)
- Reverb accent: `#E8A838` (amber/gold)
- Blend gradient: Teal → Lavender (`#C8B8FF`) → Amber

## JavaScript API

### Main Object: `window.flowstate`

#### Methods

**`init()`**
Initialize the UI, create knobs, attach event listeners

**`updateParam(paramId, normalizedValue, sendToPlugin)`**
Update a parameter value (0-1 normalized)
- `paramId`: Parameter identifier (e.g., "blend")
- `normalizedValue`: Value from 0.0 to 1.0
- `sendToPlugin`: Whether to send to C++ (default: true)

**`updateBPMDisplay(bpm, division)`**
Show BPM sync information
- `bpm`: Current tempo (e.g., 120)
- `division`: Division index (0-17)

**`hideBPMDisplay()`**
Hide BPM sync information when sync is disabled

#### Properties

**`params`**
Object storing current normalized parameter values (0-1)

**`defaults`**
Object storing default parameter values

**`currentBPM`**
Current host tempo (default: 120)

## Communication with C++

### JavaScript → C++

The UI sends parameter changes to the C++ plugin via:

```javascript
window.__juce__.postMessage(JSON.stringify({
    id: "paramId",
    value: 0.75  // normalized 0-1
}));
```

### C++ → JavaScript

The C++ plugin updates the UI via:

```cpp
webView.evaluateJavascript("window.flowstate.updateParam('blend', 0.75)");
```

## Testing

### Browser Testing

1. Open `test.html` in a web browser
2. Interact with knobs and controls
3. Check console output for parameter messages
4. Press 'a' to simulate automation
5. Press 's' to stop automation

### Integration Testing

The UI will be integrated with JUCE's WebBrowserComponent in the PluginEditor class. The Parameter Bridge will handle bidirectional communication.

## Requirements Validation

This implementation satisfies the following requirements:

- ✅ **15.1**: Single HTML file at WebUI/index.html
- ✅ **15.2**: Embedded CSS and JavaScript
- ✅ **15.5**: Custom SVG knob elements
- ✅ **15.6**: Vertical mouse drag for value changes
- ✅ **15.7**: Local font loading from WebUI/fonts/
- ✅ **18.1**: Window size 900x500 pixels
- ✅ **18.2**: Background color #0E0E1A
- ✅ **18.3**: Teal accent #3ECFCF for Delay
- ✅ **18.4**: Amber accent #E8A838 for Reverb
- ✅ **18.5**: Gradient for Blend control
- ✅ **18.6**: Inter font family
- ✅ **18.7**: No scrollbars or overflow
- ✅ **18.8**: Organized sections (Delay, Blend, Reverb, Modulation, Character, Special)
- ✅ **19.1**: Vertical drag interaction
- ✅ **19.2**: Double-click text input
- ✅ **19.3**: Right-click context menu
- ✅ **19.4**: Hover tooltips
- ✅ **19.5**: CSS transition animations
- ✅ **19.6**: Hover brightness increase
- ✅ **20.1**: Freeze pulsing glow animation
- ✅ **20.2**: Freeze inactive state
- ✅ **20.3**: Reverse mode highlighting
- ✅ **20.4**: Shimmer amber glow and halo
- ✅ **20.5**: BPM division display

## Next Steps

1. **Add Inter fonts**: Download and place font files in `fonts/` directory
2. **Integration**: Connect to JUCE WebBrowserComponent in PluginEditor
3. **Parameter Bridge**: Implement bidirectional C++ ↔ JavaScript communication
4. **Testing**: Verify all interactions work in the plugin context
5. **Refinement**: Adjust visual styling and interaction feel based on user feedback

## Notes

- All parameter values are normalized to 0-1 range for communication with C++
- The UI converts between normalized and actual values internally
- SVG knobs are generated dynamically with proper arc rendering
- The blend knob uses a special gradient defined in SVG defs
- All interactions are designed to be smooth and responsive
- No external network requests are made (fully offline operation)
