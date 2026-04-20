/*
  ==============================================================================

    StateManagementPropertyTests.cpp
    Property-Based Tests for State Serialization in FlowstateProcessor
    
    Tests validate universal correctness properties for parameter state
    serialization and deserialization (save/load functionality).
    Framework: Catch2 with custom property test harness
    Minimum iterations: 100 per property

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "../Source/PluginProcessor.h"

// Catch2 test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// Property test configuration
constexpr int PROPERTY_TEST_ITERATIONS = 100;
constexpr double TEST_SAMPLE_RATE = 44100.0;
constexpr int TEST_BUFFER_SIZE = 512;
constexpr float TOLERANCE = 0.001f; // 0.1% tolerance for parameter round-trip

// Helper functions for property testing
namespace PropertyTestHelpers
{
    // Generate random float in range [min, max]
    float randomFloat(float min, float max)
    {
        return min + (max - min) * (static_cast<float>(std::rand()) / RAND_MAX);
    }
    
    // Generate random boolean
    bool randomBool()
    {
        return (std::rand() % 2) == 0;
    }
    
    // Generate random integer in range [min, max]
    int randomInt(int min, int max)
    {
        return min + (std::rand() % (max - min + 1));
    }
    
    // Structure to hold all 21 parameter values
    struct ParameterSnapshot
    {
        float delayTime;
        float delaySync;
        float delayDivision;
        float delayFeedback;
        float delayDiffusion;
        
        float reverbSize;
        float reverbDecay;
        float reverbDamping;
        
        float blend;
        float mix;
        
        float modRate;
        float modDepth;
        
        float drive;
        float tone;
        
        float duckSensitivity;
        
        float shimmerEnabled;
        float shimmerPitch;
        
        float reverseMode;
        
        float freezeEnabled;
        
        float outputGain;
        float stereoWidth;
    };
    
    // Capture current parameter values from processor
    ParameterSnapshot captureParameters(FlowstateProcessor& processor)
    {
        ParameterSnapshot snapshot;
        
        snapshot.delayTime = processor.getParameters().getParameter(ParameterIDs::delayTime)->getValue();
        snapshot.delaySync = processor.getParameters().getParameter(ParameterIDs::delaySync)->getValue();
        snapshot.delayDivision = processor.getParameters().getParameter(ParameterIDs::delayDivision)->getValue();
        snapshot.delayFeedback = processor.getParameters().getParameter(ParameterIDs::delayFeedback)->getValue();
        snapshot.delayDiffusion = processor.getParameters().getParameter(ParameterIDs::delayDiffusion)->getValue();
        
        snapshot.reverbSize = processor.getParameters().getParameter(ParameterIDs::reverbSize)->getValue();
        snapshot.reverbDecay = processor.getParameters().getParameter(ParameterIDs::reverbDecay)->getValue();
        snapshot.reverbDamping = processor.getParameters().getParameter(ParameterIDs::reverbDamping)->getValue();
        
        snapshot.blend = processor.getParameters().getParameter(ParameterIDs::blend)->getValue();
        snapshot.mix = processor.getParameters().getParameter(ParameterIDs::mix)->getValue();
        
        snapshot.modRate = processor.getParameters().getParameter(ParameterIDs::modRate)->getValue();
        snapshot.modDepth = processor.getParameters().getParameter(ParameterIDs::modDepth)->getValue();
        
        snapshot.drive = processor.getParameters().getParameter(ParameterIDs::drive)->getValue();
        snapshot.tone = processor.getParameters().getParameter(ParameterIDs::tone)->getValue();
        
        snapshot.duckSensitivity = processor.getParameters().getParameter(ParameterIDs::duckSensitivity)->getValue();
        
        snapshot.shimmerEnabled = processor.getParameters().getParameter(ParameterIDs::shimmerEnabled)->getValue();
        snapshot.shimmerPitch = processor.getParameters().getParameter(ParameterIDs::shimmerPitch)->getValue();
        
        snapshot.reverseMode = processor.getParameters().getParameter(ParameterIDs::reverseMode)->getValue();
        
        snapshot.freezeEnabled = processor.getParameters().getParameter(ParameterIDs::freezeEnabled)->getValue();
        
        snapshot.outputGain = processor.getParameters().getParameter(ParameterIDs::outputGain)->getValue();
        snapshot.stereoWidth = processor.getParameters().getParameter(ParameterIDs::stereoWidth)->getValue();
        
        return snapshot;
    }
    
    // Set random parameter values on processor
    void setRandomParameters(FlowstateProcessor& processor)
    {
        // Delay parameters
        processor.getParameters().getParameter(ParameterIDs::delayTime)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::delaySync)->setValueNotifyingHost(randomBool() ? 1.0f : 0.0f);
        processor.getParameters().getParameter(ParameterIDs::delayDivision)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::delayFeedback)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::delayDiffusion)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Reverb parameters
        processor.getParameters().getParameter(ParameterIDs::reverbSize)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::reverbDecay)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::reverbDamping)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Core controls
        processor.getParameters().getParameter(ParameterIDs::blend)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::mix)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Modulation
        processor.getParameters().getParameter(ParameterIDs::modRate)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::modDepth)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Character
        processor.getParameters().getParameter(ParameterIDs::drive)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::tone)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Ducking
        processor.getParameters().getParameter(ParameterIDs::duckSensitivity)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Shimmer
        processor.getParameters().getParameter(ParameterIDs::shimmerEnabled)->setValueNotifyingHost(randomBool() ? 1.0f : 0.0f);
        processor.getParameters().getParameter(ParameterIDs::shimmerPitch)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Reverse
        processor.getParameters().getParameter(ParameterIDs::reverseMode)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        
        // Freeze
        processor.getParameters().getParameter(ParameterIDs::freezeEnabled)->setValueNotifyingHost(randomBool() ? 1.0f : 0.0f);
        
        // Output
        processor.getParameters().getParameter(ParameterIDs::outputGain)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
        processor.getParameters().getParameter(ParameterIDs::stereoWidth)->setValueNotifyingHost(randomFloat(0.0f, 1.0f));
    }
    
    // Compare two parameter snapshots
    bool snapshotsEqual(const ParameterSnapshot& s1, const ParameterSnapshot& s2, float tolerance)
    {
        return std::abs(s1.delayTime - s2.delayTime) < tolerance &&
               std::abs(s1.delaySync - s2.delaySync) < tolerance &&
               std::abs(s1.delayDivision - s2.delayDivision) < tolerance &&
               std::abs(s1.delayFeedback - s2.delayFeedback) < tolerance &&
               std::abs(s1.delayDiffusion - s2.delayDiffusion) < tolerance &&
               
               std::abs(s1.reverbSize - s2.reverbSize) < tolerance &&
               std::abs(s1.reverbDecay - s2.reverbDecay) < tolerance &&
               std::abs(s1.reverbDamping - s2.reverbDamping) < tolerance &&
               
               std::abs(s1.blend - s2.blend) < tolerance &&
               std::abs(s1.mix - s2.mix) < tolerance &&
               
               std::abs(s1.modRate - s2.modRate) < tolerance &&
               std::abs(s1.modDepth - s2.modDepth) < tolerance &&
               
               std::abs(s1.drive - s2.drive) < tolerance &&
               std::abs(s1.tone - s2.tone) < tolerance &&
               
               std::abs(s1.duckSensitivity - s2.duckSensitivity) < tolerance &&
               
               std::abs(s1.shimmerEnabled - s2.shimmerEnabled) < tolerance &&
               std::abs(s1.shimmerPitch - s2.shimmerPitch) < tolerance &&
               
               std::abs(s1.reverseMode - s2.reverseMode) < tolerance &&
               
               std::abs(s1.freezeEnabled - s2.freezeEnabled) < tolerance &&
               
               std::abs(s1.outputGain - s2.outputGain) < tolerance &&
               std::abs(s1.stereoWidth - s2.stereoWidth) < tolerance;
    }
    
    // Print parameter differences for debugging
    void printParameterDifferences(const ParameterSnapshot& s1, const ParameterSnapshot& s2, float tolerance)
    {
        auto checkParam = [&](const char* name, float v1, float v2) {
            float diff = std::abs(v1 - v2);
            if (diff >= tolerance)
            {
                std::cout << "  " << name << ": " << v1 << " -> " << v2 
                         << " (diff: " << diff << ")" << std::endl;
            }
        };
        
        std::cout << "Parameter differences exceeding tolerance:" << std::endl;
        checkParam("delayTime", s1.delayTime, s2.delayTime);
        checkParam("delaySync", s1.delaySync, s2.delaySync);
        checkParam("delayDivision", s1.delayDivision, s2.delayDivision);
        checkParam("delayFeedback", s1.delayFeedback, s2.delayFeedback);
        checkParam("delayDiffusion", s1.delayDiffusion, s2.delayDiffusion);
        
        checkParam("reverbSize", s1.reverbSize, s2.reverbSize);
        checkParam("reverbDecay", s1.reverbDecay, s2.reverbDecay);
        checkParam("reverbDamping", s1.reverbDamping, s2.reverbDamping);
        
        checkParam("blend", s1.blend, s2.blend);
        checkParam("mix", s1.mix, s2.mix);
        
        checkParam("modRate", s1.modRate, s2.modRate);
        checkParam("modDepth", s1.modDepth, s2.modDepth);
        
        checkParam("drive", s1.drive, s2.drive);
        checkParam("tone", s1.tone, s2.tone);
        
        checkParam("duckSensitivity", s1.duckSensitivity, s2.duckSensitivity);
        
        checkParam("shimmerEnabled", s1.shimmerEnabled, s2.shimmerEnabled);
        checkParam("shimmerPitch", s1.shimmerPitch, s2.shimmerPitch);
        
        checkParam("reverseMode", s1.reverseMode, s2.reverseMode);
        
        checkParam("freezeEnabled", s1.freezeEnabled, s2.freezeEnabled);
        
        checkParam("outputGain", s1.outputGain, s2.outputGain);
        checkParam("stereoWidth", s1.stereoWidth, s2.stereoWidth);
    }
}

using namespace PropertyTestHelpers;

// ==============================================================================
// Property 22: State Serialization Round Trip
// **Validates: Requirements 13.4, 13.5**
// ==============================================================================

TEST_CASE("Property 22: State Serialization Round Trip - Verify all 21 parameters survive save/load", 
          "[Feature: flowstate-plugin, Property 22]")
{
    INFO("Testing that all parameters are preserved through serialization/deserialization");
    
    int successCount = 0;
    int failureCount = 0;
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Create first processor and set random parameters
        FlowstateProcessor processor1;
        processor1.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Set random parameter values
        setRandomParameters(processor1);
        
        // Capture parameter state before serialization
        ParameterSnapshot beforeSnapshot = captureParameters(processor1);
        
        // Serialize state
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        // Verify serialization produced data
        REQUIRE(stateData.getSize() > 0);
        
        // Create second processor and deserialize
        FlowstateProcessor processor2;
        processor2.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        // Capture parameter state after deserialization
        ParameterSnapshot afterSnapshot = captureParameters(processor2);
        
        // Compare snapshots
        bool parametersMatch = snapshotsEqual(beforeSnapshot, afterSnapshot, TOLERANCE);
        
        if (parametersMatch)
        {
            successCount++;
        }
        else
        {
            failureCount++;
            INFO("Iteration " << iteration << " - Parameters do not match after round trip");
            printParameterDifferences(beforeSnapshot, afterSnapshot, TOLERANCE);
        }
        
        REQUIRE(parametersMatch);
    }
    
    INFO("Success rate: " << successCount << "/" << PROPERTY_TEST_ITERATIONS);
    SUCCEED("All 21 parameters survive serialization round trip");
}

// ==============================================================================
// Additional Test: Verify State Serialization with Default Values
// ==============================================================================

TEST_CASE("State Serialization with Default Values", 
          "[Feature: flowstate-plugin, Property 22]")
{
    INFO("Testing serialization with default parameter values");
    
    // Create processor with default parameters
    FlowstateProcessor processor1;
    processor1.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    // Don't set any parameters - use defaults
    ParameterSnapshot beforeSnapshot = captureParameters(processor1);
    
    // Serialize
    juce::MemoryBlock stateData;
    processor1.getStateInformation(stateData);
    
    REQUIRE(stateData.getSize() > 0);
    
    // Deserialize into new processor
    FlowstateProcessor processor2;
    processor2.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
    
    ParameterSnapshot afterSnapshot = captureParameters(processor2);
    
    // Verify defaults are preserved
    bool parametersMatch = snapshotsEqual(beforeSnapshot, afterSnapshot, TOLERANCE);
    
    if (!parametersMatch)
    {
        printParameterDifferences(beforeSnapshot, afterSnapshot, TOLERANCE);
    }
    
    REQUIRE(parametersMatch);
    SUCCEED("Default parameter values survive serialization");
}

// ==============================================================================
// Additional Test: Verify State Serialization with Extreme Values
// ==============================================================================

TEST_CASE("State Serialization with Extreme Values", 
          "[Feature: flowstate-plugin, Property 22]")
{
    INFO("Testing serialization with extreme parameter values (0.0 and 1.0)");
    
    // Test with all parameters at minimum (0.0)
    {
        FlowstateProcessor processor1;
        processor1.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Set all parameters to 0.0
        processor1.getParameters().getParameter(ParameterIDs::delayTime)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::delaySync)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayDivision)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayFeedback)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayDiffusion)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbSize)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbDecay)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbDamping)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::blend)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::mix)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::modRate)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::modDepth)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::drive)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::tone)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::duckSensitivity)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::shimmerEnabled)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::shimmerPitch)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverseMode)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::freezeEnabled)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::outputGain)->setValueNotifyingHost(0.0f);
        processor1.getParameters().getParameter(ParameterIDs::stereoWidth)->setValueNotifyingHost(0.0f);
        
        ParameterSnapshot beforeSnapshot = captureParameters(processor1);
        
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        FlowstateProcessor processor2;
        processor2.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        ParameterSnapshot afterSnapshot = captureParameters(processor2);
        
        bool parametersMatch = snapshotsEqual(beforeSnapshot, afterSnapshot, TOLERANCE);
        
        if (!parametersMatch)
        {
            INFO("Minimum values test failed");
            printParameterDifferences(beforeSnapshot, afterSnapshot, TOLERANCE);
        }
        
        REQUIRE(parametersMatch);
    }
    
    // Test with all parameters at maximum (1.0)
    {
        FlowstateProcessor processor1;
        processor1.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Set all parameters to 1.0
        processor1.getParameters().getParameter(ParameterIDs::delayTime)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::delaySync)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayDivision)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayFeedback)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::delayDiffusion)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbSize)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbDecay)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverbDamping)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::blend)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::mix)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::modRate)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::modDepth)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::drive)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::tone)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::duckSensitivity)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::shimmerEnabled)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::shimmerPitch)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::reverseMode)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::freezeEnabled)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::outputGain)->setValueNotifyingHost(1.0f);
        processor1.getParameters().getParameter(ParameterIDs::stereoWidth)->setValueNotifyingHost(1.0f);
        
        ParameterSnapshot beforeSnapshot = captureParameters(processor1);
        
        juce::MemoryBlock stateData;
        processor1.getStateInformation(stateData);
        
        FlowstateProcessor processor2;
        processor2.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        processor2.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
        
        ParameterSnapshot afterSnapshot = captureParameters(processor2);
        
        bool parametersMatch = snapshotsEqual(beforeSnapshot, afterSnapshot, TOLERANCE);
        
        if (!parametersMatch)
        {
            INFO("Maximum values test failed");
            printParameterDifferences(beforeSnapshot, afterSnapshot, TOLERANCE);
        }
        
        REQUIRE(parametersMatch);
    }
    
    SUCCEED("Extreme parameter values (0.0 and 1.0) survive serialization");
}

// ==============================================================================
// Additional Test: Multiple Serialization Cycles
// ==============================================================================

TEST_CASE("Multiple Serialization Cycles", 
          "[Feature: flowstate-plugin, Property 22]")
{
    INFO("Testing that parameters survive multiple save/load cycles");
    
    FlowstateProcessor processor;
    processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    // Set random parameters
    setRandomParameters(processor);
    
    // Capture initial state
    ParameterSnapshot initialSnapshot = captureParameters(processor);
    
    // Perform 10 serialization cycles
    for (int cycle = 0; cycle < 10; ++cycle)
    {
        juce::MemoryBlock stateData;
        processor.getStateInformation(stateData);
        
        REQUIRE(stateData.getSize() > 0);
        
        processor.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));
    }
    
    // Capture final state
    ParameterSnapshot finalSnapshot = captureParameters(processor);
    
    // Verify parameters match after multiple cycles
    bool parametersMatch = snapshotsEqual(initialSnapshot, finalSnapshot, TOLERANCE);
    
    if (!parametersMatch)
    {
        INFO("Parameters changed after multiple serialization cycles");
        printParameterDifferences(initialSnapshot, finalSnapshot, TOLERANCE);
    }
    
    REQUIRE(parametersMatch);
    SUCCEED("Parameters survive multiple serialization cycles without degradation");
}
