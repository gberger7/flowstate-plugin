/*
  ==============================================================================

    Parameters.h
    Created: Flowstate Plugin Parameter Definitions
    Description: Defines all 21 parameter IDs and their specifications

  ==============================================================================
*/

#pragma once

#include <juce_core/juce_core.h>

namespace ParameterIDs
{
    // Delay parameters
    const juce::String delayTime { "delayTime" };
    const juce::String delaySync { "delaySync" };
    const juce::String delayDivision { "delayDivision" };
    const juce::String delayFeedback { "delayFeedback" };
    const juce::String delayDiffusion { "delayDiffusion" };
    
    // Reverb parameters
    const juce::String reverbSize { "reverbSize" };
    const juce::String reverbDecay { "reverbDecay" };
    const juce::String reverbDamping { "reverbDamping" };
    
    // Core controls
    const juce::String blend { "blend" };
    const juce::String mix { "mix" };
    
    // Modulation
    const juce::String modRate { "modRate" };
    const juce::String modDepth { "modDepth" };
    
    // Character
    const juce::String drive { "drive" };
    const juce::String tone { "tone" };
    
    // Ducking
    const juce::String duckSensitivity { "duckSensitivity" };
    
    // Shimmer
    const juce::String shimmerEnabled { "shimmerEnabled" };
    const juce::String shimmerPitch { "shimmerPitch" };
    
    // Reverse
    const juce::String reverseMode { "reverseMode" }; // 0=OFF, 1=REVERB, 2=DELAY, 3=BOTH
    
    // Freeze
    const juce::String freezeEnabled { "freezeEnabled" };
    
    // Output
    const juce::String outputGain { "outputGain" };
    const juce::String stereoWidth { "stereoWidth" };
}
