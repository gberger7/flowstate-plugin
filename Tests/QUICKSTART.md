# Quick Start Guide - DelayEngine Property Tests

Get up and running with the DelayEngine property tests in 5 minutes.

## Prerequisites Check

Before starting, verify you have:
- [ ] JUCE framework installed
- [ ] C++17 compatible compiler (Clang, GCC, or MSVC)
- [ ] CMake 3.15+ (optional, for CMake build method)

## Quick Start (3 Steps)

### Step 1: Navigate to Tests Directory
```bash
cd Tests
```

### Step 2: Run the Tests
```bash
./run_tests.sh
```

That's it! The script will automatically:
- Build the test executable
- Run all property tests
- Display results

## Expected Output

If everything works, you'll see:
```
Building with CMake...
Configuring...
Building...

Running tests...
===============================================================================
Running 3 test cases...
===============================================================================

Property 1: Parameter Range Validation - Delay Time Clamping
  Delay time parameter correctly clamps to 1-2000ms range
PASSED

Property 2: BPM Sync Delay Time Calculation
  BPM sync delay time calculations work correctly across all divisions
PASSED

Property 4: Feedback Limiting
  Feedback limiting prevents output from exceeding unity gain at 90%+ feedback
PASSED

===============================================================================
Test results: 3 passed, 0 failed
===============================================================================
All tests passed!
```

## Troubleshooting

### "JUCE not found" Error

**Problem**: CMake can't find JUCE installation

**Solution**: Set JUCE_PATH environment variable:
```bash
export JUCE_PATH=/path/to/your/JUCE
./run_tests.sh
```

Or edit `Makefile` and set the JUCE_PATH variable.

### "CMake not found" Error

**Problem**: CMake is not installed

**Solution**: Use Makefile instead:
```bash
./run_tests.sh make
```

### Build Errors

**Problem**: Compilation fails

**Solutions**:
1. Check JUCE is properly installed: `ls $JUCE_PATH/modules`
2. Verify C++17 compiler is available: `clang++ --version` or `g++ --version`
3. See detailed troubleshooting in `BUILDING.md`

## Alternative Build Methods

### Using CMake Directly
```bash
mkdir build && cd build
cmake ..
cmake --build .
./DelayEnginePropertyTests
```

### Using Makefile Directly
```bash
make run
```

### Using Projucer
1. Open `FlowstatePlugin.jucer`
2. Add Tests directory to project
3. Build and run from your IDE

## What Gets Tested?

The property tests verify three critical DelayEngine behaviors:

1. **Parameter Clamping** - Delay time stays within 1-2000ms
2. **BPM Sync** - Tempo calculations work for all note divisions
3. **Feedback Limiting** - High feedback never exceeds unity gain

Each property is tested with 100 random test cases to ensure correctness across all inputs.

## Next Steps

- Read `README.md` for detailed test descriptions
- See `BUILDING.md` for advanced build options
- Check `test_verification.md` for expected behavior details
- Review `IMPLEMENTATION_SUMMARY.md` for complete documentation

## Need Help?

1. Check `BUILDING.md` for detailed build instructions
2. Review `test_verification.md` for test behavior details
3. See `IMPLEMENTATION_SUMMARY.md` for complete overview

## Clean Up

To remove build artifacts:
```bash
./run_tests.sh clean
```

This removes:
- `build/` directory (CMake artifacts)
- `DelayEnginePropertyTests` executable (Makefile artifacts)
