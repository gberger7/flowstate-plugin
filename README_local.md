# Flowstate Audio Plugin

A hybrid spatial audio effect plugin that unifies delay and reverb into a single interconnected feedback ecosystem.

## Project Structure

```
FlowstatePlugin/
├── Source/
│   ├── Parameters.h              # Parameter definitions and IDs (21 parameters)
│   ├── DSP/                      # DSP processing components
│   └── Bridge/                   # JavaScript ↔ C++ parameter bridge
├── WebUI/
│   └── fonts/                    # Local font files (Inter)
├── FlowstatePlugin.jucer         # JUCE project file
└── README.md
```

## Requirements

- JUCE Framework 7.x or later
- C++17 compatible compiler
- macOS: Xcode 12+ (for AU and VST3)
- Windows: Visual Studio 2022 (for VST3)

## Building

1. Install JUCE Framework from https://juce.com/
2. Open `FlowstatePlugin.jucer` in Projucer
3. Set the JUCE modules path in Projucer settings
4. Click "Save Project and Open in IDE"
5. Build the project in your IDE

### Build Targets

- **macOS**: VST3 and AU formats
- **Windows**: VST3 format

## Configuration

The project is configured with:
- **Plugin Formats**: VST3, AU (macOS only)
- **C++ Standard**: C++17
- **Optimization**: Release builds use -O3 optimization
- **JUCE Modules**:
  - juce_audio_processors (audio plugin framework)
  - juce_dsp (DSP utilities)
  - juce_gui_extra (WebBrowserComponent for UI)
  - And standard JUCE modules

## Parameters (21 total)

### Delay Engine
- Delay Time (1-2000ms)
- Delay Sync (on/off)
- Delay Division (1/32 to 1/1, straight/dotted/triplet)
- Delay Feedback (0-100%)
- Delay Diffusion (0-100%)

### Reverb Engine
- Reverb Size (0-100%)
- Reverb Decay (0.1-20 seconds)
- Reverb Damping (0-100%)

### Core Controls
- Blend (0-100%, delay to reverb crossfade)
- Mix (0-100%, dry/wet balance)

### Modulation
- Mod Rate (0.01-5 Hz)
- Mod Depth (0-100%)

### Character
- Drive (0-100%, saturation)
- Tone (0-100%, high-frequency rolloff)

### Ducking
- Duck Sensitivity (0-100%)

### Shimmer
- Shimmer Enabled (on/off)
- Shimmer Pitch (-24 to +24 semitones)

### Reverse
- Reverse Mode (OFF, REVERB, DELAY, BOTH)

### Freeze
- Freeze Enabled (on/off)

### Output
- Output Gain (-60 to +6 dB)
- Stereo Width (0-150%)

## Development Status

✅ Task 1: Project structure and configuration complete
- JUCE project file created with VST3 and AU formats
- Directory structure established (Source/DSP/, Source/Bridge/, WebUI/)
- Parameters.h with all 21 parameter IDs defined
- Build configurations for macOS and Windows
- JUCE modules configured (juce_audio_processors, juce_dsp, juce_gui_extra)
- C++17 compiler settings and optimization flags set

## License

Copyright 2024 Flowstate Audio
