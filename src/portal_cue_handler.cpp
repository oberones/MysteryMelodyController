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
    autoSwitchTimer(0)
{
}

void PortalCueHandler::begin(PortalController* controller) {
    portalController = controller;
    timeSinceLastActivity = 0;
    
    Serial.println("Portal Cue Handler initialized");
    Serial.println("MIDI CC Commands:");
    Serial.printf("  CC %d: Program (0-%d)\n", PORTAL_PROGRAM_CC, PORTAL_PROGRAM_COUNT-1);
    Serial.printf("  CC %d: BPM (60-180)\n", PORTAL_BPM_CC);
    Serial.printf("  CC %d: Intensity (0-127)\n", PORTAL_INTENSITY_CC);
    Serial.printf("  CC %d: Base Hue (0-127)\n", PORTAL_HUE_CC);
    Serial.printf("  CC %d: Brightness (0-127)\n", PORTAL_BRIGHTNESS_CC);
    Serial.printf("  CC %d: Flash Trigger (127)\n", PORTAL_FLASH_CC);
    Serial.printf("  CC %d: Ripple Position (0-44)\n", PORTAL_RIPPLE_CC);
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
    Serial.println("====================");
}
