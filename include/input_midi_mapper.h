#pragma once

#include "input_scanner.h"
#include "midi_out.h"

/**
 * @brief Maps raw input events to MIDI messages
 * 
 * Phase 1 implementation - naive mapping without debouncing or filtering.
 * Handles all input types according to the mapping defined in config.h
 */
class InputMidiMapper {
public:
    InputMidiMapper(InputScanner& scanner, MidiOut& midiOut);
    
    /**
     * @brief Process all input changes and send MIDI
     * Call this after InputScanner::scan()
     */
    void processInputs();
    
private:
    InputScanner& scanner_;
    MidiOut& midiOut_;
    
    void processButtons();
    void processPots();
    void processJoystick();
    void processSwitches();
    
    // Convert 10-bit ADC (0-1023) to 7-bit MIDI (0-127)
    uint8_t adcToMidi(uint16_t adcValue);
};
