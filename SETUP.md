# Flowstate Plugin - Setup Guide

## Task 1 Complete ✅

The JUCE project structure and configuration has been successfully set up.

## What Was Created

### 1. Project File
- **FlowstatePlugin.jucer** - JUCE project configuration
  - VST3 format enabled (macOS and Windows)
  - AU format enabled (macOS only)
  - C++17 language standard
  - Optimization flags configured (Release: -O3)
  - Required JUCE modules added:
    - juce_audio_processors
    - juce_dsp
    - juce_gui_extra (for WebBrowserComponent)
    - Plus standard JUCE modules

### 2. Directory Structure
```
FlowstatePlugin/
├── Source/
│   ├── Parameters.h          ✅ All 21 parameter IDs defined
│   ├── DSP/                  ✅ Ready for DSP components
│   └── Bridge/               ✅ Ready for parameter bridge
├── WebUI/
│   └── fonts/                ✅ Ready for Inter font files
├── FlowstatePlugin.jucer     ✅ JUCE project file
├── README.md                 ✅ Project documentation
├── SETUP.md                  ✅ This file
└── .gitignore                ✅ Build artifacts excluded
```

### 3. Parameters.h
All 21 parameters defined with proper namespacing:
- 5 Delay parameters (time, sync, division, feedback, diffusion)
- 3 Reverb parameters (size, decay, damping)
- 2 Core controls (blend, mix)
- 2 Modulation parameters (rate, depth)
- 2 Character parameters (drive, tone)
- 1 Ducking parameter (sensitivity)
- 2 Shimmer parameters (enabled, pitch)
- 1 Reverse parameter (mode)
- 1 Freeze parameter (enabled)
- 2 Output parameters (gain, width)

### 4. Build Configurations

#### macOS (Xcode)
- Debug and Release configurations
- Target: macOS 10.13+
- Architecture: Native
- Compiler flags: -Wall -Wextra -Wpedantic

#### Windows (Visual Studio 2022)
- Debug and Release configurations
- Target: x64
- Compiler flags: /W4
- Link-time optimization enabled in Release
- Fast math enabled in Release

## Next Steps

### Before Building
1. **Install JUCE Framework**
   - Download from https://juce.com/
   - Install to a known location
   - Note the path to JUCE modules

2. **Configure Projucer**
   - Open Projucer application
   - Go to Settings (gear icon)
   - Set "Global Paths" → "Path to JUCE" to your JUCE installation
   - Save settings

3. **Open Project**
   - Open `FlowstatePlugin.jucer` in Projucer
   - Verify all modules show green checkmarks
   - Click "Save Project and Open in IDE"

### Building the Plugin

#### macOS
```bash
# Projucer will generate Xcode project in Builds/MacOSX/
cd Builds/MacOSX
xcodebuild -configuration Release
```

#### Windows
```bash
# Projucer will generate VS solution in Builds/VisualStudio2022/
# Open the .sln file in Visual Studio 2022
# Build → Build Solution (or press Ctrl+Shift+B)
```

### Installing Built Plugins

#### macOS
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/FlowstatePlugin.vst3`
- **AU**: `~/Library/Audio/Plug-Ins/Components/FlowstatePlugin.component`

#### Windows
- **VST3**: `C:\Program Files\Common Files\VST3\FlowstatePlugin.vst3`

## Requirements Validated

This task satisfies the following requirements:

- ✅ **Requirement 12.1**: VST3 format configured
- ✅ **Requirement 12.2**: AU format configured (macOS)
- ✅ **Requirement 15.1**: WebUI directory structure created
- ✅ **Requirement 15.2**: juce_gui_extra module added for WebBrowserComponent

## Task Checklist

- [x] Create FlowstatePlugin.jucer project file with VST3 and AU formats enabled
- [x] Configure project for macOS and Windows targets
- [x] Set up Source/ directory structure with DSP/, Bridge/, and WebUI/ folders
- [x] Add JUCE modules: juce_audio_processors, juce_dsp, juce_gui_extra
- [x] Configure compiler settings for C++17 and optimization flags
- [x] Create Parameters.h with all 21 parameter ID definitions

## Troubleshooting

### "Cannot find JUCE modules"
- Open Projucer settings and verify "Path to JUCE" is set correctly
- Ensure JUCE 7.x or later is installed

### "C++17 not supported"
- Update to Xcode 12+ (macOS) or Visual Studio 2022 (Windows)
- Check compiler version supports C++17

### Build errors about missing headers
- Re-save the project in Projucer to regenerate build files
- Clean build folder and rebuild

## Development Workflow

1. Make changes to source files in `Source/`
2. If adding new files, add them in Projucer first
3. Save project in Projucer (regenerates build files)
4. Build in your IDE
5. Test plugin in a DAW

## Notes

- The project uses JUCE's global module path system
- No source files are included yet (PluginProcessor, PluginEditor will be added in Task 3)
- WebUI/index.html will be created in Task 5
- All DSP components will be implemented in Task 2
