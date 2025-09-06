# OLED Display Support

The Mystery Melody Machine firmware now includes support for a 128x64 I2C OLED display that provides real-time feedback and multiple display modes.

## Hardware Setup

### OLED Display Requirements
- **Display Type**: SSD1306 128x64 OLED
- **Interface**: I2C
- **I2C Address**: 0x3C (default)
- **Voltage**: 3.3V or 5V compatible

### I2C Connections
Connect your OLED display to the Teensy 4.1 I2C pins:
- **SDA**: Pin 18 (A4)
- **SCL**: Pin 19 (A5)
- **VCC**: 3.3V or 5V
- **GND**: Ground

## Display Modes

The OLED display supports 4 different modes that can be cycled through using buttons:

### 1. MIDI LOG Mode (Default)
- Shows a log of the most recent MIDI messages sent
- Displays note names (C4, D#5, etc.) with velocity and channel
- Shows control change messages with CC numbers and values
- Each message includes a timestamp showing seconds ago
- **Purpose**: Monitor all MIDI activity in real-time

### 2. STATUS Mode
- Shows current state of all inputs:
  - **Buttons**: 10 digital button states (0/1)
  - **Pots**: 4 potentiometer values (0-127)
  - **Switches**: 12 switch states (0/1)
  - **Joystick**: Direction indicators (U/D/L/R)
- **Purpose**: Debug input hardware and see current values

### 3. ACTIVITY Mode
- Visual representation of input activity
- **Button bars**: Height shows pressed state, outline shows recent activity
- **Pot bars**: Height shows current value, outline shows recent changes
- **Purpose**: Visual feedback for performance and activity monitoring

### 4. INFO Mode
- System information display:
  - **Loop time**: Current main loop timing in microseconds
  - **State**: ACTIVE or IDLE based on recent input activity
  - **Uptime**: System uptime in minutes and seconds
  - **Mode info**: Current display mode and USB type
- **Purpose**: Performance monitoring and system status

## Controls

### Mode Switching
- **Button 0**: Switch to next display mode
- **Button 1**: Switch to previous display mode
- Mode indicator shows current mode (1/4, 2/4, etc.) for 2 seconds after switching

### Automatic Updates
- Display updates at 20Hz (every 50ms) to avoid interference with main loop
- MIDI log updates immediately when MIDI messages are sent
- Input status updates with each main loop cycle (1kHz)

## Features

### MIDI Logging
- **Real-time logging**: All MIDI messages are logged as they are sent
- **Message types**: Note On, Note Off, Control Change
- **Note names**: Displays musical note names (C4, F#3, etc.) instead of just numbers
- **Timestamps**: Shows how many seconds ago each message was sent
- **Ring buffer**: Keeps the 8 most recent messages

### Performance Optimized
- **Non-blocking**: Display updates don't interfere with real-time audio performance
- **Efficient rendering**: Only redraws when content changes
- **Memory conscious**: Uses static buffers, no dynamic allocation

### Hardware Integration
- **Automatic detection**: Gracefully handles missing OLED display
- **Error handling**: Continues operation if I2C initialization fails
- **Resource sharing**: Shares I2C bus with other potential devices

## Configuration

The OLED display can be configured in `include/config.h`:

```cpp
// OLED Display Configuration
constexpr uint8_t OLED_UPDATE_HZ = 20;        // Display refresh rate
constexpr uint8_t OLED_DEFAULT_MODE = 0;      // Default mode (MIDI_LOG)
constexpr uint8_t OLED_MIDI_LOG_SIZE = 8;     // MIDI message history size
```

Pin definitions are in `include/pins.h`:

```cpp
// I2C and OLED configuration
constexpr uint8_t I2C_SDA_PIN = 18;           // Pin 18 (A4)
constexpr uint8_t I2C_SCL_PIN = 19;           // Pin 19 (A5)  
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;    // I2C address
```

## Troubleshooting

### Display Not Working
1. **Check connections**: Verify SDA, SCL, VCC, and GND connections
2. **Check I2C address**: Use an I2C scanner to verify the display address is 0x3C
3. **Check power**: Ensure display is receiving proper voltage (3.3V or 5V)
4. **Serial output**: Check serial monitor for OLED initialization messages

### Common Issues
- **Blank display**: Usually a wiring or power issue
- **Garbled display**: Check I2C pullup resistors (usually built into display modules)
- **Slow updates**: Normal - display updates at 20Hz to maintain real-time performance

### Debug Information
When DEBUG level is 1 or higher, initialization messages will appear on the serial console:
```
OLED: Initialized 128x64 display at 0x3C
```

If initialization fails:
```
Warning: OLED initialization failed - continuing without display
```

The firmware will continue to operate normally without the display if initialization fails.

## Integration with Existing Code

The OLED display is fully integrated with the existing MIDI output system:
- All MIDI messages sent through `MidiOut` are automatically logged
- Display modes can be controlled without affecting MIDI functionality
- Zero performance impact on real-time MIDI processing

The display is designed to complement, not replace, the existing serial debugging output.
