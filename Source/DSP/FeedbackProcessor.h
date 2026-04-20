/*
  ==============================================================================

    FeedbackProcessor.h
    Created: Flowstate Plugin Feedback Processor
    Description: Drive saturation and tone filtering for feedback path

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>

class ShimmerProcessor; // Forward declaration

class FeedbackProcessor
{
public:
    FeedbackProcessor() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    
    void setDrive(float amount); // 0.0 to 1.0
    void setTone(float amount); // 0.0 to 1.0
    
    void process(juce::AudioBuffer<float>& buffer, ShimmerProcessor* shimmer, bool shimmerEnabled);
    
private:
    juce::dsp::FirstOrderTPTFilter<float> toneFilterLeft;
    juce::dsp::FirstOrderTPTFilter<float> toneFilterRight;
    
    double currentSampleRate = 44100.0;
    float driveAmount = 0.0f;
    float toneAmount = 0.5f;
    
    float applySaturation(float sample, float drive);
    void updateToneFilter();
};
