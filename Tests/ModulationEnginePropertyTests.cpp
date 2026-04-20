/*
  ==============================================================================

    ModulationEnginePropertyTests.cpp
    Property-Based Tests for ModulationEngine
    
    Tests validate universal correctness properties across randomized inputs.
    Framework: Catch2 with custom property test harness
    Minimum iterations: 100 per property

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "../Source/DSP/ModulationEngine.h"

// Catch2 test framework
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// Property test configuration
constexpr int PROPERTY_TEST_ITERATIONS = 100;
constexpr double TEST_SAMPLE_RATE = 44100.0;

// Helper functions for property testing
namespace PropertyTestHelpers
{
    // Generate random float in range [min, max]
    float randomFloat(float min, float max)
    {
        return min + (max - min) * (static_cast<float>(std::rand()) / RAND_MAX);
    }
    
    // Check if two floats are approximately equal within tolerance
    bool approximatelyEqual(float a, float b, float tolerance)
    {
        return std::abs(a - b) <= tolerance;
    }
}

using namespace PropertyTestHelpers;

// ==============================================================================
// Property 8: Modulation Depth Zero Bypass
// **Validates: Requirements 5.3**
// ==============================================================================

TEST_CASE("Property 8: Modulation Depth Zero Bypass - No modulation when depth=0%", 
          "[Feature: flowstate-plugin, Property 8]")
{
    ModulationEngine engine;
    engine.prepare(TEST_SAMPLE_RATE);
    
    INFO("Testing that depth=0% produces no modulation regardless of rate");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Reset engine for clean test
        engine.reset();
        
        // Set random rate (should not matter when depth is 0)
        float rate = randomFloat(0.01f, 5.0f);
        engine.setRate(rate);
        
        // Set depth to 0%
        engine.setDepth(0.0f);
        
        // Sample modulation values over multiple cycles
        bool allValuesZero = true;
        int samplesToTest = static_cast<int>(TEST_SAMPLE_RATE * 2.0); // 2 seconds worth
        
        for (int i = 0; i < samplesToTest; ++i)
        {
            float modValue = engine.getNextModulationValue();
            
            // With depth=0%, all modulation values should be exactly 0.0
            if (modValue != 0.0f)
            {
                allValuesZero = false;
                INFO("Non-zero modulation value detected: " << modValue);
                INFO("Rate: " << rate << " Hz, Sample: " << i);
                break;
            }
        }
        
        REQUIRE(allValuesZero);
    }
    
    SUCCEED("Modulation depth=0% produces no modulation output");
}

// ==============================================================================
// Property 10: LFO Waveform Shape
// **Validates: Requirements 5.5**
// ==============================================================================

TEST_CASE("Property 10: LFO Waveform Shape - Validate sine wave pattern within 2% tolerance", 
          "[Feature: flowstate-plugin, Property 10]")
{
    ModulationEngine engine;
    engine.prepare(TEST_SAMPLE_RATE);
    
    INFO("Testing that LFO produces accurate sine wave pattern");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Reset engine for clean test
        engine.reset();
        
        // Set random rate
        float rate = randomFloat(0.01f, 5.0f);
        engine.setRate(rate);
        
        // Set depth to 100% for full amplitude testing
        engine.setDepth(1.0f);
        
        // Calculate samples per cycle
        int samplesPerCycle = static_cast<int>(TEST_SAMPLE_RATE / rate);
        
        // Sample one complete cycle
        std::vector<float> samples;
        samples.reserve(samplesPerCycle);
        
        for (int i = 0; i < samplesPerCycle; ++i)
        {
            samples.push_back(engine.getNextModulationValue());
        }
        
        // Verify sine wave properties
        bool waveformValid = true;
        float maxError = 0.0f;
        
        for (int i = 0; i < samplesPerCycle; ++i)
        {
            // Calculate expected sine value at this phase
            float phase = static_cast<float>(i) / static_cast<float>(samplesPerCycle);
            float expectedValue = std::sin(phase * 2.0f * juce::MathConstants<float>::pi);
            
            // Compare with actual value
            float actualValue = samples[i];
            float error = std::abs(expectedValue - actualValue);
            
            if (error > maxError)
                maxError = error;
            
            // Check if within 2% tolerance (0.02 absolute difference for normalized values)
            // This accounts for floating-point accumulation errors at very low frequencies
            if (error > 0.02f)
            {
                waveformValid = false;
                INFO("Sine wave deviation exceeds 2% tolerance");
                INFO("Rate: " << rate << " Hz, Sample: " << i << "/" << samplesPerCycle);
                INFO("Expected: " << expectedValue << ", Actual: " << actualValue);
                INFO("Error: " << error << " (max tolerance: 0.02)");
                break;
            }
        }
        
        REQUIRE(waveformValid);
        REQUIRE(maxError <= 0.02f);
        
        // Additional checks: verify peak values are close to ±1.0
        float maxValue = *std::max_element(samples.begin(), samples.end());
        float minValue = *std::min_element(samples.begin(), samples.end());
        
        REQUIRE(approximatelyEqual(maxValue, 1.0f, 0.02f));
        REQUIRE(approximatelyEqual(minValue, -1.0f, 0.02f));
    }
    
    SUCCEED("LFO produces accurate sine wave within 2% tolerance");
}
