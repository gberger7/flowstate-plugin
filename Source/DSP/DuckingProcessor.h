/*
  ==============================================================================

    DuckingProcessor.h
    Created: Flowstate Plugin Ducking Processor
    Description: Envelope follower for sidechain-style ducking

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

class DuckingProcessor
{
public:
    DuckingProcessor() = default;
    
    void prepare(double sampleRate);
    
    void setSensitivity(float sensitivity); // 0.0 to 1.0
    
    float processEnvelope(const juce::AudioBuffer<float>& dryBuffer);
    void applyDucking(juce::AudioBuffer<float>& wetBuffer, float envelopeValue);
    
private:
    float envelopeState = 0.0f;
    float sensitivityAmount = 0.0f;
    
    float attackTime = 0.01f;  // 10ms
    float releaseTime = 0.2f;  // 200ms
    
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    
    void updateCoefficients(double sampleRate);
};
