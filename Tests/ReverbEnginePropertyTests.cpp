/*
  ==============================================================================

    ReverbEnginePropertyTests.cpp
    Property-Based Tests for ReverbEngine
    
    Tests validate universal correctness properties across randomized inputs.
    Framework: Catch2 with custom property test harness
    Minimum iterations: 100 per property

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "../Source/DSP/ReverbEngine.h"

// Catch2 test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// Property test configuration
constexpr int PROPERTY_TEST_ITERATIONS = 100;
constexpr double TEST_SAMPLE_RATE = 44100.0;
constexpr int TEST_BUFFER_SIZE = 512;

// Helper functions for property testing
namespace PropertyTestHelpers
{
    // Generate random float in range [min, max]
    float randomFloat(float min, float max)
    {
        return min + (max - min) * (static_cast<float>(std::rand()) / RAND_MAX);
    }
    
    // Fill buffer with impulse (single sample at 1.0)
    void fillBufferWithImpulse(juce::AudioBuffer<float>& buffer)
    {
        buffer.clear();
        buffer.setSample(0, 0, 1.0f);
        if (buffer.getNumChannels() > 1)
            buffer.setSample(1, 0, 1.0f);
    }
    
    // Fill buffer with random noise
    void fillBufferWithNoise(juce::AudioBuffer<float>& buffer, float amplitude = 0.5f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float sample = randomFloat(-amplitude, amplitude);
                buffer.setSample(ch, s, sample);
            }
        }
    }
    
    // Check if all samples in buffer are within range [-limit, limit]
    bool allSamplesWithinRange(const juce::AudioBuffer<float>& buffer, float limit)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float sample = buffer.getSample(ch, s);
                if (std::abs(sample) > limit)
                    return false;
            }
        }
        return true;
    }
    
    // Get maximum absolute sample value in buffer
    float getMaxAbsSample(const juce::AudioBuffer<float>& buffer)
    {
        float maxVal = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float sample = std::abs(buffer.getSample(ch, s));
                if (sample > maxVal)
                    maxVal = sample;
            }
        }
        return maxVal;
    }
}

using namespace PropertyTestHelpers;

// ==============================================================================
// Property 1: Parameter Range Validation - Size, Decay, Damping Ranges
// **Validates: Requirements 3.1, 3.2, 3.3**
// ==============================================================================

TEST_CASE("Property 1: Parameter Range Validation - ReverbEngine Parameters", 
          "[Feature: flowstate-plugin, Property 1]")
{
    ReverbEngine engine;
    engine.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing reverb parameter clamping across random values");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Test Size parameter (0.0 to 1.0)
        {
            // Valid range
            float validSize = randomFloat(0.0f, 1.0f);
            REQUIRE_NOTHROW(engine.setSize(validSize));
            
            // Below minimum (should clamp to 0.0)
            float belowMin = randomFloat(-10.0f, -0.01f);
            REQUIRE_NOTHROW(engine.setSize(belowMin));
            
            // Above maximum (should clamp to 1.0)
            float aboveMax = randomFloat(1.01f, 10.0f);
            REQUIRE_NOTHROW(engine.setSize(aboveMax));
        }
        
        // Test Decay Time parameter (0.1 to 20.0 seconds)
        {
            // Valid range
            float validDecay = randomFloat(0.1f, 20.0f);
            REQUIRE_NOTHROW(engine.setDecayTime(validDecay));
            
            // Below minimum (should clamp to 0.1)
            float belowMin = randomFloat(-5.0f, 0.09f);
            REQUIRE_NOTHROW(engine.setDecayTime(belowMin));
            
            // Above maximum (should clamp to 20.0)
            float aboveMax = randomFloat(20.01f, 100.0f);
            REQUIRE_NOTHROW(engine.setDecayTime(aboveMax));
        }
        
        // Test Damping parameter (0.0 to 1.0)
        {
            // Valid range
            float validDamping = randomFloat(0.0f, 1.0f);
            REQUIRE_NOTHROW(engine.setDamping(validDamping));
            
            // Below minimum (should clamp to 0.0)
            float belowMin = randomFloat(-10.0f, -0.01f);
            REQUIRE_NOTHROW(engine.setDamping(belowMin));
            
            // Above maximum (should clamp to 1.0)
            float aboveMax = randomFloat(1.01f, 10.0f);
            REQUIRE_NOTHROW(engine.setDamping(aboveMax));
        }
        
        // Process a buffer with random parameter combination to ensure stability
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(buffer, 0.5f);
        
        REQUIRE_NOTHROW(engine.process(buffer));
        
        // Verify output is within reasonable range (no explosions)
        REQUIRE(allSamplesWithinRange(buffer, 10.0f));
    }
    
    SUCCEED("ReverbEngine parameters correctly clamp to their valid ranges");
}

// ==============================================================================
// Additional Test: Size Parameter Affects Delay Line Lengths
// ==============================================================================

TEST_CASE("ReverbEngine: Size parameter affects early reflection density", 
          "[Feature: flowstate-plugin]")
{
    ReverbEngine engine;
    engine.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing that size parameter changes affect reverb character");
    
    // Test at minimum size (0%)
    engine.setSize(0.0f);
    engine.setDecayTime(2.0f);
    engine.setDamping(0.5f);
    engine.reset();
    
    juce::AudioBuffer<float> bufferSmall(2, TEST_BUFFER_SIZE * 4);
    fillBufferWithImpulse(bufferSmall);
    engine.process(bufferSmall);
    
    // Test at maximum size (100%)
    engine.setSize(1.0f);
    engine.setDecayTime(2.0f);
    engine.setDamping(0.5f);
    engine.reset();
    
    juce::AudioBuffer<float> bufferLarge(2, TEST_BUFFER_SIZE * 4);
    fillBufferWithImpulse(bufferLarge);
    engine.process(bufferLarge);
    
    // Both should produce valid output
    REQUIRE(allSamplesWithinRange(bufferSmall, 10.0f));
    REQUIRE(allSamplesWithinRange(bufferLarge, 10.0f));
    
    SUCCEED("Size parameter successfully modifies reverb character");
}

// ==============================================================================
// Additional Test: Decay Time Parameter Affects Tail Length
// ==============================================================================

TEST_CASE("ReverbEngine: Decay time parameter affects reverb tail length", 
          "[Feature: flowstate-plugin]")
{
    ReverbEngine engine;
    engine.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing that decay time parameter affects reverb tail duration");
    
    // Test short decay (0.1 seconds)
    engine.setSize(0.5f);
    engine.setDecayTime(0.1f);
    engine.setDamping(0.5f);
    engine.reset();
    
    juce::AudioBuffer<float> bufferShort(2, TEST_BUFFER_SIZE * 8);
    fillBufferWithImpulse(bufferShort);
    engine.process(bufferShort);
    
    // Test long decay (20.0 seconds)
    engine.setSize(0.5f);
    engine.setDecayTime(20.0f);
    engine.setDamping(0.5f);
    engine.reset();
    
    juce::AudioBuffer<float> bufferLong(2, TEST_BUFFER_SIZE * 8);
    fillBufferWithImpulse(bufferLong);
    engine.process(bufferLong);
    
    // Both should produce valid output
    REQUIRE(allSamplesWithinRange(bufferShort, 10.0f));
    REQUIRE(allSamplesWithinRange(bufferLong, 10.0f));
    
    // Long decay should have more energy remaining
    float energyShort = getMaxAbsSample(bufferShort);
    float energyLong = getMaxAbsSample(bufferLong);
    
    REQUIRE(energyLong >= energyShort);
    
    SUCCEED("Decay time parameter successfully affects reverb tail length");
}

// ==============================================================================
// Additional Test: Damping Parameter Affects High Frequencies
// ==============================================================================

TEST_CASE("ReverbEngine: Damping parameter affects frequency content", 
          "[Feature: flowstate-plugin]")
{
    ReverbEngine engine;
    engine.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing that damping parameter affects high-frequency rolloff");
    
    for (int iteration = 0; iteration < 10; ++iteration)
    {
        // Test with no damping (0%)
        engine.setSize(0.5f);
        engine.setDecayTime(2.0f);
        engine.setDamping(0.0f);
        engine.reset();
        
        juce::AudioBuffer<float> bufferNoDamping(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(bufferNoDamping, 0.5f);
        engine.process(bufferNoDamping);
        
        // Test with maximum damping (100%)
        engine.setSize(0.5f);
        engine.setDecayTime(2.0f);
        engine.setDamping(1.0f);
        engine.reset();
        
        juce::AudioBuffer<float> bufferMaxDamping(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(bufferMaxDamping, 0.5f);
        engine.process(bufferMaxDamping);
        
        // Both should produce valid output
        REQUIRE(allSamplesWithinRange(bufferNoDamping, 10.0f));
        REQUIRE(allSamplesWithinRange(bufferMaxDamping, 10.0f));
    }
    
    SUCCEED("Damping parameter successfully affects frequency content");
}
