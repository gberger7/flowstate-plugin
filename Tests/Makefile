# Makefile for Flowstate Plugin Property Tests
# Alternative to CMake for quick testing

# Compiler
CXX = clang++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -I. -I..

# JUCE paths (adjust these to your JUCE installation)
JUCE_PATH = ~/JUCE
JUCE_MODULES = $(JUCE_PATH)/modules

# JUCE includes
JUCE_INCLUDES = -I$(JUCE_MODULES)

# JUCE defines
JUCE_DEFINES = -DJUCE_STANDALONE_APPLICATION=1 \
               -DJUCE_USE_CURL=0 \
               -DJUCE_WEB_BROWSER=0

# macOS frameworks (comment out on Linux/Windows)
FRAMEWORKS = -framework Accelerate \
             -framework AudioToolbox \
             -framework CoreAudio \
             -framework CoreMIDI \
             -framework IOKit \
             -framework Cocoa

# Source files for DelayEngine tests
DELAY_SOURCES = DelayEnginePropertyTests.cpp \
                ../Source/DSP/DelayEngine.cpp

# Source files for ReverbEngine tests
REVERB_SOURCES = ReverbEnginePropertyTests.cpp \
                 ../Source/DSP/ReverbEngine.cpp

# Source files for FeedbackProcessor tests
FEEDBACK_SOURCES = FeedbackProcessorPropertyTests.cpp \
                   ../Source/DSP/FeedbackProcessor.cpp \
                   ../Source/DSP/ShimmerProcessor.cpp

# Source files for ShimmerProcessor tests
SHIMMER_SOURCES = ShimmerProcessorPropertyTests.cpp \
                  ../Source/DSP/ShimmerProcessor.cpp

# Source files for ModulationEngine tests
MODULATION_SOURCES = ModulationEnginePropertyTests.cpp \
                     ../Source/DSP/ModulationEngine.cpp

# Source files for SignalFlow tests (PluginProcessor integration)
SIGNALFLOW_SOURCES = SignalFlowPropertyTests.cpp \
                     ../Source/PluginProcessor.cpp \
                     ../Source/DSP/DelayEngine.cpp \
                     ../Source/DSP/ReverbEngine.cpp \
                     ../Source/DSP/FeedbackProcessor.cpp \
                     ../Source/DSP/DuckingProcessor.cpp \
                     ../Source/DSP/ShimmerProcessor.cpp \
                     ../Source/DSP/ModulationEngine.cpp \
                     ../Source/DSP/StereoWidthProcessor.cpp \
                     ../Source/DSP/ReverseBuffer.cpp

# Source files for Freeze tests (PluginProcessor integration)
FREEZE_SOURCES = FreezePropertyTests.cpp \
                 ../Source/PluginProcessor.cpp \
                 ../Source/DSP/DelayEngine.cpp \
                 ../Source/DSP/ReverbEngine.cpp \
                 ../Source/DSP/FeedbackProcessor.cpp \
                 ../Source/DSP/DuckingProcessor.cpp \
                 ../Source/DSP/ShimmerProcessor.cpp \
                 ../Source/DSP/ModulationEngine.cpp \
                 ../Source/DSP/StereoWidthProcessor.cpp \
                 ../Source/DSP/ReverseBuffer.cpp

# Source files for StateManagement tests (PluginProcessor integration)
STATEMANAGEMENT_SOURCES = StateManagementPropertyTests.cpp \
                          ../Source/PluginProcessor.cpp \
                          ../Source/DSP/DelayEngine.cpp \
                          ../Source/DSP/ReverbEngine.cpp \
                          ../Source/DSP/FeedbackProcessor.cpp \
                          ../Source/DSP/DuckingProcessor.cpp \
                          ../Source/DSP/ShimmerProcessor.cpp \
                          ../Source/DSP/ModulationEngine.cpp \
                          ../Source/DSP/StereoWidthProcessor.cpp \
                          ../Source/DSP/ReverseBuffer.cpp

# Source files for AudioQuality tests (PluginProcessor integration)
AUDIOQUALITY_SOURCES = AudioQualityPropertyTests.cpp \
                       ../Source/PluginProcessor.cpp \
                       ../Source/DSP/DelayEngine.cpp \
                       ../Source/DSP/ReverbEngine.cpp \
                       ../Source/DSP/FeedbackProcessor.cpp \
                       ../Source/DSP/DuckingProcessor.cpp \
                       ../Source/DSP/ShimmerProcessor.cpp \
                       ../Source/DSP/ModulationEngine.cpp \
                       ../Source/DSP/StereoWidthProcessor.cpp \
                       ../Source/DSP/ReverseBuffer.cpp

# Output executables
DELAY_TARGET = DelayEnginePropertyTests
REVERB_TARGET = ReverbEnginePropertyTests
FEEDBACK_TARGET = FeedbackProcessorPropertyTests
SHIMMER_TARGET = ShimmerProcessorPropertyTests
MODULATION_TARGET = ModulationEnginePropertyTests
SIGNALFLOW_TARGET = SignalFlowPropertyTests
FREEZE_TARGET = FreezePropertyTests
STATEMANAGEMENT_TARGET = StateManagementPropertyTests
AUDIOQUALITY_TARGET = AudioQualityPropertyTests

# Build rules
all: $(DELAY_TARGET) $(REVERB_TARGET) $(FEEDBACK_TARGET) $(SHIMMER_TARGET) $(MODULATION_TARGET) $(SIGNALFLOW_TARGET) $(FREEZE_TARGET) $(STATEMANAGEMENT_TARGET) $(AUDIOQUALITY_TARGET)

$(DELAY_TARGET): $(DELAY_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(DELAY_SOURCES) -o $(DELAY_TARGET) $(FRAMEWORKS)

$(REVERB_TARGET): $(REVERB_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(REVERB_SOURCES) -o $(REVERB_TARGET) $(FRAMEWORKS)

$(FEEDBACK_TARGET): $(FEEDBACK_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(FEEDBACK_SOURCES) -o $(FEEDBACK_TARGET) $(FRAMEWORKS)

$(SHIMMER_TARGET): $(SHIMMER_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(SHIMMER_SOURCES) -o $(SHIMMER_TARGET) $(FRAMEWORKS)

$(MODULATION_TARGET): $(MODULATION_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(MODULATION_SOURCES) -o $(MODULATION_TARGET) $(FRAMEWORKS)

$(SIGNALFLOW_TARGET): $(SIGNALFLOW_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(SIGNALFLOW_SOURCES) -o $(SIGNALFLOW_TARGET) $(FRAMEWORKS)

$(FREEZE_TARGET): $(FREEZE_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(FREEZE_SOURCES) -o $(FREEZE_TARGET) $(FRAMEWORKS)

$(STATEMANAGEMENT_TARGET): $(STATEMANAGEMENT_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(STATEMANAGEMENT_SOURCES) -o $(STATEMANAGEMENT_TARGET) $(FRAMEWORKS)

$(AUDIOQUALITY_TARGET): $(AUDIOQUALITY_SOURCES)
	$(CXX) $(CXXFLAGS) $(JUCE_INCLUDES) $(JUCE_DEFINES) $(AUDIOQUALITY_SOURCES) -o $(AUDIOQUALITY_TARGET) $(FRAMEWORKS)

run: $(DELAY_TARGET) $(REVERB_TARGET) $(FEEDBACK_TARGET) $(SHIMMER_TARGET) $(MODULATION_TARGET) $(SIGNALFLOW_TARGET) $(FREEZE_TARGET) $(STATEMANAGEMENT_TARGET) $(AUDIOQUALITY_TARGET)
	./$(DELAY_TARGET)
	./$(REVERB_TARGET)
	./$(FEEDBACK_TARGET)
	./$(SHIMMER_TARGET)
	./$(MODULATION_TARGET)
	./$(SIGNALFLOW_TARGET)
	./$(FREEZE_TARGET)
	./$(STATEMANAGEMENT_TARGET)
	./$(AUDIOQUALITY_TARGET)

run-delay: $(DELAY_TARGET)
	./$(DELAY_TARGET)

run-reverb: $(REVERB_TARGET)
	./$(REVERB_TARGET)

run-feedback: $(FEEDBACK_TARGET)
	./$(FEEDBACK_TARGET)

run-shimmer: $(SHIMMER_TARGET)
	./$(SHIMMER_TARGET)

run-modulation: $(MODULATION_TARGET)
	./$(MODULATION_TARGET)

run-signalflow: $(SIGNALFLOW_TARGET)
	./$(SIGNALFLOW_TARGET)

run-freeze: $(FREEZE_TARGET)
	./$(FREEZE_TARGET)

run-statemanagement: $(STATEMANAGEMENT_TARGET)
	./$(STATEMANAGEMENT_TARGET)

run-audioquality: $(AUDIOQUALITY_TARGET)
	./$(AUDIOQUALITY_TARGET)

clean:
	rm -f $(DELAY_TARGET) $(REVERB_TARGET) $(FEEDBACK_TARGET) $(SHIMMER_TARGET) $(MODULATION_TARGET) $(SIGNALFLOW_TARGET) $(FREEZE_TARGET) $(STATEMANAGEMENT_TARGET) $(AUDIOQUALITY_TARGET)

.PHONY: all run run-delay run-reverb run-feedback run-shimmer run-modulation run-signalflow run-freeze run-statemanagement run-audioquality clean

# Usage:
# make                      - Build all tests
# make run                  - Build and run all tests
# make run-delay            - Build and run DelayEngine tests only
# make run-reverb           - Build and run ReverbEngine tests only
# make run-feedback         - Build and run FeedbackProcessor tests only
# make run-shimmer          - Build and run ShimmerProcessor tests only
# make run-modulation       - Build and run ModulationEngine tests only
# make run-signalflow       - Build and run SignalFlow tests only
# make run-freeze           - Build and run Freeze tests only
# make run-statemanagement  - Build and run StateManagement tests only
# make run-audioquality     - Build and run AudioQuality tests only
# make clean                - Remove built files
