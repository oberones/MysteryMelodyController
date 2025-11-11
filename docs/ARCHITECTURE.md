# Mystery Melody Machine - Teensy Firmware Architecture Guide

## Table of Contents
1. [System Overview](#system-overview)
2. [Entry Point & Initialization](#entry-point--initialization)
3. [Main Loop Architecture](#main-loop-architecture)
4. [Module Architecture](#module-architecture)
5. [Data Flow Diagrams](#data-flow-diagrams)
6. [Code Paths by Feature](#code-paths-by-feature)
7. [Communication Protocols](#communication-protocols)
8. [Testing Architecture](#testing-architecture)
9. [Performance & Timing](#performance--timing)

---

## System Overview

The Mystery Melody Machine firmware is a real-time embedded system built on Teensy 4.1 that combines:
- **Musical instrument interface**: Converting physical controls to MIDI messages
- **LED animation engine**: Controlling a 45-LED "infinity portal" with multiple animation programs
- **Bi-directional communication**: Serial protocol for Raspberry Pi integration + legacy MIDI CC support
- **Display feedback**: Optional OLED display for real-time monitoring

### Key Design Principles
- **Dual-rate architecture**: 1kHz input scanning + 60Hz LED rendering
- **Static memory allocation**: No dynamic allocation in hot paths
- **Layered abstraction**: Raw hardware → robust processing → mapping → output
- **Fail-safe operation**: Graceful degradation when components fail

### Hardware I/O Summary
```
INPUT:
├── 10 Buttons (digital)      → MIDI Notes 60-69
├── 4 Potentiometers (analog) → MIDI CC 1-4
├── 4 Joystick directions     → MIDI CC 10-13 (pulse)
└── 12 Switches (digital)     → MIDI CC 20-31 + Binary CC 50

OUTPUT:
├── 45 WS2812B LEDs           → Portal animations (60 FPS)
├── USB MIDI                  → Note & CC messages
├── Serial (115200 baud)      → Raspberry Pi protocol
└── OLED Display (I2C)        → Status/MIDI log (optional)
```

---

## Entry Point & Initialization

### File: `src/main.cpp`

#### Startup Sequence

```cpp
void setup() {
    1. Serial.begin(115200)           // Pi communication + debugging
    2. pinMode(BUILTIN_LED_PIN)       // Heartbeat LED
    3. oledDisplay.begin()            // Optional OLED (graceful fail)
    4. inputProcessor.begin()         // Initialize input scanner
    5. midiOut.begin()                // Initialize MIDI output
    6. FastLED configuration          // 45 LEDs, pin 1, max brightness
    7. portalController.begin()       // Animation system
    8. portalCueHandler.begin()       // Pi/MIDI portal control
    9. portalStartupSequence()        // Visual boot animation
    10. Setup complete message
}
```

#### Portal Startup Sequence
Showcases 5 animation programs (1 second each):
1. SPIRAL → PULSE → RAINBOW → WAVE → PLASMA
2. Triple flash sequence
3. Fade to AMBIENT mode (default idle state)

**Purpose**: Visual confirmation that LEDs and controller are working

---

## Main Loop Architecture

### Dual-Rate System Design

The firmware runs two concurrent loops at different rates:

```
┌─────────────────────────────────────────────────────────────┐
│ Main Loop (continuous)                                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌───────────────────────────────┐                          │
│  │ INPUT SCAN LOOP: 1000 Hz      │                          │
│  │ (every 1ms, 1000μs)           │                          │
│  ├───────────────────────────────┤                          │
│  │ 1. inputProcessor.update()    │ ← Raw scan + debounce    │
│  │ 2. inputMapper.processInputs()│ ← Edge detect + MIDI     │
│  │ 3. OLED mode switching        │ ← Button 0/1 handling    │
│  │ 4. updateOledInputData()      │ ← Feed OLED cache        │
│  │ 5. handlePortalInteractions() │ ← Flash/ripple triggers  │
│  │ 6. portalCueHandler.process() │ ← Serial + MIDI cues     │
│  └───────────────────────────────┘                          │
│                                                             │
│  ┌───────────────────────────────┐                          │
│  │ PORTAL RENDER: 60 Hz          │                          │
│  │ (every 16.667ms, ~16667μs)    │                          │
│  ├───────────────────────────────┤                          │
│  │ 1. portalController.update()  │ ← Run animation program  │
│  │ 2. FastLED.show()             │ ← Push to LED strip      │
│  └───────────────────────────────┘                          │
│                                                             │
│  ┌───────────────────────────────┐                          │
│  │ OLED UPDATE: 20 Hz            │                          │
│  │ (every 50ms)                  │                          │
│  ├───────────────────────────────┤                          │
│  │ 1. oledDisplay.update()       │ ← Refresh screen         │
│  └───────────────────────────────┘                          │
│                                                             │
│  ┌───────────────────────────────┐                          │
│  │ HEARTBEAT: 1 Hz               │                          │
│  │ (every 1000ms)                │                          │
│  ├───────────────────────────────┤                          │
│  │ 1. Toggle built-in LED        │                          │
│  │ 2. Print activity status      │                          │
│  └───────────────────────────────┘                          │
│                                                             │
│  ┌───────────────────────────────┐                          │
│  │ DEBUG DUMP: 0.2 Hz (optional) │                          │
│  │ (every 5000ms, if DEBUG >= 1) │                          │
│  ├───────────────────────────────┤                          │
│  │ 1. dumpTestValues()           │ ← Print all input states │
│  └───────────────────────────────┘                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Timing Control

All timing uses **non-blocking elapsed timers** (Teensy's `elapsedMicros`/`elapsedMillis`):

```cpp
elapsedMicros mainLoopTimer;        // Auto-incrementing μs counter
elapsedMicros portalFrameTimer;     // Auto-incrementing μs counter
elapsedMillis blinkTimer;           // Auto-incrementing ms counter
elapsedMillis oledUpdateTimer;      // Auto-incrementing ms counter

// Example: 1kHz input scan
if (mainLoopTimer >= (1000000 / SCAN_HZ)) {  // 1000000/1000 = 1000μs
    mainLoopTimer -= (1000000 / SCAN_HZ);    // Reset timer
    // ... do work ...
}
```

**Advantages**:
- Non-blocking (no `delay()`)
- Precise timing independent of loop speed
- Multiple concurrent timers
- Graceful handling of missed deadlines

---

## Module Architecture

### Layer 1: Hardware Abstraction (Raw I/O)

#### `InputScanner` (include/input_scanner.h)
**Purpose**: Direct hardware polling with minimal processing

```
┌───────────────────────────────────────┐
│ InputScanner                          │
├───────────────────────────────────────┤
│ Methods:                              │
│ • begin()           → pinMode setup   │
│ • scan()            → Read all pins   │
│ • getButtonState()  → Current value   │
│ • getPotValue()     → Raw ADC (0-1023)│
├───────────────────────────────────────┤
│ State:                                │
│ • buttonStates[10]                    │
│ • lastButtonStates[10]  → Edge detect │
│ • potValues[4]                        │
│ • lastPotValues[4]      → Change flag │
│ • switchStates[12]                    │
│ • joystickStates[4]                   │
└───────────────────────────────────────┘
```

**Called by**: `RobustInputProcessor.update()` at 1kHz

---

### Layer 2: Signal Conditioning

#### `Debouncer` (include/debouncer.h)
**Purpose**: Time-based switch debouncing

```cpp
class Debouncer {
    bool update(bool rawState, uint32_t currentTime);
    bool isPressed() const;
    bool justPressed() const;   // Rising edge
    bool justReleased() const;  // Falling edge
private:
    uint32_t lastChangeTime;
    bool stableState;
    bool lastStableState;
    uint32_t debounceTime;  // Default 5ms
};
```

**Algorithm**: State must remain stable for `debounceTime` ms before accepting change.

#### `AnalogSmoother` (include/analog_smoother.h)
**Purpose**: Reduce noise and rate-limit analog inputs

```cpp
class AnalogSmoother {
    uint8_t update(uint16_t rawValue);
    bool hasChanged() const;
private:
    uint16_t smoothedValue;      // EMA filtered
    uint16_t lastSentValue;      // For change detection
    uint32_t lastChangeTime;     // For rate limiting
    uint8_t deadband;            // ±2 counts
    uint32_t rateLimit;          // 15ms minimum between changes
};
```

**Algorithm**: 
1. EMA filter: `smooth += (raw - smooth) >> 2`  (α ≈ 0.25)
2. Deadband: Only report if change > ±2
3. Rate limit: Max 67 changes/second per pot
4. Change compression: Wait 4ms stable OR large threshold (±8)

---

### Layer 3: Robust Input Processing

#### `RobustInputProcessor` (include/robust_input_processor.h)
**Purpose**: Unified interface for all conditioned inputs

```
┌────────────────────────────────────────────────────┐
│ RobustInputProcessor                               │
├────────────────────────────────────────────────────┤
│ Composition:                                       │
│ • InputScanner scanner           (raw I/O)         │
│ • Debouncer buttonDebouncers[10] (one per button)  │
│ • Debouncer switchDebouncers[12] (one per switch)  │
│ • Debouncer joystickDebouncers[4] (one per dir)    │
│ • AnalogSmoother potSmoothers[4] (one per pot)     │
│ • uint32_t joystickRearmTime[4]  (anti-rapid-fire) │
│ • uint32_t lastActivityTime      (idle detection)  │
├────────────────────────────────────────────────────┤
│ Public API (clean, debounced states):              │
│ • getButtonPressed(i) → Rising edge detection      │
│ • getButtonReleased(i) → Falling edge detection    │
│ • getButtonState(i) → Current stable state         │
│ • getPotMidiValue(i) → Smoothed 0-127 value        │
│ • getPotChanged(i) → True if value changed         │
│ • getSwitchChanged(i) → True if value changed      │
│ • getSwitchState(i) → Current stable state         │
│ • getJoystickPressed(dir) → Single pulse per press │
│ • isIdle() → No activity for 30 seconds            │
└────────────────────────────────────────────────────┘
```

**Joystick Rearm Logic**: Prevents rapid-fire CC spam
```cpp
if (joystickDebouncer.justPressed() && 
    (currentTime - lastRearmTime) >= JOYSTICK_REARM_MS) {
    lastRearmTime = currentTime;
    return true;  // Single pulse
}
return false;
```

**Activity Tracking**: Updates `lastActivityTime` on any input change
- Buttons, pots, switches, joystick all count as activity
- `isIdle()` returns true after 30 seconds of no changes

---

### Layer 4: MIDI Mapping

#### `RobustMidiMapper` (include/robust_midi_mapper.h)
**Purpose**: Convert input events to MIDI messages

```
Input Type         Edge Detection         MIDI Output
───────────────────────────────────────────────────────
Button[i]          justPressed()    →     Note On (60+i)
                   justReleased()   →     Note Off (60+i)

Pot[i]             hasChanged()     →     CC (1+i) = value

Joystick[dir]      justPressed()    →     CC (10+dir) = 127
                   (with rearm)

Switch[i]          stateChanged()   →     CC (20+i) = 0/127

Switch[0-7]        8-bit binary     →     CC 50 = combined
```

**State Tracking**: Maintains `lastButtonStates[]`, `lastPotValues[]` for edge detection

**Binary Switch Encoding** (first 8 switches):
```cpp
uint8_t binaryValue = 0;
for (int i = 0; i < 8; i++) {
    if (getSwitchState(i)) {
        binaryValue |= (1 << i);
    }
}
if (binaryValue != lastBinaryValue) {
    sendControlChange(50, binaryValue);
}
```

---

### Layer 5: Output Systems

#### `MidiOut` (include/midi_out.h)
**Purpose**: MIDI transmission with logging

```cpp
class MidiOut {
    void sendNoteOn(note, velocity, channel);
    void sendNoteOff(note, velocity, channel);
    void sendControlChange(cc, value, channel);
    void setOledDisplay(OledDisplay* display);  // Optional logging
};
```

**Conditional Compilation**:
```cpp
#ifdef USB_MIDI
    usbMIDI.sendNoteOn(note, velocity, channel);
#endif
```

**OLED Integration**: If display is attached, all MIDI messages are logged:
```cpp
if (oledDisplay) {
    oledDisplay->logMidiNoteOn(note, velocity, channel);
}
```

---

### Portal Animation System

#### `PortalController` (include/portal_controller.h)
**Purpose**: LED animation engine with 10 programs

```
┌───────────────────────────────────────────────────────┐
│ PortalController                                      │
├───────────────────────────────────────────────────────┤
│ State:                                                │
│ • CRGB* leds                  → 45-element LED array  │
│ • uint8_t currentProgram      → 0-9 (program select)  │
│ • float bpm, intensity, baseHue, activityLevel        │
│ • uint32_t frameCount         → Animation counter     │
│ • Ripple ripples[3]           → Interaction effects   │
│ • bool flashActive            → Button flash state    │
├───────────────────────────────────────────────────────┤
│ Animation Programs (10):                              │
│ • SPIRAL      → Rotating spiral patterns              │
│ • PULSE       → BPM-synced center pulse               │
│ • WAVE        → Flowing wave patterns                 │
│ • CHAOS       → Random chaotic patterns               │
│ • AMBIENT     → Slow peaceful drift                   │
│ • IDLE        → Minimal low-power mode                │
│ • RIPPLE      → Ripple effects layer                  │
│ • RAINBOW     → Smooth rainbow rotation               │
│ • PLASMA      → Plasma-like flowing colors            │
│ • BREATHE     → Gentle breathing effect               │
├───────────────────────────────────────────────────────┤
│ Control Methods:                                      │
│ • setProgram(id)           → Switch animation         │
│ • setBpm(60-180)           → Tempo sync               │
│ • setIntensity(0.0-1.0)    → Animation strength       │
│ • setBaseHue(0.0-1.0)      → Color shift              │
│ • setBrightness(0-255)     → Global brightness        │
│ • triggerFlash()           → Button press feedback    │
│ • triggerRipple(pos)       → Positional effect        │
│ • setActivityLevel(0-1)    → Dynamic response         │
└───────────────────────────────────────────────────────┘
```

**Update Cycle** (60 FPS):
```cpp
void update() {
    frameCount++;
    updateBpmPhase();
    
    // Run current animation program
    switch (currentProgram) {
        case SPIRAL:  updateSpiral(); break;
        case PULSE:   updatePulse(); break;
        // ... etc ...
    }
    
    // Apply interaction effects (flash, ripples)
    applyInteractionEffects();
}
```

**Interaction Effects**:
- **Flash**: Button press → white flash overlay (200ms decay)
- **Ripple**: 3 concurrent ripples, expanding from trigger position
- **Activity Level**: Pot movement → animation intensity modulation

---

### Communication Systems

#### `PortalCueHandler` (include/portal_cue_handler.h)
**Purpose**: Raspberry Pi serial protocol handler + legacy MIDI CC

```
┌─────────────────────────────────────────────────────┐
│ PortalCueHandler                                    │
├─────────────────────────────────────────────────────┤
│ Dual Protocol Support:                              │
│ 1. Serial Binary Protocol (PRIMARY)                 │
│    • processSerialInput() → Parse incoming messages │
│    • handleSerialMessage() → Execute commands       │
│    • sendAck/Nak/Pong/Status → Responses            │
│                                                     │
│ 2. Legacy MIDI CC (BACKWARD COMPAT)                 │
│    • handleMidiCC(cc, value) → CC 60-66 mapping     │
├─────────────────────────────────────────────────────┤
│ Auto Behavior:                                      │
│ • setInputActivity(bool) → Track user activity      │
│ • checkIdleState() → Auto-switch to IDLE after 30s  │
│ • Auto program rotation when idle (every 60s)       │
└─────────────────────────────────────────────────────┘
```

#### Serial Protocol (include/serial_portal_protocol.h)

**Message Format** (5 bytes):
```
[0xAA][COMMAND][VALUE][CHECKSUM][0x55]

Where:
- 0xAA = Start marker
- COMMAND = enum PortalSerialCommand (0x01-0x23)
- VALUE = 8-bit parameter (0-255)
- CHECKSUM = COMMAND XOR VALUE
- 0x55 = End marker
```

**Commands** (Pi → Teensy):
```cpp
SET_PROGRAM    (0x01)  // value = 0-9 (program ID)
SET_BPM        (0x02)  // value → 60-180 BPM mapping
SET_INTENSITY  (0x03)  // value → 0.0-1.0 mapping
SET_HUE        (0x04)  // value → 0.0-1.0 mapping
SET_BRIGHTNESS (0x05)  // value = 0-255
TRIGGER_FLASH  (0x06)  // value ignored
TRIGGER_RIPPLE (0x07)  // value → LED position
PING           (0x10)  // Keepalive
RESET          (0x11)  // Reset to defaults
```

**Responses** (Teensy → Pi):
```cpp
PONG   (0x20)  // Response to PING
ACK    (0x21)  // Command successful
NAK    (0x22)  // Command failed/invalid
STATUS (0x23)  // Status report
```

**Legacy MIDI CC Mapping** (same functionality):
```
CC 60 → Program (0-9)
CC 61 → BPM (0-127 → 60-180)
CC 62 → Intensity (0-127 → 0.0-1.0)
CC 63 → Hue (0-127 → 0.0-1.0)
CC 64 → Brightness (0-127 → 0-255)
CC 65 → Flash trigger (127 = trigger)
CC 66 → Ripple position (0-127 → LED 0-44)
```

---

### Display System

#### `OledDisplay` (include/oled_display.h)
**Purpose**: 128x64 OLED with 4 display modes

```
┌──────────────────────────────────────────────────────┐
│ OledDisplay (SSD1306 I2C)                            │
├──────────────────────────────────────────────────────┤
│ Display Modes (cycle with Button 0/1):               │
│                                                      │
│ 1. MIDI_LOG (default)                                │
│    ┌──────────────────────────┐                      │
│    │ MIDI LOG                 │                      │
│    │ → NoteOn C4 vel:100 ch:1 │                      │
│    │ → CC 1 = 64 ch:1         │                      │
│    │ → NoteOff C4 ch:1        │                      │
│    │ (Shows last 8 messages)  │                      │
│    └──────────────────────────┘                      │
│                                                      │
│ 2. STATUS                                            │
│    ┌──────────────────────────┐                      │ 
│    │ STATUS                   │                      │
│    │ Buttons: [0][1]...[9]    │                      │
│    │ Pots: 64 32 127 0        │                      │
│    │ Switch: [0][1][2]...     │                      │
│    │ Joy: ← → ↑ ↓             │                      │
│    └──────────────────────────┘                      │
│                                                      │
│ 3. ACTIVITY                                          │
│    ┌──────────────────────────┐                      │
│    │ ACTIVITY                 │                      │
│    │ [Visual bars/indicators] │                      │
│    │ [for recent input events]│                      │
│    └──────────────────────────┘                      │
│                                                      │
│ 4. INFO                                              │
│    ┌──────────────────────────┐                      │
│    │ SYSTEM INFO              │                      │
│    │ Loop: 234μs / 1000μs     │                      │
│    │ Uptime: 01:23:45         │                      │
│    │ Idle: NO                 │                      │
│    └──────────────────────────┘                      │
│                                                      │
├──────────────────────────────────────────────────────┤
│ Integration:                                         │
│ • MidiOut logs all MIDI → MIDI_LOG display           │
│ • main.cpp feeds input states → STATUS/ACTIVITY      │
│ • main.cpp feeds perf data → INFO display            │
└──────────────────────────────────────────────────────┘
```

**Update Rate**: 20 Hz (every 50ms) to avoid interfering with main loop

---

## Data Flow Diagrams

### MIDI Output Flow

```
Hardware Pins
    ↓
┌───────────────┐
│ InputScanner  │ ← 1kHz scan() call
│ (Raw polling) │
└───────┬───────┘
        ↓ Raw states
┌──────────────────────┐
│ RobustInputProcessor │
│ ┌──────────────────┐ │
│ │ Debouncer[10]    │ │ ← Button debouncing
│ │ Debouncer[4]     │ │ ← Joystick debouncing
│ │ Debouncer[12]    │ │ ← Switch debouncing
│ │ AnalogSmoother[4]│ │ ← Pot smoothing
│ └──────────────────┘ │
└──────────┬───────────┘
           ↓ Clean states
┌────────────────────┐
│ RobustMidiMapper   │ ← Edge detection
│ • processButtons() │ ← Note On/Off
│ • processPots()    │ ← CC 1-4
│ • processJoystick()│ ← CC 10-13
│ • processSwitches()│ ← CC 20-31, 50
└──────────┬─────────┘
           ↓ MIDI messages
┌──────────────────┐
│ MidiOut          │
│ #ifdef USB_MIDI  │ ← usbMIDI.send*()
│ #endif           │
└──────────┬───────┘
           ├──────────────────┐
           ↓                  ↓
    ┌──────────┐      ┌────────────┐
    │ USB MIDI │      │ OledDisplay│ ← Optional logging
    │ Device   │      │ (MIDI_LOG) │
    └──────────┘      └────────────┘
```

### Portal Animation Flow

```
┌─────────────────────────────────────────────────────┐
│ Input Sources (3 paths)                             │
├─────────────────────────────────────────────────────┤
│                                                     │
│ 1. User Interactions                                │
│    handlePortalInteractions() in main.cpp           │
│    • Button press → triggerFlash()                  │
│    • Pot movement → setBaseHue(), triggerRipple()   │
│    • Joystick → triggerRipple(position)             │
│                                                     │
│ 2. Raspberry Pi Serial                              │
│    PortalCueHandler::processSerialInput()           │
│    • Binary protocol → SET_PROGRAM, SET_BPM, etc.   │
│    • Response: ACK/NAK/PONG                         │
│                                                     │
│ 3. Legacy MIDI CC (backward compat)                 │
│    PortalCueHandler::handleMidiCC()                 │
│    • CC 60-66 → Same as serial commands             │
│                                                     │
└───────────────────┬─────────────────────────────────┘
                    ↓ All converge to
        ┌───────────────────────┐
        │ PortalController      │
        │ • setProgram()        │
        │ • setBpm()            │
        │ • setIntensity()      │
        │ • setBaseHue()        │
        │ • triggerFlash()      │
        │ • triggerRipple(pos)  │
        └───────┬───────────────┘
                ↓ 60 FPS update()
        ┌──────────────────────────┐
        │ Animation Programs       │
        │ ┌───────────────────┐    │
        │ │ updateSpiral()    │    │
        │ │ updatePulse()     │    │
        │ │ updateWave()      │    │
        │ │ updateRainbow()   │    │
        │ │ ... etc (10 total)│    │
        │ └───────────────────┘    │
        │                          │
        │ applyInteractionEffects()│ ← Flash/ripple overlay
        └───────┬──────────────────┘
                ↓ Writes to
        ┌───────────────────┐
        │ CRGB leds[45]     │ ← Shared LED array
        └───────┬───────────┘
                ↓
        ┌───────────────────┐
        │ FastLED.show()    │ ← Push to WS2812B strip
        └───────────────────┘
```

---

## Code Paths by Feature

### 1. Button Press → MIDI Note

**Files involved**: 
- `src/main.cpp` (loop)
- `src/robust_input_processor.cpp`
- `src/robust_midi_mapper.cpp`
- `src/midi_out.cpp`
- `src/oled_display.cpp` (optional)

**Call chain**:
```cpp
loop()
  └─→ inputProcessor.update()
      └─→ scanner.scan()                        // Read GPIO
      └─→ buttonDebouncers[i].update(raw, time) // 5ms debounce
  └─→ inputMapper.processInputs()
      └─→ processButtons()
          └─→ if (processor.getButtonPressed(i)) {  // Rising edge
                  midiOut.sendNoteOn(60+i, 100, 1);
                  └─→ #ifdef USB_MIDI
                      └─→ usbMIDI.sendNoteOn()
                  └─→ if (oledDisplay)
                      └─→ oledDisplay->logMidiNoteOn()
              }
```

**Performance**: ~100-200μs from GPIO change to MIDI transmission

---

### 2. Potentiometer Movement → MIDI CC

**Files involved**: Same as button press

**Call chain**:
```cpp
loop()
  └─→ inputProcessor.update()
      └─→ scanner.scan()                       // Read ADC
      └─→ potSmoothers[i].update(rawADC)
          └─→ EMA filter: smooth += (raw - smooth) >> 2
          └─→ Deadband check: abs(smooth - lastSent) > 2
          └─→ Rate limit: (now - lastSent) >= 15ms
          └─→ Change compression: stable for 4ms OR change > 8
  └─→ inputMapper.processInputs()
      └─→ processPots()
          └─→ if (processor.getPotChanged(i)) {
                  uint8_t midiVal = processor.getPotMidiValue(i);
                  midiOut.sendControlChange(1+i, midiVal, 1);
              }
```

**Filtering benefits**:
- EMA smoothing eliminates ADC noise
- Deadband prevents jitter around stable values
- Rate limiting prevents MIDI flood (max 67 changes/sec)
- Change compression reduces spurious updates

---

### 3. Raspberry Pi Sets Portal Program

**Files involved**:
- Pi sends serial message
- `src/portal_cue_handler.cpp` (receives & parses)
- `src/portal_controller.cpp` (executes)

**Serial message example**:
```
Pi sends: [0xAA][0x01][0x02][0x03][0x55]
          Start  SET_PROGRAM  value=2  checksum  End
                                ↓
                            RAINBOW program
```

**Call chain**:
```cpp
loop()
  └─→ portalCueHandler.processSerialInput()
      └─→ while (Serial.available())
          └─→ parseSerialMessage()
              └─→ Check start/end bytes
              └─→ Validate checksum
              └─→ handleSerialMessage(message)
                  └─→ switch (message.command) {
                      case SET_PROGRAM:
                          portalController->setProgram(value);
                          sendAck();  // [0xAA][0x21][value][checksum][0x55]
                      case SET_BPM:
                          float bpm = mapToBpm(value);  // 0-255 → 60-180
                          portalController->setBpm(bpm);
                      // ... etc
                  }
```

**Response to Pi**: ACK message sent back for confirmation

---

### 4. Button Press Triggers Portal Flash

**Files involved**:
- `src/main.cpp` (handlePortalInteractions)
- `src/portal_controller.cpp`

**Call chain**:
```cpp
loop()
  └─→ handlePortalInteractions()
      └─→ for each button:
          if (currentState && !lastState) {  // Rising edge
              portalController.triggerFlash();
              portalController.setBaseHue(i * 0.1);
          }

// Later in same loop iteration...
loop()
  └─→ if (portalFrameTimer >= 16667μs) {
      portalController.update()
          └─→ updateSpiral() / updatePulse() / etc.
              └─→ Draw current animation to leds[]
          └─→ applyInteractionEffects()
              └─→ if (flashActive) {
                  for each LED:
                      leds[i] += CRGB(255, 255, 255) * flashIntensity;
                  flashIntensity *= decay;  // Fade over 200ms
              }
      FastLED.show()
  }
```

**Effect**: White flash overlay that decays exponentially over ~200ms

---

### 5. OLED Display Mode Switching

**Files involved**:
- `src/main.cpp` (button detection)
- `src/oled_display.cpp`

**Call chain**:
```cpp
loop()
  └─→ if (mainLoopTimer >= 1000μs) {
      bool button0IsPressed = inputProcessor.getButtonState(0);
      
      // Edge detection
      if (button0IsPressed && !button0WasPressed) {
          oledDisplay.nextMode();  // MIDI_LOG → STATUS → ACTIVITY → INFO
      }
      button0WasPressed = button0IsPressed;
  }

  └─→ if (oledUpdateTimer >= 50ms) {
      oledDisplay.update()
          └─→ switch (currentMode) {
              case MIDI_LOG:  drawMidiLog(); break;
              case STATUS:    drawStatus(); break;
              case ACTIVITY:  drawActivity(); break;
              case INFO:      drawInfo(); break;
          }
          └─→ display.display();  // Push to OLED I2C
  }
```

**User experience**: Press Button 0 to cycle, Button 1 to go back

---

### 6. Idle Detection & Auto Program Switch

**Files involved**:
- `src/robust_input_processor.cpp`
- `src/portal_cue_handler.cpp`

**Call chain**:
```cpp
// Activity tracking
RobustInputProcessor::update()
  └─→ if (any input changed) {
      lastActivityTime = millis();
  }

RobustInputProcessor::isIdle()
  └─→ return (millis() - lastActivityTime) > IDLE_TIMEOUT_MS;  // 30s

// Auto-switching
loop()
  └─→ portalCueHandler.update()
      └─→ checkIdleState()
          └─→ if (processor.isIdle() && !wasIdle) {
              lastActiveProgram = currentProgram;  // Save state
              portalController->setProgram(PORTAL_IDLE);
              portalController->setBrightness(IDLE_BRIGHTNESS);
          }
          └─→ if (!processor.isIdle() && wasIdle) {
              portalController->setProgram(lastActiveProgram);  // Restore
              portalController->setBrightness(LED_BRIGHTNESS_MAX);
          }
          └─→ if (isIdle && autoSwitchTimer > 60s) {
              // Rotate through programs while idle
              nextProgram = (currentProgram + 1) % PORTAL_PROGRAM_COUNT;
              portalController->setProgram(nextProgram);
          }
```

**Behavior**: 
- After 30s no input → Switch to IDLE program (dim, slow)
- Any input → Restore previous program at full brightness
- While idle → Auto-rotate ambient programs every 60s (demo mode)
  - AMBIENT, BREATHE, RAINBOW, PLASMA

---

## Communication Protocols

### Serial Protocol Details

**Baud Rate**: 115200 (configured in `config.h`)

**Message Validation**:
```cpp
bool PortalMessage::isValid() const {
    return (startByte == 0xAA) &&
           (endByte == 0x55) &&
           (checksum == (command ^ value));
}
```

**Error Handling**:
- Invalid start/end → Ignore, wait for next start byte
- Bad checksum → Send NAK, discard message
- Unknown command → Send NAK
- Valid command → Execute, send ACK

**Pi Integration Example** (Python pseudocode):
```python
import serial

teensy = serial.Serial('/dev/ttyACM0', 115200)

def send_portal_command(cmd, value):
    checksum = cmd ^ value
    message = bytes([0xAA, cmd, value, checksum, 0x55])
    teensy.write(message)
    
    # Wait for ACK (0x21)
    response = teensy.read(5)
    if response[1] == 0x21:
        return True  # Success
    else:
        return False  # NAK or timeout

# Set program to RAINBOW
send_portal_command(0x01, 7)  # SET_PROGRAM, value=7

# Set BPM to 150
bpm_value = int((150 - 60) / (180 - 60) * 255)  # Map to 0-255
send_portal_command(0x02, bpm_value)
```

**Latency**: ~1-2ms from Pi transmission to Teensy execution

---

### MIDI Protocol Details

**USB Device Type**: MIDI (configured via `USB_MIDI` build flag)

**MIDI Channel**: 1 (all messages)

**Message Types Used**:
- Note On/Off (buttons)
- Control Change (pots, joystick, switches, portal cues)

**Duplicate Filtering**: None (relies on input processing to prevent duplicates)

**MIDI CC List**:
```
INPUT CONTROLS:
CC 1-4    → Potentiometers 0-3
CC 10-13  → Joystick (Up, Down, Left, Right)
CC 20-31  → Switches 0-11 (individual)
CC 50     → Binary representation of switches 0-7

PORTAL CONTROLS (LEGACY):
CC 60     → Program select (0-9)
CC 61     → BPM (0-127 → 60-180)
CC 62     → Intensity (0-127 → 0.0-1.0)
CC 63     → Base hue (0-127 → 0.0-1.0)
CC 64     → Brightness (0-127 → 0-160) LED_BRIGHTNESS_MAX (160) by default
CC 65     → Flash trigger (127 = trigger)
CC 66     → Ripple position (0-127 → LED 0-44)
```

---

## Testing Architecture

### Unit Tests (test/ directory)

**Framework**: PlatformIO Unity Test Framework

**Test Files**:
```
test/
├── test_debouncer.cpp        → Debouncer timing tests
├── test_binary_switches.cpp  → Binary switch encoding
├── test_oled_display.cpp     → Display mode switching
├── test_serial_protocol.cpp  → Message parsing/validation
└── test_pi_serial.py         → Pi-side protocol test (Python)
```

**Example Test** (`test_debouncer.cpp`):
```cpp
void test_debouncer_press_sequence() {
    Debouncer db(5);  // 5ms debounce
    
    // Simulate button press
    db.update(false, 0);     // Press at t=0
    TEST_ASSERT_FALSE(db.justPressed()); // Too early
    
    db.update(false, 3);     // Still pressed at t=3ms
    TEST_ASSERT_FALSE(db.justPressed()); // Still too early
    
    db.update(false, 6);     // Still pressed at t=6ms
    TEST_ASSERT_TRUE(db.justPressed());  // Now registered
    
    db.update(false, 7);     // Still pressed
    TEST_ASSERT_FALSE(db.justPressed()); // Only fires once
}
```

**Running Tests**:
```bash
pio test                          # Run all tests
pio test -f test_debouncer        # Run specific test
pio test -e teensy41-debug        # Test on hardware
```

---

### Hardware-in-the-Loop Testing

**Test Mode** (enabled when `DEBUG >= 1`):
```cpp
#if DEBUG >= 1
inputProcessor.enableTestMode(true);
#endif
```

**Test Output** (every 5 seconds):
```
=== INPUT STATE DUMP ===
Buttons: 0:OFF 1:OFF 2:ON 3:OFF 4:OFF 5:OFF 6:OFF 7:OFF 8:OFF 9:OFF 
Switches: 0:ON 1:OFF 2:OFF 3:OFF 4:OFF 5:OFF 6:OFF 7:OFF 8:OFF 9:OFF 10:OFF 11:OFF 
Pots: 0:MIDI_64 1:MIDI_32 2:MIDI_127 3:MIDI_0 
Activity: 1240ms ago, Idle: NO
========================
```

**Performance Monitoring**:
```cpp
uint32_t loopStartTime = micros();
// ... do work ...
uint32_t loopTime = micros() - loopStartTime;
oledDisplay.updateSystemInfo(loopTime, isIdle, millis());
```

**Serial Monitor Output** (DEBUG mode):
```
Heartbeat - ACTIVE (last activity 1240ms ago)
MIDI: Button 2 pressed -> Note 62 ON
MIDI: Pot 0 changed -> CC 1 = 64
```

---

## Performance & Timing

### Real-Time Guarantees

**Target Timing**:
- Main loop: 1000 Hz (1000μs period) → **Target: <1000μs per iteration**
- Portal frame: 60 Hz (16667μs period) → **Target: <16000μs per frame**
- OLED update: 20 Hz (50ms period) → **Target: <10ms per update**

**Typical Measured Values**:
- Main loop: 200-400μs (20-40% CPU utilization)
- Portal frame: 2000-5000μs (12-30% of frame budget)
- OLED update: 5-8ms (10-16% of update period)

**Worst-Case Scenarios**:
- Main loop with all inputs changing: ~800μs
- Portal frame with 3 ripples + flash: ~8000μs
- OLED update during mode switch: ~12ms

### Memory Usage

**Static Allocation** (approximate):
```
Global Variables:
├── leds[45]               →   135 bytes (CRGB = 3 bytes each)
├── InputScanner           →   100 bytes (state arrays)
├── RobustInputProcessor   →   600 bytes (debouncers + smoothers)
├── PortalController       →   200 bytes (animation state)
├── OledDisplay            →   500 bytes (frame buffer + log)
└── Serial buffers         →    64 bytes
                           ─────────────
Total RAM:                  ~1600 bytes (~0.15% of 1MB)
```

**Flash Usage**:
```
Code:                       ~80 KB
Libraries (FastLED, etc.):  ~120 KB
                           ──────
Total:                      ~200 KB (~2.5% of 8MB)
```

**No Dynamic Allocation**: All arrays are fixed-size, no `malloc()`/`new`

---

### Optimization Techniques

**1. Fixed-Point Math**:
```cpp
// Instead of: filtered = alpha * input + (1-alpha) * prev;
// Use: filtered += (input - filtered) >> 2;  // α ≈ 0.25
```

**2. Lookup Tables**:
```cpp
constexpr uint8_t BUTTON_NOTES[] = {60, 61, 62, ...};  // Compile-time
constexpr uint8_t POT_CCS[] = {1, 2, 3, 4};
```

**3. Early Exits**:
```cpp
if (!processor.getPotChanged(i)) continue;  // Skip unchanged pots
if (!flashActive && !anyRipples) return;    // Skip effect processing
```

**4. Conditional Compilation**:
```cpp
#ifdef USB_MIDI
    usbMIDI.sendNoteOn(...);  // Only compile if MIDI enabled
#endif

#if DEBUG >= 2
    Serial.printf("Debug: ...");  // Omit in release builds
#endif
```

**5. Rate Limiting**:
```cpp
// Pot changes: max 67/second (every 15ms)
// OLED updates: 20 Hz (every 50ms)
// Test dumps: 0.2 Hz (every 5 seconds)
```

---

## Configuration Management

### Compile-Time Configuration (include/config.h)

All tunable parameters are in one file:

```cpp
// Timing
#define SCAN_HZ 1000                    // Input scan rate
#define DEBOUNCE_MS 5                   // Button debounce time
#define POT_RATE_LIMIT_MS 15            // Pot change rate
#define IDLE_TIMEOUT_MS 30000           // Idle detection

// LED
#define LED_BRIGHTNESS_MAX 160          // Max brightness (0-255)
#define IDLE_BRIGHTNESS_CAP_PCT 15      // Idle mode brightness

// Debug
#define DEBUG 1                         // 0=off, 1=basic, 2=verbose

// MIDI
constexpr uint8_t MIDI_CHANNEL = 1;
constexpr uint8_t MIDI_VELOCITY = 100;
```

**Benefits**:
- Single source of truth
- Easy to tune without code changes
- Compile-time optimization
- Self-documenting

### Runtime Configuration (Future: EEPROM)

**Planned** (not yet implemented):
```cpp
struct Config {
    uint8_t debounceMs[BUTTON_COUNT];   // Per-button tuning
    uint16_t potMin[POT_COUNT];         // Calibrated ranges
    uint16_t potMax[POT_COUNT];
    uint8_t ledBrightness;
    uint8_t version;                    // Config migration
};
```

---

## Build System (PlatformIO)

### Environments (platformio.ini)

**Production** (MIDI mode):
```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
build_flags = -D USB_MIDI        # Enable MIDI, Serial unavailable in monitor
lib_deps = 
    fastled/FastLED@^3.6.0
    adafruit/Adafruit SSD1306@^2.5.0
    adafruit/Adafruit GFX Library@^1.11.0
```

**Debug** (Serial mode):
```ini
[env:teensy41-debug]
platform = teensy
board = teensy41
framework = arduino
build_flags = -D DEBUG=2         # No USB_MIDI, full Serial debugging
lib_deps = <same as above>
monitor_speed = 115200
```

### Build Commands

```bash
# Production build & upload (MIDI mode)
pio run -e teensy41 --target upload

# Debug build & upload + monitor (Serial mode)
pio run -e teensy41-debug --target upload
pio device monitor -e teensy41-debug

# Run tests
pio test

# Clean build
pio run --target clean
```

---

## Directory Structure Summary

```
teensy-firmware/
├── platformio.ini          # Build configuration
├── Makefile                # Convenience shortcuts
│
├── include/                # Header files (interfaces)
│   ├── config.h            # Compile-time configuration
│   ├── pins.h              # Pin assignments
│   ├── input_scanner.h     # Raw I/O layer
│   ├── debouncer.h         # Signal conditioning
│   ├── analog_smoother.h   # Signal conditioning
│   ├── robust_input_processor.h  # Unified input API
│   ├── midi_out.h          # MIDI transmission
│   ├── robust_midi_mapper.h      # Input → MIDI mapping
│   ├── portal_controller.h       # Animation engine
│   ├── portal_cue_handler.h      # Pi/MIDI portal control
│   ├── serial_portal_protocol.h  # Binary protocol
│   └── oled_display.h            # Display controller
│
├── src/                    # Implementation files
│   ├── main.cpp            # Entry point & main loop
│   ├── input_scanner.cpp
│   ├── debouncer.cpp
│   ├── analog_smoother.cpp
│   ├── robust_input_processor.cpp
│   ├── midi_out.cpp
│   ├── robust_midi_mapper.cpp
│   ├── portal_controller.cpp
│   ├── portal_cue_handler.cpp
│   └── oled_display.cpp
│
├── test/                   # Unit tests
│   ├── test_debouncer.cpp
│   ├── test_binary_switches.cpp
│   ├── test_oled_display.cpp
│   ├── test_serial_protocol.cpp
│   └── test_pi_serial.py
│
└── docs/                   # Documentation
    ├── ARCHITECTURE.md     # This file
    ├── AGENTS.md           # AI agent development guide
    ├── TeensySoftwareRoadmap.md  # Development phases
    ├── OLEDDisplay.md      # OLED setup guide
    ├── BinarySwitchFeature.md    # Switch encoding
    └── RaspberryPi_Integration_Guide.md
```

---

## Common Development Tasks

### Adding a New Animation Program

**1. Define enum** (include/config.h):
```cpp
enum PortalProgram {
    PORTAL_SPIRAL = 0,
    // ... existing ...
    PORTAL_MY_NEW_ANIMATION = 10,  // Add here
};
constexpr uint8_t PORTAL_PROGRAM_COUNT = 11;  // Update count
```

**2. Add update method** (include/portal_controller.h):
```cpp
private:
    void updateMyNewAnimation();
```

**3. Implement** (src/portal_controller.cpp):
```cpp
void PortalController::updateMyNewAnimation() {
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        // Your animation logic here
        leds[i] = CHSV(hue, 255, 255);
    }
}

// Add to switch statement in update()
void PortalController::update() {
    switch (currentProgram) {
        // ... existing ...
        case PORTAL_MY_NEW_ANIMATION: updateMyNewAnimation(); break;
    }
}
```

**4. Add name** (src/portal_cue_handler.cpp):
```cpp
const char* PORTAL_PROGRAM_NAMES[] = {
    "SPIRAL", "PULSE", "WAVE", ..., "MY_NEW_ANIMATION"
};
```

---

### Adding a New MIDI CC Mapping

**1. Define CC number** (include/config.h):
```cpp
constexpr uint8_t MY_NEW_CONTROL_CC = 70;
```

**2. Add processing** (src/robust_midi_mapper.cpp):
```cpp
void RobustMidiMapper::processMyNewControl() {
    if (processor_.getControlChanged()) {
        uint8_t value = processor_.getControlValue();
        midiOut_.sendControlChange(MY_NEW_CONTROL_CC, value, MIDI_CHANNEL);
    }
}

// Call from processInputs()
void RobustMidiMapper::processInputs() {
    processButtons();
    processPots();
    processMyNewControl();  // Add here
}
```

---

### Adding a New Serial Command

**1. Define command** (include/serial_portal_protocol.h):
```cpp
enum class PortalSerialCommand : uint8_t {
    // ... existing ...
    MY_NEW_COMMAND = 0x08,
};
```

**2. Handle command** (src/portal_cue_handler.cpp):
```cpp
void PortalCueHandler::handleSerialMessage(const PortalMessage& msg) {
    switch (msg.command) {
        // ... existing ...
        case PortalSerialCommand::MY_NEW_COMMAND:
            // Process command
            portalController->doSomething(msg.value);
            sendAck();
            break;
    }
}
```

---

## Troubleshooting Guide

### No MIDI Output

**Symptoms**: DAW doesn't see Teensy device

**Checks**:
1. Verify USB mode: `platformio.ini` must have `-D USB_MIDI`
2. Check cable (data, not charge-only)
3. Try different USB port
4. Verify in Arduino IDE: Tools → USB Type → MIDI

---

### Serial Monitor Not Working (MIDI Mode)

**This is normal!** MIDI mode disables PlatformIO serial monitor.

**Solutions**:
1. Use debug build: `pio run -e teensy41-debug --target upload`
2. OR use Arduino IDE Serial Monitor (works with MIDI mode)

---

### LEDs Not Working

**Checks**:
1. Verify LED_DATA_PIN connection (Pin 1)
2. Check power supply (WS2812B needs 5V, high current)
3. Verify LED_COUNT matches your strip (45 by default)
4. Check if portal startup sequence runs (should see animations)

---

### OLED Not Initializing

**Symptoms**: "OLED initialization failed" in Serial

**Checks**:
1. Verify I2C address (0x3C default, check with I2C scanner)
2. Check SDA/SCL pins (18/19 on Teensy 4.1)
3. Verify 3.3V/5V power (SSD1306 supports both)
4. Pull-up resistors (4.7kΩ recommended, some modules have built-in)

**Note**: Firmware continues without OLED (graceful degradation)

---

### Input Values Jittery

**Pot noise**: 
- Increase `POT_DEADBAND` (default 2)
- Increase `POT_RATE_LIMIT_MS` (default 15)
- Check for power supply noise

**Button bounce**:
- Increase `DEBOUNCE_MS` (default 5)
- Check for floating pins (should have pullups)

---

## Appendix: Key Constants Reference

### Timing
```cpp
AUTO_SWITCH_INTERVAL = 60000      // Portal program rotation interval for demo mode (ms)
SCAN_HZ = 1000                    // Input scan rate (Hz)
PORTAL_FPS = 60                   // LED frame rate (Hz)
OLED_UPDATE_HZ = 20               // Display refresh (Hz)
DEBOUNCE_MS = 5                   // Button debounce (ms)
POT_RATE_LIMIT_MS = 15            // Pot update rate (ms)
JOYSTICK_REARM_MS = 120           // Joystick anti-rapid-fire (ms)
IDLE_TIMEOUT_MS = 30000           // Idle detection (ms)
```

### Pin Assignments
```cpp
BUTTON_PINS = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11}
POT_PINS = {A0, A1, A2, A3}
JOYSTICK_PINS = {33, 34, 35, 36}  // Up, Down, Left, Right
SWITCH_PINS = {22, 23, 24, 25, 26, 27, 28, 29, 16, 17, 18, 21}
LED_DATA_PIN = 1
I2C_SDA = 18, I2C_SCL = 19
```

### MIDI Mapping
```cpp
Buttons:   Note 60-69 (C4-A4), velocity 100, channel 1
Pots:      CC 1-4, value 0-127, channel 1
Joystick:  CC 10-13 (Up/Down/Left/Right), value 127, channel 1
Switches:  CC 20-31 (individual), CC 50 (binary), channel 1
Portal:    CC 60-66 (legacy), channel 1
```

### Memory Limits
```cpp
LED_COUNT = 45                    // Portal LED count
BUTTON_COUNT = 10
POT_COUNT = 4
SWITCH_COUNT = 12
MIDI_LOG_SIZE = 8                 // OLED MIDI history
```

---

## Revision History

| Version | Date       | Changes                                      |
|---------|------------|----------------------------------------------|
| 1.0     | 2025-11-11 | Initial architecture guide                   |

---

*For detailed development guidelines, see [AGENTS.md](../AGENTS.md)*
*For roadmap and future features, see [TeensySoftwareRoadmap.md](TeensySoftwareRoadmap.md)*
