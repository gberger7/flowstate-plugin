/*
  ==============================================================================

    PluginProcessor.h
    Created: Flowstate Plugin Audio Processor
    Description: Main AudioProcessor class with parameter management and signal routing

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Parameters.h"
#include "DSP/DelayEngine.h"
#include "DSP/ReverbEngine.h"
#include "DSP/FeedbackProcessor.h"
#include "DSP/DuckingProcessor.h"
#include "DSP/ShimmerProcessor.h"
#include "DSP/ModulationEngine.h"
#include "DSP/StereoWidthProcessor.h"
#include "DSP/ReverseBuffer.h"

class FlowstateProcessor : public juce::AudioProcessor
{
public:
    FlowstateProcessor();
    ~FlowstateProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    
    // DSP Components
    DelayEngine delayEngine;
    ReverbEngine reverbEngine;
    FeedbackProcessor feedbackProcessor;
    DuckingProcessor duckingProcessor;
    ShimmerProcessor shimmerProcessor;
    ModulationEngine modulationEngine;
    StereoWidthProcessor widthProcessor;
    ReverseBuffer reverseBuffer;
    
    // Internal audio buffers
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> wetBuffer;
    juce::AudioBuffer<float> delayWetBuffer;
    juce::AudioBuffer<float> reverbWetBuffer;
    juce::AudioBuffer<float> feedbackBuffer;
    juce::AudioBuffer<float> freezeBuffer;
    
    // Freeze state
    bool freezeActive = false;
    int freezeWritePos = 0;
    
    // Parameter smoothing (10ms ramp time)
    juce::SmoothedValue<float> smoothedBlend;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedOutputGain;
    juce::SmoothedValue<float> smoothedDrive;
    juce::SmoothedValue<float> smoothedTone;
    juce::SmoothedValue<float> smoothedStereoWidth;
    juce::SmoothedValue<float> smoothedDuckSensitivity;
    
    // Additional parameter smoothing for all remaining parameters
    juce::SmoothedValue<float> smoothedDelayFeedback;
    juce::SmoothedValue<float> smoothedDelayDiffusion;
    juce::SmoothedValue<float> smoothedReverbSize;
    juce::SmoothedValue<float> smoothedReverbDecay;
    juce::SmoothedValue<float> smoothedReverbDamping;
    juce::SmoothedValue<float> smoothedModRate;
    juce::SmoothedValue<float> smoothedModDepth;
    juce::SmoothedValue<float> smoothedShimmerPitch;
    
    // Helper methods
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateDSPParameters();
    void updateTempoSync();
    float validateParameter(float value, float min, float max);
    
    // Host tempo tracking
    double lastKnownBpm = 120.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlowstateProcessor)
};
