# Binary Switch Feature Documentation

## Overview
The first 8 switches (connected to pins 22-29) in the SWITCH_PINS array are grouped together to provide both individual MIDI CC outputs and a combined binary representation value.

## Feature Details

### Individual Switch Behavior
- Each of the 12 switches sends its own MIDI CC message on state change:
  - Switch 0 → CC 20
  - Switch 1 → CC 21  
  - Switch 2 → CC 22
  - Switch 3 → CC 23
  - Switch 4 → CC 24
  - Switch 5 → CC 25
  - Switch 6 → CC 26
  - Switch 7 → CC 27
  - Switch 8 → CC 28
  - Switch 9 → CC 29
  - Switch 10 → CC 30
  - Switch 11 → CC 31
- Value: 127 when switch is ON, 0 when switch is OFF

### Binary Representation Feature
In addition to individual CCs, the first 8 switches (switches 0-7) generate a combined binary representation:
- **MIDI CC**: 50 (configurable via `SWITCH_BINARY_CC` in config.h)
- **Value**: 0-255 representing the binary state of switches 0-7
- **Bit mapping**: Switch N corresponds to bit N (where bit 0 is LSB)

### Binary Value Examples

| Switch States (7-0) | Binary | Decimal | CC 50 Value |
|---------------------|--------|---------|-------------|
| 00000000            | 0b00000000 | 0   | 0   |
| 00000001            | 0b00000001 | 1   | 1   |
| 00000010            | 0b00000010 | 2   | 2   |
| 00000101            | 0b00000101 | 5   | 5   |
| 10101010            | 0b10101010 | 170 | 170 |
| 11111111            | 0b11111111 | 255 | 255 |

### Practical Example
If switches 0, 2, and 4 are ON:
- Individual CCs sent: CC 20=127, CC 22=127, CC 24=127
- Binary CC sent: CC 50 = (1<<0) | (1<<2) | (1<<4) = 1 + 4 + 16 = 21

## Implementation Details

### Code Changes

#### config.h
```cpp
// MIDI CC mapping for switches (expanded to 12 switches)
constexpr uint8_t SWITCH_CCS[] = {
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

// MIDI CC for binary representation of first 8 switches
constexpr uint8_t SWITCH_BINARY_CC = 50;
```

#### robust_midi_mapper.h
```cpp
// Added binary value tracking
uint8_t lastBinaryValue;  // Track binary representation of first 8 switches
```

#### robust_midi_mapper.cpp
```cpp
void RobustMidiMapper::processSwitches() {
    bool binaryStateChanged = false;
    
    // Process individual switches
    for (int i = 0; i < SWITCH_COUNT; i++) {
        bool currentState = processor_.getSwitchState(i);
        
        if (currentState != lastSwitchStates[i]) {
            uint8_t midiValue = currentState ? 127 : 0;
            
            // Send individual CC
            midiOut_.sendControlChange(SWITCH_CCS[i], midiValue, MIDI_CHANNEL);
            
            lastSwitchStates[i] = currentState;
            
            // Mark binary state change for first 8 switches
            if (i < 8) {
                binaryStateChanged = true;
            }
        }
    }
    
    // Process binary representation
    if (binaryStateChanged) {
        uint8_t binaryValue = 0;
        
        for (int i = 0; i < 8 && i < SWITCH_COUNT; i++) {
            if (processor_.getSwitchState(i)) {
                binaryValue |= (1 << i);
            }
        }
        
        if (binaryValue != lastBinaryValue) {
            midiOut_.sendControlChange(SWITCH_BINARY_CC, binaryValue, MIDI_CHANNEL);
            lastBinaryValue = binaryValue;
        }
    }
}
```

## Usage Notes

### MIDI Channel
All switch messages (individual and binary) are sent on the same MIDI channel (default: Channel 1).

### Performance
- Binary calculation only occurs when switches 0-7 change state
- No duplicate messages are sent if the binary value hasn't changed
- Switches 8-11 do not affect the binary representation

### Customization
- Binary CC number can be changed by modifying `SWITCH_BINARY_CC` in config.h
- Individual switch CC numbers can be modified in the `SWITCH_CCS` array
- All values are configurable at compile time

## Debug Output
When DEBUG level is 1 or higher, the firmware will output debug messages:
```
MIDI: Switch 0 ON -> CC 20 = 127
MIDI: Switch binary representation -> CC 50 = 1 (0b00000001)
```

## Integration with Control Software
This feature allows control software to:
1. Monitor individual switch states for discrete control
2. Use the binary representation for:
   - Preset selection (256 possible combinations)
   - Bitfield-based parameter control
   - State machine control
   - Pattern sequencing

The binary representation provides a compact way to handle switch combinations without requiring the control software to reconstruct the bit pattern from individual switch states.
