# StereoWidthProcessor Property Tests - Verification Document

## Test File
`Tests/StereoWidthProcessorPropertyTests.cpp`

## Properties Tested

### Property 19: Stereo Width Mono Collapse
**Validates Requirements:** 11.3

**Property Statement:**  
*For any* stereo input signal, when stereo width is set to 0%, the left and right output channels of the wet signal should be identical (mono).

**Test Strategy:**
- Generate 100 random stereo buffers with different content in L/R channels
- Set width to 0%
- Process through StereoWidthProcessor
- Verify L and R channels are identical within 0.0001 tolerance
- Verify output equals mid component (L+R)/2

**Expected Behavior:**
- Width=0% collapses stereo image to mono
- Both channels contain identical samples
- Output represents the mid (center) component only

**Edge Cases Covered:**
- Various stereo widths in input signal
- Different amplitude levels
- Verification that channels were different before processing

---

### Property 20: Width Affects Wet Signal Only
**Validates Requirements:** 11.6, 11.7

**Property Statement:**  
*For any* stereo input signal and any width value, the dry signal stereo image should remain unchanged while only the wet signal stereo width is modified.

**Test Strategy:**
- Generate 100 random width values (0.0 to 1.5)
- Create separate "dry" and "wet" buffers
- Process only the wet buffer through StereoWidthProcessor
- Verify dry buffer remains completely unchanged (never processed)
- Verify wet buffer is modified (except at width=1.0)
- Test width=1.0 as true bypass for wet signal

**Expected Behavior:**
- Dry signal path never passes through width processor
- Dry buffer remains bit-identical to original
- Wet buffer is processed and modified (unless width=1.0)
- Width=1.0 preserves wet signal (bypass)

**Edge Cases Covered:**
- Width=0% (mono collapse)
- Width=1.0 (bypass)
- Width=1.5 (hyper-wide)
- Random width values throughout range
- Verification of architectural separation between dry and wet paths

---

## Test Configuration

- **Framework:** Catch2
- **Iterations per property:** 100
- **Buffer size:** 512 samples
- **Channels:** 2 (stereo)
- **Tolerance:** 0.0001 for equality checks

## Helper Functions

1. `randomFloat(min, max)` - Generate random values for width and amplitude
2. `fillBufferWithStereoNoise()` - Create stereo content with different L/R channels
3. `buffersAreEqual()` - Compare buffers within tolerance
4. `channelsAreIdentical()` - Verify L=R for mono collapse testing

## Build Instructions

```bash
cd Tests
make StereoWidthProcessorPropertyTests
./build/StereoWidthProcessorPropertyTests
```

Or using CMake:
```bash
cd Tests/build
cmake ..
make StereoWidthProcessorPropertyTests
./StereoWidthProcessorPropertyTests
```

## Expected Output

```
All tests passed (200 assertions in 2 test cases)
```

## Requirements Coverage

| Requirement | Description | Property |
|-------------|-------------|----------|
| 11.3 | Width=0% produces mono output | Property 19 |
| 11.6 | Width processing uses M/S encoding on wet signal only | Property 20 |
| 11.7 | Dry signal stereo image unaffected by width control | Property 20 |

## Notes

- Property 20 tests the architectural principle that width processing is only applied to the wet signal path
- The test simulates the plugin's signal flow where dry and wet paths are separate
- Width=1.0 should act as true bypass (no processing artifacts)
- M/S encoding at width=0% produces mono by eliminating the side component
- The processor correctly implements the formula: mid=(L+R)/2, side=(L-R)/2, then scales side by width

