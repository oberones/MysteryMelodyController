#include "robust_midi_mapper.h"

RobustMidiMapper::RobustMidiMapper(RobustInputProcessor& processor, MidiOut& midiOut)
    : processor_(processor)
    , midiOut_(midiOut)
{
    // Initialize state tracking arrays
    for (int i = 0; i < BUTTON_COUNT; i++) {
        lastButtonStates[i] = false;
    }
    
    for (int i = 0; i < SWITCH_COUNT; i++) {
        lastSwitchStates[i] = false;
    }
    
    for (int i = 0; i < POT_COUNT; i++) {
        lastPotValues[i] = 0;
    }
}

void RobustMidiMapper::processInputs() {
    // Process all input types in order
    processButtons();
    processPots();
    processJoystick();
    processSwitches();
}

void RobustMidiMapper::processButtons() {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        bool currentState = processor_.getButtonState(i);
        
        // Check for state changes
        if (currentState != lastButtonStates[i]) {
            if (currentState) {
                // Button pressed - send Note On
                midiOut_.sendNoteOn(BUTTON_NOTES[i], MIDI_VELOCITY, MIDI_CHANNEL);
                
                #if DEBUG >= 1
                Serial.printf("MIDI: Button %d pressed -> Note %d ON\n", i, BUTTON_NOTES[i]);
                #endif
            } else {
                // Button released - send Note Off
                midiOut_.sendNoteOff(BUTTON_NOTES[i], 0, MIDI_CHANNEL);
                
                #if DEBUG >= 1
                Serial.printf("MIDI: Button %d released -> Note %d OFF\n", i, BUTTON_NOTES[i]);
                #endif
            }
            
            lastButtonStates[i] = currentState;
        }
    }
}

void RobustMidiMapper::processPots() {
    for (int i = 0; i < POT_COUNT; i++) {
        uint8_t currentValue = processor_.getPotMidiValue(i);
        
        // Check if potentiometer value changed significantly
        if (processor_.getPotChanged(i) && currentValue != lastPotValues[i]) {
            // Send CC message
            midiOut_.sendControlChange(POT_CCS[i], currentValue, MIDI_CHANNEL);
            
            #if DEBUG >= 1
            Serial.printf("MIDI: Pot %d changed -> CC %d = %d\n", i, POT_CCS[i], currentValue);
            #endif
            
            lastPotValues[i] = currentValue;
        }
    }
}

void RobustMidiMapper::processJoystick() {
    // Joystick directions send single pulse CC messages (127 on press, no release)
    if (processor_.getJoystickPressed(0)) {  // Up
        midiOut_.sendControlChange(JOY_UP_CC, 127, MIDI_CHANNEL);
        
        #if DEBUG >= 1
        Serial.printf("MIDI: Joystick UP -> CC %d = 127\n", JOY_UP_CC);
        #endif
    }
    
    if (processor_.getJoystickPressed(1)) {  // Down
        midiOut_.sendControlChange(JOY_DOWN_CC, 127, MIDI_CHANNEL);
        
        #if DEBUG >= 1
        Serial.printf("MIDI: Joystick DOWN -> CC %d = 127\n", JOY_DOWN_CC);
        #endif
    }
    
    if (processor_.getJoystickPressed(2)) {  // Left
        midiOut_.sendControlChange(JOY_LEFT_CC, 127, MIDI_CHANNEL);
        
        #if DEBUG >= 1
        Serial.printf("MIDI: Joystick LEFT -> CC %d = 127\n", JOY_LEFT_CC);
        #endif
    }
    
    if (processor_.getJoystickPressed(3)) {  // Right
        midiOut_.sendControlChange(JOY_RIGHT_CC, 127, MIDI_CHANNEL);
        
        #if DEBUG >= 1
        Serial.printf("MIDI: Joystick RIGHT -> CC %d = 127\n", JOY_RIGHT_CC);
        #endif
    }
}

void RobustMidiMapper::processSwitches() {
    for (int i = 0; i < SWITCH_COUNT; i++) {
        bool currentState = processor_.getSwitchState(i);
        
        // Check for state changes
        if (currentState != lastSwitchStates[i]) {
            uint8_t midiValue = currentState ? 127 : 0;
            
            // Send CC message for switch state
            midiOut_.sendControlChange(SWITCH_CCS[i], midiValue, MIDI_CHANNEL);
            
            #if DEBUG >= 1
            Serial.printf("MIDI: Switch %d %s -> CC %d = %d\n", 
                         i, currentState ? "ON" : "OFF", SWITCH_CCS[i], midiValue);
            #endif
            
            lastSwitchStates[i] = currentState;
        }
    }
}

void RobustMidiMapper::sendAllNotesOff() {
    // Send Note Off for all possible button notes
    for (int i = 0; i < BUTTON_COUNT; i++) {
        midiOut_.sendNoteOff(BUTTON_NOTES[i], 0, MIDI_CHANNEL);
        lastButtonStates[i] = false;  // Reset state tracking
    }
    
    #if DEBUG >= 1
    Serial.println("MIDI: All notes OFF (panic)");
    #endif
}
