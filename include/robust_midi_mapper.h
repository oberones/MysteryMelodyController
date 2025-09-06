#pragma once

#include "robust_input_processor.h"
#include "midi_out.h"

/**
 * @brief Maps robust input events to MIDI messages
 * 
 * Phase 2 implementation - works with debounced and filtered inputs.
 * Handles all input types with proper timing and change detection.
 */
class RobustMidiMapper {
public:
    RobustMidiMapper(RobustInputProcessor& processor, MidiOut& midiOut);
    
    /**
     * @brief Process all input changes and send MIDI
     * Call this after RobustInputProcessor::update()
     */
    void processInputs();
    
    /**
     * @brief Send all notes off (panic function)
     */
    void sendAllNotesOff();
    
private:
    RobustInputProcessor& processor_;
    MidiOut& midiOut_;
    
    // State tracking for edge detection
    bool lastButtonStates[BUTTON_COUNT];
    bool lastSwitchStates[SWITCH_COUNT];
    uint8_t lastPotValues[POT_COUNT];
    uint8_t lastBinaryValue;  // Track binary representation of first 8 switches
    
    void processButtons();
    void processPots();
    void processJoystick();
    void processSwitches();
};
