/*
  ==============================================================================

    ReverseBuffer.h
    Created: Flowstate Plugin Reverse Buffer
    Description: 3-second rolling buffer for reverse effects

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

class ReverseBuffer
{
public:
    ReverseBuffer() = default;
    
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    
    void write(const juce::AudioBuffer<float>& input);
    void readReverse(juce::AudioBuffer<float>& output, int mode); // mode: 0=OFF, 1=REVERB, 2=DELAY, 3=BOTH
    
private:
    juce::AudioBuffer<float> circularBuffer;
    int writePosition = 0;
    int bufferLengthSamples = 0;
    int samplesWritten = 0; // Track how many samples have been written
    
    float interpolateRead(int channel, float position);
};
