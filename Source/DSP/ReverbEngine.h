/*
  ==============================================================================

    ReverbEngine.h
    Created: Flowstate Plugin Reverb Engine
    Description: FDN reverb algorithm with size, decay, and damping controls

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <array>

class ReverbEngine
{
public:
    ReverbEngine() : feedbackMatrix(8, 8) {}  // Initialize 8x8 matrix
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    void setSize(float size); // 0.0 to 1.0
    void setDecayTime(float seconds); // 0.1 to 20.0
    void setDamping(float amount); // 0.0 to 1.0
    
    void process(juce::AudioBuffer<float>& buffer);
    
private:
    // One-pole lowpass filter for damping
    class OnePoleLowpass
    {
    public:
        void setCutoff(float cutoffHz, double sampleRate);
        void reset();
        float process(float input);
        
    private:
        float coefficient = 0.0f;
        float state = 0.0f;
    };
    
    // FDN implementation with 8 delay lines
    std::array<juce::AudioBuffer<float>, 8> fdnDelayLines;
    std::array<int, 8> fdnDelayLengths;
    std::array<int, 8> fdnWritePositions;
    std::array<OnePoleLowpass, 8> dampingFilters;
    
    juce::dsp::Matrix<float> feedbackMatrix;
    
    double currentSampleRate = 44100.0;
    float sizeAmount = 0.5f;
    float decayAmount = 2.0f;
    float dampingAmount = 0.5f;
    
    void updateDelayLengths(float size, double sampleRate);
    void updateDampingFilters(float damping, double sampleRate);
    void createHouseholderMatrix();
};
