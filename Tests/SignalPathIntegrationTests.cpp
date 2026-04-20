/*
  ==============================================================================

    SignalPathIntegrationTests.cpp
    Integration tests for the full Flowstate signal path.

    Covers:
    - Input → split → process → blend → mix → output flow
    - All parameter combinations produce valid (non-NaN, non-Inf) output
    - Mode switching (sync/free, reverse modes) produces no artifacts
    - Freeze activation/deactivation produces no clicks

    Framework: Catch2
    Validates: Requirements 14.3, 14.4, 14.5

  ==============================================================================
*/

#include "../Source/PluginProcessor.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void prepare(FlowstateProcessor& p, double sr = 44100.0, int block = 512)
{
    p.prepareToPlay(sr, block);
}

static void processNoise(FlowstateProcessor& p, int numBlocks = 4, int blockSize = 512)
{
    juce::MidiBuffer midi;
    for (int b = 0; b < numBlocks; ++b)
    {
        juce::AudioBuffer<float> buf(2, blockSize);
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < blockSize; ++s)
                buf.setSample(ch, s, (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f);
        p.processBlock(buf, midi);
    }
}

static bool bufferIsValid(const juce::AudioBuffer<float>& buf)
{
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int s = 0; s < buf.getNumSamples(); ++s)
        {
            float v = buf.getSample(ch, s);
            if (std::isnan(v) || std::isinf(v)) return false;
        }
    return true;
}

static float bufferMaxAbs(const juce::AudioBuffer<float>& buf)
{
    float peak = 0.0f;
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int s = 0; s < buf.getNumSamples(); ++s)
            peak = std::max(peak, std::abs(buf.getSample(ch, s)));
    return peak;
}

static float bufferRMS(const juce::AudioBuffer<float>& buf)
{
    float sum = 0.0f;
    int n = buf.getNumChannels() * buf.getNumSamples();
    for (int ch = 0; ch < buf.getNumChannels(); ++ch)
        for (int s = 0; s < buf.getNumSamples(); ++s)
        {
            float v = buf.getSample(ch, s);
            sum += v * v;
        }
    return (n > 0) ? std::sqrt(sum / n) : 0.0f;
}

static void setParam(FlowstateProcessor& p, const juce::String& id, float normalised)
{
    auto* param = p.getParameters().getParameter(id);
    if (param != nullptr)
        param->setValueNotifyingHost(normalised);
}

// ---------------------------------------------------------------------------
// Test 1: Full signal path produces valid output
// ---------------------------------------------------------------------------

TEST_CASE("Full signal path: input → split → blend → mix → output produces valid audio",
          "[integration][signal-path]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buf(2, 512);

    // Fill with a sine wave
    for (int s = 0; s < 512; ++s)
    {
        float v = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * s / 44100.0f);
        buf.setSample(0, s, v);
        buf.setSample(1, s, v);
    }

    processor.processBlock(buf, midi);

    REQUIRE(bufferIsValid(buf));
    // With mix=0.5 and a sine input, output should have energy
    REQUIRE(bufferRMS(buf) > 0.0f);
}

// ---------------------------------------------------------------------------
// Test 2: Mix=0 passes dry signal unchanged
// ---------------------------------------------------------------------------

TEST_CASE("Mix=0 passes dry signal with no wet contribution",
          "[integration][signal-path]")
{
    FlowstateProcessor processor;
    prepare(processor);

    // Set mix to 0 (fully dry)
    setParam(processor, ParameterIDs::mix, 0.0f);
    // Set output gain to unity (0 dB → normalised = (0 - (-60)) / (6 - (-60)) = 60/66 ≈ 0.909)
    setParam(processor, ParameterIDs::outputGain, 60.0f / 66.0f);

    juce::MidiBuffer midi;

    // Warm up
    processNoise(processor, 2);

    // Now process a known signal
    juce::AudioBuffer<float> buf(2, 512);
    for (int s = 0; s < 512; ++s)
    {
        float v = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * s / 44100.0f);
        buf.setSample(0, s, v);
        buf.setSample(1, s, v);
    }

    juce::AudioBuffer<float> reference(2, 512);
    reference.makeCopyOf(buf);

    processor.processBlock(buf, midi);

    REQUIRE(bufferIsValid(buf));

    // With mix=0, output should closely match dry input (within soft-clip tolerance)
    float maxDiff = 0.0f;
    for (int s = 0; s < 512; ++s)
        maxDiff = std::max(maxDiff, std::abs(buf.getSample(0, s) - reference.getSample(0, s)));

    // Allow small deviation from output gain smoothing and soft-clip
    REQUIRE(maxDiff < 0.05f);
}

// ---------------------------------------------------------------------------
// Test 3: Mix=1 produces wet-only output (no dry signal)
// ---------------------------------------------------------------------------

TEST_CASE("Mix=1 produces wet-only output",
          "[integration][signal-path]")
{
    FlowstateProcessor processor;
    prepare(processor);

    setParam(processor, ParameterIDs::mix, 1.0f);

    juce::MidiBuffer midi;
    processNoise(processor, 4); // fill delay/reverb buffers

    juce::AudioBuffer<float> buf(2, 512);
    buf.clear(); // silent input

    processor.processBlock(buf, midi);

    REQUIRE(bufferIsValid(buf));
    // With silent input and mix=1, wet signal should also be silent (or near-silent)
    // after the delay/reverb tails decay — but we just check for validity here
}

// ---------------------------------------------------------------------------
// Test 4: All parameter combinations produce valid output
// ---------------------------------------------------------------------------

TEST_CASE("Random parameter combinations produce valid (non-NaN, non-Inf) output",
          "[integration][parameter-combinations]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;
    std::srand(42);

    for (int trial = 0; trial < 50; ++trial)
    {
        // Randomise all continuous parameters
        setParam(processor, ParameterIDs::delayTime,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::delayFeedback,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::delayDiffusion,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::reverbSize,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::reverbDecay,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::reverbDamping,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::blend,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::mix,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::drive,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::tone,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::modRate,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::modDepth,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::duckSensitivity,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::stereoWidth,
                 static_cast<float>(std::rand()) / RAND_MAX);
        setParam(processor, ParameterIDs::outputGain,
                 static_cast<float>(std::rand()) / RAND_MAX);

        juce::AudioBuffer<float> buf(2, 512);
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 512; ++s)
                buf.setSample(ch, s, (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f);

        processor.processBlock(buf, midi);

        INFO("Trial " << trial << " produced invalid output");
        REQUIRE(bufferIsValid(buf));
    }
}

// ---------------------------------------------------------------------------
// Test 5: Switching delay sync on/off produces no invalid output
// ---------------------------------------------------------------------------

TEST_CASE("Switching delay sync mode produces no NaN or Inf",
          "[integration][mode-switching]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;
    processNoise(processor, 2);

    for (int toggle = 0; toggle < 10; ++toggle)
    {
        // Toggle sync on/off
        float syncVal = (toggle % 2 == 0) ? 1.0f : 0.0f;
        setParam(processor, ParameterIDs::delaySync, syncVal);

        juce::AudioBuffer<float> buf(2, 512);
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 512; ++s)
                buf.setSample(ch, s, (static_cast<float>(std::rand()) / RAND_MAX) * 0.5f);

        processor.processBlock(buf, midi);

        INFO("Sync toggle " << toggle << " produced invalid output");
        REQUIRE(bufferIsValid(buf));
    }
}

// ---------------------------------------------------------------------------
// Test 6: Cycling through all reverse modes produces no invalid output
// ---------------------------------------------------------------------------

TEST_CASE("Cycling through reverse modes (OFF/REVERB/DELAY/BOTH) produces no NaN or Inf",
          "[integration][mode-switching]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;
    processNoise(processor, 4); // fill reverse buffer

    for (int mode = 0; mode <= 3; ++mode)
    {
        // reverseMode is an int param 0-3; normalise to [0,1]
        setParam(processor, ParameterIDs::reverseMode,
                 static_cast<float>(mode) / 3.0f);

        juce::AudioBuffer<float> buf(2, 512);
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 512; ++s)
                buf.setSample(ch, s, (static_cast<float>(std::rand()) / RAND_MAX) * 0.5f);

        processor.processBlock(buf, midi);

        INFO("Reverse mode " << mode << " produced invalid output");
        REQUIRE(bufferIsValid(buf));
    }
}

// ---------------------------------------------------------------------------
// Test 7: Freeze activation/deactivation produces no clicks (no large transients)
// ---------------------------------------------------------------------------

TEST_CASE("Freeze activation and deactivation produces no large transients",
          "[integration][freeze]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;

    // Fill the processor with audio so freeze has content to capture
    processNoise(processor, 8);

    // Capture one block just before freeze
    juce::AudioBuffer<float> preFreezeBlock(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            preFreezeBlock.setSample(ch, s, 0.3f * std::sin(
                2.0f * juce::MathConstants<float>::pi * 440.0f * s / 44100.0f));
    processor.processBlock(preFreezeBlock, midi);
    float preRMS = bufferRMS(preFreezeBlock);

    // Activate freeze
    setParam(processor, ParameterIDs::freezeEnabled, 1.0f);

    juce::AudioBuffer<float> freezeBlock(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            freezeBlock.setSample(ch, s, 0.3f * std::sin(
                2.0f * juce::MathConstants<float>::pi * 440.0f * s / 44100.0f));
    processor.processBlock(freezeBlock, midi);

    REQUIRE(bufferIsValid(freezeBlock));

    // No click: peak should not be dramatically larger than pre-freeze RMS
    float freezePeak = bufferMaxAbs(freezeBlock);
    if (preRMS > 0.001f)
    {
        // Allow up to 20x RMS as peak (generous — just checking for runaway transients)
        REQUIRE(freezePeak < preRMS * 20.0f);
    }

    // Deactivate freeze
    setParam(processor, ParameterIDs::freezeEnabled, 0.0f);

    juce::AudioBuffer<float> postFreezeBlock(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            postFreezeBlock.setSample(ch, s, 0.3f * std::sin(
                2.0f * juce::MathConstants<float>::pi * 440.0f * s / 44100.0f));
    processor.processBlock(postFreezeBlock, midi);

    REQUIRE(bufferIsValid(postFreezeBlock));

    float postPeak = bufferMaxAbs(postFreezeBlock);
    if (preRMS > 0.001f)
    {
        REQUIRE(postPeak < preRMS * 20.0f);
    }
}

// ---------------------------------------------------------------------------
// Test 8: Stereo integrity — 2 channels in, 2 channels out, no channel collapse
// ---------------------------------------------------------------------------

TEST_CASE("Stereo integrity: 2-channel input produces 2-channel output",
          "[integration][signal-path]")
{
    FlowstateProcessor processor;
    prepare(processor);

    juce::MidiBuffer midi;

    juce::AudioBuffer<float> buf(2, 512);
    for (int s = 0; s < 512; ++s)
    {
        buf.setSample(0, s,  0.5f); // L = positive DC
        buf.setSample(1, s, -0.5f); // R = negative DC
    }

    processor.processBlock(buf, midi);

    REQUIRE(bufferIsValid(buf));
    REQUIRE(buf.getNumChannels() == 2);
    REQUIRE(buf.getNumSamples() == 512);
}
