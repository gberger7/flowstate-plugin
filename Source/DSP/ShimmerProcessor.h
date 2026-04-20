/*
  ==============================================================================

    ShimmerProcessor.h
    Pitch shifting for shimmer feedback effect.

    Simple circular buffer with variable playback speed. Each call to process()
    reads from the buffer at a rate of pitchRatio samples per output sample,
    producing a pitch shift of pow(2, semitones/12). Designed for use in the
    feedback loop — the cumulative effect of repeated passes creates the
    classic shimmer sound. Evaluated by ear, not by frequency measurement.

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

class ShimmerProcessor
{
public:
    ShimmerProcessor() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setPitchShift(float semitones); // -24.0 to +24.0, default +12
    void process(juce::AudioBuffer<float>& buffer);

private:
    static constexpr int kBufferSize = 65536; // ~1.5s at 44.1kHz, power-of-2 for fast modulo

    juce::AudioBuffer<float> circularBuffer;

    int   writePos    = 0;
    float readPos     = 0.0f;
    float pitchRatio  = 2.0f; // default +12 semitones
    double sampleRate = 44100.0;
};
