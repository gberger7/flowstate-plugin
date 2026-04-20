/*
  ==============================================================================

    DAWCompatibilityTests.cpp
    DAW compatibility and format validation tests for Flowstate.

    Covers:
    - Plugin metadata (name, channel layout, MIDI flags)
    - All 21 parameters registered and automatable
    - State serialization round-trip (save/load)
    - Corrupted state handled gracefully
    - Multiple prepare/release cycles
    - Common DAW sample rates and block sizes

    NOTE: Actual VST3/AU binary loading in Reaper, Logic, FL Studio etc. requires
    manual testing in those hosts (Tasks 10.1, 10.2). These tests cover the
    host-facing API surface that all DAWs exercise programmatically.

    Framework: Catch2
    Validates: Requirements 12.1–12.8, 13.1–13.5

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

static void processOnce(FlowstateProcessor& p, int blockSize = 512, double sr = 44100.0)
{
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buf(2, blockSize);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < blockSize; ++s)
            buf.setSample(ch, s, 0.1f * std::sin(
                2.0f * juce::MathConstants<float>::pi * 440.0f * s / static_cast<float>(sr)));
    p.processBlock(buf, midi);
}

static const std::vector<juce::String> kAllParamIDs = {
    ParameterIDs::delayTime,
    ParameterIDs::delaySync,
    ParameterIDs::delayDivision,
    ParameterIDs::delayFeedback,
    ParameterIDs::delayDiffusion,
    ParameterIDs::reverbSize,
    ParameterIDs::reverbDecay,
    ParameterIDs::reverbDamping,
    ParameterIDs::blend,
    ParameterIDs::mix,
    ParameterIDs::modRate,
    ParameterIDs::modDepth,
    ParameterIDs::drive,
    ParameterIDs::tone,
    ParameterIDs::duckSensitivity,
    ParameterIDs::shimmerEnabled,
    ParameterIDs::shimmerPitch,
    ParameterIDs::reverseMode,
    ParameterIDs::freezeEnabled,
    ParameterIDs::outputGain,
    ParameterIDs::stereoWidth,
};

// ---------------------------------------------------------------------------
// Test 1: Plugin name is non-empty
// Validates: Requirements 12.1, 12.2
// ---------------------------------------------------------------------------

TEST_CASE("Plugin name is non-empty",
          "[daw-compat][metadata]")
{
    FlowstateProcessor processor;
    REQUIRE(processor.getName().isNotEmpty());
}

// ---------------------------------------------------------------------------
// Test 2: Plugin does not accept or produce MIDI
// ---------------------------------------------------------------------------

TEST_CASE("Plugin does not accept or produce MIDI",
          "[daw-compat][metadata]")
{
    FlowstateProcessor processor;
    REQUIRE(processor.acceptsMidi() == false);
    REQUIRE(processor.producesMidi() == false);
}

// ---------------------------------------------------------------------------
// Test 3: Plugin supports stereo layout, rejects mono
// Validates: Requirements 12.1, 12.2
// ---------------------------------------------------------------------------

TEST_CASE("Plugin supports stereo in/out and rejects mono",
          "[daw-compat][metadata]")
{
    FlowstateProcessor processor;

    juce::AudioProcessor::BusesLayout stereo;
    stereo.inputBuses.add(juce::AudioChannelSet::stereo());
    stereo.outputBuses.add(juce::AudioChannelSet::stereo());
    REQUIRE(processor.isBusesLayoutSupported(stereo));

    juce::AudioProcessor::BusesLayout mono;
    mono.inputBuses.add(juce::AudioChannelSet::mono());
    mono.outputBuses.add(juce::AudioChannelSet::mono());
    REQUIRE(processor.isBusesLayoutSupported(mono) == false);
}

// ---------------------------------------------------------------------------
// Test 4: Plugin has an editor and non-negative tail length
// ---------------------------------------------------------------------------

TEST_CASE("Plugin has editor and valid tail length",
          "[daw-compat][metadata]")
{
    FlowstateProcessor processor;
    prepare(processor);
    REQUIRE(processor.hasEditor());
    REQUIRE(processor.getTailLengthSeconds() >= 0.0);
}

// ---------------------------------------------------------------------------
// Test 5: All 21 parameters are registered in APVTS
// Validates: Requirements 13.1, 13.2
// ---------------------------------------------------------------------------

TEST_CASE("All 21 parameters are registered in APVTS",
          "[daw-compat][parameters]")
{
    FlowstateProcessor processor;
    auto& apvts = processor.getParameters();

    REQUIRE(kAllParamIDs.size() == 21);

    for (const auto& id : kAllParamIDs)
    {
        INFO("Checking parameter: " << id);
        REQUIRE(apvts.getParameter(id) != nullptr);
    }
}

// ---------------------------------------------------------------------------
// Test 6: All parameters accept normalised [0,1] values without crashing
// Validates: Requirements 12.3, 12.4
// ---------------------------------------------------------------------------

TEST_CASE("All parameters accept normalised values and produce valid audio",
          "[daw-compat][automation]")
{
    FlowstateProcessor processor;
    prepare(processor);
    auto& apvts = processor.getParameters();

    std::srand(99);

    for (const auto& id : kAllParamIDs)
    {
        auto* param = apvts.getParameter(id);
        REQUIRE(param != nullptr);

        for (int i = 0; i < 10; ++i)
        {
            float normVal = static_cast<float>(std::rand()) / RAND_MAX;
            INFO("Parameter " << id << " value " << normVal);
            REQUIRE_NOTHROW(param->setValueNotifyingHost(normVal));
        }
        REQUIRE_NOTHROW(param->setValueNotifyingHost(0.0f));
        REQUIRE_NOTHROW(param->setValueNotifyingHost(1.0f));
    }

    // Audio must still be valid after all parameter changes
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buf(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            buf.setSample(ch, s, 0.1f);
    processor.processBlock(buf, midi);

    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
        {
            float v = buf.getSample(ch, s);
            INFO("Sample [" << ch << "][" << s << "] = " << v);
            REQUIRE(std::isnan(v) == false);
            REQUIRE(std::isinf(v) == false);
        }

    SUCCEED("All parameters automatable without crashing");
}

// ---------------------------------------------------------------------------
// Test 7: State serialization round-trip preserves all 21 parameters
// Validates: Requirements 13.4, 13.5
// ---------------------------------------------------------------------------

TEST_CASE("State serialization round-trip preserves all 21 parameter values",
          "[daw-compat][state]")
{
    FlowstateProcessor source;
    prepare(source);
    auto& srcApvts = source.getParameters();

    srcApvts.getParameter(ParameterIDs::delayTime)->setValueNotifyingHost(0.3f);
    srcApvts.getParameter(ParameterIDs::delaySync)->setValueNotifyingHost(1.0f);
    srcApvts.getParameter(ParameterIDs::delayDivision)->setValueNotifyingHost(0.5f);
    srcApvts.getParameter(ParameterIDs::delayFeedback)->setValueNotifyingHost(0.7f);
    srcApvts.getParameter(ParameterIDs::delayDiffusion)->setValueNotifyingHost(0.4f);
    srcApvts.getParameter(ParameterIDs::reverbSize)->setValueNotifyingHost(0.6f);
    srcApvts.getParameter(ParameterIDs::reverbDecay)->setValueNotifyingHost(0.8f);
    srcApvts.getParameter(ParameterIDs::reverbDamping)->setValueNotifyingHost(0.2f);
    srcApvts.getParameter(ParameterIDs::blend)->setValueNotifyingHost(0.25f);
    srcApvts.getParameter(ParameterIDs::mix)->setValueNotifyingHost(0.75f);
    srcApvts.getParameter(ParameterIDs::modRate)->setValueNotifyingHost(0.4f);
    srcApvts.getParameter(ParameterIDs::modDepth)->setValueNotifyingHost(0.6f);
    srcApvts.getParameter(ParameterIDs::drive)->setValueNotifyingHost(0.3f);
    srcApvts.getParameter(ParameterIDs::tone)->setValueNotifyingHost(0.9f);
    srcApvts.getParameter(ParameterIDs::duckSensitivity)->setValueNotifyingHost(0.5f);
    srcApvts.getParameter(ParameterIDs::shimmerEnabled)->setValueNotifyingHost(1.0f);
    srcApvts.getParameter(ParameterIDs::shimmerPitch)->setValueNotifyingHost(0.75f);
    srcApvts.getParameter(ParameterIDs::reverseMode)->setValueNotifyingHost(0.33f);
    srcApvts.getParameter(ParameterIDs::freezeEnabled)->setValueNotifyingHost(0.0f);
    srcApvts.getParameter(ParameterIDs::outputGain)->setValueNotifyingHost(0.6f);
    srcApvts.getParameter(ParameterIDs::stereoWidth)->setValueNotifyingHost(0.8f);

    juce::MemoryBlock stateData;
    source.getStateInformation(stateData);
    REQUIRE(stateData.getSize() > 0);

    FlowstateProcessor dest;
    prepare(dest);
    dest.setStateInformation(stateData.getData(), static_cast<int>(stateData.getSize()));

    auto& dstApvts = dest.getParameters();

    for (const auto& id : kAllParamIDs)
    {
        float srcVal = srcApvts.getParameter(id)->getValue();
        float dstVal = dstApvts.getParameter(id)->getValue();
        INFO("Parameter " << id << ": src=" << srcVal << " dst=" << dstVal);
        REQUIRE(std::abs(srcVal - dstVal) < 0.001f);
    }

    SUCCEED("State round-trip preserves all 21 parameters");
}

// ---------------------------------------------------------------------------
// Test 8: Corrupted state data does not crash the plugin
// Validates: Requirements 13.5
// ---------------------------------------------------------------------------

TEST_CASE("Corrupted state data does not crash the plugin",
          "[daw-compat][state]")
{
    FlowstateProcessor processor;
    prepare(processor);

    // nullptr / zero size
    REQUIRE_NOTHROW(processor.setStateInformation(nullptr, 0));

    // Random garbage
    std::vector<uint8_t> garbage(256);
    std::srand(7);
    for (auto& b : garbage)
        b = static_cast<uint8_t>(std::rand() & 0xFF);
    REQUIRE_NOTHROW(processor.setStateInformation(garbage.data(),
                                                   static_cast<int>(garbage.size())));

    // Audio still works after bad state
    juce::MidiBuffer midi;
    juce::AudioBuffer<float> buf(2, 512);
    for (int ch = 0; ch < 2; ++ch)
        for (int s = 0; s < 512; ++s)
            buf.setSample(ch, s, 0.1f);
    REQUIRE_NOTHROW(processor.processBlock(buf, midi));

    SUCCEED("Corrupted state handled gracefully");
}

// ---------------------------------------------------------------------------
// Test 9: Multiple prepare/release cycles are safe
// Validates: Requirements 12.5
// ---------------------------------------------------------------------------

TEST_CASE("Multiple prepareToPlay / releaseResources cycles are safe",
          "[daw-compat][lifecycle]")
{
    FlowstateProcessor processor;

    for (int cycle = 0; cycle < 5; ++cycle)
    {
        INFO("Cycle " << cycle);
        REQUIRE_NOTHROW(processor.prepareToPlay(44100.0, 512));
        processOnce(processor);
        REQUIRE_NOTHROW(processor.releaseResources());
    }

    REQUIRE_NOTHROW(processor.prepareToPlay(48000.0, 256));
    processOnce(processor, 256, 48000.0);

    SUCCEED("Multiple prepare/release cycles safe");
}

// ---------------------------------------------------------------------------
// Test 10: Common DAW sample rates and block sizes produce valid output
// Validates: Requirements 12.6
// ---------------------------------------------------------------------------

TEST_CASE("Plugin handles common DAW sample rates and block sizes",
          "[daw-compat][sample-rates]")
{
    const std::vector<double> sampleRates = { 44100.0, 48000.0, 88200.0, 96000.0 };
    const std::vector<int>    blockSizes  = { 64, 128, 256, 512, 1024 };

    for (double sr : sampleRates)
    {
        for (int bs : blockSizes)
        {
            INFO("SR=" << sr << " BS=" << bs);
            FlowstateProcessor processor;
            REQUIRE_NOTHROW(processor.prepareToPlay(sr, bs));

            juce::MidiBuffer midi;
            juce::AudioBuffer<float> buf(2, bs);
            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < bs; ++s)
                    buf.setSample(ch, s, 0.1f * std::sin(
                        2.0f * juce::MathConstants<float>::pi * 440.0f * s / static_cast<float>(sr)));

            REQUIRE_NOTHROW(processor.processBlock(buf, midi));

            for (int ch = 0; ch < 2; ++ch)
                for (int s = 0; s < bs; ++s)
                {
                    float v = buf.getSample(ch, s);
                    REQUIRE(std::isnan(v) == false);
                    REQUIRE(std::isinf(v) == false);
                }

            REQUIRE_NOTHROW(processor.releaseResources());
        }
    }

    SUCCEED("All sample rates and block sizes produce valid output");
}
