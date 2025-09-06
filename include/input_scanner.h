#pragma once

#include <Arduino.h>
#include "pins.h"
#include "config.h"

/**
 * @brief Raw input scanner for all hardware inputs
 * 
 * Provides basic input scanning without debouncing or filtering.
 * Phase 1 implementation - raw polling only.
 */
class InputScanner {
public:
    InputScanner();
    
    /**
     * @brief Initialize all input pins
     */
    void begin();
    
    /**
     * @brief Scan all inputs once
     * Call this from main loop at 1kHz
     */
    void scan();
    
    // Button state access
    bool getButtonState(uint8_t buttonIndex) const;
    bool getButtonPressed(uint8_t buttonIndex) const;
    bool getButtonReleased(uint8_t buttonIndex) const;
    
    // Joystick state access
    bool getJoystickPressed(uint8_t direction) const;  // 0=Up, 1=Down, 2=Left, 3=Right
    
    // Switch state access
    bool getSwitchState(uint8_t switchIndex) const;
    bool getSwitchChanged(uint8_t switchIndex) const;
    
    // Potentiometer access (0-1023 raw ADC)
    uint16_t getPotValue(uint8_t potIndex) const;
    bool getPotChanged(uint8_t potIndex) const;
    
private:
    // Button states
    bool buttonStates[BUTTON_COUNT];
    bool lastButtonStates[BUTTON_COUNT];
    
    // Joystick states (Up, Down, Left, Right)
    bool joystickStates[4];
    bool lastJoystickStates[4];
    
    // Switch states
    bool switchStates[SWITCH_COUNT];
    bool lastSwitchStates[SWITCH_COUNT];
    
    // Potentiometer values
    uint16_t potValues[POT_COUNT];
    uint16_t lastPotValues[POT_COUNT];
    
    void scanButtons();
    void scanJoystick();
    void scanSwitches();
    void scanPots();
};
