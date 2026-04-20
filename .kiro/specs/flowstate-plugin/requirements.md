# Requirements Document

## Introduction

Flowstate is a hybrid spatial audio effect plugin that unifies delay and reverb into a single interconnected engine. Unlike traditional serial effect chains, Flowstate's delay and reverb engines share a feedback ecosystem, allowing seamless morphing between pure delay, pure reverb, or any hybrid character in between. The plugin targets professional audio production environments (VST3/AU hosts) and features a fully custom web-based UI rendered through JUCE's WebViewComponent. The core innovation is the Blend control that crossfades between delay and reverb wet signals while maintaining independent wet/dry mixing, combined with creative tools like Reverse, Freeze, Shimmer, and intelligent ducking.

## Glossary

- **Flowstate_Plugin**: The complete audio effect plugin system including DSP engine, parameter management, and UI
- **Delay_Engine**: The delay processing subsystem with time, feedback, and diffusion controls
- **Reverb_Engine**: The reverb processing subsystem using Feedback Delay Network (FDN) or Schroeder algorithm
- **Blend_Control**: The central morphing parameter that crossfades between delay and reverb wet signals
- **Mix_Control**: The wet/dry balance control independent of the Blend parameter
- **WebView_UI**: The HTML/CSS/JavaScript interface rendered via JUCE WebViewComponent
- **Parameter_Bridge**: The bidirectional communication system between JavaScript UI and C++ audio engine
- **Feedback_Path**: The signal routing that feeds processed audio back into the effect engine
- **Duck_Processor**: The envelope follower that attenuates wet signal when dry signal is present
- **Shimmer_Processor**: The pitch-shifting effect applied in the feedback loop
- **Reverse_Buffer**: The 3-second rolling buffer used for reverse playback
- **Freeze_Engine**: The system that captures and loops the current wet tail indefinitely
- **DAW**: Digital Audio Workstation (host application like Ableton Live, Logic Pro, Reaper)
- **VST3**: Virtual Studio Technology 3 plugin format
- **AU**: Audio Unit plugin format for macOS
- **JUCE**: The C++ framework used for audio plugin development
- **FDN**: Feedback Delay Network reverb algorithm
- **M/S_Encoding**: Mid-Side stereo encoding technique for width control
- **BPM**: Beats Per Minute tempo measurement
- **LFO**: Low Frequency Oscillator for modulation effects

## Requirements

### Requirement 1: Delay Engine Time Control

**User Story:** As a music producer, I want precise control over delay timing with both free and tempo-synced modes, so that I can create rhythmic patterns that align with my track's tempo or explore non-musical delay times.

#### Acceptance Criteria

1. THE Delay_Engine SHALL accept delay time values from 1ms to 2000ms
2. WHEN the user enables BPM sync mode, THE Delay_Engine SHALL calculate delay time based on host tempo and selected division
3. WHERE BPM sync is enabled, THE Delay_Engine SHALL support divisions of 1/32, 1/16, 1/8, 1/4, 1/2, and 1/1 with straight, dotted, and triplet variants
4. WHEN the host tempo changes, THE Delay_Engine SHALL update the delay time within 10ms to maintain sync
5. WHEN the user switches from synced to free mode, THE Delay_Engine SHALL preserve the current delay time in milliseconds

### Requirement 2: Delay Engine Feedback and Diffusion

**User Story:** As a sound designer, I want to control delay feedback intensity and texture, so that I can create everything from clean discrete echoes to dense reverb-like textures.

#### Acceptance Criteria

1. THE Delay_Engine SHALL accept feedback values from 0% to 100%
2. WHEN feedback exceeds 90%, THE Delay_Engine SHALL approach self-oscillation without exceeding unity gain
3. THE Delay_Engine SHALL apply a hard limiter in the feedback path to prevent runaway signal growth
4. THE Delay_Engine SHALL accept diffusion values from 0% to 100%
5. WHEN diffusion is 0%, THE Delay_Engine SHALL produce clean distinct delay taps
6. WHEN diffusion is 100%, THE Delay_Engine SHALL produce dense reverb-like texture using cascaded allpass filters
7. THE Delay_Engine SHALL apply diffusion processing within the feedback path

### Requirement 3: Reverb Engine Spatial Control

**User Story:** As a mixing engineer, I want to control reverb size, decay time, and frequency damping, so that I can simulate different acoustic spaces from small rooms to large halls.

#### Acceptance Criteria

1. THE Reverb_Engine SHALL accept size parameter values from 0% to 100% controlling early reflection density
2. THE Reverb_Engine SHALL accept decay time values from 0.1 seconds to 20 seconds
3. THE Reverb_Engine SHALL accept damping values from 0% to 100% controlling high-frequency rolloff
4. WHEN damping is 0%, THE Reverb_Engine SHALL preserve high frequencies in the decay tail
5. WHEN damping is 100%, THE Reverb_Engine SHALL apply maximum high-frequency rolloff producing dark reverb character
6. THE Reverb_Engine SHALL implement either Feedback Delay Network (FDN) or Schroeder reverb algorithm

### Requirement 4: Blend and Mix Core Controls

**User Story:** As a music producer, I want independent control over the delay/reverb character blend and the overall wet/dry balance, so that I can morph between effect types without losing my dry signal balance.

#### Acceptance Criteria

1. THE Blend_Control SHALL accept values from 0% to 100%
2. WHEN Blend_Control is 0%, THE Flowstate_Plugin SHALL output 100% delay wet signal and 0% reverb wet signal
3. WHEN Blend_Control is 50%, THE Flowstate_Plugin SHALL output equal proportions of delay and reverb wet signals
4. WHEN Blend_Control is 100%, THE Flowstate_Plugin SHALL output 0% delay wet signal and 100% reverb wet signal
5. THE Mix_Control SHALL accept values from 0% to 100% independent of Blend_Control
6. WHEN Mix_Control is 0%, THE Flowstate_Plugin SHALL output only the dry signal with no wet signal
7. WHEN Mix_Control is 100%, THE Flowstate_Plugin SHALL output only the wet signal with no dry signal
8. THE Mix_Control SHALL default to 50%

### Requirement 5: Modulation System

**User Story:** As a sound designer, I want to add movement and animation to the spatial effects, so that I can create chorus-like textures and evolving soundscapes.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL provide an LFO with rate control from 0.01Hz to 5Hz
2. THE Flowstate_Plugin SHALL provide modulation depth control from 0% to 100%
3. WHEN modulation depth is 0%, THE Flowstate_Plugin SHALL apply no LFO modulation
4. WHEN modulation depth is greater than 0%, THE Flowstate_Plugin SHALL apply LFO modulation simultaneously to delay time and reverb diffusion
5. THE Flowstate_Plugin SHALL use a sine wave LFO waveform

### Requirement 6: Character Processing - Drive and Tone

**User Story:** As a mixing engineer, I want to add saturation and frequency shaping to the feedback path, so that I can create warm analog-style repeats and prevent harsh buildup.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL provide drive control from 0% to 100%
2. WHEN drive is 0%, THE Feedback_Path SHALL apply no saturation
3. WHEN drive is 100%, THE Feedback_Path SHALL apply tape-like saturation
4. THE Flowstate_Plugin SHALL apply drive processing to both Delay_Engine and Reverb_Engine feedback paths
5. THE Flowstate_Plugin SHALL provide tone control from 0% to 100%
6. WHEN tone is 0%, THE Feedback_Path SHALL preserve full frequency spectrum
7. WHEN tone is 100%, THE Feedback_Path SHALL apply maximum high-frequency cut
8. THE Flowstate_Plugin SHALL apply tone filtering within the feedback path

### Requirement 7: Ducking System

**User Story:** As a music producer, I want the wet signal to automatically reduce when my dry signal is present and bloom in the gaps, so that I can maintain clarity while adding spatial depth.

#### Acceptance Criteria

1. THE Duck_Processor SHALL monitor the dry signal amplitude using an envelope follower
2. THE Duck_Processor SHALL accept sensitivity control from 0% to 100%
3. WHEN duck sensitivity is 0%, THE Duck_Processor SHALL apply no attenuation to the wet signal
4. WHEN duck sensitivity is greater than 0% and dry signal exceeds threshold, THE Duck_Processor SHALL attenuate the wet signal
5. WHEN the dry signal falls below threshold, THE Duck_Processor SHALL restore wet signal level according to release time
6. THE Duck_Processor SHALL apply attenuation only to the wet signal path

### Requirement 8: Shimmer Effect

**User Story:** As a sound designer, I want to add pitched harmonics to the feedback loop, so that I can create ethereal ascending or descending textures.

#### Acceptance Criteria

1. THE Shimmer_Processor SHALL provide an on/off toggle control
2. WHEN shimmer is disabled, THE Feedback_Path SHALL apply no pitch shifting
3. WHEN shimmer is enabled, THE Shimmer_Processor SHALL pitch-shift each feedback pass by the selected interval
4. THE Shimmer_Processor SHALL accept pitch interval values from -24 semitones to +24 semitones
5. THE Shimmer_Processor SHALL default to +12 semitones (one octave up)
6. THE Shimmer_Processor SHALL implement pitch shifting using granular or overlap-add algorithm within the feedback loop

### Requirement 9: Reverse Effect

**User Story:** As a sound designer, I want to reverse the delay, reverb, or both effects in real time, so that I can create pre-swell effects and backwards textures.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL provide a 3-way reverse mode selector with options: OFF, REVERB, DELAY, BOTH
2. THE Reverse_Buffer SHALL maintain a rolling 3-second audio buffer
3. WHEN reverse mode is REVERB, THE Flowstate_Plugin SHALL read the Reverb_Engine buffer backwards
4. WHEN reverse mode is DELAY, THE Flowstate_Plugin SHALL read the Delay_Engine buffer backwards
5. WHEN reverse mode is BOTH, THE Flowstate_Plugin SHALL read both engine buffers backwards
6. WHEN reverse is active, THE Flowstate_Plugin SHALL produce pre-swell effects before dry transients where buffer content permits

### Requirement 10: Freeze Function

**User Story:** As a music producer, I want to capture and hold the current reverb/delay tail indefinitely, so that I can create sustained pad textures while continuing to play over them.

#### Acceptance Criteria

1. THE Freeze_Engine SHALL provide a latch toggle control
2. WHEN freeze is activated, THE Freeze_Engine SHALL capture the current wet buffer content
3. WHILE freeze is active, THE Freeze_Engine SHALL loop the captured content indefinitely with crossfading to prevent clicks
4. WHILE freeze is active, THE Flowstate_Plugin SHALL continue passing the dry signal unaffected
5. THE Freeze_Engine SHALL operate on the current wet signal regardless of Blend_Control position
6. WHEN freeze is deactivated, THE Flowstate_Plugin SHALL resume normal wet signal processing

### Requirement 11: Global Output Controls

**User Story:** As a mixing engineer, I want to control the final output level and stereo width, so that I can match the plugin output to my mix and create narrow or wide spatial images.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL provide output gain control from negative infinity dB to +6dB
2. THE Flowstate_Plugin SHALL provide stereo width control from 0% to 150%
3. WHEN stereo width is 0%, THE Flowstate_Plugin SHALL output mono wet signal
4. WHEN stereo width is 100%, THE Flowstate_Plugin SHALL output full stereo wet signal
5. WHEN stereo width is 150%, THE Flowstate_Plugin SHALL output hyper-wide wet signal
6. THE Flowstate_Plugin SHALL apply stereo width processing using M/S_Encoding to wet signal only
7. THE Flowstate_Plugin SHALL preserve dry signal stereo image unaffected by width control

### Requirement 12: Plugin Format and Host Compatibility

**User Story:** As a music producer, I want to use Flowstate in my DAW on macOS or Windows, so that I can integrate it into my existing production workflow.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL compile to VST3 format
2. THE Flowstate_Plugin SHALL compile to AU format
3. THE Flowstate_Plugin SHALL load without errors in Ableton Live
4. THE Flowstate_Plugin SHALL load without errors in Logic Pro
5. THE Flowstate_Plugin SHALL load without errors in Reaper
6. THE Flowstate_Plugin SHALL load without errors in FL Studio
7. THE Flowstate_Plugin SHALL load without errors in any VST3-compatible host
8. THE Flowstate_Plugin SHALL load without errors in any AU-compatible host on macOS

### Requirement 13: Parameter Automation and State Management

**User Story:** As a music producer, I want all plugin parameters to respond to DAW automation and save with my project, so that I can create dynamic effect changes and recall my settings.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL expose all 21 parameters to the DAW for automation
2. WHEN a parameter receives automation data, THE Flowstate_Plugin SHALL update the parameter value with sample-accurate timing
3. THE Flowstate_Plugin SHALL apply parameter changes without producing zipper noise or audio artifacts
4. WHEN the user saves a DAW project, THE Flowstate_Plugin SHALL serialize all parameter states
5. WHEN the user loads a DAW project, THE Flowstate_Plugin SHALL restore all parameter states exactly
6. THE Flowstate_Plugin SHALL use JUCE ValueTree or XML serialization for state management

### Requirement 14: Audio Processing Quality

**User Story:** As a mixing engineer, I want clean, artifact-free audio processing at all parameter settings, so that I can use the plugin on professional productions.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL process stereo input to stereo output throughout the signal path
2. THE Delay_Engine SHALL support minimum 4 seconds of delay buffer memory
3. WHEN the plugin loads, THE Flowstate_Plugin SHALL produce no audio artifacts or clicks
4. WHEN a preset changes, THE Flowstate_Plugin SHALL produce no audio artifacts or clicks
5. WHEN parameters are automated, THE Flowstate_Plugin SHALL produce no audio artifacts or clicks
6. THE Flowstate_Plugin SHALL prevent denormal numbers from degrading CPU performance
7. THE Flowstate_Plugin SHALL produce no audio dropouts at any parameter combination

### Requirement 15: WebView UI Architecture

**User Story:** As a plugin developer, I want the entire UI built in HTML/CSS/JavaScript and rendered through JUCE's WebViewComponent, so that I can leverage web technologies for rapid UI development and avoid JUCE native components.

#### Acceptance Criteria

1. THE WebView_UI SHALL consist of a single HTML file located at WebUI/index.html
2. THE WebView_UI SHALL embed all CSS and JavaScript within the HTML file or load from local WebUI folder
3. THE Flowstate_Plugin PluginEditor SHALL contain only a JUCE WebBrowserComponent
4. THE Flowstate_Plugin PluginEditor SHALL contain zero JUCE native UI components (no sliders, buttons, or labels)
5. THE WebView_UI SHALL render all knobs as custom SVG or canvas elements
6. THE WebView_UI SHALL implement knob value changes via vertical mouse drag
7. THE WebView_UI SHALL load all fonts from local WebUI/fonts/ directory
8. THE Flowstate_Plugin SHALL make no network requests at runtime

### Requirement 16: Parameter Bridge - JavaScript to C++

**User Story:** As a plugin developer, I want UI interactions in JavaScript to update audio parameters in C++, so that user knob movements affect the audio processing.

#### Acceptance Criteria

1. THE Parameter_Bridge SHALL register a native JavaScript callback accessible as window.__juce__.postMessage()
2. WHEN the user interacts with a WebView_UI control, THE WebView_UI SHALL call window.__juce__.postMessage() with parameter ID and value
3. WHEN the Parameter_Bridge receives a postMessage call, THE Flowstate_Plugin SHALL update the corresponding audio parameter
4. THE Parameter_Bridge SHALL update audio parameters within 5ms of receiving the JavaScript message
5. THE Parameter_Bridge SHALL support all 21 parameters defined in the parameters table

### Requirement 17: Parameter Bridge - C++ to JavaScript

**User Story:** As a music producer, I want DAW automation to visually update the plugin UI knobs in real time, so that I can see which parameters are being automated.

#### Acceptance Criteria

1. WHEN a parameter value changes via DAW automation, THE Flowstate_Plugin SHALL detect the change
2. WHEN a parameter change is detected, THE Parameter_Bridge SHALL call evaluateJavascript() with the parameter ID and new value
3. THE WebView_UI SHALL provide a window.flowstate.updateParam() function to receive parameter updates
4. WHEN window.flowstate.updateParam() is called, THE WebView_UI SHALL update the corresponding knob visual position
5. THE WebView_UI SHALL update knob positions within 16ms (one frame at 60fps) of receiving the update
6. THE Parameter_Bridge SHALL synchronize all 21 parameters bidirectionally

### Requirement 18: UI Visual Design and Layout

**User Story:** As a music producer, I want a visually appealing and intuitive interface with clear section organization, so that I can quickly understand and control the plugin.

#### Acceptance Criteria

1. THE WebView_UI SHALL set the plugin window size to 900 pixels wide by 500 pixels tall
2. THE WebView_UI SHALL use background color #0E0E1A (deep navy)
3. THE WebView_UI SHALL use teal/cyan accent color #3ECFCF for the Delay section
4. THE WebView_UI SHALL use amber/gold accent color #E8A838 for the Reverb section
5. THE WebView_UI SHALL render the Blend_Control with a CSS gradient from teal through lavender #C8B8FF to amber
6. THE WebView_UI SHALL load Inter font from local files
7. THE WebView_UI SHALL display no scrollbars or layout overflow
8. THE WebView_UI SHALL organize controls into sections: Delay (left), Blend (center), Reverb (right), Modulation (bottom left), Character (bottom center), Reverse/Freeze (bottom right)

### Requirement 19: UI Interactive Behavior

**User Story:** As a music producer, I want intuitive knob interactions with visual feedback, so that I can efficiently adjust parameters.

#### Acceptance Criteria

1. WHEN the user drags vertically on a knob, THE WebView_UI SHALL increase value on upward drag and decrease value on downward drag
2. WHEN the user double-clicks a knob, THE WebView_UI SHALL display a text input for exact value entry
3. WHEN the user right-clicks a knob, THE WebView_UI SHALL display a context menu with "Reset to default" option
4. WHEN the user hovers over a knob, THE WebView_UI SHALL display a tooltip showing current value and unit
5. WHEN a knob value changes, THE WebView_UI SHALL animate the knob arc using CSS transitions
6. WHEN the user hovers over any interactive element, THE WebView_UI SHALL increase brightness

### Requirement 20: UI Active State Visualization

**User Story:** As a music producer, I want clear visual feedback when special modes are active, so that I can see at a glance which effects are engaged.

#### Acceptance Criteria

1. WHEN freeze is active, THE WebView_UI SHALL display the freeze button with a pulsing blue-white glow animation
2. WHEN freeze is inactive, THE WebView_UI SHALL display the freeze button without glow
3. WHEN reverse mode is set to REVERB, DELAY, or BOTH, THE WebView_UI SHALL highlight the selected mode with its section color
4. WHEN shimmer is enabled, THE WebView_UI SHALL display the shimmer toggle with amber glow and outer halo
5. WHEN BPM sync is enabled, THE WebView_UI SHALL display the current BPM division next to the time knob (e.g., "1/4 · 120bpm")

### Requirement 21: Signal Flow Architecture

**User Story:** As a plugin developer, I want a clear signal flow where delay and reverb share a feedback ecosystem with proper routing for all character effects, so that the hybrid spatial design functions correctly.

#### Acceptance Criteria

1. THE Flowstate_Plugin SHALL split the input signal into dry path and wet path
2. THE Duck_Processor SHALL analyze the dry path to generate envelope control signal
3. THE Delay_Engine SHALL process the wet path with time, feedback, and diffusion
4. THE Flowstate_Plugin SHALL apply drive, tone, shimmer, and modulation within the Delay_Engine feedback path
5. THE Reverb_Engine SHALL process the wet path with size, decay, and damping
6. THE Flowstate_Plugin SHALL apply modulation to the Reverb_Engine diffusion
7. THE Blend_Control SHALL crossfade between Delay_Engine wet output and Reverb_Engine wet output
8. THE Flowstate_Plugin SHALL apply stereo width processing to the blended wet signal
9. THE Mix_Control SHALL crossfade between the processed wet signal and the dry path
10. THE Flowstate_Plugin SHALL apply output gain as the final processing stage
