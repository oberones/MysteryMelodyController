# Changelog

All notable changes to the Mystery Melody Machine Teensy firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2025-08-25

### Added - Phase 2: Robust Input Layer
- Implemented time-based debouncing with `Debouncer` class
  - Configurable debounce time per input (default 5ms)
  - Proper state change detection with edge triggering
  - Immunity to switch bounce and electrical noise
- Added analog smoothing with `AnalogSmoother` class
  - Exponential Moving Average (EMA) filtering (α ≈ 0.25)
  - Deadband filtering (minimum change threshold)
  - Rate limiting to prevent CC spam
  - Change compression (stable for 4ms OR large threshold)
- Created `RobustInputProcessor` that integrates both systems
  - Wraps raw input scanner with robust processing
  - Activity tracking and idle detection (30s timeout)
  - Joystick rearm timing (120ms minimum between pulses)
- Implemented `RobustMidiMapper` for clean MIDI output
  - Works with debounced and filtered inputs
  - Proper state tracking for all input types
  - All notes off panic function
- Added comprehensive test mode
  - Serial value dump every 5 seconds when DEBUG enabled
  - Input state display with activity tracking
  - Idle mode detection and reporting

### Enhanced Features
- **Button Processing**: Time-based debouncing eliminates false triggers
- **Potentiometer Processing**: EMA smoothing + deadband + rate limiting
  - Fixed-point arithmetic avoids floating point in hot path
  - Large changes override rate limiting for responsiveness
  - Proper 10-bit → 7-bit MIDI mapping with bounds checking
- **Joystick Processing**: Single pulse per direction with rearm timing
  - Prevents rapid-fire CC messages from sticky joysticks
  - Configurable rearm time (default 120ms)
- **Switch Processing**: Debounced state changes with proper ON/OFF detection
- **Activity Tracking**: Comprehensive idle detection across all input types

### Technical Improvements
- **Memory Management**: All static allocation, no dynamic memory in main loop
- **Performance**: Fixed-point math maintains <1ms main loop timing
- **Modularity**: Clean separation between raw scanning and robust processing
- **Configurability**: Extensive compile-time configuration options
- **Testing**: Unit tests for debouncer and analog smoother components

### Configuration Updates
- Added Phase 2 specific configuration constants
- Enhanced platformio.ini with test environment
- Separate debug build with increased verbosity
- All timing parameters configurable via build flags

### Test Infrastructure
- Unit tests for `Debouncer` class (bounce immunity, timing, edge detection)
- Unit tests for `AnalogSmoother` class (EMA, deadband, rate limiting, MIDI mapping)
- Test environment in PlatformIO configuration
- Hardware-in-the-loop test mode with serial output

### Performance Metrics
- Main loop timing: <1000μs worst case maintained
- Debounce effectiveness: 5ms minimum stable time
- Pot noise reduction: ±2 deadband + EMA filtering
- Rate limiting: Max 67 CC messages/second per pot (15ms intervals)
- Memory usage: <100KB RAM, <500KB flash (well within Teensy 4.1 limits)

### Next Phase
- Phase 3: Portal animation integration with pre-existing infinity portal code

## [0.1.0] - 2025-01-15

### Added - Phase 1: Raw Input + MIDI
- Implemented complete input scanning system (`InputScanner` class)
- Added MIDI output handler (`MidiOut` class) with conditional compilation
- Created input-to-MIDI mapping system (`InputMidiMapper` class)
- Full hardware input support:
  - 10 buttons → MIDI notes 60-69 (C4-A4) with Note On/Off
  - 6 potentiometers → MIDI CC 1-6 with raw 0-127 mapping
  - 4-direction joystick → MIDI CC 10-13 (edge triggered)
  - 3 switches → MIDI CC 20-22 (state change triggered)
- Dual-environment build system:
  - Production mode: USB_MIDI for actual MIDI device functionality
  - Debug mode: USB_SERIAL for development and testing
- Raw input polling at 1kHz without debouncing (Phase 1 specification)
- MIDI messages sent with immediate flush for low latency

### Technical Details
- All 19 physical inputs fully mapped and functional
- ADC to MIDI conversion: 10-bit (0-1023) → 7-bit (0-127) with proper rounding
- Active-low input configuration with internal pullups
- Naive edge detection for joystick and switches (no debouncing yet)
- Conditional MIDI compilation prevents debug mode conflicts
- Input state persistence for edge detection between scan cycles

### Architecture
- Modular design with separate scanner, MIDI output, and mapping classes
- Clean separation of concerns for future enhancement phases
- Input scanner provides state access methods for external consumers
- MIDI output abstraction supports both real MIDI and debug serial output

### Configuration Updates
- Removed duplicate constant definitions between pins.h and config.h
- Added complete MIDI mapping tables for all input types
- Updated main loop to use Phase 1 input system
- Portal animation maintained from Phase 0 for visual feedback

### Performance
- Flash usage: ~55KB (increased ~1KB from Phase 0 for new input system)
- RAM usage: ~11KB (minimal increase for input state storage)
- Scan rate: 1kHz maintained with full 19-input polling
- Portal rate: 60Hz maintained alongside input scanning

### Next Phase
- Phase 2: Add debouncing, smoothing, and filtering for production-ready input handling

## [0.0.1] - 2025-08-25

### Added - Phase 0: Bootstrap
- Created PlatformIO project structure
- Implemented basic firmware skeleton with main.cpp
- Added pin definitions in `include/pins.h`
- Added configuration constants in `include/config.h`
- Set up USB Dual Serial/MIDI configuration
- Implemented basic LED portal initialization and startup sequence
- Added simple breathing portal animation for testing
- Implemented basic MIDI note on/off for button 0 (test functionality)
- Added built-in LED heartbeat blink at 1 Hz
- Created comprehensive README with setup instructions
- Added serial debug output at 115200 baud

### Technical Details
- Main loop running at 1 kHz target frequency
- Portal animation running at 60 Hz target frequency
- FastLED integration for WS2812B LED strip control
- Placeholder pin assignments for 10 buttons, 6 pots, 4-direction joystick, 3 switches
- MIDI mapping: Button 0 → Note 60 (C4) on Channel 1, Velocity 100
- Memory usage: ~54KB flash, ~11KB RAM (Teensy 4.1 has 8MB flash, 1MB RAM)

### Configuration
- Target: Teensy 4.1
- USB Type: MIDI
- LED Count: 60 (configurable)
- Max LED Brightness: 160/255 (configurable)
- Debug mode: Enabled for Phase 0

### Next Phase
- Phase 1: Implement full input scanning with debouncing for all controls
