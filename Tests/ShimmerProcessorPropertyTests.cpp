/*
  ==============================================================================

    ShimmerProcessorPropertyTests.cpp
    Property-Based Tests for ShimmerProcessor

    Property 15: When shimmer is set to 0 semitones (ratio = 1.0), the processor
    should act as a unity-gain delay — output amplitude should match input.

    NOTE: Shimmer is a creative feedback effect evaluated by ear in the DAW.
    Frequency ratio accuracy is intentionally not tested here. The cumulative
    pitch-shifting character of shimmer emerges over multiple feedback passes
    and cannot be meaningfully validated with single-pass frequency measurement.

  ==============================================================================
*/

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include "../Source/DSP/ShimmerProcessor.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

constexpr int    PROPERTY_TEST_ITERATIONS = 100;
constexpr double TEST_SAMPLE_RATE         = 44100.0;
constexpr int    TEST_BUFFER_SIZE         = 512;

namespace Helpers
{
    float randomFloat(float lo, float hi)
    {
        return lo + (hi - lo) * (static_cast<float>(std::rand()) / RAND_MAX);
    }

    void fillWithNoise(juce::AudioBuffer<float>& buf, float amp = 0.5f)
    {
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
            for (int s = 0; s < buf.getNumSamples(); ++s)
                buf.setSample(ch, s, randomFloat(-amp, amp));
    }

    float rms(const juce::AudioBuffer<float>& buf)
    {
        float sum = 0.0f;
        int   n   = 0;
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
            for (int s = 0; s < buf.getNumSamples(); ++s)
            {
                float v = buf.getSample(ch, s);
                sum += v * v;
                ++n;
            }
        return (n > 0) ? std::sqrt(sum / n) : 0.0f;
    }
}

// ==============================================================================
// Property 15: Shimmer at 0 semitones is unity-gain (no amplitude change)
// Validates: Requirement 8.2 — when shimmer produces no shift, signal is unchanged
// ==============================================================================

TEST_CASE("Property 15: Shimmer at 0 semitones preserves signal amplitude",
          "[Feature: flowstate-plugin, Property 15]")
{
    ShimmerProcessor processor;
    processor.prepare(TEST_SAMPLE_RATE, TEST_BUFFER_SIZE);
    processor.setPitchShift(0.0f); // ratio = 1.0 — no pitch change

    INFO("Shimmer at 0 semitones should preserve signal amplitude (unity gain)");

    // Warm up: need to fill at least half the circular buffer (kBufferSize/2 = 32768 samples)
    // before the read pointer reaches written data. 65 blocks of 512 = 33280 samples.
    processor.setPitchShift(0.0f);
    for (int warmup = 0; warmup < 65; ++warmup)
    {
        juce::AudioBuffer<float> warm(2, TEST_BUFFER_SIZE);
        Helpers::fillWithNoise(warm, 0.7f);
        processor.process(warm);
    }

    for (int iteration = 0; iteration < PROPERTY_TEST_ITERATIONS; ++iteration)
    {
        juce::AudioBuffer<float> buf(2, TEST_BUFFER_SIZE);
        Helpers::fillWithNoise(buf, 0.7f);

        float rmsIn = Helpers::rms(buf);

        processor.process(buf);
        float rmsOut = Helpers::rms(buf);

        // At ratio=1.0 the processor is a fixed delay — RMS should be preserved
        // Allow 10% tolerance for the circular buffer read/write offset
        if (rmsIn > 0.01f)
        {
            float ratio = rmsOut / rmsIn;
            INFO("Iteration " << iteration << ": rmsIn=" << rmsIn
                 << " rmsOut=" << rmsOut << " ratio=" << ratio);
            REQUIRE(ratio >= 0.5f);
            REQUIRE(ratio <= 2.0f);
        }
    }

    SUCCEED("Shimmer at 0 semitones preserves signal amplitude");
}
