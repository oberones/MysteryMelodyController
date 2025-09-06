#pragma once

// Pin definitions for Teensy 4.1 Mystery Melody Machine
// Digital pins for buttons, joystick, switches
// Analog pins for potentiometers
// LED data pin for infinity portal

// ===== DIGITAL INPUT PINS =====
// Buttons (10 total)
constexpr uint8_t BUTTON_PINS[] = {
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};
constexpr uint8_t BUTTON_COUNT = sizeof(BUTTON_PINS) / sizeof(BUTTON_PINS[0]);

// Joystick directions (4 directions)
// Note: Moved RIGHT from pin 15 to 19 to avoid conflict with A1 analog input
// Note: Moved DOWN from pin 13 to 20 to avoid conflict with built-in LED
constexpr uint8_t JOYSTICK_UP = 12;
constexpr uint8_t JOYSTICK_DOWN = 20;
constexpr uint8_t JOYSTICK_LEFT = 14;
constexpr uint8_t JOYSTICK_RIGHT = 19;

// Switches (3 total)
constexpr uint8_t SWITCH_PINS[] = {
    16, 17, 18
};
constexpr uint8_t SWITCH_COUNT = sizeof(SWITCH_PINS) / sizeof(SWITCH_PINS[0]);

// ===== ANALOG INPUT PINS =====
// Potentiometers - only enable the ones actually connected
// Note: A4/A5 reserved for I2C, A6/A7 currently unconnected
constexpr uint8_t POT_PINS[] = {
    A0, A1, A2, A3  // Only first 4 pots to avoid noise from floating A6/A7
};
constexpr uint8_t POT_COUNT = sizeof(POT_PINS) / sizeof(POT_PINS[0]);

// ===== LED OUTPUT PINS =====
constexpr uint8_t LED_DATA_PIN = 1;  // Pin 1 for LED data
constexpr uint8_t LED_COUNT = 60;    // Typical infinity portal LED count

// ===== BUILT-IN PINS =====
constexpr uint8_t BUILTIN_LED_PIN = 13;  // Teensy 4.1 built-in LED
