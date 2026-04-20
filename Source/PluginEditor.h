#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "Bridge/ParameterBridge.h"

class FlowstateEditor : public juce::AudioProcessorEditor
{
public:
    explicit FlowstateEditor(FlowstateProcessor& processor);
    ~FlowstateEditor() override;

    void paint(juce::Graphics&) override {}
    void resized() override;

private:
    // Builds the WebBrowserComponent options: resource provider + JS→C++ event listener.
    // JS→C++: listens for "paramChange" event, calls setValueNotifyingHost(). Nothing else.
    static juce::WebBrowserComponent::Options buildWebOptions(FlowstateProcessor& proc);

    FlowstateProcessor&                    processor;
    juce::WebBrowserComponent              webView;
    std::unique_ptr<ParameterBridge>       bridge;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlowstateEditor)
};
