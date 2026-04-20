/*
  ==============================================================================

    DuckingProcessorPropertyTests.cpp
    Property-Based Tests for DuckingProcessor
    
    Tests validate universal correctness properties across randomized inputs.
    Framework: Catch2 with custom property test harness
    Minimum iterations: 100 per property

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "../Source/DSP/DuckingProcessor.h"

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
    
    // Fill buffer with sine wave
    void fillBufferWithSine(juce::AudioBuffer<float>& buffer, float frequency, double sampleRate, float amplitude = 1.0f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float phase = juce::MathConstants<float>::twoPi * frequency * s / static_cast<float>(sampleRate);
                buffer.setSample(ch, s, amplitude * std::sin(phase));
            }
        }
    }
    
    // Compare two buffers for equality within tolerance
    bool buffersAreEqual(const juce::AudioBuffer<float>& buffer1, 
                         const juce::AudioBuffer<float>& buffer2, 
                         float tolerance = 0.0001f)
    {
        if (buffer1.getNumChannels() != buffer2.getNumChannels() ||
            buffer1.getNumSamples() != buffer2.getNumSamples())
            return false;
        
        for (int ch = 0; ch < buffer1.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer1.getNumSamples(); ++s)
            {
                float diff = std::abs(buffer1.getSample(ch, s) - buffer2.getSample(ch, s));
                if (diff > tolerance)
                    return false;
            }
        }
        return true;
    }
    
    // Calculate RMS of buffer
    float calculateRMS(const juce::AudioBuffer<float>& buffer)
    {
        float sumSquares = 0.0f;
        int totalSamples = 0;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float sample = buffer.getSample(ch, s);
                sumSquares += sample * sample;
                totalSamples++;
            }
        }
        
        return std::sqrt(sumSquares / totalSamples);
    }
}

using namespace PropertyTestHelpers;

// ==============================================================================
// Property 13: Ducking Sensitivity Zero Bypass
// **Validates: Requirements 7.3**
// ==============================================================================

TEST_CASE("Property 13: Ducking Sensitivity Zero Bypass - No attenuation when sensitivity=0%", 
          "[Feature: flowstate-plugin, Property 13]")
{
    DuckingProcessor processor;
    processor.prepare(TEST_SAMPLE_RATE);
    
    INFO("Testing that sensitivity=0% produces no attenuation regardless of dry signal level");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Set sensitivity to 0%
        processor.setSensitivity(0.0f);
        
        // Create dry buffer with random amplitude (to simulate various input levels)
        float dryAmplitude = randomFloat(0.1f, 1.0f);
        juce::AudioBuffer<float> dryBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(dryBuffer, dryAmplitude);
        
        // Create wet buffer with random noise
        juce::AudioBuffer<float> wetBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(wetBuffer, 0.5f);
        
        // Make a copy of the wet buffer for comparison
        juce::AudioBuffer<float> expectedWetBuffer(2, TEST_BUFFER_SIZE);
        expectedWetBuffer.makeCopyOf(wetBuffer);
        
        // Process envelope from dry signal
        float envelope = processor.processEnvelope(dryBuffer);
        
        // Apply ducking to wet signal
        processor.applyDucking(wetBuffer, envelope);
        
        // Verify wet buffer is unchanged (no attenuation applied)
        bool buffersMatch = buffersAreEqual(expectedWetBuffer, wetBuffer, 0.0001f);
        
        if (!buffersMatch)
        {
            INFO("Iteration " << iteration << " failed");
            INFO("Sensitivity=0% should produce no attenuation");
            INFO("Dry amplitude: " << dryAmplitude);
            INFO("Envelope value: " << envelope);
            
            // Calculate difference for debugging
            float maxDiff = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(expectedWetBuffer.getSample(ch, s) - wetBuffer.getSample(ch, s));
                    if (diff > maxDiff)
                        maxDiff = diff;
                }
            }
            INFO("Maximum difference: " << maxDiff);
        }
        
        REQUIRE(buffersMatch);
    }
    
    SUCCEED("Sensitivity=0% correctly bypasses ducking attenuation");
}

// ==============================================================================
// Property 14: Ducking Attenuates Wet Only
// **Validates: Requirements 7.4, 7.6**
// ==============================================================================

TEST_CASE("Property 14: Ducking Attenuates Wet Only - Verify dry signal unchanged", 
          "[Feature: flowstate-plugin, Property 14]")
{
    DuckingProcessor processor;
    processor.prepare(TEST_SAMPLE_RATE);
    
    INFO("Testing that ducking only attenuates wet signal while preserving dry signal");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Set random sensitivity > 0% to enable ducking
        float sensitivity = randomFloat(0.1f, 1.0f);
        processor.setSensitivity(sensitivity);
        
        // Create dry buffer with high amplitude to trigger ducking
        float dryAmplitude = randomFloat(0.5f, 1.0f);
        juce::AudioBuffer<float> dryBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(dryBuffer, 440.0f, TEST_SAMPLE_RATE, dryAmplitude);
        
        // Make a copy of the dry buffer for comparison
        juce::AudioBuffer<float> originalDryBuffer(2, TEST_BUFFER_SIZE);
        originalDryBuffer.makeCopyOf(dryBuffer);
        
        // Create wet buffer with random noise
        juce::AudioBuffer<float> wetBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(wetBuffer, 0.5f);
        
        // Calculate original wet RMS
        float originalWetRMS = calculateRMS(wetBuffer);
        
        // Process envelope from dry signal
        float envelope = processor.processEnvelope(dryBuffer);
        
        // Apply ducking to wet signal
        processor.applyDucking(wetBuffer, envelope);
        
        // Calculate processed wet RMS
        float processedWetRMS = calculateRMS(wetBuffer);
        
        // Verify dry buffer is completely unchanged
        bool dryBufferUnchanged = buffersAreEqual(originalDryBuffer, dryBuffer, 0.0f);
        
        if (!dryBufferUnchanged)
        {
            INFO("Iteration " << iteration << " failed - dry buffer was modified");
            INFO("Sensitivity: " << sensitivity);
            INFO("Dry amplitude: " << dryAmplitude);
            INFO("Envelope value: " << envelope);
        }
        
        REQUIRE(dryBufferUnchanged);
        
        // Verify wet signal was attenuated (RMS should be lower or equal)
        // When sensitivity > 0 and dry signal is present, wet should be attenuated
        bool wetWasAttenuated = (processedWetRMS <= originalWetRMS);
        
        if (!wetWasAttenuated)
        {
            INFO("Iteration " << iteration << " failed - wet signal was not attenuated");
            INFO("Sensitivity: " << sensitivity);
            INFO("Dry amplitude: " << dryAmplitude);
            INFO("Envelope value: " << envelope);
            INFO("Original wet RMS: " << originalWetRMS);
            INFO("Processed wet RMS: " << processedWetRMS);
        }
        
        REQUIRE(wetWasAttenuated);
        
        // Additional check: with high sensitivity and high dry amplitude,
        // wet signal should be significantly attenuated
        if (sensitivity > 0.5f && dryAmplitude > 0.7f)
        {
            float attenuationRatio = processedWetRMS / originalWetRMS;
            bool significantAttenuation = (attenuationRatio < 0.9f); // At least 10% reduction
            
            if (!significantAttenuation)
            {
                INFO("Iteration " << iteration << " - expected significant attenuation");
                INFO("Sensitivity: " << sensitivity << " (>0.5)");
                INFO("Dry amplitude: " << dryAmplitude << " (>0.7)");
                INFO("Attenuation ratio: " << attenuationRatio << " (expected <0.9)");
            }
            
            REQUIRE(significantAttenuation);
        }
    }
    
    SUCCEED("Ducking correctly attenuates only wet signal while preserving dry signal");
}
