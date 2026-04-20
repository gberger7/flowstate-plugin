/*
  ==============================================================================

    ReverbEngine.cpp
    Created: Flowstate Plugin Reverb Engine Implementation

  ==============================================================================
*/

#include "ReverbEngine.h"

void ReverbEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Initialize feedback matrix (already sized in constructor)
    createHouseholderMatrix();
    
    // Initialize delay lines and filters
    updateDelayLengths(sizeAmount, sampleRate);
    updateDampingFilters(dampingAmount, sampleRate);
    
    for (int i = 0; i < 8; ++i)
    {
        fdnWritePositions[i] = 0;
        dampingFilters[i].reset();
    }
}

void ReverbEngine::reset()
{
    for (int i = 0; i < 8; ++i)
    {
        fdnDelayLines[i].clear();
        fdnWritePositions[i] = 0;
        dampingFilters[i].reset();
    }
}

void ReverbEngine::setSize(float size)
{
    sizeAmount = juce::jlimit(0.0f, 1.0f, size);
    updateDelayLengths(sizeAmount, currentSampleRate);
}

void ReverbEngine::setDecayTime(float seconds)
{
    decayAmount = juce::jlimit(0.1f, 20.0f, seconds);
}

void ReverbEngine::setDamping(float amount)
{
    dampingAmount = juce::jlimit(0.0f, 1.0f, amount);
    updateDampingFilters(dampingAmount, currentSampleRate);
}

void ReverbEngine::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Calculate feedback gain based on decay time
    // Longer decay = higher feedback gain (approaching 1.0)
    float feedbackGain = std::tanh(decayAmount / 20.0f) * 0.98f;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get input (mix L+R for mono input to FDN)
        float input = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            input += buffer.getSample(ch, sample);
        input *= 0.5f;
        
        // Read from all delay lines
        std::array<float, 8> delayOutputs;
        for (int i = 0; i < 8; ++i)
        {
            int readPos = fdnWritePositions[i];
            delayOutputs[i] = fdnDelayLines[i].getSample(0, readPos);
        }
        
        // Apply feedback matrix and damping
        std::array<float, 8> feedbackSignals;
        for (int i = 0; i < 8; ++i)
        {
            float sum = 0.0f;
            for (int j = 0; j < 8; ++j)
            {
                sum += feedbackMatrix(i, j) * delayOutputs[j];
            }
            
            // Apply damping filter and feedback gain
            sum = dampingFilters[i].process(sum);
            feedbackSignals[i] = sum * feedbackGain;
        }
        
        // Write input + feedback to delay lines
        for (int i = 0; i < 8; ++i)
        {
            float toWrite = input + feedbackSignals[i];
            fdnDelayLines[i].setSample(0, fdnWritePositions[i], toWrite);
            fdnWritePositions[i] = (fdnWritePositions[i] + 1) % fdnDelayLengths[i];
        }
        
        // Mix delay outputs to stereo
        // Use alternating delays for L/R to create stereo image
        float leftOut = 0.0f;
        float rightOut = 0.0f;
        for (int i = 0; i < 8; ++i)
        {
            if (i % 2 == 0)
                leftOut += delayOutputs[i];
            else
                rightOut += delayOutputs[i];
        }
        leftOut *= 0.25f;  // Normalize (4 delays per channel)
        rightOut *= 0.25f;
        
        // Write output
        buffer.setSample(0, sample, leftOut);
        if (numChannels > 1)
            buffer.setSample(1, sample, rightOut);
    }
}

void ReverbEngine::updateDelayLengths(float size, double sampleRate)
{
    // Prime number delay lengths for good diffusion
    // Scale based on size parameter (0.0 = small room, 1.0 = large hall)
    const int baseLengths[] = { 1051, 1123, 1277, 1381, 1511, 1657, 1777, 1913 };
    
    float sizeScale = 0.3f + size * 1.7f; // Range: 0.3x to 2.0x
    
    for (int i = 0; i < 8; ++i)
    {
        fdnDelayLengths[i] = static_cast<int>(baseLengths[i] * sizeScale);
        
        // Allocate buffer if needed
        if (fdnDelayLines[i].getNumSamples() < fdnDelayLengths[i])
        {
            fdnDelayLines[i].setSize(1, fdnDelayLengths[i], false, true, false);
            fdnDelayLines[i].clear();
        }
    }
}

void ReverbEngine::updateDampingFilters(float damping, double sampleRate)
{
    // Map damping 0-1 to cutoff frequency 20kHz to 800Hz
    float cutoffHz = 20000.0f * std::pow(0.04f, damping); // Exponential mapping
    
    for (auto& filter : dampingFilters)
    {
        filter.setCutoff(cutoffHz, sampleRate);
    }
}

void ReverbEngine::createHouseholderMatrix()
{
    // Create Householder feedback matrix for FDN
    // This provides good diffusion and decorrelation
    
    const float v = 1.0f / std::sqrt(8.0f);
    
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            if (i == j)
                feedbackMatrix(i, j) = 1.0f - 2.0f * v * v;
            else
                feedbackMatrix(i, j) = -2.0f * v * v;
        }
    }
}

// OnePoleLowpass implementation
void ReverbEngine::OnePoleLowpass::setCutoff(float cutoffHz, double sampleRate)
{
    float omega = juce::MathConstants<float>::twoPi * cutoffHz / static_cast<float>(sampleRate);
    coefficient = 1.0f - std::exp(-omega);
}

void ReverbEngine::OnePoleLowpass::reset()
{
    state = 0.0f;
}

float ReverbEngine::OnePoleLowpass::process(float input)
{
    state = state + coefficient * (input - state);
    return state;
}
