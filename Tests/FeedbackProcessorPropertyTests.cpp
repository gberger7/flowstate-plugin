/*
  ==============================================================================

    FeedbackProcessorPropertyTests.cpp
    Property-Based Tests for FeedbackProcessor
    
    Tests validate universal correctness properties across randomized inputs.
    Framework: Catch2 with custom property test harness
    Minimum iterations: 100 per property

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include "../Source/DSP/FeedbackProcessor.h"
#include "../Source/DSP/ShimmerProcessor.h"

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
    void fillBufferWithSine(juce::AudioBuffer<float>& buffer, float frequency, double sampleRate)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float phase = juce::MathConstants<float>::twoPi * frequency * s / static_cast<float>(sampleRate);
                buffer.setSample(ch, s, std::sin(phase));
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
    
    // Calculate spectral centroid (simplified - measures high frequency content)
    float calculateSpectralCentroid(const juce::AudioBuffer<float>& buffer, double sampleRate)
    {
        // Simple approximation: measure high frequency energy vs total energy
        float totalEnergy = 0.0f;
        float highFreqEnergy = 0.0f;
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 1; s < buffer.getNumSamples(); ++s)
            {
                float sample = buffer.getSample(ch, s);
                float prevSample = buffer.getSample(ch, s - 1);
                float diff = sample - prevSample;
                
                totalEnergy += sample * sample;
                highFreqEnergy += diff * diff;
            }
        }
        
        if (totalEnergy < 0.0001f)
            return 0.0f;
        
        return highFreqEnergy / totalEnergy;
    }
}

using namespace PropertyTestHelpers;

// ==============================================================================
// Property 11: Drive Zero Bypass
// **Validates: Requirements 6.2**
// ==============================================================================

TEST_CASE("Property 11: Drive Zero Bypass - No saturation when drive=0%", 
          "[Feature: flowstate-plugin, Property 11]")
{
    FeedbackProcessor processor;
    processor.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing that drive=0% produces no saturation (output equals input)");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Set drive to 0%
        processor.setDrive(0.0f);
        
        // Set tone to 0% to isolate drive effect
        processor.setTone(0.0f);
        
        // Create input buffer with random noise
        juce::AudioBuffer<float> inputBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithNoise(inputBuffer, 0.8f);
        
        // Make a copy of the input for comparison
        juce::AudioBuffer<float> expectedOutput(2, TEST_BUFFER_SIZE);
        expectedOutput.makeCopyOf(inputBuffer);
        
        // Make a copy for processing
        juce::AudioBuffer<float> processedBuffer(2, TEST_BUFFER_SIZE);
        processedBuffer.makeCopyOf(inputBuffer);
        
        // Process with drive=0%
        processor.process(processedBuffer, nullptr, false);
        
        // Verify output equals input (no saturation applied)
        // Note: Tone filter at 0% should also be bypass, so output should match input exactly
        bool buffersMatch = buffersAreEqual(expectedOutput, processedBuffer, 0.001f);
        
        if (!buffersMatch)
        {
            INFO("Iteration " << iteration << " failed");
            INFO("Drive=0% should produce no saturation");
            
            // Calculate difference for debugging
            float maxDiff = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(expectedOutput.getSample(ch, s) - processedBuffer.getSample(ch, s));
                    if (diff > maxDiff)
                        maxDiff = diff;
                }
            }
            INFO("Maximum difference: " << maxDiff);
        }
        
        REQUIRE(buffersMatch);
    }
    
    SUCCEED("Drive=0% correctly bypasses saturation processing");
}

// ==============================================================================
// Property 12: Tone Zero Bypass
// **Validates: Requirements 6.6**
// ==============================================================================

TEST_CASE("Property 12: Tone Zero Bypass - No filtering when tone=0%", 
          "[Feature: flowstate-plugin, Property 12]")
{
    FeedbackProcessor processor;
    processor.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    
    INFO("Testing that tone=0% preserves frequency spectrum (no filtering)");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Set tone to 0% (should preserve full frequency spectrum)
        processor.setTone(0.0f);
        
        // Set drive to 0% to isolate tone effect
        processor.setDrive(0.0f);
        
        // Create input buffer with sine wave at random frequency
        float testFrequency = randomFloat(100.0f, 10000.0f);
        juce::AudioBuffer<float> inputBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(inputBuffer, testFrequency, TEST_SAMPLE_RATE);
        
        // Calculate input spectral characteristics
        float inputCentroid = calculateSpectralCentroid(inputBuffer, TEST_SAMPLE_RATE);
        float inputRMS = calculateRMS(inputBuffer);
        
        // Make a copy for processing
        juce::AudioBuffer<float> processedBuffer(2, TEST_BUFFER_SIZE);
        processedBuffer.makeCopyOf(inputBuffer);
        
        // Process with tone=0%
        processor.process(processedBuffer, nullptr, false);
        
        // Calculate output spectral characteristics
        float outputCentroid = calculateSpectralCentroid(processedBuffer, TEST_SAMPLE_RATE);
        float outputRMS = calculateRMS(processedBuffer);
        
        // Verify frequency content is preserved
        // With tone=0%, the cutoff should be at 20kHz (no filtering for audio range)
        // Allow small tolerance for filter initialization transients
        float centroidDiff = std::abs(outputCentroid - inputCentroid);
        float centroidTolerance = inputCentroid * 0.1f; // 10% tolerance
        
        float rmsDiff = std::abs(outputRMS - inputRMS);
        float rmsTolerance = inputRMS * 0.05f; // 5% tolerance
        
        if (centroidDiff > centroidTolerance || rmsDiff > rmsTolerance)
        {
            INFO("Iteration " << iteration << " failed");
            INFO("Test frequency: " << testFrequency << " Hz");
            INFO("Input centroid: " << inputCentroid << ", Output centroid: " << outputCentroid);
            INFO("Centroid difference: " << centroidDiff << " (tolerance: " << centroidTolerance << ")");
            INFO("Input RMS: " << inputRMS << ", Output RMS: " << outputRMS);
            INFO("RMS difference: " << rmsDiff << " (tolerance: " << rmsTolerance << ")");
        }
        
        REQUIRE(centroidDiff <= centroidTolerance);
        REQUIRE(rmsDiff <= rmsTolerance);
    }
    
    SUCCEED("Tone=0% correctly preserves frequency spectrum without filtering");
}
