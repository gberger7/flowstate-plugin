/*
  ==============================================================================

    ModulationEngine.h
    Created: Flowstate Plugin Modulation Engine
    Description: Sine wave LFO for delay time and reverb diffusion modulation

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

class ModulationEngine
{
public:
    ModulationEngine() = default;
    
    void prepare(double sampleRate);
    void reset();
    
    void setRate(float hz); // 0.01 to 5.0 Hz
    void setDepth(float depth); // 0.0 to 1.0
    
    float getNextModulationValue(); // Returns -1.0 to +1.0
    
private:
    double phase = 0.0;  // Use double for better precision
    double phaseIncrement = 0.0;  // Use double for better precision
    float depthAmount = 0.0f;
    double sampleRate = 44100.0;
};
