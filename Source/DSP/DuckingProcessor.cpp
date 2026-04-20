/*
  ==============================================================================

    DuckingProcessor.cpp
    Created: Flowstate Plugin Ducking Processor Implementation

  ==============================================================================
*/

#include "DuckingProcessor.h"

void DuckingProcessor::prepare(double sampleRate)
{
    updateCoefficients(sampleRate);
    envelopeState = 0.0f;
}

void DuckingProcessor::setSensitivity(float sensitivity)
{
    sensitivityAmount = juce::jlimit(0.0f, 1.0f, sensitivity);
}

float DuckingProcessor::processEnvelope(const juce::AudioBuffer<float>& dryBuffer)
{
    const int numSamples = dryBuffer.getNumSamples();
    const int numChannels = dryBuffer.getNumChannels();
    
    float maxEnvelope = envelopeState;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Calculate peak amplitude across all channels
        float peak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float absSample = std::abs(dryBuffer.getSample(ch, sample));
            peak = std::max(peak, absSample);
        }
        
        // Envelope follower with attack/release
        if (peak > envelopeState)
        {
            // Attack
            envelopeState = envelopeState + attackCoeff * (peak - envelopeState);
        }
        else
        {
            // Release
            envelopeState = envelopeState + releaseCoeff * (peak - envelopeState);
        }
        
        maxEnvelope = std::max(maxEnvelope, envelopeState);
    }
    
    return maxEnvelope;
}

void DuckingProcessor::applyDucking(juce::AudioBuffer<float>& wetBuffer, float envelopeValue)
{
    if (sensitivityAmount <= 0.0f)
        return; // No ducking
    
    // Calculate threshold based on sensitivity (-40dB to -10dB range)
    float thresholdDb = -40.0f + (sensitivityAmount * 30.0f);
    float threshold = juce::Decibels::decibelsToGain(thresholdDb);
    
    // Calculate attenuation
    float attenuation = 1.0f;
    if (envelopeValue > threshold)
    {
        // Reduce wet signal by up to 80% based on sensitivity
        attenuation = 1.0f - (sensitivityAmount * 0.8f);
    }
    
    // Apply attenuation to wet buffer
    wetBuffer.applyGain(attenuation);
}

void DuckingProcessor::updateCoefficients(double sampleRate)
{
    // Calculate coefficients for exponential smoothing
    attackCoeff = 1.0f - std::exp(-1.0f / (attackTime * static_cast<float>(sampleRate)));
    releaseCoeff = 1.0f - std::exp(-1.0f / (releaseTime * static_cast<float>(sampleRate)));
}
