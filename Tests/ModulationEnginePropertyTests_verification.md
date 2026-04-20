# ModulationEngine Property Tests - Verification Document

## Overview

This document describes the property-based tests for the ModulationEngine component of the Flowstate audio plugin. These tests validate that the LFO modulation system behaves correctly across all parameter ranges and produces accurate sine wave output.

## Test File

`ModulationEnginePropertyTests.cpp`

## Properties Tested

### Property 8: Modulation Depth Zero Bypass

**Description**: Verifies that when modulation depth is set to 0%, no modulation is applied regardless of the LFO rate setting.

**Validates**: Requirements 5.3

**Test Strategy**:
- Set depth to 0%
- Set random LFO rates (0.01Hz to 5.0Hz)
- Sample modulation output over 2 seconds
- Verify all output values are exactly 0.0

**Expected Behavior**:
- With depth=0%, `getNextModulationValue()` should always return 0.0
- This should hold true regardless of the LFO rate
- No modulation should be applied to delay time or reverb diffusion

**Test Iterations**: 100 random test cases

**Pass Criteria**:
- All modulation values must be exactly 0.0 when depth=0%
- No exceptions or crashes during processing

### Property 10: LFO Waveform Shape

**Description**: Verifies that the LFO produces an accurate sine wave pattern within 1% tolerance across all rates.

**Validates**: Requirements 5.5

**Test Strategy**:
- Set depth to 100% for full amplitude testing
- Set random LFO rates (0.01Hz to 5.0Hz)
- Sample one complete LFO cycle
- Compare actual samples against expected sine wave values
- Calculate maximum error across the cycle

**Expected Behavior**:
- LFO output should match `sin(phase * 2π)` within 1% tolerance (0.01 absolute error)
- Peak values should be approximately ±1.0
- Waveform should be smooth and continuous

**Test Iterations**: 100 random test cases

**Pass Criteria**:
- Maximum error between actual and expected sine values ≤ 0.01
- Peak positive value ≈ 1.0 (within 0.01 tolerance)
- Peak negative value ≈ -1.0 (within 0.01 tolerance)
- No discontinuities or phase jumps

## Test Configuration

```cpp
constexpr int PROPERTY_TEST_ITERATIONS = 100;
constexpr double TEST_SAMPLE_RATE = 44100.0;
```

## Helper Functions Used

- `randomFloat(min, max)` - Generate random float values for rate parameters
- `approximatelyEqual(a, b, tolerance)` - Compare floating-point values with tolerance

## Building and Running

### Using CMake (Recommended)
```bash
cd Tests
mkdir build && cd build
cmake ..
cmake --build .
./ModulationEnginePropertyTests
```

### Using Makefile
```bash
cd Tests
make run-modulation
```

### Using CTest
```bash
cd Tests/build
ctest -R ModulationEnginePropertyTests --verbose
```

## Expected Output

```
===============================================================================
All tests passed (2 assertions in 2 test cases)

Property 8: Modulation Depth Zero Bypass - No modulation when depth=0%
  Modulation depth=0% produces no modulation output
PASSED

Property 10: LFO Waveform Shape - Validate sine wave pattern within 1% tolerance
  LFO produces accurate sine wave within 1% tolerance
PASSED

===============================================================================
Test results: 2 passed, 0 failed
===============================================================================
```

## Failure Scenarios

### Property 8 Failures

**Symptom**: Non-zero modulation values when depth=0%

**Possible Causes**:
- Depth parameter not properly applied in `getNextModulationValue()`
- Incorrect scaling logic
- Uninitialized depth variable

**Debug Steps**:
1. Check that `depthAmount` is set to 0.0 in `setDepth(0.0f)`
2. Verify multiplication by `depthAmount` in `getNextModulationValue()`
3. Check for any additive offsets being applied

### Property 10 Failures

**Symptom**: Sine wave error exceeds 1% tolerance

**Possible Causes**:
- Incorrect phase increment calculation
- Phase wraparound issues
- Incorrect sine wave formula

**Debug Steps**:
1. Verify phase increment: `phaseIncrement = hz / sampleRate`
2. Check phase wraparound logic (should wrap at 1.0)
3. Verify sine calculation: `sin(phase * 2π)`
4. Check for numerical precision issues

**Symptom**: Peak values not close to ±1.0

**Possible Causes**:
- Incorrect depth scaling when depth=1.0
- Sine wave amplitude scaling issues

**Debug Steps**:
1. Verify depth is set to 1.0 in test
2. Check that sine output is not being scaled incorrectly
3. Verify no clipping or limiting is applied

## Integration Notes

The ModulationEngine is used in the signal flow to modulate:
- Delay time (±10% variation)
- Reverb diffusion parameters

These tests verify the core LFO generation is correct. Integration tests should verify:
- Modulation is applied to both delay and reverb simultaneously
- Modulation depth parameter correctly scales the effect
- No audio artifacts when modulation is active

## Requirements Traceability

| Property | Requirements | Acceptance Criteria |
|----------|--------------|---------------------|
| Property 8 | 5.3 | When modulation depth is 0%, no LFO modulation applied |
| Property 10 | 5.5 | LFO uses sine wave waveform |

## Test Maintenance

When modifying ModulationEngine:
- If changing LFO waveform, update Property 10 expected values
- If adding new waveform types, add new property tests
- If changing depth scaling, verify Property 8 still passes
- If changing rate range, update test parameter ranges

## Performance Considerations

These tests are lightweight:
- Each iteration processes 1-2 seconds of samples
- Total test time: ~5-10 seconds for 100 iterations
- No heavy DSP processing required
- Suitable for CI/CD pipelines

## Related Tests

- **DelayEnginePropertyTests.cpp** - Tests delay time modulation application
- **ReverbEnginePropertyTests.cpp** - Tests reverb diffusion modulation
- **Integration tests** (future) - Test modulation affects multiple targets simultaneously

## Conclusion

These property tests provide confidence that the ModulationEngine:
1. Correctly bypasses modulation when depth=0%
2. Generates accurate sine wave LFO output
3. Maintains numerical precision across all rate settings
4. Handles edge cases (very slow and very fast rates)

The tests use randomized inputs to explore the parameter space thoroughly, ensuring correctness across all valid configurations.
