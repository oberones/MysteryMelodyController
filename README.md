# Teensy Firmware - Mystery Melody Machine

This is the Teensy 4.1 firmware for the Mystery Melody Machine project.

## Hardware Requirements

- Teensy 4.1 microcontroller
- WS2812B LED strip (60 LEDs recommended for infinity portal)
- 10 buttons (connected to digital pins with pullup resistors)
- 6 potentiometers (connected to analog pins)
- 4-direction joystick (connected to digital pins)
- 3 switches (connected to digital pins)
- **OLED Display** (optional): 128x64 I2C SSD1306 OLED for real-time feedback

## Software Requirements

- [PlatformIO](https://platformio.org/) IDE or CLI
- [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) (automatically handled by PlatformIO)

## USB Configuration Instructions

**IMPORTANT**: For this firmware to work properly, you need to configure the Teensy for MIDI mode:

### Using Arduino IDE with Teensyduino:
1. Install [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
2. In Arduino IDE, go to **Tools** menu
3. Set **Board** to "Teensy 4.1"
4. Set **USB Type** to "MIDI"
5. This enables MIDI functionality (Serial is available for debugging when connected to PC)

### Using PlatformIO:
The `platformio.ini` file is already configured with:
```ini
build_flags = -D USB_MIDI
```
This automatically sets the USB type to MIDI mode.

## Building and Uploading

### Using PlatformIO CLI:
```bash
# Build the project
pio run

# Upload to Teensy
pio run --target upload

# Open serial monitor for debugging
pio device monitor
```

### Using PlatformIO IDE:
1. Open this folder in PlatformIO IDE
2. Click the build button (✓)
3. Click the upload button (→)
4. Use the serial monitor to see debug output

## Project Structure

```
teensy-firmware/
├── src/
│   └── main.cpp              # Main firmware code
├── include/
│   ├── pins.h                # Pin definitions
│   └── config.h              # Configuration constants
├── platformio.ini            # PlatformIO configuration
├── docs/
│   └── TeensySoftwareRoadmap.md
└── README.md                 # This file
```

## Phase 2 Implementation Status (COMPLETE)

- [x] Implement time-based debouncing with configurable parameters
- [x] Add analog smoothing with EMA filtering and deadband
- [x] Implement change compression and rate limiting
- [x] Create RobustInputProcessor for integrated robust input handling
- [x] Build comprehensive test mode with serial diagnostics
- [x] Add activity tracking and idle detection
- [x] Implement joystick rearm timing to prevent rapid-fire

### Current Features (Phase 2):
- **Robust Input Processing**: Time-based debouncing eliminates switch bounce
- **Analog Smoothing**: EMA filtering (α≈0.25) + deadband (±2) + rate limiting (15ms)
- **Change Compression**: Send CC only after stable for 4ms OR large threshold change
- **Activity Tracking**: Comprehensive idle detection (30s timeout) across all inputs
- **Joystick Rearm**: 120ms minimum between pulses prevents rapid-fire CC spam
- **Test Mode**: Serial value dump every 5 seconds when DEBUG enabled
- **Performance**: Fixed-point math maintains <1ms main loop timing
- **Memory**: All static allocation, no dynamic memory in main loop

## Testing Phase 2

1. **Upload the firmware** to your Teensy 4.1:
   ```bash
   # For production (MIDI mode) - robust input processing with MIDI output
   pio run -e teensy41 --target upload
   
   # For debugging (Serial mode) - includes test mode with value dumps
   pio run -e teensy41-debug --target upload
   ```

2. **Monitor debug output** with enhanced test mode:
   ```bash
   pio device monitor -e teensy41-debug
   ```

3. **Test robust input behavior**:
   - **Debounced Buttons**: No duplicate note events from switch bounce
   - **Smoothed Potentiometers**: Stable CC values, reduced noise, rate-limited
   - **Rearm Joystick**: Single pulse per direction, 120ms minimum between
   - **Activity Detection**: Monitor idle state in debug output
   - **Test Value Dump**: See all input states every 5 seconds in debug mode

4. **Verify improvements over Phase 1**:
   - Button presses now debounced (5ms minimum stable time)
   - Pot CC messages rate-limited to max 67/second per pot (vs unlimited in Phase 1)
   - Joystick prevents rapid-fire CC spam
   - Idle detection works correctly after 30s of no activity

## Development Workflow

### For Production/MIDI Testing:
```bash
pio run --target upload          # Upload MIDI version
# Use Arduino IDE Serial Monitor or DAW to verify MIDI
```

### For Debugging/Development:
```bash
pio run -e teensy41-debug --target upload    # Upload debug version
pio device monitor -e teensy41-debug         # Monitor serial output
```

### Switch Between Modes:
- **Production**: Full MIDI functionality, no serial monitoring via PlatformIO
- **Debug**: Serial monitoring, no MIDI functionality
- Use Arduino IDE Serial Monitor for MIDI mode debugging if needed

## Expected Serial Output (Debug Mode - Phase 2)

```
=== Mystery Melody Machine Teensy Firmware ===
Phase 2: Robust Input Layer + MIDI
Firmware compiled: Aug 25 2025 14:30:45
USB Type: Serial (Debug Mode)
Initializing robust input processor...
RobustInputProcessor: Initialized with debouncing and smoothing
  Button debounce: 5ms
  Pot deadband: 2, rate limit: 15ms
  Joystick rearm: 120ms
Initializing MIDI output...
Input mapping: 10 buttons, 6 pots, 3 switches, 4-way joystick
Features: debouncing, analog smoothing, change compression
FastLED initialized: 60 LEDs on pin 1
Testing MIDI enumeration...
MIDI not available - debug mode active
Starting portal initialization sequence...
Portal startup sequence complete
Test mode enabled - will dump input values every 5 seconds
=== Setup Complete ===
Main loop target: 1000 Hz
Portal target: 60 Hz
Phase 2: Debouncing, smoothing, and rate limiting active
Entering main loop...

MIDI: Button 0 pressed -> Note 60 ON      # Debounced button press
MIDI: Button 0 released -> Note 60 OFF    # Debounced button release
MIDI: Pot 0 changed -> CC 1 = 64          # Smoothed and rate-limited pot
MIDI: Joystick UP -> CC 10 = 127          # Single pulse with rearm
MIDI: Switch 0 ON -> CC 20 = 127          # Debounced switch change

=== INPUT STATE DUMP ===                   # Every 5 seconds
Buttons: 0:OFF 1:OFF 2:OFF 3:OFF 4:OFF 5:OFF 6:OFF 7:OFF 8:OFF 9:OFF 
Switches: 0:OFF 1:OFF 2:OFF 
Pots: 0:MIDI_0 1:MIDI_0 2:MIDI_0 3:MIDI_0 4:MIDI_0 5:MIDI_0 
Activity: 1240ms ago, Idle: NO
========================

Heartbeat - ACTIVE (last activity 1240ms ago)    # Every second
```

## Pin Assignments

### Digital Inputs (with pullups):
- **Buttons**: Pins 2-11 (10 buttons)
- **Joystick**: Pins 12, 20, 14, 19 (Up/Down/Left/Right) - Reassigned to avoid conflicts
- **Switches**: Pins 16-18 (3 switches)

### Analog Inputs:
- **Potentiometers**: Pins A0-A3 (4 pots) - A4/A5 reserved for I2C, A6/A7 disabled to avoid noise

### I2C Interface:
- **SDA**: Pin 18 (A4) - For OLED display
- **SCL**: Pin 19 (A5) - For OLED display

### Outputs:
- **LED Data**: Pin 1 (WS2812B data)
- **Built-in LED**: Pin 13 (heartbeat)

## MIDI Mapping (Phase 1 Complete)

### Buttons (Digital Input, Active Low):
- **Button 0-9 (Pins 2-11)**: MIDI Notes 60-69 (C4-A4) on Channel 1, Velocity 100

### Potentiometers (Analog Input):
- **Pot 0-3 (Pins A0-A3)**: MIDI CC 1-4 on Channel 1, Value 0-127

### Joystick (Digital Input, Edge Triggered):
- **Up (Pin 12)**: MIDI CC 10 = 127 on press
- **Down (Pin 20)**: MIDI CC 11 = 127 on press  
- **Left (Pin 14)**: MIDI CC 12 = 127 on press
- **Right (Pin 19)**: MIDI CC 13 = 127 on press

### Switches (Digital Input, State Change):
- **Switch 0-2 (Pins 16-18)**: MIDI CC 20-22 on Channel 1, Value 0 (off) or 127 (on)

## OLED Display (Optional)

The firmware includes support for a 128x64 I2C OLED display that provides real-time feedback. See [OLED Display Documentation](docs/OLEDDisplay.md) for detailed setup and usage instructions.

### Quick Setup:
1. Connect SSD1306 OLED to I2C pins (SDA=18, SCL=19)
2. Use **Button 0** to cycle through display modes:
   - **MIDI LOG**: Shows recent MIDI messages with note names
   - **STATUS**: Shows current input states
   - **ACTIVITY**: Visual activity indicators  
   - **INFO**: System performance and uptime

The display enhances debugging and live performance monitoring without affecting real-time MIDI performance.

## Next Phases

- **Phase 3**: Integrate complete portal animation system with pre-existing infinity portal code
- **Phase 4**: Performance optimization and hardening for live performance use
- **Phase 5**: Diagnostics & safety features

## Troubleshooting

### No serial output (production/MIDI mode):
1. This is normal - MIDI mode doesn't provide serial monitoring via PlatformIO
2. Use debug mode for serial monitoring: `pio run -e teensy41-debug --target upload`
3. Or use Arduino IDE Serial Monitor with production MIDI mode

### No MIDI device appears (debug mode):
1. This is normal - debug mode has no MIDI functionality
2. Use production mode for MIDI: `pio run -e teensy41 --target upload`
3. Verify device appears as "Teensy MIDI" in your DAW

### No MIDI device appears (production mode):
1. Verify USB Type is set to "MIDI"
2. Try different USB cable
3. Check if Teensy Loader shows the device

### LEDs not working:
1. Verify LED_DATA_PIN connection (Pin 1)
2. Check LED strip power supply
3. Ensure LED_COUNT matches your strip length
