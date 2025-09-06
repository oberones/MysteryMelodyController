#include "input_midi_mapper.h"
#include "config.h"

InputMidiMapper::InputMidiMapper(InputScanner& scanner, MidiOut& midiOut)
    : scanner_(scanner), midiOut_(midiOut) {
}

void InputMidiMapper::processInputs() {
    processButtons();
    processPots();
    processJoystick();
    processSwitches();
}

void InputMidiMapper::processButtons() {
    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        if (scanner_.getButtonPressed(i)) {
            // Button pressed - send note on
            midiOut_.sendNoteOn(BUTTON_NOTES[i], MIDI_VELOCITY, MIDI_CHANNEL);
        }
        else if (scanner_.getButtonReleased(i)) {
            // Button released - send note off
            midiOut_.sendNoteOff(BUTTON_NOTES[i], 0, MIDI_CHANNEL);
        }
    }
}

void InputMidiMapper::processPots() {
    for (uint8_t i = 0; i < POT_COUNT; i++) {
        if (scanner_.getPotChanged(i)) {
            // Pot value changed - send CC with raw 0-127 mapping
            uint16_t adcValue = scanner_.getPotValue(i);
            uint8_t midiValue = adcToMidi(adcValue);
            midiOut_.sendControlChange(POT_CCS[i], midiValue, MIDI_CHANNEL);
        }
    }
}

void InputMidiMapper::processJoystick() {
    // Check each direction for press events (edge triggered)
    if (scanner_.getJoystickPressed(0)) {  // Up
        midiOut_.sendControlChange(JOY_UP_CC, 127, MIDI_CHANNEL);
    }
    if (scanner_.getJoystickPressed(1)) {  // Down
        midiOut_.sendControlChange(JOY_DOWN_CC, 127, MIDI_CHANNEL);
    }
    if (scanner_.getJoystickPressed(2)) {  // Left
        midiOut_.sendControlChange(JOY_LEFT_CC, 127, MIDI_CHANNEL);
    }
    if (scanner_.getJoystickPressed(3)) {  // Right
        midiOut_.sendControlChange(JOY_RIGHT_CC, 127, MIDI_CHANNEL);
    }
}

void InputMidiMapper::processSwitches() {
    for (uint8_t i = 0; i < SWITCH_COUNT; i++) {
        if (scanner_.getSwitchChanged(i)) {
            // Switch state changed - send CC with on/off value
            uint8_t value = scanner_.getSwitchState(i) ? 127 : 0;
            midiOut_.sendControlChange(SWITCH_CCS[i], value, MIDI_CHANNEL);
        }
    }
}

uint8_t InputMidiMapper::adcToMidi(uint16_t adcValue) {
    // Convert 10-bit ADC (0-1023) to 7-bit MIDI (0-127)
    // Simple linear mapping with proper rounding
    return (adcValue * 127 + 511) / 1023;
}
