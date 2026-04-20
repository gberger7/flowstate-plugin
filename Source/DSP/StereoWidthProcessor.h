/*
  ==============================================================================

    StereoWidthProcessor.h
    Created: Flowstate Plugin Stereo Width Processor
    Description: M/S encoding for stereo width control

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

class StereoWidthProcessor
{
public:
    StereoWidthProcessor() = default;
    
    void setWidth(float width); // 0.0 to 1.5 (0%=mono, 100%=normal, 150%=hyper-wide)
    
    void process(juce::AudioBuffer<float>& buffer);
    
private:
    float widthAmount = 1.0f;
    
    void encodeToMS(float left, float right, float& mid, float& side);
    void decodeFromMS(float mid, float side, float& left, float& right);
};
