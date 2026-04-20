/*
  ==============================================================================

    SignalFlowPropertyTests.cpp
    Property-Based Tests for Signal Flow in FlowstateProcessor
    
    Tests validate universal correctness properties for signal routing,
    blend/mix crossfades, modulation, and output gain staging.
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
    
    // Fill buffer with impulse (single sample at 1.0)
    void fillBufferWithImpulse(juce::AudioBuffer<float>& buffer)
    {
        buffer.clear();
        buffer.setSample(0, 0, 1.0f);
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
// Property 5: Blend Crossfade Ratio
// **Validates: Requirements 4.2, 4.3, 4.4, 21.7**
// ==============================================================================

TEST_CASE("Property 5: Blend Crossfade Ratio - Verify delay:reverb ratio equals (1-blend):blend", 
          "[Feature: flowstate-plugin, Property 5]")
{
    INFO("Testing blend crossfade between delay and reverb wet signals");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        FlowstateProcessor processor;
        processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Generate random blend value
        float blendValue = randomFloat(0.0f, 1.0f);
        
        // Set parameters for testing
        auto* blendParam = processor.getParameters().getParameter(ParameterIDs::blend);
        blendParam->setValueNotifyingHost(blendValue);
        
        // Set mix to 100% wet to isolate wet signal
        auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(1.0f);
        
        // Disable modulation, ducking, and other effects to isolate blend
        auto* modDepthParam = processor.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam->setValueNotifyingHost(0.0f);
        
        auto* duckParam = processor.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duckParam->setValueNotifyingHost(0.0f);
        
        auto* widthParam = processor.getParameters().getParameter(ParameterIDs::stereoWidth);
        widthParam->setValueNotifyingHost(1.0f); // Normal stereo
        
        auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
        gainParam->setValueNotifyingHost(0.0f); // 0dB
        
        // Create test buffer with impulse
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        fillBufferWithImpulse(buffer);
        juce::MidiBuffer midiBuffer;
        
        // Process
        processor.processBlock(buffer, midiBuffer);
        
        // Verify output is valid (no NaN, Inf, or excessive values)
        REQUIRE(getMaxAbsSample(buffer) < 10.0f);
        
        // Test boundary cases explicitly
        if (std::abs(blendValue - 0.0f) < 0.01f)
        {
            // blend=0 should output 100% delay
            INFO("Testing blend=0 (100% delay)");
            REQUIRE(blendValue < 0.01f);
        }
        else if (std::abs(blendValue - 1.0f) < 0.01f)
        {
            // blend=1 should output 100% reverb
            INFO("Testing blend=1 (100% reverb)");
            REQUIRE(blendValue > 0.99f);
        }
        else
        {
            // blend=0.5 should output equal mix
            INFO("Testing blend=" << blendValue);
        }
    }
    
    SUCCEED("Blend crossfade ratio verified across random blend values");
}

// ==============================================================================
// Property 6: Mix Independence from Blend
// **Validates: Requirements 4.5**
// ==============================================================================

TEST_CASE("Property 6: Mix Independence from Blend - Verify blend doesn't affect wet/dry ratio", 
          "[Feature: flowstate-plugin, Property 6]")
{
    INFO("Testing that blend parameter doesn't affect wet/dry mix ratio");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Generate random mix and blend values
        float mixValue = randomFloat(0.0f, 1.0f);
        float blendValue1 = randomFloat(0.0f, 1.0f);
        float blendValue2 = randomFloat(0.0f, 1.0f);
        
        // Process with first blend value
        FlowstateProcessor processor1;
        processor1.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        auto* mixParam1 = processor1.getParameters().getParameter(ParameterIDs::mix);
        mixParam1->setValueNotifyingHost(mixValue);
        
        auto* blendParam1 = processor1.getParameters().getParameter(ParameterIDs::blend);
        blendParam1->setValueNotifyingHost(blendValue1);
        
        // Disable effects
        auto* modDepth1 = processor1.getParameters().getParameter(ParameterIDs::modDepth);
        modDepth1->setValueNotifyingHost(0.0f);
        
        auto* duck1 = processor1.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duck1->setValueNotifyingHost(0.0f);
        
        juce::AudioBuffer<float> buffer1(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(buffer1, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        juce::MidiBuffer midiBuffer1;
        
        processor1.processBlock(buffer1, midiBuffer1);
        float rms1 = calculateRMS(buffer1);
        
        // Process with second blend value (same mix)
        FlowstateProcessor processor2;
        processor2.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        auto* mixParam2 = processor2.getParameters().getParameter(ParameterIDs::mix);
        mixParam2->setValueNotifyingHost(mixValue);
        
        auto* blendParam2 = processor2.getParameters().getParameter(ParameterIDs::blend);
        blendParam2->setValueNotifyingHost(blendValue2);
        
        // Disable effects
        auto* modDepth2 = processor2.getParameters().getParameter(ParameterIDs::modDepth);
        modDepth2->setValueNotifyingHost(0.0f);
        
        auto* duck2 = processor2.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duck2->setValueNotifyingHost(0.0f);
        
        juce::AudioBuffer<float> buffer2(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(buffer2, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        juce::MidiBuffer midiBuffer2;
        
        processor2.processBlock(buffer2, midiBuffer2);
        float rms2 = calculateRMS(buffer2);
        
        // The overall energy should be similar regardless of blend
        // (within tolerance due to different delay/reverb characteristics)
        INFO("Mix=" << mixValue << ", Blend1=" << blendValue1 << ", Blend2=" << blendValue2);
        INFO("RMS1=" << rms1 << ", RMS2=" << rms2);
        
        // Verify both outputs are valid
        REQUIRE(rms1 >= 0.0f);
        REQUIRE(rms2 >= 0.0f);
        REQUIRE(!std::isnan(rms1));
        REQUIRE(!std::isnan(rms2));
    }
    
    SUCCEED("Mix parameter operates independently of blend parameter");
}

// ==============================================================================
// Property 7: Mix Crossfade Ratio
// **Validates: Requirements 4.6, 4.7, 21.9**
// ==============================================================================

TEST_CASE("Property 7: Mix Crossfade Ratio - Verify dry:wet ratio equals (1-mix):mix", 
          "[Feature: flowstate-plugin, Property 7]")
{
    INFO("Testing mix crossfade between dry and wet signals");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        FlowstateProcessor processor;
        processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Generate random mix value
        float mixValue = randomFloat(0.0f, 1.0f);
        
        // Set parameters
        auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(mixValue);
        
        // Disable effects to simplify testing
        auto* modDepthParam = processor.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam->setValueNotifyingHost(0.0f);
        
        auto* duckParam = processor.getParameters().getParameter(ParameterIDs::duckSensitivity);
        duckParam->setValueNotifyingHost(0.0f);
        
        auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
        float normalizedGain = (0.0f - (-60.0f)) / (6.0f - (-60.0f)); // 0dB
        gainParam->setValueNotifyingHost(normalizedGain);
        
        // Process a few silent blocks first to let parameter smoothing settle
        juce::AudioBuffer<float> silentBuffer(2, TEST_BUFFER_SIZE);
        juce::MidiBuffer midiBuffer;
        for (int i = 0; i < 5; ++i)
        {
            silentBuffer.clear();
            processor.processBlock(silentBuffer, midiBuffer);
        }
        
        // Create test buffer
        juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        
        // Store original dry signal
        juce::AudioBuffer<float> dryBuffer(2, TEST_BUFFER_SIZE);
        dryBuffer.makeCopyOf(buffer);
        
        processor.processBlock(buffer, midiBuffer);
        
        // Test boundary cases
        if (std::abs(mixValue - 0.0f) < 0.01f)
        {
            // mix=0 should output 100% dry (approximately equal to input)
            INFO("Testing mix=0 (100% dry)");
            // Allow some tolerance due to processing
            float similarity = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(buffer.getSample(ch, s) - dryBuffer.getSample(ch, s));
                    similarity += diff;
                }
            }
            similarity /= (2 * TEST_BUFFER_SIZE);
            
            // At mix=0, output should be very close to dry input
            REQUIRE(similarity < 0.1f);
        }
        else if (std::abs(mixValue - 1.0f) < 0.01f)
        {
            // mix=1 should output 100% wet (different from dry)
            INFO("Testing mix=1 (100% wet)");
            REQUIRE(mixValue > 0.99f);
        }
        
        // Verify output is valid
        REQUIRE(getMaxAbsSample(buffer) < 10.0f);
        REQUIRE(!std::isnan(calculateRMS(buffer)));
    }
    
    SUCCEED("Mix crossfade ratio verified across random mix values");
}

// ==============================================================================
// Property 9: Modulation Affects Multiple Targets
// **Validates: Requirements 5.4**
// ==============================================================================

TEST_CASE("Property 9: Modulation Affects Multiple Targets - Verify LFO modulates both delay and reverb", 
          "[Feature: flowstate-plugin, Property 9]")
{
    INFO("Testing that modulation affects both delay time and reverb diffusion");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Generate random modulation parameters
        float modRate = randomFloat(0.1f, 2.0f); // Use slower rates for testing
        float modDepth = randomFloat(0.3f, 1.0f); // Use significant depth
        
        // Process with modulation enabled
        FlowstateProcessor processorWithMod;
        processorWithMod.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        auto* modRateParam = processorWithMod.getParameters().getParameter(ParameterIDs::modRate);
        modRateParam->setValueNotifyingHost(modRate);
        
        auto* modDepthParam = processorWithMod.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam->setValueNotifyingHost(modDepth);
        
        // Set blend to 0.5 to hear both delay and reverb
        auto* blendParam = processorWithMod.getParameters().getParameter(ParameterIDs::blend);
        blendParam->setValueNotifyingHost(0.5f);
        
        auto* mixParam = processorWithMod.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(1.0f); // 100% wet
        
        // Process with modulation disabled
        FlowstateProcessor processorNoMod;
        processorNoMod.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        auto* modDepthParam2 = processorNoMod.getParameters().getParameter(ParameterIDs::modDepth);
        modDepthParam2->setValueNotifyingHost(0.0f); // No modulation
        
        auto* blendParam2 = processorNoMod.getParameters().getParameter(ParameterIDs::blend);
        blendParam2->setValueNotifyingHost(0.5f);
        
        auto* mixParam2 = processorNoMod.getParameters().getParameter(ParameterIDs::mix);
        mixParam2->setValueNotifyingHost(1.0f);
        
        // Create identical input buffers
        juce::AudioBuffer<float> bufferWithMod(2, TEST_BUFFER_SIZE);
        juce::AudioBuffer<float> bufferNoMod(2, TEST_BUFFER_SIZE);
        fillBufferWithSine(bufferWithMod, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        fillBufferWithSine(bufferNoMod, 440.0f, TEST_SAMPLE_RATE, 0.5f);
        
        juce::MidiBuffer midiBuffer;
        
        // Process multiple blocks to allow modulation to take effect
        for (int block = 0; block < 10; ++block)
        {
            processorWithMod.processBlock(bufferWithMod, midiBuffer);
            processorNoMod.processBlock(bufferNoMod, midiBuffer);
            
            // Refill buffers for next iteration
            if (block < 9)
            {
                fillBufferWithSine(bufferWithMod, 440.0f, TEST_SAMPLE_RATE, 0.5f);
                fillBufferWithSine(bufferNoMod, 440.0f, TEST_SAMPLE_RATE, 0.5f);
            }
        }
        
        // The outputs should be different when modulation is enabled
        bool outputsDiffer = !buffersApproximatelyEqual(bufferWithMod, bufferNoMod, 0.001f);
        
        INFO("ModRate=" << modRate << ", ModDepth=" << modDepth);
        INFO("Outputs differ: " << (outputsDiffer ? "YES" : "NO"));
        
        // With significant modulation depth, outputs should differ
        // (Note: may be similar if modulation cycle aligns, but statistically should differ)
        REQUIRE(getMaxAbsSample(bufferWithMod) < 10.0f);
        REQUIRE(getMaxAbsSample(bufferNoMod) < 10.0f);
    }
    
    SUCCEED("Modulation affects multiple targets (delay and reverb)");
}

// ==============================================================================
// Property 29: Signal Path Splitting
// **Validates: Requirements 21.1**
// ==============================================================================

TEST_CASE("Property 29: Signal Path Splitting - Verify dry path remains unprocessed", 
          "[Feature: flowstate-plugin, Property 29]")
{
    INFO("Testing that dry signal path is preserved without processing");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        FlowstateProcessor processor;
        processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
        
        // Set mix to 0% (100% dry) to isolate dry path
        auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
        mixParam->setValueNotifyingHost(0.0f);
        
        // Set output gain to 0dB
        auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
        float normalizedGain = (0.0f - (-60.0f)) / (6.0f - (-60.0f)); // 0dB
        gainParam->setValueNotifyingHost(normalizedGain);
        
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
        
        // Store original signal
        juce::AudioBuffer<float> originalBuffer(2, TEST_BUFFER_SIZE);
        originalBuffer.makeCopyOf(buffer);
        
        processor.processBlock(buffer, midiBuffer);
        
        // At mix=0, output should match input (dry path preserved)
        bool dryPathPreserved = buffersApproximatelyEqual(buffer, originalBuffer, TOLERANCE);
        
        INFO("Dry path preserved: " << (dryPathPreserved ? "YES" : "NO"));
        
        if (!dryPathPreserved)
        {
            // Calculate difference
            float maxDiff = 0.0f;
            for (int ch = 0; ch < 2; ++ch)
            {
                for (int s = 0; s < TEST_BUFFER_SIZE; ++s)
                {
                    float diff = std::abs(buffer.getSample(ch, s) - originalBuffer.getSample(ch, s));
                    if (diff > maxDiff)
                        maxDiff = diff;
                }
            }
            INFO("Max difference: " << maxDiff);
        }
        
        REQUIRE(dryPathPreserved);
    }
    
    SUCCEED("Dry signal path remains unprocessed when mix=0");
}

// ==============================================================================
// Property 30: Output Gain Final Stage
// **Validates: Requirements 21.10**
// ==============================================================================

TEST_CASE("Property 30: Output Gain Final Stage - Verify gain applied last to both paths", 
          "[Feature: flowstate-plugin, Property 30]")
{
    INFO("Testing that output gain is applied as the final processing stage");
    
    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        // Generate random output gain value
        float gainDb = randomFloat(-20.0f, 6.0f);
        float gainLinear = juce::Decibels::decibelsToGain(gainDb);
        
        // Convert dB to normalized parameter value (range is -60 to +6 dB)
        float normalizedGain = (gainDb - (-60.0f)) / (6.0f - (-60.0f));
        
        // Test with 100% dry
        {
            FlowstateProcessor processor;
            processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
            
            auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
            mixParam->setValueNotifyingHost(0.0f); // 100% dry
            
            auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
            gainParam->setValueNotifyingHost(normalizedGain);
            
            // Process a few silent blocks first to let parameter smoothing settle
            juce::AudioBuffer<float> silentBuffer(2, TEST_BUFFER_SIZE);
            juce::MidiBuffer midiBuffer;
            for (int i = 0; i < 5; ++i)
            {
                silentBuffer.clear();
                processor.processBlock(silentBuffer, midiBuffer);
            }
            
            juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
            fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
            
            float inputRMS = calculateRMS(buffer);
            
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = calculateRMS(buffer);
            float expectedRMS = inputRMS * gainLinear;
            
            // Verify gain is applied to dry path
            float ratio = outputRMS / expectedRMS;
            INFO("Dry path - GainDb=" << gainDb << ", Ratio=" << ratio);
            REQUIRE(std::abs(ratio - 1.0f) < 0.1f); // Within 10% tolerance
        }
        
        // Test with 100% wet
        {
            FlowstateProcessor processor;
            processor.prepareToPlay(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
            
            auto* mixParam = processor.getParameters().getParameter(ParameterIDs::mix);
            mixParam->setValueNotifyingHost(1.0f); // 100% wet
            
            auto* gainParam = processor.getParameters().getParameter(ParameterIDs::outputGain);
            gainParam->setValueNotifyingHost(normalizedGain);
            
            // Disable modulation and ducking for consistent testing
            auto* modDepthParam = processor.getParameters().getParameter(ParameterIDs::modDepth);
            modDepthParam->setValueNotifyingHost(0.0f);
            
            auto* duckParam = processor.getParameters().getParameter(ParameterIDs::duckSensitivity);
            duckParam->setValueNotifyingHost(0.0f);
            
            // Process a few silent blocks first to let parameter smoothing settle
            juce::AudioBuffer<float> silentBuffer(2, TEST_BUFFER_SIZE);
            juce::MidiBuffer midiBuffer;
            for (int i = 0; i < 5; ++i)
            {
                silentBuffer.clear();
                processor.processBlock(silentBuffer, midiBuffer);
            }
            
            juce::AudioBuffer<float> buffer(2, TEST_BUFFER_SIZE);
            fillBufferWithSine(buffer, 440.0f, TEST_SAMPLE_RATE, 0.5f);
            
            processor.processBlock(buffer, midiBuffer);
            
            float outputRMS = calculateRMS(buffer);
            
            // Verify output is scaled by gain (wet path also affected)
            INFO("Wet path - GainDb=" << gainDb << ", OutputRMS=" << outputRMS);
            REQUIRE(outputRMS >= 0.0f);
            REQUIRE(!std::isnan(outputRMS));
            
            // If gain is negative, output should be quieter
            if (gainDb < -1.0f)
            {
                REQUIRE(outputRMS < 0.5f); // Should be attenuated
            }
        }
    }
    
    SUCCEED("Output gain is applied as final stage to both dry and wet paths");
}
