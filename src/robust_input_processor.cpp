#include "robust_input_processor.h"

RobustInputProcessor::RobustInputProcessor()
    : lastActivityTime(0)
    , testModeEnabled(false)
{
    // Initialize joystick rearm times
    for (int i = 0; i < 4; i++) {
        joystickRearmTime[i] = 0;
    }
}

void RobustInputProcessor::begin() {
    // Initialize the raw scanner
    scanner.begin();
    
    // Initialize all debouncers with default timing
    for (int i = 0; i < BUTTON_COUNT; i++) {
        buttonDebouncers[i] = Debouncer(DEBOUNCE_MS);
    }
    
    for (int i = 0; i < 4; i++) {
        joystickDebouncers[i] = Debouncer(DEBOUNCE_MS);
    }
    
    for (int i = 0; i < SWITCH_COUNT; i++) {
        switchDebouncers[i] = Debouncer(DEBOUNCE_MS);
    }
    
    // Initialize analog smoothers with configured parameters
    for (int i = 0; i < POT_COUNT; i++) {
        potSmoothers[i] = AnalogSmoother(64, POT_DEADBAND, POT_RATE_LIMIT_MS);
        // Initialize with current pot reading to prevent startup spikes
        uint16_t currentValue = scanner.getPotValue(i);
        potSmoothers[i].reset(currentValue);
    }
    
    lastActivityTime = millis();
    
    #if DEBUG
    Serial.println("RobustInputProcessor: Initialized with debouncing and smoothing");
    Serial.printf("  Button debounce: %dms\n", DEBOUNCE_MS);
    Serial.printf("  Pot deadband: %d, rate limit: %dms\n", POT_DEADBAND, POT_RATE_LIMIT_MS);
    Serial.printf("  Joystick rearm: %dms\n", JOYSTICK_REARM_MS);
    #endif
}

void RobustInputProcessor::update() {
    // Scan raw inputs first
    scanner.scan();
    
    // Process all input types with robust filtering
    processButtons();
    processJoystick();
    processSwitches();
    processPotentiometers();
}

void RobustInputProcessor::processButtons() {
    uint32_t currentTime = millis();
    
    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool rawState = scanner.getButtonState(i);
        if (buttonDebouncers[i].update(rawState, currentTime)) {
            // State changed after debouncing
            updateActivity();
            
            #if DEBUG >= 2
            Serial.printf("Button %d: %s\n", i, 
                         buttonDebouncers[i].isPressed() ? "PRESSED" : "RELEASED");
            #endif
        }
    }
}

void RobustInputProcessor::processJoystick() {
    uint32_t currentTime = millis();
    
    for (int i = 0; i < 4; i++) {
        bool rawState = false;
        
        // Only read joystick if rearm time has passed
        if (currentTime >= joystickRearmTime[i]) {
            switch (i) {
                case 0: rawState = !digitalRead(JOYSTICK_UP); break;
                case 1: rawState = !digitalRead(JOYSTICK_DOWN); break;
                case 2: rawState = !digitalRead(JOYSTICK_LEFT); break;
                case 3: rawState = !digitalRead(JOYSTICK_RIGHT); break;
            }
        }
        
        if (joystickDebouncers[i].update(rawState, currentTime)) {
            if (joystickDebouncers[i].justPressed()) {
                // Set rearm time to prevent rapid repeat
                joystickRearmTime[i] = currentTime + JOYSTICK_REARM_MS;
                updateActivity();
                
                #if DEBUG >= 2
                const char* directions[] = {"UP", "DOWN", "LEFT", "RIGHT"};
                Serial.printf("Joystick %s pressed (rearm: %dms)\n", 
                             directions[i], JOYSTICK_REARM_MS);
                #endif
            }
        }
    }
}

void RobustInputProcessor::processSwitches() {
    uint32_t currentTime = millis();
    
    for (int i = 0; i < SWITCH_COUNT; i++) {
        bool rawState = scanner.getSwitchState(i);
        if (switchDebouncers[i].update(rawState, currentTime)) {
            // State changed after debouncing
            updateActivity();
            
            #if DEBUG >= 2
            Serial.printf("Switch %d: %s\n", i, 
                         switchDebouncers[i].isPressed() ? "ON" : "OFF");
            #endif
        }
    }
}

void RobustInputProcessor::processPotentiometers() {
    uint32_t currentTime = millis();
    
    for (int i = 0; i < POT_COUNT; i++) {
        uint16_t rawValue = scanner.getPotValue(i);
        if (potSmoothers[i].update(rawValue, currentTime)) {
            // Smoothed value changed significantly
            updateActivity();
            
            #if DEBUG >= 2
            Serial.printf("Pot %d: %d -> MIDI %d\n", i, rawValue, 
                         potSmoothers[i].getMidiValue());
            #endif
        }
    }
}

void RobustInputProcessor::updateActivity() {
    lastActivityTime = millis();
}

// Public interface methods
bool RobustInputProcessor::getButtonPressed(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return buttonDebouncers[buttonIndex].justPressed();
}

bool RobustInputProcessor::getButtonReleased(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return buttonDebouncers[buttonIndex].justReleased();
}

bool RobustInputProcessor::getButtonState(uint8_t buttonIndex) const {
    if (buttonIndex >= BUTTON_COUNT) return false;
    return buttonDebouncers[buttonIndex].isPressed();
}

bool RobustInputProcessor::getJoystickPressed(uint8_t direction) const {
    if (direction >= 4) return false;
    return joystickDebouncers[direction].justPressed();
}

bool RobustInputProcessor::getSwitchState(uint8_t switchIndex) const {
    if (switchIndex >= SWITCH_COUNT) return false;
    return switchDebouncers[switchIndex].isPressed();
}

bool RobustInputProcessor::getSwitchChanged(uint8_t switchIndex) const {
    if (switchIndex >= SWITCH_COUNT) return false;
    return switchDebouncers[switchIndex].justPressed() || 
           switchDebouncers[switchIndex].justReleased();
}

uint8_t RobustInputProcessor::getPotMidiValue(uint8_t potIndex) const {
    if (potIndex >= POT_COUNT) return 0;
    return potSmoothers[potIndex].getMidiValue();
}

bool RobustInputProcessor::getPotChanged(uint8_t potIndex) const {
    if (potIndex >= POT_COUNT) return false;
    return potSmoothers[potIndex].hasSignificantChange();
}

uint32_t RobustInputProcessor::getTimeSinceLastActivity() const {
    return millis() - lastActivityTime;
}

bool RobustInputProcessor::isIdle() const {
    return getTimeSinceLastActivity() >= IDLE_TIMEOUT_MS;
}

void RobustInputProcessor::dumpTestValues() const {
    if (!testModeEnabled) return;
    
    Serial.println("=== INPUT STATE DUMP ===");
    
    // Button states
    Serial.print("Buttons: ");
    for (int i = 0; i < BUTTON_COUNT; i++) {
        Serial.printf("%d:%s ", i, getButtonState(i) ? "ON" : "OFF");
    }
    Serial.println();
    
    // Switch states
    Serial.print("Switches: ");
    for (int i = 0; i < SWITCH_COUNT; i++) {
        Serial.printf("%d:%s ", i, getSwitchState(i) ? "ON" : "OFF");
    }
    Serial.println();
    
    // Potentiometer values
    Serial.print("Pots: ");
    for (int i = 0; i < POT_COUNT; i++) {
        Serial.printf("%d:MIDI_%d ", i, getPotMidiValue(i));
    }
    Serial.println();
    
    // Activity status
    Serial.printf("Activity: %lums ago, Idle: %s\n", 
                  getTimeSinceLastActivity(), isIdle() ? "YES" : "NO");
    
    Serial.println("========================");
}
