/*
  ==============================================================================

    ReverseBuffer.cpp
    Created: Flowstate Plugin Reverse Buffer Implementation

  ==============================================================================
*/

#include "ReverseBuffer.h"

void ReverseBuffer::prepare(double sampleRate, int samplesPerBlock)
{
    // Allocate 3 seconds of buffer
    bufferLengthSamples = static_cast<int>(sampleRate * 3.0);
    circularBuffer.setSize(2, bufferLengthSamples, false, true, false);
    circularBuffer.clear();
    writePosition = 0;
    samplesWritten = 0;
}

void ReverseBuffer::reset()
{
    circularBuffer.clear();
    writePosition = 0;
    samplesWritten = 0;
}

void ReverseBuffer::write(const juce::AudioBuffer<float>& input)
{
    const int numSamples = input.getNumSamples();
    const int numChannels = input.getNumChannels();
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        for (int ch = 0; ch < numChannels; ++ch)
        {
            circularBuffer.setSample(ch, writePosition, input.getSample(ch, sample));
        }
        
        writePosition = (writePosition + 1) % bufferLengthSamples;
    }
    
    // Track how many samples have been written (saturate at buffer length)
    samplesWritten = std::min(samplesWritten + numSamples, bufferLengthSamples);
}

void ReverseBuffer::readReverse(juce::AudioBuffer<float>& output, int mode)
{
    if (mode == 0) // OFF
        return;
    
    // Only apply reverse effect if buffer has been sufficiently filled
    // Require at least 0.5 seconds of audio to avoid outputting silence
    const int minSamplesRequired = bufferLengthSamples / 6; // 0.5 seconds out of 3 seconds
    if (samplesWritten < minSamplesRequired)
    {
        // Buffer not filled enough yet - pass through the input unchanged
        return;
    }
    
    const int numSamples = output.getNumSamples();
    const int numChannels = output.getNumChannels();
    
    // Read backwards from current write position
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Calculate reverse read position
        float readPos = static_cast<float>(writePosition) - static_cast<float>(sample);
        if (readPos < 0.0f)
            readPos += static_cast<float>(bufferLengthSamples);
        
        for (int ch = 0; ch < numChannels; ++ch)
        {
            float reversedSample = interpolateRead(ch, readPos);
            
            // Mix or replace based on mode
            // For now, we'll replace the output with reversed signal
            // In actual implementation, this would be integrated with delay/reverb engines
            output.setSample(ch, sample, reversedSample);
        }
    }
}

float ReverseBuffer::interpolateRead(int channel, float position)
{
    int index0 = static_cast<int>(std::floor(position));
    int index1 = (index0 + 1) % bufferLengthSamples;
    float frac = position - std::floor(position);
    
    // Ensure indices are within bounds
    index0 = (index0 + bufferLengthSamples) % bufferLengthSamples;
    
    float sample0 = circularBuffer.getSample(channel, index0);
    float sample1 = circularBuffer.getSample(channel, index1);
    
    // Linear interpolation
    return sample0 + frac * (sample1 - sample0);
}
