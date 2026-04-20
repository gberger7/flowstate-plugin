# Building DelayEngine Property Tests

## Prerequisites

The property tests require JUCE framework to be installed. There are several ways to set this up:

### Option 1: Using Projucer (Recommended)

1. Open `FlowstatePlugin.jucer` in Projucer
2. Add a new build configuration for tests:
   - Add the `Tests/` directory to the project
   - Add `DelayEnginePropertyTests.cpp` as a compile target
   - Set the project type to "Console Application" for the test build
3. Export and build using your IDE (Xcode/Visual Studio)

### Option 2: Using CMake

1. Ensure JUCE is installed and CMake can find it:
   ```bash
   # Install JUCE via package manager or download from juce.com
   # Set JUCE_PATH environment variable if needed
   export JUCE_PATH=/path/to/JUCE
   ```

2. Build the tests:
   ```bash
   cd Tests
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

3. Run the tests:
   ```bash
   ./DelayEnginePropertyTests
   # or
   ctest --verbose
   ```

### Option 3: Using Makefile (macOS/Linux)

1. Edit `Tests/Makefile` and set the correct JUCE path:
   ```makefile
   JUCE_PATH = /path/to/your/JUCE/installation
   ```

2. Build and run:
   ```bash
   cd Tests
   make run
   ```

## JUCE Installation

If you don't have JUCE installed:

1. Download from https://juce.com/get-juce/download
2. Extract to a location like `~/JUCE` or `/usr/local/JUCE`
3. Update the paths in your build configuration

## Troubleshooting

### "JUCE not found" errors

- Verify JUCE is installed: `ls $JUCE_PATH/modules`
- Check that `juce_core`, `juce_audio_basics`, etc. directories exist
- Update the JUCE_PATH in your build configuration

### Linker errors on macOS

- Ensure you're linking against required frameworks:
  - Accelerate
  - AudioToolbox
  - CoreAudio
  - CoreMIDI
  - IOKit
  - Cocoa

### Linker errors on Windows

- Ensure you're linking against:
  - winmm.lib
  - ole32.lib
  - oleaut32.lib

### Linker errors on Linux

- Install required packages:
  ```bash
  sudo apt-get install libasound2-dev libfreetype6-dev libx11-dev
  ```

## Running Tests

Once built, run the test executable:

```bash
./DelayEnginePropertyTests
```

Expected output:
```
===============================================================================
Running 3 test cases...
===============================================================================

Property 1: Parameter Range Validation - Delay Time Clamping
[Feature: flowstate-plugin, Property 1]
  Delay time parameter correctly clamps to 1-2000ms range
PASSED

Property 2: BPM Sync Delay Time Calculation
[Feature: flowstate-plugin, Property 2]
  BPM sync delay time calculations work correctly across all divisions
PASSED

Property 4: Feedback Limiting
[Feature: flowstate-plugin, Property 4]
  Feedback limiting prevents output from exceeding unity gain at 90%+ feedback
PASSED

===============================================================================
Test results: 3 passed, 0 failed
===============================================================================
```

## Test Configuration

You can modify test parameters in `DelayEnginePropertyTests.cpp`:

```cpp
constexpr int PROPERTY_TEST_ITERATIONS = 100;  // Number of random test cases
constexpr double TEST_SAMPLE_RATE = 44100.0;   // Sample rate for testing
constexpr int TEST_BUFFER_SIZE = 512;          // Audio buffer size
```

Increase `PROPERTY_TEST_ITERATIONS` for more thorough testing (e.g., 1000 for nightly builds).
