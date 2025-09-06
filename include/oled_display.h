#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"
#include "config.h"

/**
 * @brief OLED Display Controller for Mystery Melody Machine
 * 
 * Manages a 128x64 I2C OLED display with multiple display modes.
 * Default mode logs all outgoing MIDI messages.
 * 
 * Display modes:
 * - MIDI_LOG: Shows recent MIDI messages (default)
 * - STATUS: Shows system status and input values
 * - ACTIVITY: Shows input activity visualization
 * - INFO: Shows device information and settings
 */
class OledDisplay {
public:
    enum DisplayMode {
        MIDI_LOG = 0,    // Log of outgoing MIDI messages
        STATUS = 1,      // System status and input values
        ACTIVITY = 2,    // Input activity visualization
        INFO = 3,        // Device info and settings
        MODE_COUNT       // Total number of modes
    };
    
    /**
     * @brief Constructor
     */
    OledDisplay();
    
    /**
     * @brief Initialize the OLED display
     * @return true if initialization successful, false otherwise
     */
    bool begin();
    
    /**
     * @brief Update the display content (call regularly)
     */
    void update();
    
    /**
     * @brief Switch to next display mode
     */
    void nextMode();
    
    /**
     * @brief Switch to previous display mode
     */
    void prevMode();
    
    /**
     * @brief Set specific display mode
     * @param mode The mode to set
     */
    void setMode(DisplayMode mode);
    
    /**
     * @brief Get current display mode
     * @return Current display mode
     */
    DisplayMode getMode() const { return currentMode; }
    
    /**
     * @brief Log a MIDI note on message
     * @param note MIDI note number
     * @param velocity Note velocity
     * @param channel MIDI channel
     */
    void logMidiNoteOn(uint8_t note, uint8_t velocity, uint8_t channel);
    
    /**
     * @brief Log a MIDI note off message
     * @param note MIDI note number
     * @param velocity Release velocity
     * @param channel MIDI channel
     */
    void logMidiNoteOff(uint8_t note, uint8_t velocity, uint8_t channel);
    
    /**
     * @brief Log a MIDI control change message
     * @param controller CC number
     * @param value CC value
     * @param channel MIDI channel
     */
    void logMidiCC(uint8_t controller, uint8_t value, uint8_t channel);
    
    /**
     * @brief Update input values for status display
     * @param buttonStates Array of button states
     * @param potValues Array of potentiometer values (0-127)
     * @param switchStates Array of switch states
     * @param joystickStates Array of 4 joystick direction states
     */
    void updateInputStatus(const bool* buttonStates, const uint8_t* potValues, 
                          const bool* switchStates, const bool* joystickStates);
    
    /**
     * @brief Set activity indicators for visualization
     * @param buttonActivity Bitmask of recently active buttons
     * @param potActivity Bitmask of recently active pots
     * @param switchActivity Bitmask of recently active switches
     */
    void setActivity(uint16_t buttonActivity, uint8_t potActivity, uint16_t switchActivity);
    
    /**
     * @brief Update system information
     * @param loopTimeUs Current main loop time in microseconds
     * @param isIdle Whether system is in idle state
     * @param uptime System uptime in milliseconds
     */
    void updateSystemInfo(uint32_t loopTimeUs, bool isIdle, uint32_t uptime);

private:
    // Display hardware
    Adafruit_SSD1306 display;
    bool initialized;
    
    // Display state
    DisplayMode currentMode;
    uint32_t lastUpdate;
    uint32_t modeDisplayTime;  // Time when current mode was set
    
    // MIDI log state
    static constexpr uint8_t MIDI_LOG_SIZE = 8;
    struct MidiMessage {
        char text[32];
        uint32_t timestamp;
        bool valid;
    };
    MidiMessage midiLog[MIDI_LOG_SIZE];
    uint8_t midiLogIndex;
    
    // Input status cache
    bool buttonStates[10];
    uint8_t potValues[6];
    bool switchStates[12];
    bool joystickStates[4];
    
    // Activity indicators
    uint16_t buttonActivity;
    uint8_t potActivity;
    uint16_t switchActivity;
    
    // System info
    uint32_t loopTimeUs;
    bool isIdle;
    uint32_t uptime;
    
    // Private methods
    void drawMidiLog();
    void drawStatus();
    void drawActivity();
    void drawInfo();
    void addMidiLogEntry(const char* message);
    void drawHeader();
    void drawScrollIndicator();
    
    // Note name conversion
    const char* getNoteNameFromMidi(uint8_t note);
    static const char* NOTE_NAMES[12];
};
