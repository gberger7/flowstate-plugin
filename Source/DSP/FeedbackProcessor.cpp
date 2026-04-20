/*
  ==============================================================================

    FeedbackProcessor.cpp
    Created: Flowstate Plugin Feedback Processor Implementation

  ==============================================================================
*/

#include "FeedbackProcessor.h"
#include "ShimmerProcessor.h"

void FeedbackProcessor::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    
    toneFilterLeft.prepare(spec);
    toneFilterRight.prepare(spec);
    
    toneFilterLeft.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    toneFilterRight.setType(juce::dsp::FirstOrderTPTFilterType::lowpass);
    
    updateToneFilter();
}

void FeedbackProcessor::setDrive(float amount)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void FeedbackProcessor::setTone(float amount)
{
    toneAmount = juce::jlimit(0.0f, 1.0f, amount);
    updateToneFilter();
}

void FeedbackProcessor::process(juce::AudioBuffer<float>& buffer, ShimmerProcessor* shimmer, bool shimmerEnabled)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Apply drive saturation (only if drive > 0)
    if (driveAmount > 0.001f)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float inputSample = buffer.getSample(channel, sample);
                float saturated = applySaturation(inputSample, driveAmount);
                buffer.setSample(channel, sample, saturated);
            }
        }
    }
    
    // Apply tone filtering (only if tone > 0, which means cutoff < 20kHz)
    // At tone=0, cutoff is 20kHz which should not filter audio range
    // Only apply filter if tone > 0.001 to avoid unnecessary processing
    if (toneAmount > 0.001f)
    {
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float leftSample = buffer.getSample(0, sample);
            leftSample = toneFilterLeft.processSample(0, leftSample);
            buffer.setSample(0, sample, leftSample);
            
            if (numChannels > 1)
            {
                float rightSample = buffer.getSample(1, sample);
                rightSample = toneFilterRight.processSample(0, rightSample);
                buffer.setSample(1, sample, rightSample);
            }
        }
    }
    
    // Apply shimmer if enabled
    if (shimmerEnabled && shimmer != nullptr)
    {
        shimmer->process(buffer);
    }
}

float FeedbackProcessor::applySaturation(float sample, float drive)
{
    // Input gain staging: increase gain based on drive amount
    float gained = sample * (1.0f + drive * 3.0f);
    
    // Soft clipping using tanh
    float saturated = std::tanh(gained);
    
    // Output compensation to maintain perceived loudness
    float compensated = saturated / (1.0f + drive * 0.5f);
    
    return compensated;
}

void FeedbackProcessor::updateToneFilter()
{
    // Map tone 0.0-1.0 to cutoff frequency 20kHz to 800Hz
    float cutoffHz = 20000.0f * std::pow(0.04f, toneAmount); // Exponential mapping
    
    toneFilterLeft.setCutoffFrequency(static_cast<float>(cutoffHz));
    toneFilterRight.setCutoffFrequency(static_cast<float>(cutoffHz));
}
