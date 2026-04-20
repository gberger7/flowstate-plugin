/*
  ==============================================================================

    StereoWidthProcessor.cpp
    Created: Flowstate Plugin Stereo Width Processor Implementation

  ==============================================================================
*/

#include "StereoWidthProcessor.h"

void StereoWidthProcessor::setWidth(float width)
{
    widthAmount = juce::jlimit(0.0f, 1.5f, width);
}

void StereoWidthProcessor::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    if (numChannels < 2)
        return; // Need stereo signal
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float left = buffer.getSample(0, sample);
        float right = buffer.getSample(1, sample);
        
        // Encode to M/S
        float mid, side;
        encodeToMS(left, right, mid, side);
        
        // Scale side signal by width
        side *= widthAmount;
        
        // Decode back to L/R
        float newLeft, newRight;
        decodeFromMS(mid, side, newLeft, newRight);
        
        buffer.setSample(0, sample, newLeft);
        buffer.setSample(1, sample, newRight);
    }
}

void StereoWidthProcessor::encodeToMS(float left, float right, float& mid, float& side)
{
    mid = (left + right) * 0.5f;
    side = (left - right) * 0.5f;
}

void StereoWidthProcessor::decodeFromMS(float mid, float side, float& left, float& right)
{
    left = mid + side;
    right = mid - side;
}
