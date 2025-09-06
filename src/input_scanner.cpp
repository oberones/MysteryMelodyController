#include "input_scanner.h"

InputScanner::InputScanner() {
    // Initialize all state arrays
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttonStates[i] = false;
        lastButtonStates[i] = false;
    }
    
    for (int i = 0; i < 4; i++) {
        joystickStates[i] = false;
        lastJoystickStates[i] = false;
    }
    
    for (int i = 0; i < SWITCH_COUNT; i++) {
        switchStates[i] = false;
        lastSwitchStates[i] = false;
    }
    
    for (int i = 0; i < POT_COUNT; i++) {
        potValues[i] = 0;
        lastPotValues[i] = 0;
    }
}

void InputScanner::begin() {
    // Initialize button pins (INPUT_PULLUP for active low)
    for (int i = 0; i < BUTTON_COUNT; i++) {
        pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    }
    
    // Initialize joystick pins (INPUT_PULLUP for active low)
    pinMode(JOYSTICK_UP, INPUT_PULLUP);
    pinMode(JOYSTICK_DOWN, INPUT_PULLUP);
    pinMode(JOYSTICK_LEFT, INPUT_PULLUP);
    pinMode(JOYSTICK_RIGHT, INPUT_PULLUP);
    
    // Initialize switch pins (INPUT_PULLUP for active low)
    for (int i = 0; i < SWITCH_COUNT; i++) {
        pinMode(SWITCH_PINS[i], INPUT_PULLUP);
    }
    
    // Potentiometer pins are analog - no pinMode needed
    
    // Initial scan to populate starting values
    scan();
    // Copy to "last" arrays to prevent initial false triggers
    for (int i = 0; i < BUTTON_COUNT; i++) {
        lastButtonStates[i] = buttonStates[i];
    }
    for (int i = 0; i < 4; i++) {
        lastJoystickStates[i] = joystickStates[i];
    }
    for (int i = 0; i < SWITCH_COUNT; i++) {
        lastSwitchStates[i] = switchStates[i];
    }
    for (int i = 0; i < POT_COUNT; i++) {
        lastPotValues[i] = potValues[i];
    }
}

void InputScanner::scan() {
    // Save last states
    for (int i = 0; i < BUTTON_COUNT; i++) {
        lastButtonStates[i] = buttonStates[i];
    }
    for (int i = 0; i < 4; i++) {
        lastJoystickStates[i] = joystickStates[i];
    }
    for (int i = 0; i < SWITCH_COUNT; i++) {
        lastSwitchStates[i] = switchStates[i];
    }
    for (int i = 0; i < POT_COUNT; i++) {
        lastPotValues[i] = potValues[i];
    }
    
    // Scan all inputs
    scanButtons();
    scanJoystick();
    scanSwitches();
    scanPots();
}

void InputScanner::scanButtons() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        // Active low (pressed = LOW)
        buttonStates[i] = !digitalRead(BUTTON_PINS[i]);
    }
}

void InputScanner::scanJoystick() {
    // Active low (pressed = LOW)
    joystickStates[0] = !digitalRead(JOYSTICK_UP);
    joystickStates[1] = !digitalRead(JOYSTICK_DOWN);
    joystickStates[2] = !digitalRead(JOYSTICK_LEFT);
    joystickStates[3] = !digitalRead(JOYSTICK_RIGHT);
}

void InputScanner::scanSwitches() {
    for (int i = 0; i < SWITCH_COUNT; i++) {
        // Active low (on = LOW)
        switchStates[i] = !digitalRead(SWITCH_PINS[i]);
    }
}

void InputScanner::scanPots() {
    for (int i = 0; i < POT_COUNT; i++) {
        potValues[i] = analogRead(POT_PINS[i]);
    }
}

// Button state access
bool InputScanner::getButtonState(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return buttonStates[buttonIndex];
}

bool InputScanner::getButtonPressed(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return buttonStates[buttonIndex] && !lastButtonStates[buttonIndex];
}

bool InputScanner::getButtonReleased(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return !buttonStates[buttonIndex] && lastButtonStates[buttonIndex];
}

// Joystick state access
bool InputScanner::getJoystickPressed(uint8_t direction) const {
    if (direction >= 4) return false;
    return joystickStates[direction] && !lastJoystickStates[direction];
}

// Switch state access
bool InputScanner::getSwitchState(uint8_t switchIndex) const {
    if (switchIndex >= SWITCH_COUNT) return false;
    return switchStates[switchIndex];
}

bool InputScanner::getSwitchChanged(uint8_t switchIndex) const {
    if (switchIndex >= SWITCH_COUNT) return false;
    return switchStates[switchIndex] != lastSwitchStates[switchIndex];
}

// Potentiometer access
uint16_t InputScanner::getPotValue(uint8_t potIndex) const {
    if (potIndex >= POT_COUNT) return 0;
    return potValues[potIndex];
}

bool InputScanner::getPotChanged(uint8_t potIndex) const {
    if (potIndex >= POT_COUNT) return false;
    // Simple change detection - any difference
    return potValues[potIndex] != lastPotValues[potIndex];
}
