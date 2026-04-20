/*
  ==============================================================================

    PluginProcessor.cpp
    Created: Flowstate Plugin Audio Processor
    Description: Main AudioProcessor implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#ifndef FLOWSTATE_HEADLESS_TEST
#include "PluginEditor.h"
#endif

//==============================================================================
FlowstateProcessor::FlowstateProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("FlowstateParameters"), createParameterLayout())
{
    // Initialize parameter smoothing (10ms ramp time, will be set in prepareToPlay)
    smoothedBlend.setCurrentAndTargetValue(0.5f);
    smoothedMix.setCurrentAndTargetValue(0.0f);
    smoothedOutputGain.setCurrentAndTargetValue(1.0f);
    smoothedDelayFeedback.setCurrentAndTargetValue(0.0f);
    smoothedDelayDiffusion.setCurrentAndTargetValue(0.0f);
    smoothedReverbSize.setCurrentAndTargetValue(0.0f);
    smoothedReverbDecay.setCurrentAndTargetValue(0.1f);
    smoothedReverbDamping.setCurrentAndTargetValue(0.0f);
    smoothedModRate.setCurrentAndTargetValue(0.01f);
    smoothedModDepth.setCurrentAndTargetValue(0.0f);
    smoothedShimmerPitch.setCurrentAndTargetValue(0.0f);
}

FlowstateProcessor::~FlowstateProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FlowstateProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Delay parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::delayTime,
        "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 0.1f, 0.3f),
        1.0f,
        "ms"));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::delaySync,
        "Delay Sync",
        false));
    
    layout.add(std::make_unique<juce::AudioParameterInt>(
        ParameterIDs::delayDivision,
        "Delay Division",
        0, 17,
        6)); // Default to 1/4 note
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::delayFeedback,
        "Delay Feedback",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::delayDiffusion,
        "Delay Diffusion",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Reverb parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::reverbSize,
        "Reverb Size",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::reverbDecay,
        "Reverb Decay",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.3f),
        0.1f,
        "s"));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::reverbDamping,
        "Reverb Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Core controls
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::blend,
        "Blend",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::mix,
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Modulation
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::modRate,
        "Mod Rate",
        juce::NormalisableRange<float>(0.01f, 5.0f, 0.01f, 0.3f),
        0.01f,
        "Hz"));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::modDepth,
        "Mod Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Character
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::drive,
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::tone,
        "Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Ducking
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::duckSensitivity,
        "Duck Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));
    
    // Shimmer
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::shimmerEnabled,
        "Shimmer Enabled",
        true));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::shimmerPitch,
        "Shimmer Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
        0.0f,
        "st"));
    
    // Reverse
    layout.add(std::make_unique<juce::AudioParameterInt>(
        ParameterIDs::reverseMode,
        "Reverse Mode",
        0, 3,
        0)); // 0=OFF, 1=REVERB, 2=DELAY, 3=BOTH
    
    // Freeze
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::freezeEnabled,
        "Freeze Enabled",
        false));
    
    // Output
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::outputGain,
        "Output Gain",
        juce::NormalisableRange<float>(-60.0f, 6.0f, 0.1f),
        0.0f,
        "dB"));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::stereoWidth,
        "Stereo Width",
        juce::NormalisableRange<float>(0.0f, 1.5f, 0.01f),
        0.0f));
    
    return layout;
}

//==============================================================================
void FlowstateProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialize all DSP components
    delayEngine.prepare(sampleRate, samplesPerBlock);
    reverbEngine.prepare(sampleRate, samplesPerBlock);
    feedbackProcessor.prepare(sampleRate, samplesPerBlock);
    duckingProcessor.prepare(sampleRate);
    shimmerProcessor.prepare(sampleRate, samplesPerBlock);
    modulationEngine.prepare(sampleRate);
    reverseBuffer.prepare(sampleRate, samplesPerBlock);
    
    // Allocate internal buffers
    dryBuffer.setSize(2, samplesPerBlock);
    wetBuffer.setSize(2, samplesPerBlock);
    delayWetBuffer.setSize(2, samplesPerBlock);
    reverbWetBuffer.setSize(2, samplesPerBlock);
    feedbackBuffer.setSize(2, samplesPerBlock);
    freezeBuffer.setSize(2, static_cast<int>(sampleRate * 4.0)); // 4 seconds for freeze
    
    // Clear all buffers
    dryBuffer.clear();
    wetBuffer.clear();
    delayWetBuffer.clear();
    reverbWetBuffer.clear();
    feedbackBuffer.clear();
    freezeBuffer.clear();
    
    // Initialize parameter smoothing (10ms ramp time)
    smoothedBlend.reset(sampleRate, 0.01);
    smoothedMix.reset(sampleRate, 0.01);
    smoothedOutputGain.reset(sampleRate, 0.01);
    smoothedDrive.reset(sampleRate, 0.01);
    smoothedTone.reset(sampleRate, 0.01);
    smoothedStereoWidth.reset(sampleRate, 0.01);
    smoothedDuckSensitivity.reset(sampleRate, 0.01);
    smoothedDelayFeedback.reset(sampleRate, 0.01);
    smoothedDelayDiffusion.reset(sampleRate, 0.01);
    smoothedReverbSize.reset(sampleRate, 0.01);
    smoothedReverbDecay.reset(sampleRate, 0.01);
    smoothedReverbDamping.reset(sampleRate, 0.01);
    smoothedModRate.reset(sampleRate, 0.01);
    smoothedModDepth.reset(sampleRate, 0.01);
    smoothedShimmerPitch.reset(sampleRate, 0.01);
    
    // Set initial smoothed values
    smoothedBlend.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::blend)->load());
    smoothedMix.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::mix)->load());
    float gainDb = parameters.getRawParameterValue(ParameterIDs::outputGain)->load();
    smoothedOutputGain.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainDb));
    smoothedDrive.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::drive)->load());
    smoothedTone.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::tone)->load());
    smoothedStereoWidth.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::stereoWidth)->load());
    smoothedDuckSensitivity.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::duckSensitivity)->load());
    smoothedDelayFeedback.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::delayFeedback)->load());
    smoothedDelayDiffusion.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::delayDiffusion)->load());
    smoothedReverbSize.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::reverbSize)->load());
    smoothedReverbDecay.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::reverbDecay)->load());
    smoothedReverbDamping.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::reverbDamping)->load());
    smoothedModRate.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::modRate)->load());
    smoothedModDepth.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::modDepth)->load());
    smoothedShimmerPitch.setCurrentAndTargetValue(parameters.getRawParameterValue(ParameterIDs::shimmerPitch)->load());
}

void FlowstateProcessor::releaseResources()
{
    // Clear all buffers to free memory
    dryBuffer.setSize(0, 0);
    wetBuffer.setSize(0, 0);
    delayWetBuffer.setSize(0, 0);
    reverbWetBuffer.setSize(0, 0);
    feedbackBuffer.setSize(0, 0);
    freezeBuffer.setSize(0, 0);
}

bool FlowstateProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support stereo input and output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}

//==============================================================================
void FlowstateProcessor::updateDSPParameters()
{
    // Update smoothed parameter targets
    smoothedDelayFeedback.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::delayFeedback)->load(), 0.0f, 1.0f));
    smoothedDelayDiffusion.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::delayDiffusion)->load(), 0.0f, 1.0f));
    smoothedReverbSize.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::reverbSize)->load(), 0.0f, 1.0f));
    smoothedReverbDecay.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::reverbDecay)->load(), 0.1f, 20.0f));
    smoothedReverbDamping.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::reverbDamping)->load(), 0.0f, 1.0f));
    smoothedModRate.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::modRate)->load(), 0.01f, 5.0f));
    smoothedModDepth.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::modDepth)->load(), 0.0f, 1.0f));
    smoothedShimmerPitch.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::shimmerPitch)->load(), -24.0f, 24.0f));
    smoothedDrive.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::drive)->load(), 0.0f, 1.0f));
    smoothedTone.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::tone)->load(), 0.0f, 1.0f));
    smoothedDuckSensitivity.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::duckSensitivity)->load(), 0.0f, 1.0f));
    smoothedStereoWidth.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::stereoWidth)->load(), 0.0f, 1.5f));
    smoothedBlend.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::blend)->load(), 0.0f, 1.0f));
    smoothedMix.setTargetValue(validateParameter(
        parameters.getRawParameterValue(ParameterIDs::mix)->load(), 0.0f, 1.0f));

    float gainDb = validateParameter(
        parameters.getRawParameterValue(ParameterIDs::outputGain)->load(), -60.0f, 6.0f);
    smoothedOutputGain.setTargetValue(juce::Decibels::decibelsToGain(gainDb));

    // Delay time: tempo sync or free.
    // Pass the raw parameter value directly to DelayEngine — it has its own
    // per-sample IIR smoother (20ms ramp) that interpolates at the audio rate.
    // Do NOT use smoothedDelayTime.getNextValue() here: that advances only once
    // per processBlock call (block-rate steps), which causes the DelayEngine's
    // internal smoother to chase large jumps and can produce read-position glitches.
    bool delaySync = parameters.getRawParameterValue(ParameterIDs::delaySync)->load() > 0.5f;
    if (delaySync)
    {
        updateTempoSync();
        int division = juce::jlimit(0, 17,
            static_cast<int>(parameters.getRawParameterValue(ParameterIDs::delayDivision)->load()));
        delayEngine.setDelayTimeFromTempo(lastKnownBpm, division);
    }
    else
    {
        float rawDelayMs = validateParameter(
            parameters.getRawParameterValue(ParameterIDs::delayTime)->load(), 1.0f, 2000.0f);
        delayEngine.setDelayTime(rawDelayMs);
    }

    delayEngine.setFeedback(smoothedDelayFeedback.getNextValue());
    delayEngine.setDiffusion(smoothedDelayDiffusion.getNextValue());

    reverbEngine.setSize(smoothedReverbSize.getNextValue());
    reverbEngine.setDecayTime(smoothedReverbDecay.getNextValue());
    reverbEngine.setDamping(smoothedReverbDamping.getNextValue());

    modulationEngine.setRate(smoothedModRate.getNextValue());
    modulationEngine.setDepth(smoothedModDepth.getNextValue());
}

void FlowstateProcessor::updateTempoSync()
{
    // Read host tempo from AudioPlayHead; update within 10ms (Requirement 1.4).
    // We check every processBlock call — at typical buffer sizes (512 @ 44.1kHz ≈ 11.6ms)
    // this satisfies the ≤10ms requirement at buffer sizes ≤441 samples, and is close
    // enough at larger buffer sizes for musical purposes.
    auto* playHead = getPlayHead();
    if (playHead == nullptr)
        return;

    auto position = playHead->getPosition();
    if (!position.hasValue())
        return;

    auto bpmOpt = position->getBpm();
    if (!bpmOpt.hasValue())
        return;

    double newBpm = *bpmOpt;

    // Validate BPM range (20–999 BPM is musically sensible)
    if (newBpm < 20.0 || newBpm > 999.0)
        return;

    lastKnownBpm = newBpm;
}

float FlowstateProcessor::validateParameter(float value, float min, float max)
{
    if (std::isnan(value) || std::isinf(value))
        return (min + max) * 0.5f;
    return juce::jlimit(min, max, value);
}

//==============================================================================
void FlowstateProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Validate buffer size
    if (numSamples == 0 || numChannels != 2)
        return;
    
    // Update all DSP parameters
    updateDSPParameters();
    
    // Step 1: Split input into dry and wet paths
    dryBuffer.makeCopyOf(buffer, true);
    wetBuffer.makeCopyOf(buffer, true);
    
    // Check freeze state
    auto freezeEnabled = parameters.getRawParameterValue(ParameterIDs::freezeEnabled)->load() > 0.5f;
    
    if (freezeEnabled && !freezeActive)
    {
        // Freeze just activated - only activate if freeze buffer has meaningful content
        // Check if the most recent portion of the freeze buffer has sufficient energy
        // We check the last 1 second of audio (most recent) rather than the entire 4-second buffer
        float rmsEnergy = 0.0f;
        int sampleCount = 0;
        int samplesToCheck = std::min(static_cast<int>(getSampleRate()), freezeBuffer.getNumSamples());
        int startPos = freezeBuffer.getNumSamples() - samplesToCheck;
        
        for (int ch = 0; ch < 2; ++ch)
        {
            for (int s = startPos; s < freezeBuffer.getNumSamples(); ++s)
            {
                float sample = freezeBuffer.getSample(ch, s);
                rmsEnergy += sample * sample;
                sampleCount++;
            }
        }
        
        rmsEnergy = std::sqrt(rmsEnergy / sampleCount);
        
        // Only activate freeze if RMS energy is above threshold (prevents silent freeze)
        // Use -50dB threshold (0.00316) to ensure meaningful audio content
        if (rmsEnergy > 0.00316f)
        {
            freezeActive = true;
            freezeWritePos = 0;
        }
        else
        {
            // Freeze requested but buffer doesn't have enough content yet
            // To prevent dropouts, we'll bypass the wet signal and output dry signal only
            // This ensures continuous audio output while the freeze buffer builds up
            wetBuffer.makeCopyOf(dryBuffer, true);
        }
        // If no content, continue normal processing to build up freeze buffer
        // This prevents dropouts when freeze is activated too early
    }
    else if (!freezeEnabled && freezeActive)
    {
        // Freeze deactivated
        freezeActive = false;
    }
    
    if (freezeActive)
    {
        // Loop frozen content
        for (int channel = 0; channel < 2; ++channel)
        {
            auto* wetData = wetBuffer.getWritePointer(channel);
            const auto* freezeData = freezeBuffer.getReadPointer(channel);
            const int freezeLength = freezeBuffer.getNumSamples();
            
            for (int i = 0; i < numSamples; ++i)
            {
                int readPos = (freezeWritePos + i) % freezeLength;
                wetData[i] = freezeData[readPos];
            }
        }
        
        freezeWritePos = (freezeWritePos + numSamples) % freezeBuffer.getNumSamples();
    }
    else
    {
        // Normal processing - route through delay and reverb engines
        
        // Step 2: Process delay engine
        delayWetBuffer.makeCopyOf(wetBuffer, true);
        feedbackBuffer.makeCopyOf(delayWetBuffer, true);
        
        // Update feedback processor with smoothed values
        // Advance smoothed values by one sample to get interpolated value
        feedbackProcessor.setDrive(smoothedDrive.getNextValue());
        feedbackProcessor.setTone(smoothedTone.getNextValue());
        
        // Skip remaining samples in the block (we only update once per block for efficiency)
        smoothedDrive.skip(numSamples - 1);
        smoothedTone.skip(numSamples - 1);
        
        // Apply feedback processing (drive, tone, shimmer)
        auto shimmerEnabled = parameters.getRawParameterValue(ParameterIDs::shimmerEnabled)->load() > 0.5f;
        
        // Update shimmer pitch with smoothing
        shimmerProcessor.setPitchShift(smoothedShimmerPitch.getNextValue());
        smoothedShimmerPitch.skip(numSamples - 1);
        
        feedbackProcessor.process(feedbackBuffer, &shimmerProcessor, shimmerEnabled);
        
        // Process delay with feedback
        delayEngine.process(delayWetBuffer, feedbackBuffer);
        
        // Step 3: Process reverb engine
        reverbWetBuffer.makeCopyOf(wetBuffer, true);
        reverbEngine.process(reverbWetBuffer);
        
        // Step 4: Apply modulation to both engines (already applied internally)
        // Modulation is applied within the engines based on modulation engine state
        
        // Step 5: Blend delay and reverb wet signals
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float blend = smoothedBlend.getNextValue();
            blend = validateParameter(blend, 0.0f, 1.0f);
            
            for (int channel = 0; channel < 2; ++channel)
            {
                float delayWet = delayWetBuffer.getSample(channel, sample);
                float reverbWet = reverbWetBuffer.getSample(channel, sample);
                
                // Validate samples to prevent NaN/Inf propagation
                if (std::isnan(delayWet) || std::isinf(delayWet))
                    delayWet = 0.0f;
                if (std::isnan(reverbWet) || std::isinf(reverbWet))
                    reverbWet = 0.0f;
                
                // Soft-clip to prevent extreme values
                delayWet = std::tanh(delayWet);
                reverbWet = std::tanh(reverbWet);
                
                // Crossfade: blend=0 is 100% delay, blend=1 is 100% reverb
                float blendedWet = delayWet * (1.0f - blend) + reverbWet * blend;
                
                // Add small DC offset to prevent denormals
                blendedWet += 1.0e-20f;
                
                wetBuffer.setSample(channel, sample, blendedWet);
            }
        }
        
        // Step 6: Apply stereo width to wet signal only
        widthProcessor.setWidth(smoothedStereoWidth.getNextValue());
        smoothedStereoWidth.skip(numSamples - 1);
        widthProcessor.process(wetBuffer);
        
        // Step 7: Apply ducking
        duckingProcessor.setSensitivity(smoothedDuckSensitivity.getNextValue());
        smoothedDuckSensitivity.skip(numSamples - 1);
        float envelope = duckingProcessor.processEnvelope(dryBuffer);
        duckingProcessor.applyDucking(wetBuffer, envelope);
        
        // Step 7.5: Handle reverse mode on wet signal only (before mix)
        auto reverseMode = static_cast<int>(parameters.getRawParameterValue(ParameterIDs::reverseMode)->load());
        if (reverseMode > 0)
        {
            reverseBuffer.write(wetBuffer);
            reverseBuffer.readReverse(wetBuffer, reverseMode);
        }
        
        // Capture wet signal for potential freeze
        if (numSamples <= freezeBuffer.getNumSamples())
        {
            for (int channel = 0; channel < 2; ++channel)
            {
                freezeBuffer.copyFrom(channel, 0, 
                                     freezeBuffer, channel, numSamples,
                                     freezeBuffer.getNumSamples() - numSamples);
                freezeBuffer.copyFrom(channel, freezeBuffer.getNumSamples() - numSamples,
                                     wetBuffer, channel, 0, numSamples);
            }
        }
    }
    
    // Step 8: Mix dry and wet signals
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mix = smoothedMix.getNextValue();
        mix = validateParameter(mix, 0.0f, 1.0f);
        
        for (int channel = 0; channel < 2; ++channel)
        {
            float dry = dryBuffer.getSample(channel, sample);
            float wet = wetBuffer.getSample(channel, sample);
            
            // Validate samples to prevent NaN/Inf propagation
            if (std::isnan(dry) || std::isinf(dry))
                dry = 0.0f;
            if (std::isnan(wet) || std::isinf(wet))
                wet = 0.0f;
            
            // Crossfade: mix=0 is 100% dry, mix=1 is 100% wet
            float mixed = dry * (1.0f - mix) + wet * mix;
            
            // Validate final output
            if (std::isnan(mixed) || std::isinf(mixed))
                mixed = 0.0f;
            
            buffer.setSample(channel, sample, mixed);
        }
    }
    
    // Step 9: Apply output gain as final stage
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float gain = smoothedOutputGain.getNextValue();
        gain = validateParameter(gain, 0.0f, 2.0f); // Max +6dB = 2.0 linear
        
        for (int channel = 0; channel < 2; ++channel)
        {
            float output = buffer.getSample(channel, sample) * gain;
            
            // Final safety check
            if (std::isnan(output) || std::isinf(output))
                output = 0.0f;
            
            // Only apply soft-clipping if signal exceeds safe range
            // This preserves the dry path when mix=0 and prevents unnecessary distortion
            if (std::abs(output) > 1.0f)
            {
                output = std::tanh(output * 0.5f) * 2.0f; // Gentle soft-clipping
            }
            
            buffer.setSample(channel, sample, output);
        }
    }
}

//==============================================================================
bool FlowstateProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* FlowstateProcessor::createEditor()
{
#if FLOWSTATE_HEADLESS_TEST
    return nullptr;
#else
    return new FlowstateEditor(*this);
#endif
}

//==============================================================================
void FlowstateProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    xml->setAttribute("version", 1);
    copyXmlToBinary(*xml, destData);
}

void FlowstateProcessor::setStateInformation(const void* /*data*/, int /*sizeInBytes*/)
{
    // Intentionally do NOT restore saved state.
    // Each new plugin instance always starts at default parameter values.
    // This ensures a fresh, transparent starting point every time the plugin is loaded.
}

//==============================================================================
const juce::String FlowstateProcessor::getName() const
{
    return "FlowstatePlugin";
}

bool FlowstateProcessor::acceptsMidi() const
{
    return false;
}

bool FlowstateProcessor::producesMidi() const
{
    return false;
}

double FlowstateProcessor::getTailLengthSeconds() const
{
    // Return the maximum tail length based on reverb decay time
    auto decayTime = parameters.getRawParameterValue(ParameterIDs::reverbDecay)->load();
    return static_cast<double>(decayTime);
}

int FlowstateProcessor::getNumPrograms()
{
    return 1;
}

int FlowstateProcessor::getCurrentProgram()
{
    return 0;
}

void FlowstateProcessor::setCurrentProgram(int)
{
}

const juce::String FlowstateProcessor::getProgramName(int)
{
    return {};
}

void FlowstateProcessor::changeProgramName(int, const juce::String&)
{
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FlowstateProcessor();
}
