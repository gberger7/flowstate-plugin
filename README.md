# Flowstate

A professional hybrid delay/reverb spatial designer plugin for macOS, 
built with JUCE (C++) and deployed as VST3 and AU formats compatible 
with Logic Pro, Ableton Live, Reaper, and any standard DAW.

## Overview

Flowstate blurs the line between delay and reverb into one unified 
spatial engine. Rather than chaining two separate effects, Flowstate 
layers them into a shared feedback ecosystem where the two effects 
blend, interact, and evolve together.

The centerpiece is the **Blend knob** — sweeping it from left to right 
morphs the character of the wet signal from pure delay (distinct echoes 
and rhythmic repeats) through a hybrid zone where delay and reverb blur 
into one cohesive texture, all the way to pure reverb (atmospheric space 
and diffuse tail). A separate **Mix knob** controls wet/dry balance 
independently so the effect never drowns out the source.

## Features

- **Hybrid Delay/Reverb Engine** — delay and reverb share a feedback 
  ecosystem rather than running in series
- **Blend Control** — morphs seamlessly between pure delay, hybrid 
  spatial texture, and pure reverb
- **Shimmer** — pitches each feedback pass by a set interval (default 
  +12 semitones) creating an infinitely rising ethereal cloud of 
  harmonics in the feedback loop
- **Freeze** — captures the current wet tail and loops it infinitely 
  with a crossfade, letting you play over a suspended wash of sound
- **Reverse** — reverses the reverb tail, delay echoes, or both with 
  a 3-way selector producing a pre-swell before the dry transient
- **Modulation** — LFO applied simultaneously to delay time and reverb 
  diffusion creating liquid chorus-like animation
- **Diffusion** — smears delay taps from clean distinct echoes into 
  washy reverb-like density via a cascade of allpass filters
- **Drive** — subtle to moderate saturation in the feedback loop for 
  analog warmth and tape-like grit
- **Duck** — wet signal attenuates when dry signal is present and 
  blooms in the gaps between notes keeping the mix clean
- **Stereo Width** — M/S encoding on the wet signal only from mono 
  to hyper-wide

## Technical Stack

- **Audio Engine:** C++ with JUCE 7 framework
- **Plugin Formats:** VST3 and AU
- **UI:** Custom HTML/CSS/JavaScript interface rendered via JUCE 
  WebBrowserComponent
- **Parameter Bridge:** Bidirectional JS↔C++ communication via JUCE 
  WebView bridge
- **Build System:** CMake
- **Testing:** Catch2 property-based tests (14/14 passing)

## Development Approach

Flowstate was built using a spec-driven AI-assisted development 
workflow. A comprehensive technical specification covering DSP 
signal flow, parameter architecture, UI requirements, and acceptance 
criteria was authored upfront and used to guide implementation through 
Kiro and Cursor AI development tools.

This approach required deep engagement with:
- Real-time audio DSP concepts (FDN reverb, allpass diffusion, 
  shimmer pitch shifting, reverse buffers)
- Thread safety in real-time audio processing environments
- WebView bridge architecture for plugin UI communication
- DAW integration and AU/VST3 plugin validation

## UI

The plugin interface is built entirely in HTML/CSS/JavaScript and 
rendered via JUCE's WebBrowserComponent — no native JUCE UI components 
are used. Features include:

- Animated particle system background
- Dynamic SVG arc visualization on the Blend knob with teal/lavender/
  amber gradient that responds to knob position
- Center detent snap on the Blend knob at 50%
- Transient value display on the Output knob
- Custom Rubik Glitch / Rajdhani typography

<img width="901" height="622" alt="Screenshot 2026-04-20 at 1 14 11 PM" src="https://github.com/user-attachments/assets/e03f903f-be69-4ecb-a160-56e422f75c82" />

## Live UI Demo

https://flowstate-plugin.vercel.app

## Status

Currently in active development. Core DSP engine complete with 14/14 
property tests passing. UI complete and interactive. DAW integration 
testing in progress.
