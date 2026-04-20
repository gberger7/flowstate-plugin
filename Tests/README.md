# Flowstate Plugin Tests

This directory contains property-based tests for the Flowstate audio plugin.

## Test Structure

- `DelayEnginePropertyTests.cpp` - Property-based tests for DelayEngine component
- `ReverbEnginePropertyTests.cpp` - Property-based tests for ReverbEngine component
- `catch2/catch.hpp` - Minimal Catch2 test framework implementation
- `CMakeLists.txt` - CMake build configuration
- `Makefile` - Alternative build system for quick testing

## Property Tests Implemented

### DelayEngine Tests

#### Property 1: Parameter Range Validation
Tests that delay time parameter correctly clamps to 1-2000ms range across random inputs.
**Validates: Requirements 1.1**

#### Property 2: BPM Sync Delay Time Calculation
Tests tempo-synced delay calculations for all 18 divisions (1/32 through 1/1, straight/dotted/triplet) across random BPM values.
**Validates: Requirements 1.2**

#### Property 4: Feedback Limiting
Tests that high feedback values (90%+) never produce output exceeding unity gain after 100 iterations.
**Validates: Requirements 2.1, 2.2, 2.3**

### ReverbEngine Tests

#### Property 1: Parameter Range Validation
Tests that size (0.0-1.0), decay time (0.1-20.0s), and damping (0.0-1.0) parameters correctly clamp to their valid ranges across random inputs.
**Validates: Requirements 3.1, 3.2, 3.3**

## Building and Running Tests

### Prerequisites

- CMake 3.15 or higher
- JUCE framework installed and configured
- C++17 compatible compiler

### Build Instructions

```bash
# From the Tests directory
mkdir build
cd build
cmake ..
cmake --build .
```

### Running Tests

```bash
# From the Tests/build directory
./DelayEnginePropertyTests
./ReverbEnginePropertyTests

# Or using CTest (runs all tests)
ctest --verbose

# Or using Makefile
cd Tests
make run              # Run all tests
make run-delay        # Run DelayEngine tests only
make run-reverb       # Run ReverbEngine tests only
```

## Test Configuration

- **Iterations per property**: 100 (configurable via `PROPERTY_TEST_ITERATIONS`)
- **Sample rate**: 44100 Hz
- **Buffer size**: 512 samples

## Adding New Tests

To add new property tests:

1. Add a new `TEST_CASE` in the appropriate test file
2. Use the format: `TEST_CASE("Property N: Description", "[Feature: flowstate-plugin, Property N]")`
3. Include validation comment: `**Validates: Requirements X.Y**`
4. Use helper functions from `PropertyTestHelpers` namespace
5. Run at least 100 iterations with randomized inputs

## Test Helpers

The `PropertyTestHelpers` namespace provides:

- `randomFloat(min, max)` - Generate random float in range
- `randomDouble(min, max)` - Generate random double in range
- `randomInt(min, max)` - Generate random int in range
- `fillBufferWithImpulse(buffer)` - Fill buffer with impulse signal
- `fillBufferWithNoise(buffer, amplitude)` - Fill buffer with random noise
- `allSamplesWithinRange(buffer, limit)` - Check all samples within ±limit
- `getMaxAbsSample(buffer)` - Get maximum absolute sample value

## Integration with JUCE Project

These tests are standalone and don't require modifications to the main plugin project. They link against the DSP source files directly.

To integrate with the main JUCE project, you can add a test target to the `.jucer` file or use this CMake configuration separately.
