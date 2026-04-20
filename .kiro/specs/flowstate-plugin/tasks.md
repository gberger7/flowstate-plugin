# Implementation Plan: Flowstate Audio Plugin

## Overview

This implementation plan breaks down the Flowstate hybrid spatial audio plugin into discrete coding tasks. The plugin combines delay and reverb processing with a shared feedback ecosystem, featuring a web-based UI rendered through JUCE's WebBrowserComponent. Implementation follows a bottom-up approach: core DSP components first, then signal routing, parameter management, UI integration, and finally comprehensive testing.

## Tasks

- [x] 1. Set up JUCE project structure and configuration
  - Create FlowstatePlugin.jucer project file with VST3 and AU formats enabled
  - Configure project for macOS and Windows targets
  - Set up Source/ directory structure with DSP/, Bridge/, and WebUI/ folders
  - Add JUCE modules: juce_audio_processors, juce_dsp, juce_gui_extra (for WebBrowserComponent)
  - Configure compiler settings for C++17 and optimization flags
  - Create Parameters.h with all 21 parameter ID definitions
  - _Requirements: 12.1, 12.2, 15.1, 15.2_

- [x] 2. Implement core DSP components
  - [x] 2.1 Implement DelayEngine with circular buffer and tempo sync
    - Create DelayEngine class with 4-second stereo circular buffer
    - Implement setDelayTime() for free mode (1ms-2000ms range)
    - Implement setDelayTimeFromTempo() with BPM division calculation (1/32 to 1/1, straight/dotted/triplet)
    - Implement cascaded allpass filters for diffusion (0-100%)
    - Implement feedback path with hard limiter to prevent runaway gain
    - _Requirements: 1.1, 1.2, 1.3, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 14.2_
  
  - [x] 2.2 Write property tests for DelayEngine
    - **Property 1: Parameter Range Validation** - Test delay time clamping (1-2000ms)
    - **Property 2: BPM Sync Delay Time Calculation** - Validate tempo-synced delay calculations
    - **Property 4: Feedback Limiting** - Verify no output exceeds unity gain after 100 iterations at 90%+ feedback
    - **Validates: Requirements 1.1, 1.2, 2.1, 2.2, 2.3**
  
  - [x] 2.3 Implement ReverbEngine with FDN algorithm
    - Create ReverbEngine class with 8 parallel delay lines
    - Implement Householder feedback matrix for diffusion
    - Implement size parameter controlling delay line lengths (prime numbers)
    - Implement decay time parameter (0.1-20 seconds) controlling feedback gain
    - Implement one-pole lowpass filters for damping (0-100%)
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 3.5, 3.6_
  
  - [x] 2.4 Write property tests for ReverbEngine
    - **Property 1: Parameter Range Validation** - Test size, decay, damping ranges
    - **Validates: Requirements 3.1, 3.2, 3.3**

  - [x] 2.5 Implement FeedbackProcessor for drive and tone
    - Create FeedbackProcessor class with tanh() saturation
    - Implement setDrive() with input gain staging and output compensation
    - Implement setTone() with first-order lowpass filter (20kHz to 800Hz cutoff range)
    - Apply saturation before tone filtering
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 6.8_
  
  - [x] 2.6 Write property tests for FeedbackProcessor
    - **Property 11: Drive Zero Bypass** - Verify no saturation when drive=0%
    - **Property 12: Tone Zero Bypass** - Verify no filtering when tone=0%
    - **Validates: Requirements 6.2, 6.6**

  - [x] 2.7 Implement ShimmerProcessor with pitch shifting
    - Create ShimmerProcessor class using overlap-add algorithm
    - Implement setPitchShift() for -24 to +24 semitones
    - Use 2048-sample grain size with Hann windowing and 50% overlap
    - Calculate pitch ratio as pow(2.0, semitones/12.0)
    - _Requirements: 8.1, 8.2, 8.3, 8.4, 8.5, 8.6_
  
  - [x] 2.8 Write property tests for ShimmerProcessor
    - **Property 15: Shimmer Disabled Bypass** - Verify no pitch shift when disabled
    - **Property 16: Shimmer Pitch Shift Amount** - Validate frequency ratio matches 2^(semitones/12)
    - **Validates: Requirements 8.2, 8.3**

  - [x] 2.9 Implement ModulationEngine with sine LFO
    - Create ModulationEngine class with phase accumulator
    - Implement setRate() for 0.01Hz to 5Hz range
    - Implement setDepth() for 0-100% modulation amount
    - Generate sine wave LFO output (-1.0 to +1.0)
    - _Requirements: 5.1, 5.2, 5.5_
  
  - [x] 2.10 Write property tests for ModulationEngine
    - **Property 8: Modulation Depth Zero Bypass** - Verify no modulation when depth=0%
    - **Property 10: LFO Waveform Shape** - Validate sine wave pattern within 1% tolerance
    - **Validates: Requirements 5.3, 5.5**

  - [x] 2.11 Implement DuckingProcessor with envelope follower
    - Create DuckingProcessor class with peak detection
    - Implement envelope follower with 10ms attack and 200ms release
    - Implement setSensitivity() controlling threshold (-40dB to -10dB range)
    - Calculate attenuation coefficient based on dry signal envelope
    - Apply attenuation only to wet signal
    - _Requirements: 7.1, 7.2, 7.3, 7.4, 7.5, 7.6_
  
  - [x] 2.12 Write property tests for DuckingProcessor
    - **Property 13: Ducking Sensitivity Zero Bypass** - Verify no attenuation when sensitivity=0%
    - **Property 14: Ducking Attenuates Wet Only** - Verify dry signal unchanged
    - **Validates: Requirements 7.3, 7.4, 7.6**

  - [x] 2.13 Implement StereoWidthProcessor with M/S encoding
    - Create StereoWidthProcessor class
    - Implement encodeToMS() converting L/R to Mid/Side
    - Implement setWidth() for 0% to 150% range
    - Scale side signal by width parameter
    - Implement decodeFromMS() converting back to L/R
    - _Requirements: 11.2, 11.3, 11.4, 11.5, 11.6, 11.7_
  
  - [x] 2.14 Write property tests for StereoWidthProcessor
    - **Property 19: Stereo Width Mono Collapse** - Verify L=R when width=0%
    - **Property 20: Width Affects Wet Signal Only** - Verify dry signal unchanged
    - **Validates: Requirements 11.3, 11.6, 11.7**

  - [x] 2.15 Implement ReverseBuffer for reverse effects
    - Create ReverseBuffer class with 3-second circular buffer
    - Implement write() method for continuous buffer filling
    - Implement readReverse() with mode selector (OFF, REVERB, DELAY, BOTH)
    - Handle buffer wraparound smoothly with crossfading
    - _Requirements: 9.1, 9.2, 9.3, 9.4, 9.5, 9.6_

- [x] 3. Implement AudioProcessor and parameter management
  - [x] 3.1 Create FlowstateProcessor class with AudioProcessorValueTreeState
    - Create PluginProcessor.h/cpp with JUCE AudioProcessor inheritance
    - Initialize AudioProcessorValueTreeState with all 21 parameters
    - Define parameter ranges, defaults, and units from Parameters.h
    - Implement prepareToPlay() to initialize all DSP components
    - Implement releaseResources() to clean up buffers
    - _Requirements: 13.1, 13.2, 13.6_
  
  - [x] 3.2 Implement signal flow and routing in processBlock()
    - Split input into dry and wet paths
    - Route wet signal through DelayEngine and ReverbEngine in parallel
    - Apply FeedbackProcessor (drive, tone) to both engine feedback paths
    - Apply ShimmerProcessor when enabled in feedback path
    - Apply ModulationEngine to delay time and reverb diffusion
    - Implement blend crossfade between delay and reverb wet outputs
    - Apply StereoWidthProcessor to blended wet signal
    - Process DuckingProcessor using dry signal envelope
    - Implement mix crossfade between dry and wet signals
    - Apply output gain as final stage
    - _Requirements: 21.1, 21.2, 21.3, 21.4, 21.5, 21.6, 21.7, 21.8, 21.9, 21.10_
  
  - [x] 3.3 Write property tests for signal flow
    - **Property 5: Blend Crossfade Ratio** - Verify delay:reverb ratio equals (1-blend):blend
    - **Property 6: Mix Independence from Blend** - Verify blend doesn't affect wet/dry ratio
    - **Property 7: Mix Crossfade Ratio** - Verify dry:wet ratio equals (1-mix):mix
    - **Property 9: Modulation Affects Multiple Targets** - Verify LFO modulates both delay and reverb
    - **Property 29: Signal Path Splitting** - Verify dry path remains unprocessed
    - **Property 30: Output Gain Final Stage** - Verify gain applied last to both paths
    - **Validates: Requirements 4.2, 4.3, 4.4, 4.5, 4.6, 4.7, 5.4, 21.1, 21.7, 21.9, 21.10**
  
  - [x] 3.4 Implement Freeze functionality
    - Add freeze buffer to capture current wet signal
    - Implement freeze toggle logic in processBlock()
    - When freeze active, loop captured content with crossfading
    - Continue passing dry signal unaffected during freeze
    - _Requirements: 10.1, 10.2, 10.3, 10.4, 10.5, 10.6_
  
  - [x] 3.5 Write property tests for Freeze
    - **Property 17: Freeze Preserves Dry Signal** - Verify dry signal unchanged when frozen
    - **Property 18: Freeze Independence from Blend** - Verify blend changes don't affect frozen content
    - **Validates: Requirements 10.4, 10.5**
  
  - [x] 3.6 Implement state serialization and deserialization
    - Implement getStateInformation() using ValueTree XML serialization
    - Implement setStateInformation() with error handling for corrupted data
    - Add version attribute for future compatibility
    - Provide default values for missing parameters
    - _Requirements: 13.4, 13.5, 13.6_
  
  - [x] 3.7 Write property tests for state management
    - **Property 22: State Serialization Round Trip** - Verify all 21 parameters survive save/load
    - **Validates: Requirements 13.4, 13.5**
  
  - [x] 3.8 Implement parameter smoothing and denormal prevention
    - Add parameter smoothing to prevent zipper noise (10ms ramp time)
    - Add small DC offset (1e-20) to feedback paths to prevent denormals
    - Validate all parameter values for NaN/Inf before use
    - _Requirements: 13.3, 14.5, 14.6_
  
  - [x] 3.9 Write property tests for audio quality
    - **Property 21: Parameter Smoothing** - Verify no zipper noise with rapid parameter changes
    - **Property 23: Stereo Processing Preservation** - Verify 2-channel in, 2-channel out
    - **Property 24: No Audio Dropouts** - Verify continuous output with random parameter combinations
    - **Validates: Requirements 13.3, 14.1, 14.5, 14.7**

- [x] 4. Checkpoint - Ensure DSP engine tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 5. Implement WebView UI
  - [x] 5.1 Create HTML/CSS structure for plugin interface
    - Create WebUI/index.html with embedded CSS
    - Set window size to 900x500 pixels
    - Apply background color #0E0E1A (deep navy)
    - Create layout sections: Delay (left, teal #3ECFCF), Blend (center, gradient), Reverb (right, amber #E8A838)
    - Create bottom sections: Modulation, Character, Reverse/Freeze
    - Load Inter font from local WebUI/fonts/ directory
    - Ensure no scrollbars or layout overflow
    - _Requirements: 15.1, 15.2, 15.7, 18.1, 18.2, 18.3, 18.4, 18.5, 18.6, 18.7, 18.8_
  
  - [x] 5.2 Implement SVG/Canvas knob controls with interaction
    - Create knob elements for all 21 parameters using SVG or Canvas
    - Implement vertical drag interaction (up=increase, down=decrease)
    - Implement double-click for text input value entry
    - Implement right-click context menu with "Reset to default"
    - Implement hover tooltips showing current value and unit
    - Apply CSS transitions for smooth knob rotation animations
    - Apply hover brightness increase for interactive elements
    - _Requirements: 15.5, 15.6, 19.1, 19.2, 19.3, 19.4, 19.5, 19.6_
  
  - [x] 5.3 Implement JavaScript controller and parameter management
    - Create window.flowstate object with params state
    - Implement initKnobs() to attach event listeners
    - Implement startDrag() for mouse drag handling
    - Implement updateParam() to update knob visual rotation (-135° to +135°)
    - Implement sendToPlugin() to call window.__juce__.postMessage()
    - Format messages as JSON: {id: "paramID", value: 0.0-1.0}
    - _Requirements: 16.1, 16.2, 16.5_
  
  - [x] 5.4 Implement active state visualizations
    - Add pulsing blue-white glow animation for freeze button when active
    - Add section color highlighting for reverse mode selection
    - Add amber glow and outer halo for shimmer toggle when enabled
    - Display BPM division text next to time knob when sync enabled (e.g., "1/4 · 120bpm")
    - _Requirements: 20.1, 20.2, 20.3, 20.4, 20.5_

- [x] 6. Implement parameter bridge for bidirectional communication
  - [x] 6.1 Create ParameterBridge class for JS ↔ C++ communication
    - Create Bridge/ParameterBridge.h/cpp
    - Store references to FlowstateProcessor and WebBrowserComponent
    - Implement registerJavaScriptCallbacks() to set up window.__juce__.postMessage()
    - Implement handleMessageFromJS() to parse JSON and update APVTS parameters
    - Add error handling for invalid messages, missing IDs, and unknown parameters
    - _Requirements: 16.1, 16.2, 16.3, 16.4, 16.5_
  
  - [ ]* 6.2 Write property tests for JS to C++ bridge
    - **Property 26: Parameter Bridge JS to C++** - Verify parameter updates within one buffer callback
    - **Validates: Requirements 16.3**
  
  - [x] 6.3 Implement C++ to JavaScript parameter updates
    - Implement updateUIParameter() to call webView.evaluateJavascript()
    - Format JavaScript call as: window.flowstate.updateParam('paramID', value)
    - Add APVTS parameter listeners to detect automation changes
    - Ensure UI updates within 16ms (60fps) of parameter change
    - Add try-catch blocks and timeout handling for JS execution
    - _Requirements: 17.1, 17.2, 17.3, 17.4, 17.5, 17.6_
  
  - [ ]* 6.4 Write property tests for C++ to JS bridge
    - **Property 27: Parameter Bridge C++ to JS** - Verify JS function called on automation
    - **Property 28: Bidirectional Parameter Sync** - Verify consistency in both directions
    - **Validates: Requirements 17.2, 17.6**

- [x] 7. Create PluginEditor with WebBrowserComponent
  - [x] 7.1 Implement PluginEditor class
    - Create PluginEditor.h/cpp inheriting from AudioProcessorEditor
    - Add WebBrowserComponent as only child component
    - Set editor size to 900x500 pixels
    - Load WebUI/index.html in WebBrowserComponent
    - Initialize ParameterBridge in constructor
    - Ensure no JUCE native UI components (no sliders, buttons, labels)
    - _Requirements: 15.3, 15.4, 18.1_
  
  - [ ]* 7.2 Write unit tests for WebView initialization
    - Test WebBrowserComponent loads index.html successfully
    - Test no network requests during runtime
    - Test fonts load from local directory
    - **Property 25: Offline Operation** - Verify zero network activity
    - **Validates: Requirements 15.7, 15.8**

- [x] 8. Checkpoint - Ensure UI and bridge tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 9. Integration and final wiring
  - [x] 9.1 Wire all components together in FlowstateProcessor
    - Connect parameter listeners to all DSP components
    - Ensure host tempo information flows to DelayEngine
    - Verify all 21 parameters update their respective components
    - Add parameter value clamping and validation
    - _Requirements: 1.4, 13.1, 13.2_
  
  - [x] 9.2 Implement host tempo tracking for BPM sync
    - Read host tempo from AudioPlayHead in processBlock()
    - Update DelayEngine when tempo changes (within 10ms)
    - Handle missing tempo information gracefully (default to 120 BPM)
    - _Requirements: 1.4_
  
  - [x]* 9.3 Write integration tests for full signal path
    - Test input → split → process → blend → mix → output flow
    - Test all parameter combinations produce valid output
    - Test mode switching (sync/free, reverse modes) produces no artifacts
    - Test freeze activation/deactivation produces no clicks
    - **Validates: Requirements 14.3, 14.4, 14.5**

- [x] 10. DAW compatibility and format testing
  - [x]* 10.1 Test VST3 format loading
    - Verify plugin loads in Reaper (macOS/Windows)
    - Verify plugin loads in FL Studio (Windows)
    - Verify plugin loads in any VST3-compatible host
    - **Validates: Requirements 12.1, 12.7**
  
  - [x]* 10.2 Test AU format loading
    - Verify plugin loads in Logic Pro (macOS)
    - Verify plugin loads in any AU-compatible host on macOS
    - **Validates: Requirements 12.2, 12.8**
  
  - [x]* 10.3 Test automation and state management in DAWs
    - Test parameter automation recording and playback in Ableton Live
    - Test parameter automation in Logic Pro
    - Test parameter automation in Reaper
    - Test project save/load preserves all parameter states
    - Test preset save/load functionality
    - **Validates: Requirements 12.3, 12.4, 12.5, 13.1, 13.2, 13.4, 13.5**

- [x] 11. Final checkpoint - Comprehensive testing
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Property tests validate universal correctness properties across random inputs
- Unit tests validate specific examples, edge cases, and integration points
- Checkpoints ensure incremental validation at major milestones
- The plugin uses JUCE 7.x framework with C++17
- WebView UI uses HTML5/CSS3/JavaScript with no external dependencies
- All 30 correctness properties from the design document are covered in property tests
- DSP components are built bottom-up before integration
- Parameter bridge enables bidirectional communication between UI and audio engine
