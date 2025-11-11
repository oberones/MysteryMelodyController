# Mystery Melody Machine - Raspberry Pi Integration Guide

## Overview

This document provides comprehensive guidance for integrating with the Mystery Melody Machine's Teensy 4.1 firmware via the new serial communication protocol. This system replaces the previous MIDI-based communication with a more reliable, faster, and feature-rich binary protocol.

---

## System Architecture

### Communication Flow
```
Raspberry Pi (Sequencer/AI) ──serial──> Teensy 4.1 (Hardware Controller)
                           <──────────
        (Commands & Cues)              (Responses & Status)
                    
┌─────────────────┐                    ┌──────────────────┐
│ Raspberry Pi    │   115200 baud     │ Teensy 4.1       │
│ - Music Engine  │ ◄────────────────► │ - Input Scanning │
│ - AI Control    │   Binary Protocol  │ - MIDI Output    │
│ - Sequencer     │                    │ - LED Control    │
└─────────────────┘                    └──────────────────┘
```

### Key Benefits of Serial Protocol
- **Reliability**: Error detection via checksums, ACK/NAK responses
- **Speed**: Direct communication, no MIDI routing overhead  
- **Bidirectional**: Teensy can send status updates back to Pi
- **Extensible**: Easy to add new commands without MIDI limitations
- **Real-time**: Non-blocking processing at 1kHz on Teensy

---

## Serial Protocol Specification

### Message Format
All messages use a 5-byte binary format:
```
[START_BYTE] [COMMAND] [VALUE] [CHECKSUM] [END_BYTE]
    0xAA       0x01     0x05     0x04       0x55
```

### Protocol Constants
```python
PORTAL_MSG_START_BYTE = 0xAA
PORTAL_MSG_END_BYTE = 0x55
MESSAGE_SIZE = 5
```

### Checksum Calculation
```python
def calculate_checksum(command: int, value: int) -> int:
    """Simple XOR checksum for message validation"""
    return command ^ value
```

---

## Command Reference

### Animation Control Commands

#### SET_PROGRAM (0x01)
Switch between animation programs.
```python
# Available programs (value 0-9):
PROGRAMS = {
    0: "SPIRAL",    # Rotating spiral patterns
    1: "PULSE",     # Rhythmic pulsing from center  
    2: "WAVE",      # Flowing wave patterns
    3: "CHAOS",     # Random chaotic patterns
    4: "AMBIENT",   # Slow peaceful patterns
    5: "IDLE",      # Minimal ambient mode
    6: "RIPPLE",    # Ripple effects
    7: "RAINBOW",   # Smooth rainbow rotation
    8: "PLASMA",    # Plasma-like flowing colors
    9: "BREATHE"    # Gentle breathing effect
}

# Example: Set to SPIRAL program
command = 0x01
value = 0  # SPIRAL
checksum = command ^ value  # 0x01
message = bytes([0xAA, 0x01, 0x00, 0x01, 0x55])
```

#### SET_BPM (0x02)
Control animation tempo synchronized to your sequencer.
```python
# Map your BPM (60-180) to protocol value (0-255)
def bpm_to_protocol_value(bpm: float) -> int:
    """Convert BPM to protocol value"""
    bpm_clamped = max(60, min(180, bpm))  # Clamp to valid range
    return int(((bpm_clamped - 60) / 120) * 255)

# Example: Set 120 BPM
bpm = 120
value = bpm_to_protocol_value(bpm)  # Results in ~127
message = create_message(0x02, value)
```

#### SET_INTENSITY (0x03) 
Control animation energy level based on musical activity.
```python
# Map intensity (0.0-1.0) to protocol value (0-255)  
def intensity_to_protocol_value(intensity: float) -> int:
    """Convert normalized intensity to protocol value"""
    intensity_clamped = max(0.0, min(1.0, intensity))
    return int(intensity_clamped * 255)

# Example: Set 75% intensity during active musical sections
intensity = 0.75
value = intensity_to_protocol_value(intensity)  # 191
message = create_message(0x03, value)
```

#### SET_HUE (0x04)
Shift base color palette for mood/key changes.
```python
# Map hue (0.0-1.0) to protocol value (0-255)
def hue_to_protocol_value(hue: float) -> int:
    """Convert normalized hue to protocol value"""
    hue_normalized = hue % 1.0  # Wrap around
    return int(hue_normalized * 255)

# Example: Set hue based on musical key
# C major = 0.0 (red), D = 0.083 (orange), E = 0.167 (yellow), etc.
key_hue_map = {
    'C': 0.0, 'C#': 0.042, 'D': 0.083, 'D#': 0.125,
    'E': 0.167, 'F': 0.208, 'F#': 0.25, 'G': 0.292,
    'G#': 0.333, 'A': 0.375, 'A#': 0.417, 'B': 0.458
}

current_key = 'G'  # G major
hue = key_hue_map[current_key]  # 0.292
value = hue_to_protocol_value(hue)  # 74
message = create_message(0x04, value)
```

#### SET_BRIGHTNESS (0x05)
Control overall LED brightness (0-255).
```python
# Direct brightness control
def set_brightness(brightness_percent: float) -> bytes:
    """Set brightness as percentage (0.0-1.0)"""
    value = int(max(0, min(255, brightness_percent * 255)))
    return create_message(0x05, value)

# Example: Dim lights during quiet sections
message = set_brightness(0.3)  # 30% brightness
```

### Interactive Effect Commands

#### TRIGGER_FLASH (0x06)
Trigger flash effects for musical accents.
```python
# Trigger on beat emphasis, chord changes, etc.
def trigger_flash():
    return create_message(0x06, 0)  # Value ignored

# Example: Flash on kick drum hits
if kick_drum_detected:
    message = trigger_flash()
    send_message(message)
```

#### TRIGGER_RIPPLE (0x07)
Trigger ripple effects at specific LED positions.
```python
def trigger_ripple(position: float) -> bytes:
    """Trigger ripple effect at normalized position (0.0-1.0)"""
    value = int(max(0, min(255, position * 255)))
    return create_message(0x07, value)

# Example: Ripples following stereo field or frequency content
left_channel_position = 0.25   # 25% from start
right_channel_position = 0.75  # 75% from start

if left_channel_peak:
    send_message(trigger_ripple(left_channel_position))
if right_channel_peak:
    send_message(trigger_ripple(right_channel_position))
```

### System Commands

#### PING (0x10)
Test connectivity and get response time.
```python
def ping() -> bytes:
    return create_message(0x10, 0)

# Use for heartbeat monitoring
def check_connection():
    send_message(ping())
    # Wait for PONG response (0x20)
    response = read_response()
    return response and response['command'] == 0x20
```

#### RESET (0x11)
Reset Teensy to default state.
```python
def reset_portal() -> bytes:
    """Reset portal to default state (ambient mode, 120 BPM, etc.)"""
    return create_message(0x11, 0)

# Use during setup or error recovery
message = reset_portal()
send_message(message)
```

---

## Python Implementation

### Basic Communication Class
```python
import serial
import struct
import time
from enum import IntEnum
from typing import Optional, Dict, Any

class PortalCommand(IntEnum):
    SET_PROGRAM = 0x01
    SET_BPM = 0x02
    SET_INTENSITY = 0x03
    SET_HUE = 0x04
    SET_BRIGHTNESS = 0x05
    TRIGGER_FLASH = 0x06
    TRIGGER_RIPPLE = 0x07
    PING = 0x10
    RESET = 0x11
    # Response commands
    PONG = 0x20
    ACK = 0x21
    NAK = 0x22
    STATUS = 0x23

class TeensyPortalController:
    """High-level interface for controlling Teensy portal animations"""
    
    def __init__(self, serial_port: str = '/dev/ttyACM0', baud_rate: int = 115200):
        self.serial = serial.Serial(serial_port, baud_rate, timeout=0.1)
        self.last_ping_time = 0
        self.connection_ok = False
        
    def create_message(self, command: PortalCommand, value: int = 0) -> bytes:
        """Create a properly formatted message"""
        checksum = command ^ value
        return struct.pack('BBBBB', 0xAA, command, value, checksum, 0x55)
    
    def send_command(self, command: PortalCommand, value: int = 0) -> bool:
        """Send command and wait for ACK"""
        message = self.create_message(command, value)
        self.serial.write(message)
        self.serial.flush()
        
        # Wait for response
        response = self.read_response()
        return response and response['command'] in [PortalCommand.ACK, PortalCommand.PONG]
    
    def read_response(self, timeout: float = 0.1) -> Optional[Dict[str, Any]]:
        """Read and parse response message"""
        start_time = time.time()
        buffer = bytearray()
        
        while time.time() - start_time < timeout:
            if self.serial.in_waiting:
                buffer.extend(self.serial.read(self.serial.in_waiting))
                
                # Look for complete message
                if len(buffer) >= 5:
                    for i in range(len(buffer) - 4):
                        if buffer[i] == 0xAA and buffer[i + 4] == 0x55:
                            # Found message
                            cmd = buffer[i + 1]
                            val = buffer[i + 2] 
                            chk = buffer[i + 3]
                            
                            # Validate checksum
                            if chk == (cmd ^ val):
                                return {
                                    'command': cmd,
                                    'value': val,
                                    'valid': True
                                }
            time.sleep(0.001)
        
        return None
    
    # High-level control methods
    def set_program(self, program: int) -> bool:
        """Set animation program (0-9)"""
        return self.send_command(PortalCommand.SET_PROGRAM, program)
    
    def set_bpm(self, bpm: float) -> bool:
        """Set BPM (60-180)"""
        value = int(((max(60, min(180, bpm)) - 60) / 120) * 255)
        return self.send_command(PortalCommand.SET_BPM, value)
    
    def set_intensity(self, intensity: float) -> bool:
        """Set intensity (0.0-1.0)"""
        value = int(max(0, min(1, intensity)) * 255)
        return self.send_command(PortalCommand.SET_INTENSITY, value)
    
    def set_hue(self, hue: float) -> bool:
        """Set base hue (0.0-1.0)"""
        value = int((hue % 1.0) * 255)
        return self.send_command(PortalCommand.SET_HUE, value)
    
    def set_brightness(self, brightness: float) -> bool:
        """Set brightness (0.0-1.0)"""
        value = int(max(0, min(1, brightness)) * 255)
        return self.send_command(PortalCommand.SET_BRIGHTNESS, value)
    
    def flash(self) -> bool:
        """Trigger flash effect"""
        return self.send_command(PortalCommand.TRIGGER_FLASH)
    
    def ripple(self, position: float) -> bool:
        """Trigger ripple at position (0.0-1.0)"""
        value = int(max(0, min(1, position)) * 255)
        return self.send_command(PortalCommand.TRIGGER_RIPPLE, value)
    
    def ping(self) -> bool:
        """Test connection"""
        return self.send_command(PortalCommand.PING)
    
    def reset(self) -> bool:
        """Reset to default state"""
        return self.send_command(PortalCommand.RESET)
    
    def check_connection(self) -> bool:
        """Check if Teensy is responding"""
        try:
            self.connection_ok = self.ping()
            return self.connection_ok
        except:
            self.connection_ok = False
            return False
```

---

## Integration Patterns

### Music-Responsive Animation
```python
class MusicResponsivePortal:
    def __init__(self, portal_controller: TeensyPortalController):
        self.portal = portal_controller
        self.last_bpm = 120
        self.last_key = 'C'
    
    def update_from_sequencer(self, musical_state: dict):
        """Update portal based on current musical state"""
        
        # Sync BPM changes
        if musical_state['bpm'] != self.last_bpm:
            self.portal.set_bpm(musical_state['bpm'])
            self.last_bpm = musical_state['bpm']
        
        # Change hue on key changes  
        if musical_state['key'] != self.last_key:
            key_hue_map = {
                'C': 0.0, 'G': 0.292, 'D': 0.083, 'A': 0.375,
                'E': 0.167, 'B': 0.458, 'F#': 0.25, 'C#': 0.042,
                'F': 0.208, 'Bb': 0.417, 'Eb': 0.125, 'Ab': 0.333
            }
            hue = key_hue_map.get(musical_state['key'], 0.0)
            self.portal.set_hue(hue)
            self.last_key = musical_state['key']
        
        # Set intensity based on musical activity
        activity = musical_state.get('activity_level', 0.5)
        self.portal.set_intensity(activity)
        
        # Flash on strong beats
        if musical_state.get('strong_beat', False):
            self.portal.flash()
        
        # Ripples on note triggers
        for note_event in musical_state.get('note_events', []):
            if note_event['type'] == 'note_on':
                # Map note pitch to LED position
                position = (note_event['note'] - 60) / 48.0  # C4-C8 range
                self.portal.ripple(position)

    def set_mood_program(self, mood: str):
        """Set animation program based on musical mood"""
        mood_programs = {
            'energetic': 1,  # PULSE
            'flowing': 2,    # WAVE  
            'chaotic': 3,    # CHAOS
            'calm': 4,       # AMBIENT
            'mysterious': 0, # SPIRAL
            'dreamy': 9,     # BREATHE
            'colorful': 7    # RAINBOW
        }
        program = mood_programs.get(mood, 4)  # Default to AMBIENT
        self.portal.set_program(program)
```

### Error Handling and Recovery
```python
class RobustPortalController:
    def __init__(self, *args, **kwargs):
        self.portal = TeensyPortalController(*args, **kwargs)
        self.last_successful_command = time.time()
        self.connection_failures = 0
        
    def safe_send(self, method_name: str, *args, **kwargs):
        """Send command with error handling and retry logic"""
        max_retries = 3
        
        for attempt in range(max_retries):
            try:
                method = getattr(self.portal, method_name)
                if method(*args, **kwargs):
                    self.last_successful_command = time.time()
                    self.connection_failures = 0
                    return True
                else:
                    print(f"Command {method_name} was NAK'd, retrying...")
                    
            except Exception as e:
                print(f"Communication error on attempt {attempt + 1}: {e}")
                self.connection_failures += 1
                
                if self.connection_failures > 10:
                    print("Too many failures, attempting reconnection...")
                    self.reconnect()
                
                time.sleep(0.01 * (attempt + 1))  # Exponential backoff
        
        return False
    
    def reconnect(self):
        """Attempt to reconnect to Teensy"""
        try:
            self.portal.serial.close()
            time.sleep(1)
            self.portal.serial.open()
            
            # Test connection
            if self.portal.ping():
                print("Reconnection successful")
                self.connection_failures = 0
                return True
            else:
                print("Reconnection failed - Teensy not responding")
                return False
                
        except Exception as e:
            print(f"Reconnection error: {e}")
            return False
    
    def health_check(self):
        """Periodic health check"""
        if time.time() - self.last_successful_command > 30:  # 30 second timeout
            print("Portal controller health check...")
            if not self.portal.ping():
                print("Health check failed, attempting recovery")
                self.reconnect()
```

---

## Performance Guidelines

### Timing Considerations
- **Command Rate**: Don't exceed ~100 commands/second to avoid overwhelming Teensy
- **Response Timeout**: Use 100ms timeout for command responses
- **Heartbeat**: Send ping every 10-30 seconds for connection monitoring
- **Batch Updates**: Group related commands (e.g., program + BPM + intensity) with small delays

### Best Practices
```python
# ✅ Good: Batch related updates
def transition_to_energetic_section(bpm: float):
    portal.set_program(1)      # PULSE
    time.sleep(0.01)
    portal.set_bpm(bpm)
    time.sleep(0.01) 
    portal.set_intensity(0.9)
    time.sleep(0.01)
    portal.flash()

# ❌ Avoid: Rapid-fire commands without delays
def bad_update_loop():
    while True:
        portal.set_hue(random.random())  # Too fast!
        portal.set_intensity(random.random())
```

### Memory and CPU Usage
- Use connection pooling - don't create new serial connections frequently
- Cache the last sent values to avoid redundant commands
- Use async/threading for non-blocking communication if needed

---

## Debugging and Troubleshooting

### Common Issues

1. **No Response from Teensy**
   - Check serial port permissions (`sudo usermod -a -G dialout $USER`)
   - Verify correct port (`ls /dev/ttyACM*`)
   - Ensure Teensy is running portal firmware
   - Check baud rate (115200)

2. **Commands Rejected (NAK responses)**
   - Verify command values are in valid ranges
   - Check message checksums
   - Ensure Teensy isn't in error state

3. **Timing Issues**
   - Add small delays between rapid commands
   - Use proper timeout values
   - Check for serial buffer overflows

### Debug Logging
```python
import logging

# Enable debug logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

class DebugPortalController(TeensyPortalController):
    def send_command(self, command: PortalCommand, value: int = 0) -> bool:
        logger.debug(f"Sending {command.name} with value {value}")
        result = super().send_command(command, value)
        logger.debug(f"Command {'succeeded' if result else 'failed'}")
        return result
```

### Test Scripts
The firmware includes a Python test script (`test_pi_serial.py`) for validation:
```bash
# Run automated tests
python test_pi_serial.py /dev/ttyACM0

# Interactive mode for manual testing
python test_pi_serial.py /dev/ttyACM0 interactive
```

---

## Migration from MIDI

If you're migrating from the old MIDI CC system:

### MIDI CC to Serial Command Mapping
```python
# Old MIDI CC approach
midi_cc_map = {
    60: 'SET_PROGRAM',     # CC 60 -> SET_PROGRAM  
    61: 'SET_BPM',         # CC 61 -> SET_BPM
    62: 'SET_INTENSITY',   # CC 62 -> SET_INTENSITY
    63: 'SET_HUE',         # CC 63 -> SET_HUE
    64: 'SET_BRIGHTNESS',  # CC 64 -> SET_BRIGHTNESS
    65: 'TRIGGER_FLASH',   # CC 65 -> TRIGGER_FLASH
    66: 'TRIGGER_RIPPLE'   # CC 66 -> TRIGGER_RIPPLE
}

# Migration helper
class MIDIToSerialBridge:
    def __init__(self, portal_controller):
        self.portal = portal_controller
    
    def handle_midi_cc(self, cc_number: int, cc_value: int):
        """Convert MIDI CC to serial command"""
        if cc_number == 60:  # Program
            self.portal.set_program(cc_value)
        elif cc_number == 61:  # BPM
            bpm = 60 + (cc_value / 127) * 120
            self.portal.set_bpm(bpm)
        elif cc_number == 62:  # Intensity
            self.portal.set_intensity(cc_value / 127)
        # ... etc
```

---

## Future Extensibility

The protocol is designed for easy extension:

### Adding New Commands
1. Define new command constant (e.g., `SET_STROBE = 0x08`)
2. Add handler in Teensy firmware
3. Add method to Python class
4. Update documentation

### Protocol Versioning
Consider adding version negotiation for future compatibility:
```python
def get_firmware_version():
    # Future: Query Teensy firmware version
    # Could use STATUS command with special value
    pass
```

This integration guide provides everything needed for a Raspberry Pi AI agent to effectively control the Mystery Melody Machine's visual system using the new serial protocol. The combination of reliable communication, rich command set, and music-responsive features enables sophisticated audio-visual experiences.
