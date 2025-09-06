# Teensy Firmware Software Roadmap

Target MCU: Teensy 4.1  (USB Type: MIDI)
Primary Role: Scan physical controls, generate debounced + smoothed event state, emit MIDI (Notes + CC), drive RGB LED infinity portal animations using pre-existing portal code, receive portal animation cues from Pi, expose diagnostic + configuration hooks.
Stretch Role: Light-weight rule hooks (ONLY if round‑trip to Pi causes unacceptable latency for a subset of interactions).

---
## 1. Goals & Non‑Goals
### Core Goals
- Reliable low‑latency input → MIDI event pipeline (≤3 ms scan → send typical worst case)
- Integrate pre-existing infinity portal animation code with program switching and BPM sync
- Portal animation responsiveness to physical interactions and Pi cues
- Stable over multi‑hour soak (memory, timing, heat)
- Easily extensible mapping layer (table/data‑driven vs. hard coded switch blocks)
- Resilient to switch bounce & pot jitter
- Simple field debugging (serial diagnostics, optional MIDI debug CC channel)

### Non‑Goals (Handled on Pi)
- Generative logic, scale/pitch selection, probability engine
- Long‑term state persistence (beyond optional calibration constants)
- External synth synthesis (Pi routes MIDI to hardware synths)
- Complex portal scene choreography (Pi sends high-level animation cues)

---
## 2. High‑Level Architecture
```
+------------------+
|   Main Loop      |
|  (1 kHz tick)    |
+---------+--------+
          |
          v
+------------------+     +------------------+     +------------------+
| Input Sampler     |--> | State Manager     |--> | MIDI Dispatcher   |
| (GPIO/ADC read)   |    | (debounce, delta) |    | (usbMIDI.* calls) |
+------------------+     +------------------+     +------------------+
          |                                              ^
          v                                              |
+------------------+       +--------------------+        |
| (pots smoothing) |       | (Notes/CC cfg)     |--------+
+------------------+       +--------------------+
          |
          v
+------------------+       +--------------------+
| Portal Controller |<----> | Portal Cue Handler |
| (animation progs) |       | (Pi → Teensy cues) |
+------------------+       +--------------------+
          |
          v
+------------------+
| Diagnostics (USB) |
+------------------+
```

---
## 3. Module Breakdown
| Module | Responsibility | Key Techniques | Deliverables |
|--------|----------------|----------------|--------------|
| `pins.h` | Central pin & message maps | `constexpr` arrays | Header file |
| `InputScanner` | Periodic read of digital & analog | Direct register read (optional), bounce window | `scan()` function |
| `Debouncer` | Edge detection for buttons/joystick/switches | Time-based (e.g., 5–8 ms) | Utility class |
| `AnalogSmoother` | Pot noise reduction & 10-bit→7-bit mapping | Exponential / median filter + deadband | `filter(channel)` |
| `Mapping` | Physical index → MIDI note/CC/channel | Tables; easily edited | Config struct + arrays |
| `MidiOut` | Encapsulation of usbMIDI sends | Throttle identical repeats | `sendNoteOn/Off`, `sendCC` |
| `PortalFx` | Animation programs: spiral, pulse, wave, chaos, ambient, idle | Pre-existing portal code integration | `updateProgram()`, `setBpm()` |
| `PortalCues` | Receive animation cues from Pi | Serial/MIDI message parsing | `handleCue()`, `switchProgram()` |
| `Scheduler` | Fixed timestep tick (1 ms or 2 ms) | `elapsedMicros` | Main loop timing |
| `Diagnostics` | Serial prints & optional CC echo | Rate-limited | `dumpState()` |
| `ConfigStore` (future) | (Optional) store calibration | EEPROM / LittleFS | Accessors |

---
## 4. Development Phases & Milestones

### Phase 0: Bootstrap
- [X] Create `teensy/firmware/` structure
- [X] Basic blink / Serial Hello
- [X] Confirm USB Type = MIDI enumerates

### Phase 1: Raw Input + MIDI
- [X] Define pin & mapping tables
- [X] Poll buttons → send NoteOn/NoteOff (no debounce)
- [X] Poll pots → send CC on value change (raw 0–127 mapping)
- [X] Poll joystick & switches → send CC (edge triggered naive)

### Phase 2: Robust Input Layer
- [X] Implement time-based debounce (per control configurable)
- [X] Add analog smoothing: EMA (α ≈ 0.25) + deadband (Δ≥2 -> send)
- [X] Implement change compression (only send CC after stable for 4 ms OR threshold exceeded)
- [X] Unit-like serial test mode: dump values each second

### Phase 3: Portal Animation Integration
- [ ] Integrate pre-existing infinity portal code from `/Users/oberon/Projects/coding/arduino/uno/arduino-infinity-portal`
- [ ] Implement portal animation programs: spiral, pulse, wave, chaos, ambient, idle
- [ ] Add portal cue handler for receiving Pi commands (program switch, BPM sync, intensity)
- [ ] Button press visual feedback within current portal program
- [ ] Pot activity visual feedback (color/intensity shifts)
- [ ] Idle detector (≥30 s no events) → automatic switch to ambient/idle portal program

### Phase 4: Performance Hardening
- [ ] Measure loop time histogram (micros min/avg/max over 10k cycles)
- [ ] Ensure worst-case loop < 1000 µs (if 1 kHz target)
- [ ] Replace slow operations (avoid floating point in hot path where possible)
- [ ] Add conditional compilation `#define DEBUG 0`

### Phase 5: Diagnostics & Safety
- [ ] Rate-limited serial debug command handler (list: `?` help)
- [ ] Runtime toggle of portal brightness / debug prints
- [ ] MIDI panic handler (all notes off) bound to a hidden combo
- [ ] Portal animation diagnostic mode (cycle through all programs)

### Phase 6: Refinements / Polish
- [ ] Per-input configurable debounce via table
- [ ] Optional calibration (store pot min/max into EEPROM)
- [ ] Portal animation theming (configurable color palettes)
- [ ] Add version string & semantic version bump policy
- [ ] Portal BPM synchronization fine-tuning

### Stretch (Future)
- [ ] Lightweight scripting hook (e.g., simple rule DSL for portal tie-ins) – only if needed
- [ ] USB Vendor SysEx channel for remote config from Pi
- [ ] Boot self-test (portal animation sequence, read each input once)
- [ ] Portal cross-fade between programs for smooth transitions

---
## 5. Timing & Rates
| Element | Target | Notes |
|---------|--------|-------|
| Main scan tick | 1 kHz (1 ms) | Enough for responsive buttons & portal updates |
| Portal refresh | 60–100 Hz | Decouple from scan using modulus (e.g., every 10th tick) |
| Pot CC emission | ≤20 Hz per channel when moving | Apply deadband & min interval (e.g., 15 ms) |
| Idle detection window | 30,000 ms | Configurable constant |
| Portal BPM sync | 1–4 Hz typical | Synced to Pi sequencer BPM via cues |

---
## 6. Debounce & Filtering Strategy
- Digital (buttons/joystick/switches): store last stable state + timestamp; require `stableFor >= DEBOUNCE_MS` (default 5 ms) before signaling change.
- Analog (pots):
  - Raw 10-bit read (0–1023)
  - EMA: `filtered = filtered + α*(raw - filtered)`
  - Quantize to 0–127 after filtering
  - Deadband: ignore if |new - lastSent| < 2 (tunable)
  - Rate limit: enforce ≥15 ms between sends unless delta ≥8

---
## 7. Data Structures (Proposed)
```cpp
struct ButtonCfg { uint8_t pin; uint8_t note; };
struct PotCfg { uint8_t pin; uint8_t cc; };
struct AxisCfg { uint8_t pin; uint8_t cc; };
struct SwitchCfg { uint8_t pin; uint8_t cc; };

extern const ButtonCfg BUTTONS[10];
extern const PotCfg    POTS[6];
extern const AxisCfg   JOY[4];
extern const SwitchCfg SWITCHES[3];
```

Runtime state arrays mirror configs for last raw, filtered, and timing metadata.

---
## 8. Portal Animation System (Replacing LED Pattern System)
Portal Programs: Using pre-existing infinity portal code with 6 main animation programs:

**Core Animation Programs:**
- `spiral`: Rotating spiral patterns with configurable direction and speed
- `pulse`: Rhythmic pulsing synchronized to BPM from Pi
- `wave`: Flowing wave patterns that respond to activity level
- `chaos`: Random/chaotic patterns triggered by mutation events
- `ambient`: Slow, peaceful patterns for background ambiance
- `idle`: Minimal ambient mode with very low brightness (≤15% of max)

**Portal Control Interface:**
```cpp
class PortalController {
  void setProgram(uint8_t program_id);
  void setBpmRate(float bpm);
  void setIntensity(float intensity);  // 0.0-1.0
  void setColorHue(float hue);         // 0.0-1.0 hue shift
  void triggerFlash();                 // Button press feedback
  void update();                       // Called from main loop
};
```

**Pi → Teensy Portal Cues:**
- Program switching commands
- BPM synchronization values
- Intensity/activity level updates
- Color palette adjustments
- Idle mode enter/exit commands

**Integration with Controls:**
- Button presses trigger visual flashes within current program
- Pot movements create temporary color/intensity shifts
- Switch changes may trigger program transitions
- Idle detection (30s) automatically switches to ambient/idle programs

**Brightness Policy:**
- Global max brightness limited (LED_BRIGHTNESS_MAX = 160 out of 255)
- Idle brightness cap: 15% of max (≈24) to reduce glare
- Dynamic brightness scaling based on activity and time of day

---
## 9. MIDI Policy
- Channel: 1 (single channel sufficient). Channel 2 reserved future.
- Notes: 60–71 fixed velocity 100 (constant for v1; velocity variation deferred).
- CC: 7-bit standard values. Debounced / smoothed. Deadband + rate limit to keep within latency budget.
- Joystick: Single 127 pulse (edge) per direction actuation (no trailing 0) — firmware enforces a minimum re-arm time to avoid chatter.
- Switches: Send 127 on ON edge, 0 on OFF edge (latched state needed for mode).
- Panic: Send NoteOff for 60–71 if error condition or explicit command.

---
## 10. Error Handling & Fault Modes
| Fault | Detection | Response |
|-------|-----------|----------|
| Stuck button (never releases 5 s) | Timer since press | Force note-off + mark flagged in diagnostics |
| ADC noise storm | Excessive jitter detections | Temporarily increase smoothing α or lock channel |
| Portal driver hang | Frame time > 5 ms repeatedly | Switch to simpler program / reduce complexity |
| USB MIDI stall | Queue full (rare) | Skip low-priority CC until queue clears |
| Portal cue timeout | No Pi cues for >60s | Switch to autonomous mode, continue animations |

---
## 11. Testing Plan
### Bench Tests
- Button bounce capture (logic analyzer or serial timestamps) to validate debounce constant
- Pot sweep linearity & jitter (log min/max, std dev at rest)
- Loop timing (collect 10k cycles, compute worst-case)

### Automated (Lightweight)
- Add a compile-time `#define TEST_MODE` that simulates deterministic input patterns and asserts transitions via serial output (manually verified first, could parse on Pi later).

### Soak
- 6 hr run with periodic scripted MIDI input simulation (optional) verifying no memory growth (inspect free RAM) & no stuck states.

---
## 12. Configuration & Build Flags
```cpp
#define SCAN_HZ 1000
#define DEBOUNCE_MS 5
#define POT_DEADBAND 2            // ±2 -> ignore; tuned for <=20 ms latency target
#define POT_RATE_LIMIT_MS 15       // ensures worst-case <20 ms perceived response
#define IDLE_TIMEOUT_MS 30000
#define DEBUG 0
#define LED_BRIGHTNESS_MAX 160     // global max (out of 255)
#define IDLE_BRIGHTNESS_CAP_PCT 15 // percent of max during idle ambient
#define JOYSTICK_REARM_MS 120      // min time between pulses per direction
```
Use `#if DEBUG` blocks for serial prints to keep hot path lean.

---
## 13. Memory & Performance Budget (Estimates)
| Item | Estimate |
|------|----------|
| State arrays (inputs, metadata) | < 2 KB |
| Portal buffer (pre-existing code) | < 1 KB (varies by program complexity) |
| Stack (main) | < 4 KB typical |
| Total headroom (Teensy 4.1 has 1 MB RAM) | Abundant |

CPU: Input + Portal + MIDI expected << 10% @ 600 MHz.

---
## 14. Versioning & Release Process
- Semantic style: MAJOR.MINOR.PATCH (e.g. 0.1.0 initial public firmware)
- Tag builds after soak test pass: `teensy-v0.1.0`
- Maintain `CHANGELOG.md` in `teensy/`

---
## 15. Risks & Mitigations
| Risk | Impact | Mitigation |
|------|--------|------------|
| Excessive CC spam | MIDI congestion / Pi CPU | Deadband + rate limit |
| Portal animation complexity | CPU overload / frame drops | Adaptive complexity reduction, simpler fallback programs |
| Future mapping changes require flash | Slow iteration | Keep remap on Pi where possible; only static physical indexes in firmware |
| Portal cue communication failure | Visual-musical desync | Autonomous fallback mode, graceful degradation |
| Hidden mode latency (if added on Pi) | Perceived delay | Only offload *non-time-critical* transforms |

---
## 16. Decisions (Previously Open Questions)
1. Portal Integration: Use pre-existing infinity portal code from `~/Projects/coding/arduino/uno/arduino-infinity-portal`
2. Animation Programs: Six main programs (spiral, pulse, wave, chaos, ambient, idle) with Pi cue control
3. Portal Cues: Pi sends high-level commands (program, BPM, intensity) via serial/MIDI, not frame data
4. Joystick: Single 127 pulse per press (edge), no 0 release message
5. Buttons: Fixed velocity for v1 (no dynamic velocity / aftertouch)
6. Startup Self-Test: Implement portal animation sequence + success indication
7. Remote Config Protocol: Deferred (no SysEx / config channel v1)
8. Pot Latency Budget: ≤20 ms acceptable → current smoothing & rate limit chosen accordingly
9. Portal Control: Teensy fully owns animation rendering; Pi sends only semantic cues
10. Idle Brightness: Cap at 15% of max brightness during idle/ambient programs
11. BPM Sync: Portal animations sync to Pi sequencer BPM via rate control cues
12. License: Apache 2.0 (aligns with overall project license)

---
## 17. Immediate Next Actions
- [ ] Confirm integration approach for pre-existing portal code
- [ ] Scaffold directory + headers (`pins.h`, `config.h`, `main.cpp`, `portal_controller.h`)
- [ ] Implement Phase 1 MVP & commit
- [ ] Start loop timing instrumentation before adding portal complexity
- [ ] Test portal animation programs individually before integration

---
## 18. Directory Skeleton (Proposed)
```
teensy/
  firmware/
    src/
      main.cpp
      pins.h
      config.h
      input_scanner.cpp
      input_scanner.h
      debounce.h
      analog_smoother.cpp
      analog_smoother.h
      mapping.h
      midi_out.cpp
      midi_out.h
      led_fx.cpp
      led_fx.h
      portal_controller.cpp
      portal_controller.h
      portal_cues.cpp
      portal_cues.h
      diagnostics.cpp
      diagnostics.h
    include/ (optional if using PlatformIO)
    platformio.ini (if using PlatformIO instead of Arduino IDE)
  CHANGELOG.md
```

---
## 19. Example Pseudocode Main Loop
```cpp
void loop() {
  static elapsedMicros tick;
  if (tick >= 1000) { // ~1 kHz
    tick -= 1000;
    scanInputs();              // read raw states
    processButtons();          // debounce & note events
    processJoystick();
    processSwitches();
    processPots();             // smoothing + CC
    updateIdleTimer();
    handlePortalCues();        // process Pi → Teensy commands
    schedulePortalFrame();
  }
  if (shouldRenderPortalFrame()) {
    portalController.update();
    FastLED.show();
  }
  while (usbMIDI.read()) { /* handle incoming portal cues */ }
}
```

---
## 20. Acceptance Criteria for v0.1.0
- All 10 buttons produce correct NoteOn/NoteOff (no duplicates, <10 ms worst-case latency)
- Pots produce monotonic CC ramps on slow sweep; jitter ≤ ±1 when still; event latency ≤20 ms
- Joystick directions emit a single 127 pulse per actuation (re-arm enforced)
- Switches latch ON (127) / OFF (0) correctly
- Portal integration functional: all 6 animation programs working (spiral, pulse, wave, chaos, ambient, idle)
- Portal cue system: receives and responds to Pi commands for program switching and BPM sync
- Button press visual feedback, pot activity feedback, idle ambient mode (≤15% brightness)
- Portal startup self-test sequence functional
- CPU headroom > 90%; no memory fragmentation (no dynamic alloc in loop)
- Soak test 4 hr: no stuck notes, no crash, loop time stable, portal animations stable

---
## 21. Future Enhancements (Backlog)
- SysEx config channel
- Input event batching (pack multiple changes in a millisecond bucket)
- HID composite device (MIDI + Serial + Custom) for config UI
- Portal cross-fade between programs for seamless transitions
- Advanced portal effects (particle systems, 3D-style effects)
- MIDI clock in (from Pi) to flash tempo portal group
- Portal color palette streaming from Pi

---

Provide clarifications for integration approach and portal cue protocol to refine this roadmap before coding Phase 3.
