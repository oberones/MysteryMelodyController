#pragma once

#include <Arduino.h>

#ifdef USB_MIDI
#include <usb_midi.h>
#endif

// Forward declaration to avoid circular dependency
class OledDisplay;

/**
 * @brief MIDI output handler for Phase 1
 * 
 * Sends MIDI messages for raw input events.
 * Conditionally compiled based on USB_MIDI mode.
 * Supports optional OLED display logging.
 */
class MidiOut {
public:
    MidiOut();
    
    /**
     * @brief Initialize MIDI output
     */
    void begin();
    
    /**
     * @brief Set OLED display for logging (optional)
     * @param display Pointer to OledDisplay instance, or nullptr to disable
     */
    void setOledDisplay(OledDisplay* display);
    
    /**
     * @brief Send note on message
     * @param note MIDI note number (0-127)
     * @param velocity Velocity (0-127), 0 = note off
     * @param channel MIDI channel (1-16), defaults to 1
     */
    void sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel = 1);
    
    /**
     * @brief Send note off message
     * @param note MIDI note number (0-127)
     * @param velocity Release velocity (0-127)
     * @param channel MIDI channel (1-16), defaults to 1
     */
    void sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel = 1);
    
    /**
     * @brief Send control change message
     * @param controller CC number (0-127)
     * @param value CC value (0-127)
     * @param channel MIDI channel (1-16), defaults to 1
     */
    void sendControlChange(uint8_t controller, uint8_t value, uint8_t channel = 1);
    
private:
    OledDisplay* oledDisplay;
    
    void debugMidi(const char* type, uint8_t param1, uint8_t param2, uint8_t channel);
};
