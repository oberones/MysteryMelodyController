#include "portal_cue_handler.h"
#include "config.h"
#include "pins.h"

// Program names for debugging and display
const char* PORTAL_PROGRAM_NAMES[] = {
    "SPIRAL", "PULSE", "WAVE", "CHAOS", "AMBIENT", 
    "IDLE", "RIPPLE", "RAINBOW", "PLASMA", "BREATHE"
};

PortalCueHandler::PortalCueHandler() :
    portalController(nullptr),
    timeSinceLastActivity(0),
    wasIdle(false),
    lastActiveProgram(PORTAL_AMBIENT),
    autoSwitchTimer(0),
    bufferIndex(0),
    lastMessageTime(0),
    messagesReceived(0),
    messagesValid(0),
    messagesInvalid(0)
{
}

void PortalCueHandler::begin(PortalController* controller) {
    portalController = controller;
    timeSinceLastActivity = 0;
    resetSerialBuffer();
    
    Serial.println("Portal Cue Handler initialized with Serial Protocol");
    Serial.printf("Serial baud rate: %lu\n", PORTAL_SERIAL_BAUD);
    Serial.println("Serial Commands:");
    Serial.println("  0x01: SET_PROGRAM (0-9)");
    Serial.println("  0x02: SET_BPM (0-255 -> 60-180 BPM)");
    Serial.println("  0x03: SET_INTENSITY (0-255)");
    Serial.println("  0x04: SET_HUE (0-255)");
    Serial.println("  0x05: SET_BRIGHTNESS (0-255)");
    Serial.println("  0x06: TRIGGER_FLASH");
    Serial.println("  0x07: TRIGGER_RIPPLE (position)");
    Serial.println("  0x10: PING (keepalive)");
    Serial.println("Legacy MIDI CC support still available");
}

void PortalCueHandler::handleMidiCC(uint8_t cc, uint8_t value) {
    if (!portalController) return;
    
    #if DEBUG >= 1
    Serial.printf("Portal MIDI CC: %d = %d\n", cc, value);
    #endif
    
    switch (cc) {
        case PORTAL_PROGRAM_CC:
            if (value < PORTAL_PROGRAM_COUNT) {
                portalController->setProgram(value);
                lastActiveProgram = value;
                
                #if DEBUG >= 1
                Serial.printf("Portal program changed to: %s (%d)\n", 
                             PORTAL_PROGRAM_NAMES[value], value);
                #endif
                
                // Reset idle timer when program is manually changed
                timeSinceLastActivity = 0;
                wasIdle = false;
            }
            break;
            
        case PORTAL_BPM_CC:
            {
                float bpm = map(value, 0, 127, 60, 180);
                portalController->setBpm(bpm);
                
                #if DEBUG >= 1
                Serial.printf("Portal BPM set to: %.1f\n", bpm);
                #endif
            }
            break;
            
        case PORTAL_INTENSITY_CC:
            {
                float intensity = value / 127.0;
                portalController->setIntensity(intensity);
                
                #if DEBUG >= 1
                Serial.printf("Portal intensity set to: %.2f\n", intensity);
                #endif
            }
            break;
            
        case PORTAL_HUE_CC:
            {
                float hue = value / 127.0;
                portalController->setBaseHue(hue);
                
                #if DEBUG >= 1
                Serial.printf("Portal base hue set to: %.2f\n", hue);
                #endif
            }
            break;
            
        case PORTAL_BRIGHTNESS_CC:
            {
                uint8_t brightness = map(value, 0, 127, 0, LED_BRIGHTNESS_MAX);
                portalController->setBrightness(brightness);
                
                #if DEBUG >= 1
                Serial.printf("Portal brightness set to: %d\n", brightness);
                #endif
            }
            break;
            
        case PORTAL_FLASH_CC:
            if (value >= 64) {  // Trigger on high values
                portalController->triggerFlash();
                
                #if DEBUG >= 1
                Serial.println("Portal flash triggered");
                #endif
            }
            break;
            
        case PORTAL_RIPPLE_CC:
            {
                uint8_t position = map(value, 0, 127, 0, LED_COUNT - 1);
                portalController->triggerRipple(position);
                
                #if DEBUG >= 1
                Serial.printf("Portal ripple triggered at position: %d\n", position);
                #endif
            }
            break;
            
        default:
            // Not a portal CC - ignore
            break;
    }
}

void PortalCueHandler::handleSerialCommand(const String& command) {
    if (!portalController) return;
    
    String cmd = command;
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd.startsWith("portal ")) {
        String param = cmd.substring(7);
        
        if (param.startsWith("program ")) {
            int program = param.substring(8).toInt();
            if (program >= 0 && program < PORTAL_PROGRAM_COUNT) {
                portalController->setProgram(program);
                Serial.printf("Portal program set to: %s (%d)\n", 
                             PORTAL_PROGRAM_NAMES[program], program);
            }
        }
        else if (param.startsWith("bpm ")) {
            float bpm = param.substring(4).toFloat();
            if (bpm >= 60 && bpm <= 180) {
                portalController->setBpm(bpm);
                Serial.printf("Portal BPM set to: %.1f\n", bpm);
            }
        }
        else if (param.startsWith("intensity ")) {
            float intensity = param.substring(10).toFloat();
            if (intensity >= 0.0 && intensity <= 1.0) {
                portalController->setIntensity(intensity);
                Serial.printf("Portal intensity set to: %.2f\n", intensity);
            }
        }
        else if (param == "flash") {
            portalController->triggerFlash();
            Serial.println("Portal flash triggered");
        }
        else if (param == "status") {
            printStatus();
        }
        else if (param == "help") {
            Serial.println("Portal commands:");
            Serial.println("  portal program <0-9>");
            Serial.println("  portal bpm <60-180>");
            Serial.println("  portal intensity <0.0-1.0>");
            Serial.println("  portal flash");
            Serial.println("  portal status");
        }
    }
}

void PortalCueHandler::update() {
    if (!portalController) return;
    
    checkIdleState();
}

void PortalCueHandler::setInputActivity(bool hasActivity) {
    if (hasActivity) {
        timeSinceLastActivity = 0;
        
        // If we were idle and now have activity, switch back to active program
        if (wasIdle && portalController->getCurrentProgram() == PORTAL_IDLE) {
            portalController->setProgram(lastActiveProgram);
            wasIdle = false;
            
            #if DEBUG >= 1
            Serial.printf("Activity detected - switching from IDLE to %s\n", 
                         PORTAL_PROGRAM_NAMES[lastActiveProgram]);
            #endif
        }
    }
}

void PortalCueHandler::checkIdleState() {
    bool isCurrentlyIdle = (timeSinceLastActivity > IDLE_TIMEOUT_MS);
    
    // Transition from active to idle
    if (isCurrentlyIdle && !wasIdle) {
        lastActiveProgram = portalController->getCurrentProgram();
        portalController->setProgram(PORTAL_IDLE);
        wasIdle = true;
        autoSwitchTimer = 0;
        
        #if DEBUG >= 1
        Serial.printf("No activity for %ds - switching to IDLE mode (was %s)\n", 
                     IDLE_TIMEOUT_MS / 1000, 
                     PORTAL_PROGRAM_NAMES[lastActiveProgram]);
        #endif
    }
    
    // Auto-switch between programs when idle (for demo/ambient purposes)
    if (isCurrentlyIdle && autoSwitchTimer > AUTO_SWITCH_INTERVAL) {
        autoSwitchTimer = 0;
        
        // Cycle through ambient programs when idle
        uint8_t ambientPrograms[] = {PORTAL_AMBIENT, PORTAL_BREATHE, PORTAL_RAINBOW, PORTAL_PLASMA};
        uint8_t nextProgram = ambientPrograms[random(4)];
        
        portalController->setProgram(nextProgram);
        
        #if DEBUG >= 1
        Serial.printf("Auto-switching to %s for ambient display\n", 
                     PORTAL_PROGRAM_NAMES[nextProgram]);
        #endif
    }
}

void PortalCueHandler::printStatus() {
    if (!portalController) return;
    
    Serial.println("=== Portal Status ===");
    Serial.printf("Current Program: %s (%d)\n", 
                 PORTAL_PROGRAM_NAMES[portalController->getCurrentProgram()],
                 portalController->getCurrentProgram());
    Serial.printf("Frame Count: %lu\n", portalController->getFrameCount());
    Serial.printf("Time Since Activity: %lu ms\n", (uint32_t)timeSinceLastActivity);
    Serial.printf("Idle State: %s\n", wasIdle ? "YES" : "NO");
    if (wasIdle) {
        Serial.printf("Last Active Program: %s (%d)\n", 
                     PORTAL_PROGRAM_NAMES[lastActiveProgram], lastActiveProgram);
    }
    Serial.printf("Serial Messages - RX: %lu, Valid: %lu, Invalid: %lu\n", 
                 messagesReceived, messagesValid, messagesInvalid);
    Serial.println("====================");
}

// ===== NEW SERIAL PROTOCOL METHODS =====

void PortalCueHandler::handleSerialMessage(const PortalMessage& message) {
    if (!portalController) return;
    
    #if DEBUG >= 1
    Serial.printf("Portal Serial: %s (0x%02X) = %d\n", 
                 SerialPortalProtocol::getCommandName(message.command),
                 static_cast<uint8_t>(message.command), message.value);
    #endif
    
    switch (message.command) {
        case PortalSerialCommand::SET_PROGRAM:
            if (message.value < PORTAL_PROGRAM_COUNT) {
                portalController->setProgram(message.value);
                lastActiveProgram = message.value;
                
                #if DEBUG >= 1
                Serial.printf("Portal program changed to: %s (%d)\n", 
                             PORTAL_PROGRAM_NAMES[message.value], message.value);
                #endif
                
                // Reset idle timer when program is manually changed
                timeSinceLastActivity = 0;
                wasIdle = false;
                sendAck();
            } else {
                sendNak();
            }
            break;
            
        case PortalSerialCommand::SET_BPM:
            {
                float bpm = SerialPortalProtocol::mapToBpm(message.value);
                portalController->setBpm(bpm);
                
                #if DEBUG >= 1
                Serial.printf("Portal BPM set to: %.1f\n", bpm);
                #endif
                sendAck();
            }
            break;
            
        case PortalSerialCommand::SET_INTENSITY:
            {
                float intensity = SerialPortalProtocol::mapToNormalized(message.value);
                portalController->setIntensity(intensity);
                
                #if DEBUG >= 1
                Serial.printf("Portal intensity set to: %.2f\n", intensity);
                #endif
                sendAck();
            }
            break;
            
        case PortalSerialCommand::SET_HUE:
            {
                float hue = SerialPortalProtocol::mapToNormalized(message.value);
                portalController->setBaseHue(hue);
                
                #if DEBUG >= 1
                Serial.printf("Portal base hue set to: %.2f\n", hue);
                #endif
                sendAck();
            }
            break;
            
        case PortalSerialCommand::SET_BRIGHTNESS:
            {
                portalController->setBrightness(message.value);
                
                #if DEBUG >= 1
                Serial.printf("Portal brightness set to: %d\n", message.value);
                #endif
                sendAck();
            }
            break;
            
        case PortalSerialCommand::TRIGGER_FLASH:
            portalController->triggerFlash();
            
            #if DEBUG >= 1
            Serial.println("Portal flash triggered");
            #endif
            sendAck();
            break;
            
        case PortalSerialCommand::TRIGGER_RIPPLE:
            {
                uint8_t position = SerialPortalProtocol::mapToLedPosition(message.value, LED_COUNT);
                portalController->triggerRipple(position);
                
                #if DEBUG >= 1
                Serial.printf("Portal ripple triggered at position: %d\n", position);
                #endif
                sendAck();
            }
            break;
            
        case PortalSerialCommand::PING:
            sendPong();
            break;
            
        case PortalSerialCommand::RESET:
            // Reset to default state
            portalController->setProgram(PORTAL_AMBIENT);
            portalController->setBpm(120.0);
            portalController->setIntensity(0.7);
            portalController->setBaseHue(0.6);
            portalController->setBrightness(LED_BRIGHTNESS_MAX);
            
            #if DEBUG >= 1
            Serial.println("Portal reset to default state");
            #endif
            sendAck();
            break;
            
        default:
            #if DEBUG >= 1
            Serial.printf("Unknown serial command: 0x%02X\n", static_cast<uint8_t>(message.command));
            #endif
            sendNak();
            break;
    }
}

void PortalCueHandler::processSerialInput() {
    while (Serial.available()) {
        uint8_t byte = Serial.read();
        
        // Handle buffer overflow
        if (bufferIndex >= PORTAL_SERIAL_BUFFER_SIZE) {
            #if DEBUG >= 2
            Serial.println("Serial buffer overflow - resetting");
            #endif
            resetSerialBuffer();
        }
        
        serialBuffer[bufferIndex++] = byte;
        lastMessageTime = 0;
        
        // Try to parse message if we have minimum bytes
        if (bufferIndex >= PORTAL_MSG_MIN_SIZE) {
            if (parseSerialMessage()) {
                resetSerialBuffer();
            }
        }
    }
    
    // Timeout incomplete messages
    if (bufferIndex > 0 && lastMessageTime > PORTAL_SERIAL_TIMEOUT_MS) {
        #if DEBUG >= 2
        Serial.println("Serial message timeout - resetting buffer");
        #endif
        resetSerialBuffer();
    }
}

bool PortalCueHandler::parseSerialMessage() {
    // Look for start byte
    int startIndex = -1;
    for (int i = 0; i < bufferIndex; i++) {
        if (serialBuffer[i] == PORTAL_MSG_START_BYTE) {
            startIndex = i;
            break;
        }
    }
    
    if (startIndex == -1) {
        // No start byte found - clear buffer
        resetSerialBuffer();
        return false;
    }
    
    // Shift buffer to start at start byte
    if (startIndex > 0) {
        for (int i = 0; i < bufferIndex - startIndex; i++) {
            serialBuffer[i] = serialBuffer[i + startIndex];
        }
        bufferIndex -= startIndex;
    }
    
    // Check if we have a complete message
    if (bufferIndex >= PORTAL_MSG_MIN_SIZE && 
        serialBuffer[PORTAL_MSG_MIN_SIZE - 1] == PORTAL_MSG_END_BYTE) {
        
        // Parse the message
        PortalMessage message = PortalMessage::fromBytes(serialBuffer);
        messagesReceived++;
        
        if (message.isValid()) {
            messagesValid++;
            handleSerialMessage(message);
            return true;
        } else {
            messagesInvalid++;
            #if DEBUG >= 2
            Serial.printf("Invalid message checksum: got 0x%02X, expected 0x%02X\n",
                         message.checksum, 
                         PortalMessage::calculateChecksum(message.command, message.value));
            #endif
            sendNak();
        }
    }
    
    return false;
}

void PortalCueHandler::resetSerialBuffer() {
    bufferIndex = 0;
    memset(serialBuffer, 0, PORTAL_SERIAL_BUFFER_SIZE);
}

void PortalCueHandler::sendMessage(const PortalMessage& message) {
    uint8_t buffer[5];
    message.toBytes(buffer);
    Serial.write(buffer, 5);
    Serial.flush();
}

void PortalCueHandler::sendAck() {
    PortalMessage ack(PortalSerialCommand::ACK, 0);
    sendMessage(ack);
}

void PortalCueHandler::sendNak() {
    PortalMessage nak(PortalSerialCommand::NAK, 0);
    sendMessage(nak);
}

void PortalCueHandler::sendPong() {
    PortalMessage pong(PortalSerialCommand::PONG, 0);
    sendMessage(pong);
}

void PortalCueHandler::sendStatus() {
    if (!portalController) return;
    
    PortalMessage status(PortalSerialCommand::STATUS, portalController->getCurrentProgram());
    sendMessage(status);
}
