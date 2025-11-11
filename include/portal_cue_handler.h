#pragma once

#include <Arduino.h>
#include "portal_controller.h"
#include "serial_portal_protocol.h"

class PortalCueHandler {
public:
    PortalCueHandler();
    
    void begin(PortalController* controller);
    
    // Legacy MIDI CC support (can be removed after transition)
    void handleMidiCC(uint8_t cc, uint8_t value);
    void handleSerialCommand(const String& command);
    
    // New serial protocol handling
    void handleSerialMessage(const PortalMessage& message);
    void processSerialInput();
    
    // Automatic activity detection and idle switching
    void update();
    void setInputActivity(bool hasActivity);
    
    // Auto-program switching based on idle state
    void checkIdleState();
    
    // Response methods for Pi communication
    void sendAck();
    void sendNak();
    void sendPong();
    void sendStatus();
    
private:
    PortalController* portalController;
    
    // Activity tracking for auto idle mode
    elapsedMillis timeSinceLastActivity;
    bool wasIdle;
    uint8_t lastActiveProgram;  // Remember last program before going idle
    
    // Auto-switching parameters
    static constexpr uint32_t AUTO_SWITCH_INTERVAL = 60000;  // Switch programs every minute when idle
    
    elapsedMillis autoSwitchTimer;
    
    // Serial communication state
    uint8_t serialBuffer[PORTAL_SERIAL_BUFFER_SIZE];
    uint8_t bufferIndex;
    elapsedMillis lastMessageTime;
    
    // Statistics
    uint32_t messagesReceived;
    uint32_t messagesValid;
    uint32_t messagesInvalid;
    
    void printStatus();
    void resetSerialBuffer();
    bool parseSerialMessage();
    void sendMessage(const PortalMessage& message);
};

// Portal program names for debugging
extern const char* PORTAL_PROGRAM_NAMES[];
