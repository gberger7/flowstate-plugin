#include "ParameterBridge.h"
#include "../PluginProcessor.h"

ParameterBridge::ParameterBridge(FlowstateProcessor& proc, juce::WebBrowserComponent& web)
    : processor(proc), webView(web)
{
}

ParameterBridge::~ParameterBridge()
{
    stopTimer();
}

void ParameterBridge::startSync()
{
    startTimerHz(20);
}

// Called on the message thread at 20Hz.
// Reads all 21 APVTS normalized values and sends them to JS in one call.
// No diffing. No rate limiting. No flags. Just read and send.
void ParameterBridge::timerCallback()
{
    auto& apvts = processor.getParameters();

    auto get = [&](const juce::String& id) -> float
    {
        auto* p = apvts.getParameter(id);
        return p ? p->getValue() : 0.0f;
    };

    juce::String js =
        "if(window.flowstate&&window.flowstate.syncParams){"
        "window.flowstate.syncParams({"
        "delayTime:"       + juce::String(get(ParameterIDs::delayTime),       6) + ","
        "delaySync:"       + juce::String(get(ParameterIDs::delaySync),       6) + ","
        "delayDivision:"   + juce::String(get(ParameterIDs::delayDivision),   6) + ","
        "delayFeedback:"   + juce::String(get(ParameterIDs::delayFeedback),   6) + ","
        "delayDiffusion:"  + juce::String(get(ParameterIDs::delayDiffusion),  6) + ","
        "reverbSize:"      + juce::String(get(ParameterIDs::reverbSize),      6) + ","
        "reverbDecay:"     + juce::String(get(ParameterIDs::reverbDecay),     6) + ","
        "reverbDamping:"   + juce::String(get(ParameterIDs::reverbDamping),   6) + ","
        "blend:"           + juce::String(get(ParameterIDs::blend),           6) + ","
        "mix:"             + juce::String(get(ParameterIDs::mix),             6) + ","
        "modRate:"         + juce::String(get(ParameterIDs::modRate),         6) + ","
        "modDepth:"        + juce::String(get(ParameterIDs::modDepth),        6) + ","
        "drive:"           + juce::String(get(ParameterIDs::drive),           6) + ","
        "tone:"            + juce::String(get(ParameterIDs::tone),            6) + ","
        "duckSensitivity:" + juce::String(get(ParameterIDs::duckSensitivity), 6) + ","
        "shimmerEnabled:"  + juce::String(get(ParameterIDs::shimmerEnabled),  6) + ","
        "shimmerPitch:"    + juce::String(get(ParameterIDs::shimmerPitch),    6) + ","
        "reverseMode:"     + juce::String(get(ParameterIDs::reverseMode),     6) + ","
        "freezeEnabled:"   + juce::String(get(ParameterIDs::freezeEnabled),   6) + ","
        "outputGain:"      + juce::String(get(ParameterIDs::outputGain),      6) + ","
        "stereoWidth:"     + juce::String(get(ParameterIDs::stereoWidth),     6) +
        "});}";

    webView.evaluateJavascript(js);
}
