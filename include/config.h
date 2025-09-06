#pragma once

// Configuration constants for Mystery Melody Machine Teensy Firmware

// ===== TIMING CONFIGURATION =====
#ifndef SCAN_HZ
#define SCAN_HZ 1000
#endif

#ifndef DEBOUNCE_MS
#define DEBOUNCE_MS 5
#endif

#ifndef POT_DEADBAND
#define POT_DEADBAND 2
#endif

#ifndef POT_RATE_LIMIT_MS
#define POT_RATE_LIMIT_MS 15
#endif

#ifndef IDLE_TIMEOUT_MS
#define IDLE_TIMEOUT_MS 30000
#endif

#ifndef JOYSTICK_REARM_MS
#define JOYSTICK_REARM_MS 120
#endif

// ===== LED CONFIGURATION =====
#ifndef LED_BRIGHTNESS_MAX
#define LED_BRIGHTNESS_MAX 160
#endif

#ifndef IDLE_BRIGHTNESS_CAP_PCT
#define IDLE_BRIGHTNESS_CAP_PCT 15
#endif

// ===== DEBUG CONFIGURATION =====
#ifndef DEBUG
#define DEBUG 1  // Phase 2: Enable debug output by default
#endif

// ===== PHASE 2 ROBUST INPUT CONFIGURATION =====
// EMA smoothing alpha (0-255, where 64 ≈ 0.25)
#ifndef POT_SMOOTHING_ALPHA
#define POT_SMOOTHING_ALPHA 64
#endif

// Minimum stable time for digital state changes
#ifndef SWITCH_DEBOUNCE_MS
#define SWITCH_DEBOUNCE_MS DEBOUNCE_MS
#endif

// Large change threshold that overrides rate limiting
#ifndef POT_LARGE_CHANGE_THRESHOLD
#define POT_LARGE_CHANGE_THRESHOLD 8
#endif

// Stable time before sending pot change (change compression)
#ifndef POT_STABLE_TIME_MS
#define POT_STABLE_TIME_MS 4
#endif

// ===== MIDI CONFIGURATION =====
constexpr uint8_t MIDI_CHANNEL = 1;
constexpr uint8_t MIDI_VELOCITY = 100;

// MIDI Note mapping for buttons (starting from middle C)
constexpr uint8_t BUTTON_NOTES[] = {
    60, 61, 62, 63, 64, 65, 66, 67, 68, 69  // C4 to A4
};

// MIDI CC mapping for potentiometers
constexpr uint8_t POT_CCS[] = {
    1, 2, 3, 4, 5, 6  // CC 1-6 for the 6 pots
};

// MIDI CC mapping for joystick directions
constexpr uint8_t JOY_UP_CC = 10;
constexpr uint8_t JOY_DOWN_CC = 11;
constexpr uint8_t JOY_LEFT_CC = 12;
constexpr uint8_t JOY_RIGHT_CC = 13;

// MIDI CC mapping for switches
constexpr uint8_t SWITCH_CCS[] = {
    20, 21, 22  // CC 20-22 for the 3 switches
};

// ===== PORTAL ANIMATION CONFIGURATION =====
constexpr uint8_t PORTAL_PROGRAM_COUNT = 6;
enum PortalProgram {
    PORTAL_SPIRAL = 0,
    PORTAL_PULSE = 1,
    PORTAL_WAVE = 2,
    PORTAL_CHAOS = 3,
    PORTAL_AMBIENT = 4,
    PORTAL_IDLE = 5
};

// Portal frame rate (Hz)
constexpr uint8_t PORTAL_FPS = 60;
constexpr uint32_t PORTAL_FRAME_INTERVAL_US = 1000000 / PORTAL_FPS;
