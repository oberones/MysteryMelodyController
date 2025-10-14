# Changelog

## 0.4.0 (2025-09-25) - Phase 3: Portal Animation System

### 🎨 Major Features - Portal Animation Integration

- **10 Psychedelic Animation Programs**: Complete portal animation system built from scratch using FastLED
  - `SPIRAL`: Rotating spiral patterns with BPM sync and configurable direction
  - `PULSE`: Rhythmic pulsing from center, synchronized to BPM
  - `WAVE`: Multi-frequency flowing wave patterns responsive to activity level
  - `CHAOS`: Random/chaotic patterns with activity-based mutation rates
  - `AMBIENT`: Slow, peaceful color cycling for background ambiance
  - `IDLE`: Minimal ambient mode with 15% brightness cap (auto-activated after 30s inactivity)
  - `RIPPLE`: Continuous ripple effects from random positions + ambient base
  - `RAINBOW`: Smooth rainbow rotation with BPM-controlled speed
  - `PLASMA`: Multi-sine wave plasma-like flowing colors
  - `BREATHE`: Gentle breathing effect synchronized to BPM

- **MIDI CC Portal Control (CC 60-66)**: Full remote control capability
  - CC 60: Program select (0-9 for 10 programs)
  - CC 61: BPM control (60-180 BPM range, affects pulse, breathe, spiral)
  - CC 62: Intensity level (0.0-1.0, affects animation strength)
  - CC 63: Base hue shift (0.0-1.0, global color adjustment)
  - CC 64: Global brightness (0-160 max, respects LED_BRIGHTNESS_MAX)
  - CC 65: Flash trigger (127 = trigger white flash effect)
  - CC 66: Ripple position (0-44 LEDs, triggers ripple at specific position)

- **Interactive Feedback System**: Real-time response to physical controls
  - Button presses: Trigger flash + unique hue shift per button
  - Pot activity: Hue rotation + positional ripple effects based on pot values
  - Joystick directions: Directional ripples (Up/Down/Left/Right mapped to circle positions)
  - Switch 0: Manual program cycling through all 10 programs

### 🧠 Intelligence Features

- **Automatic Idle Detection**: 30-second timeout switches to IDLE mode with 15% brightness
- **Ambient Auto-Switching**: When idle, cycles between ambient programs every 60 seconds
- **Activity Resume**: Returns to previous program when activity is detected
- **Multiple Ripple Effects**: Up to 3 simultaneous ripples with fade-out timing

### ⚡ Performance Optimizations

- **45-LED Circular Portal**: Optimized specifically for infinity portal layout
- **60 FPS Rendering**: Smooth animations at 16.67ms intervals
- **1kHz Input Processing**: Maintained responsive input handling
- **Integer Math**: All animations use integer arithmetic for performance
- **Memory Efficient**: ~80KB code, ~30KB RAM usage, well within Teensy 4.1 limits

### 🔧 Technical Architecture

- **PortalController Class**: Core animation engine with 10 programs
- **PortalCueHandler Class**: MIDI CC processing and idle state management
- **Modular Design**: Clean separation between animation logic and control interface
- **FastLED Integration**: Direct CRGB array manipulation for maximum performance
- **BPM Synchronization**: Mathematical phase tracking for tempo-sync animations

### 📚 Documentation

- **Comprehensive Roadmap Update**: Phase 3 marked complete with implementation details
- **MIDI CC Mapping**: Fully documented control protocol for Pi integration
- **Animation Descriptions**: Detailed behavior documentation for each program
- **Performance Metrics**: Memory and timing characteristics documented

### 🔧 Configuration Updates

- **LED Count**: Updated to 45 LEDs for actual hardware configuration
- **Program Count**: Expanded to 10 animation programs
- **MIDI CC Range**: Reserved CC 60-66 for portal control
- **Timing Constants**: Optimized frame rates and idle detection

### Next Phase Ready

- Phase 4: Performance hardening and loop time optimization
- Pi integration testing with documented MIDI CC protocol
- Individual program testing and fine-tuning

## 0.3.0 (2025-09-05)

### Major Features

- **OLED Display Support**: Added support for 128x64 I2C SSD1306 OLED display
  - Real-time MIDI message logging with note names and timestamps
  - Multiple display modes: MIDI LOG, STATUS, ACTIVITY, INFO
  - Mode switching via Button 0 (next) and Button 1 (previous)
  - Non-blocking updates at 20Hz to maintain real-time performance
  - Automatic graceful degradation if display is not connected

### Technical Improvements

- **Enhanced MIDI Output**: Integrated OLED logging with existing MIDI system
- **Performance Monitoring**: Real-time loop timing and system status display
- **Memory Efficient**: Static buffers and optimized rendering
- **Comprehensive Documentation**: Added detailed OLED setup and usage guide

### Library Dependencies

- Added Adafruit SSD1306 and Adafruit GFX Library dependencies
- Maintained compatibility with existing FastLED and core functionality

- Switch states: [ON, OFF, ON, OFF, ON, OFF, OFF, OFF] 
- Individual CCs: CC20=127, CC21=0, CC22=127, CC23=0, CC24=127, CC25=0, CC26=0, CC27=0
- Binary CC: CC50=21 (binary: 00010101 = 1+4+16 = 21)

## 0.2.0 (2025-09-05)

### Feat

- add binary switch mode for first 8 switches

### Fix

- update pin assignment to match controller targets

## 0.1.0 (2025-09-05)
