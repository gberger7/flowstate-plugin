/*
  ==============================================================================

    DelayEngine.cpp
    Created: Flowstate Plugin Delay Engine Implementation

  ==============================================================================
*/

#include "DelayEngine.h"

void DelayEngine::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // 4-second buffer — max delay is 2000ms so this is always sufficient
    const int bufferSize = static_cast<int>(sampleRate * 4.0);
    delayBuffer.setSize(2, bufferSize, false, true, false);
    delayBuffer.clear();

    writePosition = 0;
    currentDelayInSamples = 0.0f;
    targetDelayInSamples  = 0.0f;

    // Smoothing: 20ms ramp at the given sample rate
    // coefficient = 1 - exp(-1 / (sampleRate * rampTime))
    smoothingCoeff = 1.0f - std::exp(-1.0f / (static_cast<float>(sampleRate) * 0.020f));

    // Diffusion allpass filters (5–15ms randomised delays)
    juce::Random random;
    for (int i = 0; i < 4; ++i)
    {
        int delaySamples = static_cast<int>(sampleRate * (0.005 + random.nextFloat() * 0.010));
        diffusionFilters[i].prepare(sampleRate, delaySamples);
    }

    isPrepared = true;
}

void DelayEngine::reset()
{
    delayBuffer.clear();
    writePosition = 0;
    currentDelayInSamples = 0.0f;
    targetDelayInSamples  = 0.0f;

    for (auto& filter : diffusionFilters)
        filter.reset();
}

void DelayEngine::setDelayTime(float milliseconds)
{
    if (!isPrepared)
        return;

    // Clamp to valid range — never exceed what the buffer can hold
    const float maxMs = (static_cast<float>(delayBuffer.getNumSamples()) /
                         static_cast<float>(currentSampleRate)) * 1000.0f - 1.0f;
    milliseconds = juce::jlimit(1.0f, maxMs, milliseconds);

    targetDelayInSamples = (milliseconds / 1000.0f) * static_cast<float>(currentSampleRate);

    // Clamp target to buffer size as a hard safety net
    targetDelayInSamples = juce::jlimit(
        1.0f,
        static_cast<float>(delayBuffer.getNumSamples() - 2),
        targetDelayInSamples);

    // On first call, snap current to target so we don't ramp from 0
    if (currentDelayInSamples <= 0.0f)
        currentDelayInSamples = targetDelayInSamples;
}

void DelayEngine::setDelayTimeFromTempo(double bpm, int division)
{
    float milliseconds = calculateDelayFromDivision(bpm, division);
    setDelayTime(milliseconds);
}

void DelayEngine::setFeedback(float amount)
{
    feedbackAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void DelayEngine::setDiffusion(float amount)
{
    diffusionAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void DelayEngine::process(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& feedbackInput)
{
    // Guard: do nothing until prepare() has been called
    if (!isPrepared)
        return;

    const int numSamples   = buffer.getNumSamples();
    const int numChannels  = buffer.getNumChannels();
    const int bufferLength = delayBuffer.getNumSamples();

    if (bufferLength == 0 || numSamples == 0)
        return;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Smooth delay time per-sample using a proper 1-pole IIR coefficient.
        // This gives a ~15ms ramp regardless of sample rate.
        currentDelayInSamples += (targetDelayInSamples - currentDelayInSamples) * smoothingCoeff;

        // Hard-clamp the smoothed value so it can never exceed the buffer
        currentDelayInSamples = juce::jlimit(
            1.0f,
            static_cast<float>(bufferLength - 2),
            currentDelayInSamples);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            // Compute read position with correct modular wrap
            float readPosition = static_cast<float>(writePosition) - currentDelayInSamples;

            // Wrap into [0, bufferLength) — use fmod for robustness when
            // readPosition is more negative than -bufferLength
            if (readPosition < 0.0f)
            {
                readPosition += static_cast<float>(bufferLength);
                // If still negative (shouldn't happen after clamp above, but be safe)
                if (readPosition < 0.0f)
                    readPosition = 0.0f;
            }
            if (readPosition >= static_cast<float>(bufferLength))
                readPosition = 0.0f;

            float delayedSample = interpolateRead(delayBuffer, channel, readPosition);

            // NaN/Inf guard on the read output
            if (!std::isfinite(delayedSample))
                delayedSample = 0.0f;

            // Apply diffusion if enabled
            if (diffusionAmount > 0.0f)
            {
                float diffused = delayedSample;
                for (auto& filter : diffusionFilters)
                    diffused = filter.process(diffused);

                // Guard diffused output
                if (!std::isfinite(diffused))
                    diffused = delayedSample;

                delayedSample = delayedSample * (1.0f - diffusionAmount) + diffused * diffusionAmount;
            }

            // Get input and feedback
            float input    = buffer.getSample(channel, sample);
            float feedback = feedbackInput.getSample(channel, sample);

            // Guard inputs
            if (!std::isfinite(input))    input    = 0.0f;
            if (!std::isfinite(feedback)) feedback = 0.0f;

            // Write to delay buffer: input + limited feedback
            float toWrite = applyFeedbackLimiter(delayedSample * feedbackAmount + feedback);
            if (!std::isfinite(toWrite)) toWrite = 0.0f;

            delayBuffer.setSample(channel, writePosition, input + toWrite);

            // Output the delayed signal
            buffer.setSample(channel, sample, delayedSample);
        }

        // Advance write position
        writePosition = (writePosition + 1) % bufferLength;
    }
}

float DelayEngine::calculateDelayFromDivision(double bpm, int division)
{
    const float noteDivisions[] = {
        1.0f/32.0f, 1.5f/32.0f, 1.0f/48.0f,
        1.0f/16.0f, 1.5f/16.0f, 1.0f/24.0f,
        1.0f/8.0f,  1.5f/8.0f,  1.0f/12.0f,
        1.0f/4.0f,  1.5f/4.0f,  1.0f/6.0f,
        1.0f/2.0f,  1.5f/2.0f,  1.0f/3.0f,
        1.0f,       1.5f,       2.0f/3.0f
    };

    division = juce::jlimit(0, 17, division);

    float beatsPerMs = static_cast<float>(bpm) / 60000.0f;
    float delayMs    = noteDivisions[division] / beatsPerMs;

    return delayMs;
}

float DelayEngine::applyFeedbackLimiter(float sample)
{
    return std::tanh(sample);
}

float DelayEngine::interpolateRead(const juce::AudioBuffer<float>& buffer, int channel, float position)
{
    const int bufferLength = buffer.getNumSamples();
    if (bufferLength == 0)
        return 0.0f;

    // Integer and fractional parts
    int   index0 = static_cast<int>(position);
    float frac   = position - static_cast<float>(index0);

    // Clamp indices into valid range
    index0 = juce::jlimit(0, bufferLength - 1, index0);
    int index1 = (index0 + 1) % bufferLength;

    float s0 = buffer.getSample(channel, index0);
    float s1 = buffer.getSample(channel, index1);

    // Guard against NaN/Inf in the buffer itself
    if (!std::isfinite(s0)) s0 = 0.0f;
    if (!std::isfinite(s1)) s1 = 0.0f;

    return s0 + frac * (s1 - s0);
}

// AllpassFilter implementation
void DelayEngine::AllpassFilter::prepare(double /*sampleRate*/, int delaySamples)
{
    delayLength = std::max(1, delaySamples);
    buffer.setSize(1, delayLength, false, true, false);
    buffer.clear();
    writePos = 0;
}

void DelayEngine::AllpassFilter::reset()
{
    buffer.clear();
    writePos = 0;
}

float DelayEngine::AllpassFilter::process(float input)
{
    if (delayLength == 0)
        return input;

    float delayed = buffer.getSample(0, writePos);
    if (!std::isfinite(delayed)) delayed = 0.0f;

    float output = -gain * input + delayed;
    if (!std::isfinite(output)) output = 0.0f;

    buffer.setSample(0, writePos, input + gain * output);
    writePos = (writePos + 1) % delayLength;

    return output;
}
