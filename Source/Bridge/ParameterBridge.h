#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../Parameters.h"

class FlowstateProcessor;

// C++ → JS: 20Hz timer. Reads all 21 APVTS values. Sends one evaluateJavascript call.
// Nothing else.
class ParameterBridge : public juce::Timer
{
public:
    ParameterBridge(FlowstateProcessor& processor, juce::WebBrowserComponent& webView);
    ~ParameterBridge() override;

    void startSync();

private:
    void timerCallback() override;

    FlowstateProcessor&        processor;
    juce::WebBrowserComponent& webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterBridge)
};
