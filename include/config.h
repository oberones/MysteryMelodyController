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
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31  // CC 20-31 for the 12 switches
};

// MIDI CC for binary representation of first 8 switches
constexpr uint8_t SWITCH_BINARY_CC = 50;

// ===== PORTAL ANIMATION CONFIGURATION =====
constexpr uint8_t PORTAL_PROGRAM_COUNT = 10;
enum PortalProgram {
    PORTAL_SPIRAL = 0,      // Rotating spiral patterns
    PORTAL_PULSE = 1,       // Rhythmic pulsing from center
    PORTAL_WAVE = 2,        // Flowing wave patterns
    PORTAL_CHAOS = 3,       // Random chaotic patterns
    PORTAL_AMBIENT = 4,     // Slow peaceful patterns
    PORTAL_IDLE = 5,        // Minimal ambient mode
    PORTAL_RIPPLE = 6,      // Ripple effects from interactions
    PORTAL_RAINBOW = 7,     // Smooth rainbow rotation
    PORTAL_PLASMA = 8,      // Plasma-like flowing colors
    PORTAL_BREATHE = 9      // Gentle breathing effect
};

// Portal frame rate (Hz)
constexpr uint8_t PORTAL_FPS = 60;
constexpr uint32_t PORTAL_FRAME_INTERVAL_US = 1000000 / PORTAL_FPS;

// ===== OLED DISPLAY CONFIGURATION =====
// Display update rate (Hz) - keep low to avoid interference with main loop
constexpr uint8_t OLED_UPDATE_HZ = 20;

// Default display mode on startup
constexpr uint8_t OLED_DEFAULT_MODE = 0;  // MIDI_LOG mode

// MIDI log settings
constexpr uint8_t OLED_MIDI_LOG_SIZE = 8;  // Number of MIDI messages to keep in log
