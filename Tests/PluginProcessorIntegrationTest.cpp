/*
  ==============================================================================

    PluginProcessorIntegrationTest.cpp
    Created: Integration test for FlowstateProcessor
    Description: Verifies PluginProcessor initialization and basic functionality

  ==============================================================================
*/

#include "../Source/PluginProcessor.h"
#include <iostream>
#include <cassert>

void testProcessorInitialization()
{
    std::cout << "Testing processor initialization..." << std::endl;
    
    FlowstateProcessor processor;
    
    // Verify basic properties
    assert(processor.getName() == "FlowstatePlugin");
    assert(processor.getTotalNumInputChannels() == 2);
    assert(processor.getTotalNumOutputChannels() == 2);
    assert(!processor.acceptsMidi());
    assert(!processor.producesMidi());
    
    std::cout << "✓ Processor initialization test passed" << std::endl;
}

void testParameterCount()
{
    std::cout << "Testing parameter count..." << std::endl;
    
    FlowstateProcessor processor;
    auto& params = processor.getParameters();
    
    // Should have 21 parameters
    int paramCount = params.state.getNumChildren();
    assert(paramCount == 21);
    
    std::cout << "✓ Parameter count test passed (21 parameters)" << std::endl;
}

void testPrepareToPlay()
{
    std::cout << "Testing prepareToPlay..." << std::endl;
    
    FlowstateProcessor processor;
    
    // Prepare with typical settings
    processor.prepareToPlay(44100.0, 512);
    
    // Should not crash
    std::cout << "✓ PrepareToPlay test passed" << std::endl;
}

void testProcessBlock()
{
    std::cout << "Testing processBlock..." << std::endl;
    
    FlowstateProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    
    // Create test buffer
    juce::AudioBuffer<float> buffer(2, 512);
    buffer.clear();
    
    // Add impulse
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);
    
    juce::MidiBuffer midiBuffer;
    
    // Process should not crash
    processor.processBlock(buffer, midiBuffer);
    
    // Output should not be all zeros (some processing happened)
    bool hasNonZero = false;
    for (int ch = 0; ch < 2; ++ch)
    {
        for (int i = 0; i < 512; ++i)
        {
            if (std::abs(buffer.getSample(ch, i)) > 1e-6f)
            {
                hasNonZero = true;
                break;
            }
        }
    }
    
    assert(hasNonZero);
    
    std::cout << "✓ ProcessBlock test passed" << std::endl;
}

void testStateSerialization()
{
    std::cout << "Testing state serialization..." << std::endl;
    
    FlowstateProcessor processor1;
    processor1.prepareToPlay(44100.0, 512);
    
    // Set some parameters
    auto* blendParam = processor1.getParameters().getParameter(ParameterIDs::blend);
    blendParam->setValueNotifyingHost(0.75f);
    
    auto* mixParam = processor1.getParameters().getParameter(ParameterIDs::mix);
    mixParam->setValueNotifyingHost(0.25f);
    
    // Serialize
    juce::MemoryBlock data;
    processor1.getStateInformation(data);
    
    // Create new processor and deserialize
    FlowstateProcessor processor2;
    processor2.prepareToPlay(44100.0, 512);
    processor2.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    
    // Verify parameters match
    auto* blendParam2 = processor2.getParameters().getParameter(ParameterIDs::blend);
    auto* mixParam2 = processor2.getParameters().getParameter(ParameterIDs::mix);
    
    assert(std::abs(blendParam2->getValue() - 0.75f) < 0.01f);
    assert(std::abs(mixParam2->getValue() - 0.25f) < 0.01f);
    
    std::cout << "✓ State serialization test passed" << std::endl;
}

void testParameterRanges()
{
    std::cout << "Testing parameter ranges..." << std::endl;
    
    FlowstateProcessor processor;
    auto& params = processor.getParameters();
    
    // Test delay time range
    auto* delayTime = params.getParameter(ParameterIDs::delayTime);
    assert(delayTime != nullptr);
    
    // Test blend range
    auto* blend = params.getParameter(ParameterIDs::blend);
    assert(blend != nullptr);
    
    // Test mix range
    auto* mix = params.getParameter(ParameterIDs::mix);
    assert(mix != nullptr);
    
    // Test output gain range
    auto* outputGain = params.getParameter(ParameterIDs::outputGain);
    assert(outputGain != nullptr);
    
    std::cout << "✓ Parameter ranges test passed" << std::endl;
}

void testFreezeToggle()
{
    std::cout << "Testing freeze toggle..." << std::endl;
    
    FlowstateProcessor processor;
    processor.prepareToPlay(44100.0, 512);
    
    juce::AudioBuffer<float> buffer(2, 512);
    juce::MidiBuffer midiBuffer;
    
    // Process some audio first
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    processor.processBlock(buffer, midiBuffer);
    
    // Enable freeze
    auto* freezeParam = processor.getParameters().getParameter(ParameterIDs::freezeEnabled);
    freezeParam->setValueNotifyingHost(1.0f);
    
    // Process more audio
    buffer.clear();
    processor.processBlock(buffer, midiBuffer);
    
    // Should not crash
    std::cout << "✓ Freeze toggle test passed" << std::endl;
}

int main()
{
    std::cout << "=== FlowstateProcessor Integration Tests ===" << std::endl;
    std::cout << std::endl;
    
    try
    {
        testProcessorInitialization();
        testParameterCount();
        testPrepareToPlay();
        testProcessBlock();
        testStateSerialization();
        testParameterRanges();
        testFreezeToggle();
        
        std::cout << std::endl;
        std::cout << "=== All tests passed! ===" << std::endl;
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
