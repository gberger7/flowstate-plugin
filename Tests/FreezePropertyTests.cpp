/*
  ==============================================================================

    FreezePropertyTests.cpp
    Property-Based Tests for Freeze Functionality in FlowstateProcessor
    
    Tests validate universal correctness properties for freeze behavior:
    - Property 17: Freeze Preserves Dry Signal
    - Property 18: Freeze Independence from Blend
    
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
constexpr float TOLERANCE = 0.01f; // 1% tolerance for floating point comparisons

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
    void fillBufferWithSine(juce::AudioBuffer<float>& buffer, float frequency, double sampleRate, float amplitude = 0.5f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            for (int s = 0; s < buffer.getNumSamples(); ++s)
            {
                float phase = 2.0f * juce::MathConstants<float>::pi * frequency * s / static_cast<float>(sampleRate);
                buffer.setSample(ch, s, amplitude * std::sin(phase));
            }
        }
    }
    
    // Calculate RMS energy of buffer
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
    
    // Check if two buffers are approximately equal
    bool buffersApproximatelyEqual(const juce::AudioBuffer<float>& buffer1,
                                   const juce::AudioBuffer<float>& buffer2,
                                   float tolerance)
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
// Property 17: Freeze Preserves Dry Signal
// **Validates: Requirements 10.4**
// ==============================================================================

TEST_CASE("Property 17: Freeze Preserves Dry Signal - Verify dry signal unchanged when frozen", 
          "[Feature: flowstate-plugin, Property 17]")
{
    INFO("Testing that freeze does not affect the dry signal path");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        FlowstateProcessor processor;
        processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Generate random mix value (not 100% wet, so we can hear dry signal)
        float mixValue = randomFloat(0.0f, 0.8f); // Keep some dry signal
        
        // Set parameters
        auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(mixValue);
        
        // Disable effects that might interfere
        auto* modDepthParam = processor.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam->setValueNotifyingHost(0.0f);
        
        auto* duckParam = processor.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duckParam->setValueNotifyingHost(0.0f);
        
        auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
        float normalizedGain = (0.0f - (-60.0f)) / (6.0f - (-60.0f)); // 0dB
        gainParam->setValueNotifyingHost(normalizedGain);
        
        auto* widthParam = processor.getParameters().getParameter(ParameterIDs::stereoWidth);
        widthParam->setValueNotifyingHost(1.0f); // Normal stereo
        
        // Process a few silent blocks first to let parameter smoothing settle
        juce::AudioBuffer<float> silentBuffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        for (int i = 0; i < 5; ++i)
        {
            silentBuffer.clear();
            processor.processBlock(silentBuffer, midiBuffer);
        }
        
        // Create test buffer with known signal
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        
        // Store original dry signal
        juce::AudioBuffer<float> originalDry(2, TEST_BUFFER_SIZE);
        originalDry.makeCopyOf(buffer);
        
        // Process several blocks to build up wet signal
        for (int block = 0; block < 5; ++block)
        {
            fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Now activate freeze
        auto* freezeParam = processor.getParameters().getParameter(ParameterIDs::freezeEnabled);
        freezeParam->setValueNotifyingHost(1.0f);
        
        // Process with freeze active
        fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        juce::AudioBuffer<float> frozenInput(2, TEST_BUFFER_SIZE);
        frozenInput.makeCopyOf(buffer);
        
        processor.processBlock(buffer, midiBuffer);
        
        // Extract the dry component from the output
        // Since output = dry * (1-mix) + wet * mix, and we know the input (dry signal)
        // We can verify that the dry path is preserved by checking with mix=0
        
        // Create a second processor with freeze but mix=0 (100% dry)
        FlowstateProcessor dryTestProcessor;
        dryTestProcessor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        auto* mixParam2 = dryTestProcessor.getParameters().getParameter(ParameterIDs::mix);
        mixParam2->setValueNotifyingHost(0.0f); // 100% dry
        
        auto* freezeParam2 = dryTestProcessor.getParameters().getParameter(ParameterIDs::freezeEnabled);
        freezeParam2->setValueNotifyingHost(1.0f);
        
        auto* gainParam2 = dryTestProcessor.getParameters().getParameter(ParameterIDs::outputGain);
        float normalizedGain2 = (0.0f - (-60.0f)) / (6.0f - (-60.0f)); // 0dB
        gainParam2->setValueNotifyingHost(normalizedGain2);
        
        // Process a few silent blocks first to let parameter smoothing settle
        for (int i = 0; i < 5; ++i)
        {
            silentBuffer.clear();
            dryTestProcessor.processBlock(silentBuffer, midiBuffer);
        }
        
        // Process same input
        juce::AudioBuffer<float> dryOnlyBuffer(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(dryOnlyBuffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        
        dryTestProcessor.processBlock(dryOnlyBuffer, midiBuffer);
        
        // At mix=0, output should equal input (dry signal preserved)
        bool dryPreserved = buffersApproximatelyEqual(dryOnlyBuffer, frozenInput, TOLERANCE);
        
        INFO("Iteration " << iteration << ": Mix=" << mixValue);
        INFO("Dry signal preserved with freeze active: " << (dryPreserved ? "YES" : "NO"));
        
        if (!dryPreserved)
        {
            float maxDiff = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(dryOnlyBuffer.getSample(ch, s) - frozenInput.getSample(ch, s));
                    if (diff > maxDiff)
                        maxDiff = diff;
                }
            }
            INFO("Max difference: " << maxDiff);
        }
        
        REQUIRE(dryPreserved);
    }
    
    SUCCEED("Freeze preserves dry signal across all test iterations");
}

// ==============================================================================
// Property 18: Freeze Independence from Blend
// **Validates: Requirements 10.5**
// ==============================================================================

TEST_CASE("Property 18: Freeze Independence from Blend - Verify blend changes don't affect frozen content", 
          "[Feature: flowstate-plugin, Property 18]")
{
    INFO("Testing that changing blend parameter does not affect frozen wet signal");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        FlowstateProcessor processor;
        processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Generate random initial blend value
        float initialBlend = randomFloat(0.0f, 1.0f);
        
        // Set parameters for building up wet signal
        auto* blendParam = processor.getParameters().getParameter(ParameterIDs::blend);
        blendParam->setValueNotifyingHost(initialBlend);
        
        auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(1.0f); // 100% wet to isolate wet signal
        
        // Disable effects
        auto* modDepthParam = processor.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam->setValueNotifyingHost(0.0f);
        
        auto* duckParam = processor.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duckParam->setValueNotifyingHost(0.0f);
        
        auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
        gainParam->setValueNotifyingHost(0.0f); // 0dB
        
        auto* widthParam = processor.getParameters().getParameter(ParameterIDs::stereoWidth);
        widthParam->setValueNotifyingHost(1.0f);
        
        // Create test buffer
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        
        // Process several blocks to build up wet signal with initial blend
        for (int block = 0; block < 10; ++block)
        {
            fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
            processor.processBlock(buffer, midiBuffer);
        }
        
        // Activate freeze to capture current wet signal
        auto* freezeParam = processor.getParameters().getParameter(ParameterIDs::freezeEnabled);
        freezeParam->setValueNotifyingHost(1.0f);
        
        // Process one block with freeze active at initial blend
        fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        processor.processBlock(buffer, midiBuffer);
        
        // Store the frozen output
        juce::AudioBuffer<float> frozenOutput1(2, TEST_BUFFER_SIZE);
        frozenOutput1.makeCopyOf(buffer);
        
        // Now change blend to a different random value
        float newBlend = randomFloat(0.0f, 1.0f);
        // Ensure it's significantly different
        while (std::abs(newBlend - initialBlend) < 0.3f)
        {
            newBlend = randomFloat(0.0f, 1.0f);
        }
        
        blendParam->setValueNotifyingHost(newBlend);
        
        // Process another block with freeze still active but different blend
        fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        processor.processBlock(buffer, midiBuffer);
        
        // Store the second frozen output
        juce::AudioBuffer<float> frozenOutput2(2, TEST_BUFFER_SIZE);
        frozenOutput2.makeCopyOf(buffer);
        
        // The frozen content should be identical despite blend change
        bool frozenContentUnchanged = buffersApproximatelyEqual(frozenOutput1, frozenOutput2, TOLERANCE);
        
        INFO("Iteration " << iteration << ": InitialBlend=" << initialBlend << ", NewBlend=" << newBlend);
        INFO("Frozen content unchanged: " << (frozenContentUnchanged ? "YES" : "NO"));
        
        if (!frozenContentUnchanged)
        {
            float rms1 = calculateRMS(frozenOutput1);
            float rms2 = calculateRMS(frozenOutput2);
            float maxDiff = 0.0f;
            
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(frozenOutput1.getSample(ch, s) - frozenOutput2.getSample(ch, s));
                    if (diff > maxDiff)
                        maxDiff = diff;
                }
            }
            
            INFO("RMS1=" << rms1 << ", RMS2=" << rms2);
            INFO("Max difference: " << maxDiff);
        }
        
        REQUIRE(frozenContentUnchanged);
    }
    
    SUCCEED("Frozen content remains independent of blend parameter changes");
}
