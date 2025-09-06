#include "midi_out.h"
#include "oled_display.h"

MidiOut::MidiOut() : oledDisplay(nullptr) {
    // Constructor
}

void MidiOut::begin() {
#ifdef USB_MIDI
    // USB MIDI is automatically initialized when USB_MIDI is defined
    // No explicit initialization needed for Teensy USB MIDI
#else
    // In debug mode, just initialize serial for debug output
    // Serial is already initialized in main.cpp
#endif
}

void MidiOut::setOledDisplay(OledDisplay* display) {
    oledDisplay = display;
}

void MidiOut::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
#ifdef USB_MIDI
    usbMIDI.sendNoteOn(note, velocity, channel);
    usbMIDI.send_now();  // Force immediate send
#else
    debugMidi("NoteOn", note, velocity, channel);
#endif
    
    // Log to OLED if available
    if (oledDisplay) {
        oledDisplay->logMidiNoteOn(note, velocity, channel);
    }
}

void MidiOut::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
#ifdef USB_MIDI
    usbMIDI.sendNoteOff(note, velocity, channel);
    usbMIDI.send_now();  // Force immediate send
#else
    debugMidi("NoteOff", note, velocity, channel);
#endif
    
    // Log to OLED if available
    if (oledDisplay) {
        oledDisplay->logMidiNoteOff(note, velocity, channel);
    }
}

void MidiOut::sendControlChange(uint8_t controller, uint8_t value, uint8_t channel) {
#ifdef USB_MIDI
    usbMIDI.sendControlChange(controller, value, channel);
    usbMIDI.send_now();  // Force immediate send
#else
    debugMidi("CC", controller, value, channel);
#endif
    
    // Log to OLED if available
    if (oledDisplay) {
        oledDisplay->logMidiCC(controller, value, channel);
    }
}

void MidiOut::debugMidi(const char* type, uint8_t param1, uint8_t param2, uint8_t channel) {
#ifndef USB_MIDI
    Serial.print("MIDI ");
    Serial.print(type);
    Serial.print(" Ch:");
    Serial.print(channel);
    Serial.print(" P1:");
    Serial.print(param1);
    Serial.print(" P2:");
    Serial.println(param2);
#endif
}
