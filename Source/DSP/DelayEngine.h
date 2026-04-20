/*
  ==============================================================================

    DelayEngine.h
    Created: Flowstate Plugin Delay Engine
    Description: Delay processing with circular buffer, tempo sync, and diffusion

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_core/juce_core.h>
#include <array>

class DelayEngine
{
public:
    DelayEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // All setters are safe to call from the audio thread.
    // setDelayTime() is a no-op until prepare() has been called.
    void setDelayTime(float milliseconds);
    void setDelayTimeFromTempo(double bpm, int division);
    void setFeedback(float amount);   // 0.0 – 1.0
    void setDiffusion(float amount);  // 0.0 – 1.0

    void process(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& feedbackInput);

private:
    // Allpass filter for diffusion
    class AllpassFilter
    {
    public:
        void prepare(double sampleRate, int delaySamples);
        void reset();
        float process(float input);

    private:
        juce::AudioBuffer<float> buffer;
        int   writePos    = 0;
        int   delayLength = 0;
        float gain        = 0.7f;
    };

    juce::AudioBuffer<float> delayBuffer;
    int   writePosition           = 0;
    float currentDelayInSamples   = 0.0f;
    float targetDelayInSamples    = 0.0f;
    double currentSampleRate      = 44100.0;

    // 1-pole IIR smoothing coefficient — set in prepare() for a ~15ms ramp
    float smoothingCoeff          = 0.0015f;

    float feedbackAmount          = 0.5f;
    float diffusionAmount         = 0.0f;

    // Guard: prevents any buffer operations before prepare() completes
    bool isPrepared               = false;

    std::array<AllpassFilter, 4> diffusionFilters;

    float calculateDelayFromDivision(double bpm, int division);
    float applyFeedbackLimiter(float sample);
    float interpolateRead(const juce::AudioBuffer<float>& buffer, int channel, float position);
};
