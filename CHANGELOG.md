# Changelog

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
