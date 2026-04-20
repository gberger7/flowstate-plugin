cmake_minimum_required(VERSION 3.22)
project(FlowstatePlugin VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── JUCE ──────────────────────────────────────────────────────────────────────
set(JUCE_PATH "$ENV{HOME}/JUCE" CACHE PATH "Path to JUCE SDK")
add_subdirectory(${JUCE_PATH} JUCE EXCLUDE_FROM_ALL)

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin(FlowstatePlugin
    COMPANY_NAME            "Flowstate Audio"
    COMPANY_COPYRIGHT       "2024 Flowstate Audio"
    BUNDLE_ID               "com.flowstateaudio.flowstateplugin"
    PLUGIN_MANUFACTURER_CODE Flow
    PLUGIN_CODE             Flst
    FORMATS                 VST3 AU
    PRODUCT_NAME            "FlowstatePlugin"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES         "Fx" "Delay" "Reverb"
    AU_MAIN_TYPE            kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# ── Embedded WebUI ────────────────────────────────────────────────────────────
juce_add_binary_data(FlowstateWebUI SOURCES
    WebUI/index.html
    WebUI/fonts/RubikGlitch-Regular.woff2
    WebUI/fonts/Rajdhani-Regular.woff2
    WebUI/fonts/Rajdhani-Medium.woff2
    WebUI/fonts/Rajdhani-SemiBold.woff2
    WebUI/fonts/Rajdhani-Bold.woff2
)

# ── Sources ───────────────────────────────────────────────────────────────────
target_sources(FlowstatePlugin PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Bridge/ParameterBridge.cpp
    Source/DSP/DelayEngine.cpp
    Source/DSP/ReverbEngine.cpp
    Source/DSP/FeedbackProcessor.cpp
    Source/DSP/DuckingProcessor.cpp
    Source/DSP/ShimmerProcessor.cpp
    Source/DSP/ModulationEngine.cpp
    Source/DSP/StereoWidthProcessor.cpp
    Source/DSP/ReverseBuffer.cpp
)

# ── Include paths ─────────────────────────────────────────────────────────────
target_include_directories(FlowstatePlugin PRIVATE Source)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(FlowstatePlugin PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

# ── JUCE modules ──────────────────────────────────────────────────────────────
target_link_libraries(FlowstatePlugin PRIVATE
    FlowstateWebUI
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
