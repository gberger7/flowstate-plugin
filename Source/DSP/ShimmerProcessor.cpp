/*
  ==============================================================================

    ShimmerProcessor.cpp
    Pitch shifting for shimmer feedback effect.

    Writes input into a circular buffer, then reads back at pitchRatio samples
    per output sample. The read pointer stays a fixed distance behind the write
    pointer (half the buffer). When the read pointer laps the write pointer,
    it wraps — this is the nature of the effect and contributes to the washy,
    cumulative shimmer character.

  ==============================================================================
*/

#include "ShimmerProcessor.h"

void ShimmerProcessor::prepare(double sr, int /*samplesPerBlock*/)
{
    sampleRate = sr;
    circularBuffer.setSize(2, kBufferSize, false, true, false);
    reset();
}

void ShimmerProcessor::reset()
{
    circularBuffer.clear();
    writePos = 0;
    readPos  = static_cast<float>(kBufferSize / 2); // start half a buffer behind write
}

void ShimmerProcessor::setPitchShift(float semitones)
{
    semitones  = juce::jlimit(-24.0f, 24.0f, semitones);
    pitchRatio = std::pow(2.0f, semitones / 12.0f);
}

void ShimmerProcessor::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int s = 0; s < numSamples; ++s)
    {
        // Write current input into circular buffer
        for (int ch = 0; ch < numChannels; ++ch)
            circularBuffer.setSample(ch, writePos, buffer.getSample(ch, s));

        // Read back at pitchRatio speed using linear interpolation
        int   r0   = static_cast<int>(readPos) & (kBufferSize - 1);
        int   r1   = (r0 + 1) & (kBufferSize - 1);
        float frac = readPos - std::floor(readPos);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s0 = circularBuffer.getSample(ch, r0);
            float s1 = circularBuffer.getSample(ch, r1);
            buffer.setSample(ch, s, s0 + frac * (s1 - s0));
        }

        // Advance pointers
        writePos = (writePos + 1) & (kBufferSize - 1);
        readPos  += pitchRatio;
        if (readPos >= static_cast<float>(kBufferSize))
            readPos -= static_cast<float>(kBufferSize);
    }
}
