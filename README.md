# Teensy Firmware - Mystery Melody Machine

This is the Teensy 4.1 firmware for the Mystery Melody Machine project.

## Hardware Requirements

- Teensy 4.1 microcontroller
- WS2812B LED strip (45 LEDs for infinity portal)
- 10 buttons (connected to digital pins with pullup resistors)
- 4 potentiometers (connected to analog pins)
- 4-direction joystick (connected to digital pins)
- 12 switches (connected to digital pins)
- **OLED Display** (optional): 128x64 I2C SSD1306 OLED for real-time feedback
- **Raspberry Pi** (optional): For advanced portal control via serial protocol

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
│   ├── main.cpp                      # Main firmware code & entry point
│   ├── input_scanner.cpp             # Raw hardware I/O layer
│   ├── debouncer.cpp                 # Time-based debouncing
│   ├── analog_smoother.cpp           # EMA filtering for pots
│   ├── robust_input_processor.cpp    # Unified input processing
│   ├── robust_midi_mapper.cpp        # Input to MIDI mapping
│   ├── midi_out.cpp                  # MIDI transmission
│   ├── portal_controller.cpp         # LED animation engine
│   ├── portal_cue_handler.cpp        # Pi/MIDI portal control
│   └── oled_display.cpp              # OLED display controller
├── include/
│   ├── pins.h                        # Pin definitions
│   ├── config.h                      # Configuration constants
│   ├── input_scanner.h               # Raw I/O interface
│   ├── debouncer.h                   # Debouncing interface
│   ├── analog_smoother.h             # Smoothing interface
│   ├── robust_input_processor.h      # Robust input API
│   ├── robust_midi_mapper.h          # MIDI mapping interface
│   ├── midi_out.h                    # MIDI output interface
│   ├── portal_controller.h           # Animation engine interface
│   ├── portal_cue_handler.h          # Portal control interface
│   ├── serial_portal_protocol.h      # Binary protocol definitions
│   └── oled_display.h                # Display interface
├── test/
│   ├── test_debouncer.cpp            # Debouncing tests
│   ├── test_binary_switches.cpp      # Switch encoding tests
│   ├── test_oled_display.cpp         # Display tests
│   ├── test_serial_protocol.cpp      # Protocol tests
│   └── test_pi_serial.py             # Pi integration tests
├── docs/
│   ├── ARCHITECTURE.md               # Complete architecture guide
│   ├── TeensySoftwareRoadmap.md      # Development roadmap
│   ├── OLEDDisplay.md                # OLED setup guide
│   ├── BinarySwitchFeature.md        # Binary switch encoding
│   └── RaspberryPi_Integration_Guide.md
├── platformio.ini                    # PlatformIO configuration
└── README.md                         # This file
```

## Implementation Status

### Phase 1: Basic MIDI Controller (COMPLETE)
- [x] Button to MIDI Note mapping
- [x] Potentiometer to MIDI CC mapping
- [x] Joystick to MIDI CC mapping
- [x] Switch to MIDI CC mapping
- [x] Basic LED strip integration

### Phase 2: Robust Input Processing (COMPLETE)
- [x] Time-based debouncing (5ms configurable)
- [x] Analog smoothing with EMA filtering (α≈0.25)
- [x] Deadband filtering (±2) and rate limiting (15ms)
- [x] Change compression (4ms stable time)
- [x] Joystick rearm timing (120ms anti-rapid-fire)
- [x] Activity tracking and idle detection (30s timeout)
- [x] Comprehensive test mode and diagnostics

### Phase 3: Portal Animation System (COMPLETE)
- [x] 10 animation programs (Spiral, Pulse, Wave, Rainbow, Plasma, etc.)
- [x] 60 FPS animation engine with dual-rate architecture
- [x] Interactive effects (flash on button press, ripple effects)
- [x] BPM synchronization, intensity, hue controls
- [x] Auto-idle mode with program rotation
- [x] Raspberry Pi serial protocol integration
- [x] Legacy MIDI CC support for portal control

### Phase 4: OLED Display (COMPLETE)
- [x] 4 display modes (MIDI Log, Status, Activity, Info)
- [x] Real-time MIDI message logging
- [x] Input state visualization
- [x] Performance monitoring
- [x] Mode switching with buttons

### Current Features:
- **Musical Interface**: 10 buttons (Notes), 4 pots (CCs), 4-way joystick, 12 switches
- **Robust Processing**: Debouncing, smoothing, rate limiting, change compression
- **LED Portal**: 45-LED infinity portal with 10 animation programs @ 60 FPS
- **Dual Control**: Raspberry Pi serial protocol + legacy MIDI CC for portal
- **Display Feedback**: Optional OLED with 4 modes, 20 Hz update rate
- **Performance**: 1kHz input scan, <1ms main loop, all static memory allocation
- **Idle Detection**: Auto-switch to low-power mode after 30s inactivity

## Testing the Firmware

1. **Upload the firmware** to your Teensy 4.1:
   ```bash
   # For production (MIDI mode) - full functionality with MIDI output
   pio run -e teensy41 --target upload
   
   # For debugging (Serial mode) - includes test mode with value dumps
   pio run -e teensy41-debug --target upload
   ```

2. **Monitor debug output** with test mode:
   ```bash
   pio device monitor -e teensy41-debug
   ```

3. **Test MIDI functionality**:
   - **Buttons**: Press to send MIDI Note On/Off (Notes 60-69)
   - **Potentiometers**: Turn to send smoothed MIDI CC (CC 1-4)
   - **Joystick**: Direction presses send single CC pulses (CC 10-13)
   - **Switches**: Toggle to send CC 0/127 (CC 20-31, binary on CC 50)

4. **Test Portal animations**:
   - LEDs should run startup sequence on boot
   - Press buttons to trigger white flash effects
   - Move pots to trigger ripple effects
   - Goes to IDLE mode after 30s of no activity
   - Auto-rotates programs while idle (every 60s)

5. **Test OLED display** (if connected):
   - Press **Button 0** to cycle through display modes
   - Press **Button 1** to go back to previous mode
   - Verify MIDI messages appear in MIDI LOG mode
   - Check input states in STATUS mode

6. **Test Raspberry Pi integration** (optional):
   - Connect Pi via USB serial (115200 baud)
   - Send binary protocol commands to control portal
   - See [docs/RaspberryPi_Integration_Guide.md](docs/RaspberryPi_Integration_Guide.md)

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

## Expected Serial Output (Debug Mode)

```
=== Mystery Melody Machine Teensy Firmware ===
Firmware compiled: Nov 11 2025 14:30:45
USB Type: Serial (Debug Mode)

Initializing OLED display...
OLED Display: Initialized successfully
OLED: Set to MIDI_LOG mode

Initializing robust input processor...
RobustInputProcessor: Initialized with debouncing and smoothing
  Button debounce: 5ms
  Pot deadband: 2, rate limit: 15ms
  Joystick rearm: 120ms

Initializing MIDI output...
Input mapping: 10 buttons, 4 pots, 12 switches, 4-way joystick
Features: debouncing, analog smoothing, change compression

FastLED initialized: 45 LEDs on pin 1
Portal Controller: Initialized with 10 programs
Portal Cue Handler: Ready for serial and MIDI control

Starting portal initialization sequence...
  Program demo: SPIRAL → PULSE → RAINBOW → WAVE → PLASMA
  Flash sequence...
  Fade to AMBIENT mode
Portal startup sequence complete

Test mode enabled - will dump input values every 5 seconds
=== Setup Complete ===
Main loop target: 1000 Hz (1000μs)
Portal target: 60 Hz (16667μs)
OLED update: 20 Hz (50ms)
Entering main loop...

MIDI: Button 0 pressed -> Note 60 ON      # Debounced button press
MIDI: Button 0 released -> Note 60 OFF    # Debounced button release
MIDI: Pot 0 changed -> CC 1 = 64          # Smoothed and rate-limited pot
MIDI: Joystick UP -> CC 10 = 127          # Single pulse with rearm
MIDI: Switch 0 ON -> CC 20 = 127          # Debounced switch change
Portal: Program changed to PULSE          # Portal control

=== INPUT STATE DUMP ===                   # Every 5 seconds
Buttons: 0:OFF 1:OFF 2:OFF 3:OFF 4:OFF 5:OFF 6:OFF 7:OFF 8:OFF 9:OFF 
Switches: 0:OFF 1:OFF 2:OFF 3:OFF 4:OFF 5:OFF 6:OFF 7:OFF 8:OFF 9:OFF 10:OFF 11:OFF
Pots: 0:MIDI_0 1:MIDI_0 2:MIDI_0 3:MIDI_0
Activity: 1240ms ago, Idle: NO
Portal: AMBIENT (BPM:120 Bright:160 Idle:NO)
========================

Heartbeat - ACTIVE (last activity 1240ms ago)    # Every second
```

## Pin Assignments

### Digital Inputs (with pullups):
- **Buttons**: Pins 2-11 (10 buttons)
- **Joystick**: Pins 33-36 (Up=33, Down=34, Left=35, Right=36)
- **Switches**: Pins 22-29, 16-18, 21 (12 switches total)

### Analog Inputs:
- **Potentiometers**: Pins A0-A3 (4 pots)
  - *Note: A6/A7 left unconnected to avoid noise*

### I2C Interface (for OLED):
- **SDA**: Pin 18 (I2C data)
- **SCL**: Pin 19 (I2C clock)

### Outputs:
- **LED Data**: Pin 1 (WS2812B data line for 45 LEDs)
- **Built-in LED**: Pin 13 (heartbeat indicator)

## MIDI Mapping

### Input Controls (MIDI Channel 1)

#### Buttons (Digital Input, Active Low):
- **Button 0-9 (Pins 2-11)**: MIDI Notes 60-69 (C4-A4), Velocity 100

#### Potentiometers (Analog Input, Smoothed):
- **Pot 0-3 (Pins A0-A3)**: MIDI CC 1-4, Value 0-127

#### Joystick (Digital Input, Pulse Mode):
- **Up (Pin 33)**: MIDI CC 10 = 127 (single pulse, 120ms rearm)
- **Down (Pin 34)**: MIDI CC 11 = 127 (single pulse, 120ms rearm)
- **Left (Pin 35)**: MIDI CC 12 = 127 (single pulse, 120ms rearm)
- **Right (Pin 36)**: MIDI CC 13 = 127 (single pulse, 120ms rearm)

#### Switches (Digital Input, State Change):
- **Switch 0-11**: MIDI CC 20-31, Value 0 (off) or 127 (on)
- **Binary Mode (Switches 0-7)**: MIDI CC 50 = 0-255 (8-bit binary representation)

### Portal Control (MIDI Channel 1, Legacy)

For backward compatibility with MIDI-only setups (prefer Raspberry Pi serial protocol):

- **CC 60**: Program select (0-9)
  - 0=SPIRAL, 1=PULSE, 2=WAVE, 3=CHAOS, 4=AMBIENT, 5=IDLE, 6=RIPPLE, 7=RAINBOW, 8=PLASMA, 9=BREATHE
- **CC 61**: BPM (0-127 maps to 60-180 BPM)
- **CC 62**: Intensity (0-127 maps to 0.0-1.0)
- **CC 63**: Base Hue (0-127 maps to 0.0-1.0)
- **CC 64**: Brightness (0-127 maps to 0-160)
- **CC 65**: Flash trigger (127 = trigger white flash)
- **CC 66**: Ripple position (0-127 maps to LED 0-44)

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

## Raspberry Pi Serial Protocol

For advanced portal control, the firmware supports a binary serial protocol at 115200 baud.

### Message Format (5 bytes):
```
[0xAA][COMMAND][VALUE][CHECKSUM][0x55]

Where:
- 0xAA = Start marker
- COMMAND = Command ID (0x01-0x23)
- VALUE = 8-bit parameter (0-255)
- CHECKSUM = COMMAND XOR VALUE
- 0x55 = End marker
```

### Commands (Pi → Teensy):
- **0x01**: Set Program (value = 0-9)
- **0x02**: Set BPM (value maps to 60-180 BPM)
- **0x03**: Set Intensity (value maps to 0.0-1.0)
- **0x04**: Set Hue (value maps to 0.0-1.0)
- **0x05**: Set Brightness (value = 0-255)
- **0x06**: Trigger Flash
- **0x07**: Trigger Ripple (value = LED position 0-44)
- **0x10**: Ping (keepalive)
- **0x11**: Reset to defaults

### Responses (Teensy → Pi):
- **0x20**: PONG (response to PING)
- **0x21**: ACK (command successful)
- **0x22**: NAK (command failed)
- **0x23**: STATUS (status report)

See [docs/RaspberryPi_Integration_Guide.md](docs/RaspberryPi_Integration_Guide.md) for Python examples.

## Next Steps

- **Phase 5**: Performance optimization and live performance hardening
- **Phase 6**: Advanced diagnostics and safety features
- **Future**: EEPROM configuration storage, calibration system

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
2. Check LED strip power supply (WS2812B needs 5V, adequate current)
3. Ensure LED_COUNT matches your strip (45 LEDs default)
4. Check if portal startup sequence runs (should see 5 programs demo + flash)

### Portal animations frozen:
1. Check serial output for "Portal frame time" warnings
2. Verify FastLED library version (3.6.0+)
3. Reduce LED_BRIGHTNESS_MAX if power supply is insufficient

### Raspberry Pi serial not responding:
1. Verify baud rate is 115200
2. Check USB cable connection
3. Use debug mode to see incoming serial data
4. Verify checksum calculation in Pi code

### OLED showing wrong data:
1. Press Button 0 to cycle display modes
2. Check I2C connections (SDA=18, SCL=19)
3. Verify OLED I2C address (0x3C default)
4. Update rate is 20 Hz, expect slight delay
