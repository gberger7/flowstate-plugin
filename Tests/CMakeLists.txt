cmake_minimum_required(VERSION 3.15)

project(FlowstatePluginTests VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
add_subdirectory($ENV{HOME}/JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Create test executables
add_executable(DelayEnginePropertyTests
    DelayEnginePropertyTests.cpp
    ../Source/DSP/DelayEngine.cpp
)

add_executable(ReverbEnginePropertyTests
    ReverbEnginePropertyTests.cpp
    ../Source/DSP/ReverbEngine.cpp
)

add_executable(FeedbackProcessorPropertyTests
    FeedbackProcessorPropertyTests.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
)

add_executable(ModulationEnginePropertyTests
    ModulationEnginePropertyTests.cpp
    ../Source/DSP/ModulationEngine.cpp
)

add_executable(DuckingProcessorPropertyTests
    DuckingProcessorPropertyTests.cpp
    ../Source/DSP/DuckingProcessor.cpp
)

add_executable(PluginProcessorIntegrationTest
    PluginProcessorIntegrationTest.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(SignalFlowPropertyTests
    SignalFlowPropertyTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(FreezePropertyTests
    FreezePropertyTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(StateManagementPropertyTests
    StateManagementPropertyTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(AudioQualityPropertyTests
    AudioQualityPropertyTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(ShimmerProcessorPropertyTests
    ShimmerProcessorPropertyTests.cpp
    ../Source/DSP/ShimmerProcessor.cpp
)

add_executable(StereoWidthProcessorPropertyTests
    StereoWidthProcessorPropertyTests.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
)

add_executable(SignalPathIntegrationTests
    SignalPathIntegrationTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

add_executable(DAWCompatibilityTests
    DAWCompatibilityTests.cpp
    ../Source/PluginProcessor.cpp
    ../Source/DSP/DelayEngine.cpp
    ../Source/DSP/ReverbEngine.cpp
    ../Source/DSP/FeedbackProcessor.cpp
    ../Source/DSP/DuckingProcessor.cpp
    ../Source/DSP/ShimmerProcessor.cpp
    ../Source/DSP/ModulationEngine.cpp
    ../Source/DSP/StereoWidthProcessor.cpp
    ../Source/DSP/ReverseBuffer.cpp
)

# Include directories
target_include_directories(DelayEnginePropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(ReverbEnginePropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(FeedbackProcessorPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(ModulationEnginePropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(DuckingProcessorPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(PluginProcessorIntegrationTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(SignalFlowPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(FreezePropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(StateManagementPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(AudioQualityPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(ShimmerProcessorPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(StereoWidthProcessorPropertyTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(SignalPathIntegrationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(SignalPathIntegrationTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)

target_include_directories(DAWCompatibilityTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_compile_definitions(DAWCompatibilityTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)

# Headless define for all targets that include PluginProcessor.cpp (no editor dependency)
target_compile_definitions(PluginProcessorIntegrationTest PRIVATE FLOWSTATE_HEADLESS_TEST=1)
target_compile_definitions(SignalFlowPropertyTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)
target_compile_definitions(FreezePropertyTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)
target_compile_definitions(StateManagementPropertyTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)
target_compile_definitions(AudioQualityPropertyTests PRIVATE FLOWSTATE_HEADLESS_TEST=1)

# Link JUCE modules
target_link_libraries(DelayEnginePropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(ReverbEnginePropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(FeedbackProcessorPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(ModulationEnginePropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(DuckingProcessorPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(PluginProcessorIntegrationTest PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(SignalFlowPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(FreezePropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(StateManagementPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(AudioQualityPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(ShimmerProcessorPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
)

target_link_libraries(StereoWidthProcessorPropertyTests PRIVATE
    juce::juce_audio_basics
    juce::juce_core
)

target_link_libraries(SignalPathIntegrationTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

target_link_libraries(DAWCompatibilityTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_dsp
    juce::juce_data_structures
)

# Compiler flags
if(MSVC)
    target_compile_options(DelayEnginePropertyTests PRIVATE /W4)
    target_compile_options(ReverbEnginePropertyTests PRIVATE /W4)
    target_compile_options(FeedbackProcessorPropertyTests PRIVATE /W4)
    target_compile_options(ModulationEnginePropertyTests PRIVATE /W4)
    target_compile_options(DuckingProcessorPropertyTests PRIVATE /W4)
    target_compile_options(PluginProcessorIntegrationTest PRIVATE /W4)
    target_compile_options(SignalFlowPropertyTests PRIVATE /W4)
    target_compile_options(FreezePropertyTests PRIVATE /W4)
    target_compile_options(StateManagementPropertyTests PRIVATE /W4)
    target_compile_options(AudioQualityPropertyTests PRIVATE /W4)
    target_compile_options(ShimmerProcessorPropertyTests PRIVATE /W4)
    target_compile_options(StereoWidthProcessorPropertyTests PRIVATE /W4)
else()
    target_compile_options(DelayEnginePropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(ReverbEnginePropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(FeedbackProcessorPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(ModulationEnginePropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(DuckingProcessorPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(PluginProcessorIntegrationTest PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(SignalFlowPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(FreezePropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(StateManagementPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(AudioQualityPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(ShimmerProcessorPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(StereoWidthProcessorPropertyTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(SignalPathIntegrationTests PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(DAWCompatibilityTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Enable testing
enable_testing()
add_test(NAME DelayEnginePropertyTests COMMAND DelayEnginePropertyTests)
add_test(NAME ReverbEnginePropertyTests COMMAND ReverbEnginePropertyTests)
add_test(NAME FeedbackProcessorPropertyTests COMMAND FeedbackProcessorPropertyTests)
add_test(NAME ModulationEnginePropertyTests COMMAND ModulationEnginePropertyTests)
add_test(NAME DuckingProcessorPropertyTests COMMAND DuckingProcessorPropertyTests)
add_test(NAME PluginProcessorIntegrationTest COMMAND PluginProcessorIntegrationTest)
add_test(NAME SignalFlowPropertyTests COMMAND SignalFlowPropertyTests)
add_test(NAME FreezePropertyTests COMMAND FreezePropertyTests)
add_test(NAME StateManagementPropertyTests COMMAND StateManagementPropertyTests)
add_test(NAME AudioQualityPropertyTests COMMAND AudioQualityPropertyTests)
add_test(NAME ShimmerProcessorPropertyTests COMMAND ShimmerProcessorPropertyTests)
add_test(NAME StereoWidthProcessorPropertyTests COMMAND StereoWidthProcessorPropertyTests)
add_test(NAME SignalPathIntegrationTests COMMAND SignalPathIntegrationTests)
add_test(NAME DAWCompatibilityTests COMMAND DAWCompatibilityTests)
