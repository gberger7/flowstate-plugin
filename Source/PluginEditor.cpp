#include "PluginEditor.h"
#include <BinaryData.h>

// Resource provider: serves index.html and fonts from BinaryData.
// JS→C++ event handler: receives "paramChange" {id, value}, calls setValueNotifyingHost(). Done.
juce::WebBrowserComponent::Options
FlowstateEditor::buildWebOptions(FlowstateProcessor& proc)
{
    // ── Resource provider ────────────────────────────────────────────────────
    auto provider = [](const juce::String& url)
        -> std::optional<juce::WebBrowserComponent::Resource>
    {
        juce::String path = url;
        if (path.startsWith("/")) path = path.substring(1);

        struct Entry { const char* name; const char* data; int size; const char* mime; };
        const Entry entries[] = {
            { "index.html",                      BinaryData::index_html,               BinaryData::index_htmlSize,               "text/html"  },
            { "fonts/RubikGlitch-Regular.woff2", BinaryData::RubikGlitchRegular_woff2, BinaryData::RubikGlitchRegular_woff2Size, "font/woff2" },
            { "fonts/Rajdhani-Regular.woff2",    BinaryData::RajdhaniRegular_woff2,    BinaryData::RajdhaniRegular_woff2Size,    "font/woff2" },
            { "fonts/Rajdhani-Medium.woff2",     BinaryData::RajdhaniMedium_woff2,     BinaryData::RajdhaniMedium_woff2Size,     "font/woff2" },
            { "fonts/Rajdhani-SemiBold.woff2",   BinaryData::RajdhaniSemiBold_woff2,   BinaryData::RajdhaniSemiBold_woff2Size,   "font/woff2" },
            { "fonts/Rajdhani-Bold.woff2",       BinaryData::RajdhaniBold_woff2,       BinaryData::RajdhaniBold_woff2Size,       "font/woff2" },
        };
        for (const auto& e : entries)
        {
            if (path == e.name)
            {
                std::vector<std::byte> bytes(static_cast<size_t>(e.size));
                std::memcpy(bytes.data(), e.data, static_cast<size_t>(e.size));
                return juce::WebBrowserComponent::Resource{ std::move(bytes), e.mime };
            }
        }
        return std::nullopt;
    };

    // ── JS → C++ event handler ───────────────────────────────────────────────
    // JS fires: window.__JUCE__.backend.emitEvent("paramChange", {id, value})
    // We call setValueNotifyingHost() and stop. Nothing else.
    FlowstateProcessor* procPtr = &proc;

    auto onParamChange = [procPtr](const juce::var& data)
    {
        auto* obj = data.getDynamicObject();
        if (obj == nullptr) return;

        juce::String id = obj->getProperty("id").toString();
        if (id.isEmpty()) return;

        auto* param = procPtr->getParameters().getParameter(id);
        if (param == nullptr) return;

        float v = juce::jlimit(0.0f, 1.0f,
            static_cast<float>(static_cast<double>(obj->getProperty("value"))));

        param->setValueNotifyingHost(v);
    };

    return juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled()
        .withResourceProvider(std::move(provider),
                              juce::WebBrowserComponent::getResourceProviderRoot())
        .withEventListener("paramChange", std::move(onParamChange));
}

//==============================================================================
FlowstateEditor::FlowstateEditor(FlowstateProcessor& proc)
    : AudioProcessorEditor(proc),
      processor(proc),
      webView(buildWebOptions(proc))
{
    setSize(900, 620);
    setResizable(false, false);
    addAndMakeVisible(webView);

    bridge = std::make_unique<ParameterBridge>(processor, webView);

    // Load the UI, then start the 20Hz sync timer after the page is ready.
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot() + "index.html");
    juce::Timer::callAfterDelay(600, [this]()
    {
        if (bridge != nullptr)
            bridge->startSync();
    });
}

FlowstateEditor::~FlowstateEditor() {}

void FlowstateEditor::resized()
{
    webView.setBounds(getLocalBounds());
}
