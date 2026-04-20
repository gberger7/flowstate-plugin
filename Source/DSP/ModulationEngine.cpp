/*
  ==============================================================================

    ModulationEngine.cpp
    Created: Flowstate Plugin Modulation Engine Implementation

  ==============================================================================
*/

#include "ModulationEngine.h"

void ModulationEngine::prepare(double sampleRate)
{
    this->sampleRate = sampleRate;
    phase = 0.0;
}

void ModulationEngine::reset()
{
    phase = 0.0;
}

void ModulationEngine::setRate(float hz)
{
    hz = juce::jlimit(0.01f, 5.0f, hz);
    phaseIncrement = static_cast<double>(hz) / sampleRate;
}

void ModulationEngine::setDepth(float depth)
{
    depthAmount = juce::jlimit(0.0f, 1.0f, depth);
}

float ModulationEngine::getNextModulationValue()
{
    // Generate sine wave
    float sineValue = std::sin(phase * 2.0 * juce::MathConstants<double>::pi);
    
    // Advance phase
    phase += phaseIncrement;
    if (phase >= 1.0)
        phase -= 1.0;
    
    // Scale by depth
    return sineValue * depthAmount;
}
