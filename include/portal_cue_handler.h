#pragma once

#include <Arduino.h>
#include "portal_controller.h"

class PortalCueHandler {
public:
    PortalCueHandler();
    
    void begin(PortalController* controller);
    void handleMidiCC(uint8_t cc, uint8_t value);
    void handleSerialCommand(const String& command);
    
    // Automatic activity detection and idle switching
    void update();
    void setInputActivity(bool hasActivity);
    
    // Auto-program switching based on idle state
    void checkIdleState();
    
private:
    PortalController* portalController;
    
    // Activity tracking for auto idle mode
    elapsedMillis timeSinceLastActivity;
    bool wasIdle;
    uint8_t lastActiveProgram;  // Remember last program before going idle
    
    // Auto-switching parameters
    static constexpr uint32_t AUTO_SWITCH_INTERVAL = 60000;  // Switch programs every minute when idle
    
    elapsedMillis autoSwitchTimer;
    
    void printStatus();
};

// Portal program names for debugging
extern const char* PORTAL_PROGRAM_NAMES[];
