# Mystery Melody Machine - Teensy Firmware Specification

## Project Overview

### Purpose
The Mystery Melody Machine is an interactive musical device that combines physical controls with generative music algorithms. The Teensy 4.1 firmware serves as the real-time hardware interface layer, handling:
- Low-latency input scanning and debouncing (≤3ms worst case)
- MIDI event generation and transmission
- RGB LED "infinity portal" animations
- Communication with Raspberry Pi for high-level control

### Architecture Philosophy
- **Real-time first**: Prioritize deterministic timing and low-latency response
- **Modular design**: Clean separation of concerns with well-defined interfaces
- **Data-driven configuration**: Avoid hard-coded magic numbers, use configurable constants
- **Memory conscious**: Static allocation preferred, avoid dynamic memory in hot paths
- **Fail-safe operation**: Graceful degradation and error recovery

---

## Hardware Specifications

### Target Platform
- **MCU**: Teensy 4.1 (IMXRT1062, 600MHz ARM Cortex-M7)
- **Memory**: 1MB RAM, 8MB Flash
- **USB Type**: MIDI (with Serial debugging capability)

### I/O Configuration
```cpp
// Digital Inputs (with pullups)
Buttons:     Pins 2-11   (10 buttons)
Joystick:    Pins 12-15  (4 directions: Up/Down/Left/Right)
Switches:    Pins 16-18  (3 switches)

// Analog Inputs
Potentiometers: A0-A3, A6-A7 (6 pots, A4/A5 reserved for I2C)

// Outputs
LED Data:    Pin 1       (WS2812B strip, 60 LEDs default)
Built-in LED: Pin 13     (heartbeat indicator)
```

### MIDI Mapping
```cpp
Channel: 1 (primary), Channel 2 (reserved)
Buttons: Notes 60-69 (C4-A4), Velocity 100
Pots:    CC 1-6
Joystick: CC 10-13 (Up/Down/Left/Right)
Switches: CC 20-22
```

---

## Software Architecture

### Core Modules

#### 1. Input System
- **InputScanner**: Raw GPIO/ADC reading at 1kHz
- **Debouncer**: Time-based debouncing (5ms default, configurable per input)
- **AnalogSmoother**: EMA filtering + deadband for pots (α=0.25, deadband=2)

#### 2. MIDI System
- **MidiOut**: Encapsulates usbMIDI calls with duplicate filtering
- **Mapping**: Data-driven physical→MIDI mapping tables

#### 3. Portal Animation System
- **PortalController**: Main animation engine (60Hz target)
- **PortalCues**: Pi→Teensy command handling
- Programs: spiral, pulse, wave, chaos, ambient, idle

#### 4. Timing & Diagnostics
- **Scheduler**: Fixed 1ms timestep main loop
- **Diagnostics**: Rate-limited serial output, performance monitoring

### Main Loop Structure
```cpp
void loop() {
  // 1kHz main scan
  if (mainLoopTimer >= 1000) {
    scanInputs();              // Read all hardware
    processButtons();          // Debounce & MIDI notes
    processJoystick();         // Edge detection
    processSwitches();         // State changes
    processPots();             // Smoothing & CC
    updateIdleTimer();         // Activity tracking
    handlePortalCues();        // Pi commands
  }
  
  // 60Hz portal rendering
  if (portalTimer >= 16667) {
    portalController.update();
    FastLED.show();
  }
}
```

---

## Development Guidelines

### PlatformIO Best Practices

#### Project Structure
```
teensy-firmware/
├── platformio.ini         # Environment configuration
├── src/                   # Source code
│   └── main.cpp          # Main application
├── include/              # Header files
│   ├── pins.h           # Pin definitions
│   └── config.h         # Configuration constants
├── test/                 # Unit tests
├── lib/                  # Project-specific libraries
└── docs/                # Documentation
```

#### Environment Configuration
```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
build_flags = 
    -D USB_MIDI              # USB device type
    -D DEBUG=1               # Debug level (0-2)
    -D SCAN_HZ=1000         # Main loop frequency
lib_deps = 
    fastled/FastLED@^3.6.0  # Pin to major version
monitor_speed = 115200
```

#### Build Flags Philosophy
- Use `#define` constants for compile-time configuration
- Separate debug/release builds with different optimization levels
- Pin library versions to avoid breaking changes
- Use meaningful flag names that self-document

#### Memory Management
- **Prefer static allocation**: Use fixed-size arrays and buffers
- **Avoid heap allocation in main loop**: No `malloc()`, `new`, or dynamic containers
- **Monitor memory usage**: Use `teensy_size` output to track consumption
- **Use `constexpr`**: For compile-time constants and lookup tables

### Test-Driven Development (TDD)

#### Testing Philosophy
1. **Write tests first**: Define expected behavior before implementation
2. **Test at module level**: Each module should have focused unit tests
3. **Integration tests**: Verify module interactions
4. **Hardware-in-the-loop**: Test timing and real-world behavior

#### Testing Structure
```cpp
// test/test_debouncer.cpp
#include <unity.h>
#include "debouncer.h"

void test_debouncer_initial_state() {
    Debouncer db(5); // 5ms debounce
    TEST_ASSERT_FALSE(db.isPressed());
    TEST_ASSERT_FALSE(db.justPressed());
}

void test_debouncer_press_sequence() {
    Debouncer db(5);
    
    // Simulate button press
    db.update(false, 0);     // Press at t=0
    TEST_ASSERT_FALSE(db.justPressed()); // Too early
    
    db.update(false, 6);     // Still pressed at t=6ms
    TEST_ASSERT_TRUE(db.justPressed());  // Should register
}
```

#### Hardware Testing Guidelines
- **Use test modes**: Compile-time flags for deterministic input simulation
- **Timing validation**: Measure actual loop times, not just target times
- **Boundary conditions**: Test edge cases (max rates, stuck buttons, etc.)
- **Soak testing**: Multi-hour runs to verify stability

#### Mock Strategy
```cpp
// For testing without hardware
class MockInputScanner : public InputScanner {
public:
    void setButtonState(int button, bool pressed) { /* simulate */ }
    void setPotValue(int pot, int value) { /* simulate */ }
};
```

### Code Quality Standards

#### Naming Conventions
```cpp
// Constants: UPPER_SNAKE_CASE
constexpr uint8_t BUTTON_COUNT = 10;
constexpr uint32_t SCAN_INTERVAL_US = 1000;

// Functions: camelCase
void scanInputs();
bool isButtonPressed(uint8_t button);

// Classes: PascalCase
class PortalController;
class InputScanner;

// Variables: camelCase
uint32_t lastScanTime;
bool buttonPressed[BUTTON_COUNT];
```

#### Documentation Standards
```cpp
/**
 * @brief Debounces a digital input signal
 * 
 * Uses time-based debouncing with configurable window.
 * Call update() every scan cycle with current input state.
 * 
 * @param debounceMs Minimum stable time required (typically 5-10ms)
 */
class Debouncer {
public:
    /**
     * @brief Update debouncer with current input state
     * @param currentState Raw input reading (true = active)
     * @param timestampMs Current system time in milliseconds
     * @return true if state change occurred
     */
    bool update(bool currentState, uint32_t timestampMs);
};
```

#### Error Handling
```cpp
// Use return codes for recoverable errors
enum class ScanResult {
    Success,
    AdcTimeout,
    InvalidPin
};

ScanResult scanPot(uint8_t potIndex, uint16_t& value) {
    if (potIndex >= POT_COUNT) {
        return ScanResult::InvalidPin;
    }
    // ... implementation
}

// Use assertions for programming errors
void processButton(uint8_t buttonIndex) {
    assert(buttonIndex < BUTTON_COUNT);
    // ... implementation
}
```

### Performance Guidelines

#### Timing Requirements
- **Main loop**: ≤1000μs worst case (1kHz target)
- **Portal frame**: ≤16.6ms (60Hz target)
- **Input→MIDI latency**: ≤3ms typical, ≤10ms worst case
- **Memory**: <100KB RAM usage, <500KB flash

#### Optimization Strategies
```cpp
// Use lookup tables instead of calculations
constexpr uint8_t MIDI_NOTES[BUTTON_COUNT] = {
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69
};

// Minimize floating point in hot paths
// Instead of: float filtered = alpha * input + (1-alpha) * previous;
// Use fixed point: filtered += (input - filtered) >> 2; // α ≈ 0.25

// Batch operations
void scanAllInputs() {
    // Read all buttons in one call instead of individual digitalRead()
    uint32_t portState = GPIO_PSR(gpio_pin_to_port(BUTTON_PINS[0]));
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttonStates[i] = !(portState & gpio_pin_to_mask(BUTTON_PINS[i]));
    }
}
```

#### Profiling & Monitoring
```cpp
// Built-in performance monitoring
#if DEBUG >= 2
static uint32_t maxLoopTime = 0;
static uint32_t loopCount = 0;

void profileMainLoop() {
    uint32_t startTime = micros();
    // ... main loop work ...
    uint32_t loopTime = micros() - startTime;
    
    maxLoopTime = max(maxLoopTime, loopTime);
    if (++loopCount % 10000 == 0) {
        Serial.printf("Loop stats: max=%luus, avg=%luus\n", 
                     maxLoopTime, totalTime/loopCount);
    }
}
#endif
```

---

## Development Workflow

### Phase-Based Development
Following the established roadmap phases:
1. **Phase 0**: Bootstrap (✅ Complete)
2. **Phase 1**: Raw input + MIDI
3. **Phase 2**: Robust input layer
4. **Phase 3**: Portal integration
5. **Phase 4**: Performance hardening
6. **Phase 5**: Diagnostics & safety

### Git Workflow
```bash
# Feature branches for each phase
git checkout -b phase-1-raw-input
# ... implement and test ...
git commit -m "Phase 1: Implement raw MIDI input scanning"

# Tag releases
git tag -a v0.1.0 -m "Phase 1 complete: Raw input + MIDI"
```

### Testing Workflow
```bash
# Run tests before committing
pio test

# Build for different configurations
pio run -e teensy41-debug
pio run -e teensy41-release

# Upload and monitor
pio run --target upload
pio device monitor
```

### Documentation Updates
- Update `CHANGELOG.md` for each phase completion
- Maintain `README.md` with current setup instructions
- Document API changes in code comments
- Update this `SPEC.md` when architecture evolves

---

## Integration Patterns

### Pi↔Teensy Communication
```cpp
// Portal cue protocol (via Serial or MIDI SysEx)
struct PortalCue {
    enum Type : uint8_t {
        SET_PROGRAM = 0x01,    // Switch animation program
        SET_BPM = 0x02,        // Sync to sequencer BPM
        SET_INTENSITY = 0x03,  // Activity level
        SET_HUE = 0x04         // Color shift
    };
    
    Type type;
    uint8_t value;
    uint16_t checksum;
};
```

### Configuration Management
```cpp
// Runtime configuration (stored in EEPROM future phase)
struct Config {
    uint8_t debounceMs[BUTTON_COUNT];     // Per-button debounce
    uint16_t potMin[POT_COUNT];           // Calibrated ranges
    uint16_t potMax[POT_COUNT];
    uint8_t ledBrightness;                // Global brightness
    uint8_t version;                      // Config version
};
```

### Error Recovery
```cpp
// Watchdog patterns
void setup() {
    // Enable watchdog timer
    wdt_enable(WDTO_500MS);
}

void loop() {
    wdt_reset(); // Reset watchdog each loop
    
    // Detect stuck states
    if (millis() - lastInputActivity > STUCK_TIMEOUT) {
        sendAllNotesOff();
        lastInputActivity = millis();
    }
}
```

---

## AI Agent Guidelines

### When Extending This Codebase

1. **Read the roadmap first**: Check `docs/TeensySoftwareRoadmap.md` for context
2. **Follow the phase structure**: Don't skip ahead, build incrementally
3. **Maintain timing guarantees**: Profile any changes that affect main loop
4. **Test thoroughly**: Write tests before implementation, verify on hardware
5. **Update documentation**: Keep SPEC.md, README.md, and CHANGELOG.md current

### Code Review Checklist

- [ ] Does it maintain <1ms main loop timing?
- [ ] Are constants configurable via `config.h`?
- [ ] Is memory allocation static (no dynamic allocation in loop)?
- [ ] Are error conditions handled gracefully?
- [ ] Do tests cover the new functionality?
- [ ] Is the code documented with clear comments?
- [ ] Does it follow the established naming conventions?

### Common Pitfalls to Avoid

1. **Blocking operations in main loop**: Use state machines instead
2. **Floating point in hot paths**: Use fixed-point arithmetic
3. **Dynamic memory allocation**: Prefer static buffers
4. **Magic numbers**: Use named constants
5. **Tight coupling**: Maintain clean module interfaces
6. **Inadequate testing**: Test timing, edge cases, and error conditions

### Useful PlatformIO Commands for AI Agents

```bash
# Get project information
pio project data

# Check for issues
pio check

# Clean build artifacts
pio run --target clean

# Build with verbose output
pio run -v

# Upload with specific port
pio run --target upload --upload-port /dev/ttyACM0

# Run specific test
pio test -f test_debouncer

# Generate documentation
pio run --target docs
```

---

## Version History & Migration

### Current Version: 0.0.1 (Phase 0)
- Basic firmware skeleton
- MIDI enumeration
- Portal startup sequence
- Heartbeat LED

### Planned Migrations
- **v0.1.0**: Full input scanning (Phase 1)
- **v0.2.0**: Debouncing and smoothing (Phase 2)
- **v0.3.0**: Complete portal integration (Phase 3)
- **v1.0.0**: Production-ready with full feature set

### Breaking Changes Policy
- Major version increments for incompatible changes
- Maintain backward compatibility within minor versions
- Document migration path in CHANGELOG.md
- Provide upgrade utilities where necessary

---

*This specification should be treated as a living document and updated as the project evolves. AI agents should refer to this document for context and constraints when making modifications to the codebase.*
