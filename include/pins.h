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
constexpr uint8_t JOYSTICK_UP = 33;
constexpr uint8_t JOYSTICK_DOWN = 34;
constexpr uint8_t JOYSTICK_LEFT = 35;
constexpr uint8_t JOYSTICK_RIGHT = 36;

// Switches (12 total)
constexpr uint8_t SWITCH_PINS[] = {
    22, 23, 24, 25, 26, 27, 28, 29, 16, 17, 18, 21
};
constexpr uint8_t SWITCH_COUNT = sizeof(SWITCH_PINS) / sizeof(SWITCH_PINS[0]);

// ===== ANALOG INPUT PINS =====
// Potentiometers - only enable the ones actually connected
// Note: A4/A5 reserved for I2C, A6/A7 currently unconnected
constexpr uint8_t POT_PINS[] = {
    A0, A1, A2, A3  // Only first 4 pots to avoid noise from floating A6/A7
};
constexpr uint8_t POT_COUNT = sizeof(POT_PINS) / sizeof(POT_PINS[0]);

// ===== I2C PINS =====
// I2C for OLED display (Teensy 4.1 Wire/I2C)
constexpr uint8_t I2C_SDA_PIN = 18;  // Pin 18 (A4) - SDA
constexpr uint8_t I2C_SCL_PIN = 19;  // Pin 19 (A5) - SCL

// OLED display configuration
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 64;
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;  // Common I2C address for SSD1306

// ===== LED OUTPUT PINS =====
constexpr uint8_t LED_DATA_PIN = 1;  // Pin 1 for LED data
constexpr uint8_t LED_COUNT = 45;    // Circular infinity portal LED count

// ===== BUILT-IN PINS =====
constexpr uint8_t BUILTIN_LED_PIN = 13;  // Teensy 4.1 built-in LED
