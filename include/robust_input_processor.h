#pragma once

#include <Arduino.h>
#include "input_scanner.h"
#include "debouncer.h"
#include "analog_smoother.h"
#include "config.h"

/**
 * @brief Robust input processing layer with debouncing and smoothing
 * 
 * Phase 2: Wraps raw InputScanner with debouncing for digital inputs
 * and EMA smoothing for analog inputs. Provides clean interfaces for
 * MIDI mapping layer.
 */
class RobustInputProcessor {
public:
    RobustInputProcessor();
    
    /**
     * @brief Initialize the processor and underlying scanner
     */
    void begin();
    
    /**
     * @brief Process all inputs with debouncing and smoothing
     * Call this from main loop at 1kHz
     */
    void update();
    
    // Debounced button access
    bool getButtonPressed(uint8_t buttonIndex) const;
    bool getButtonReleased(uint8_t buttonIndex) const;
    bool getButtonState(uint8_t buttonIndex) const;
    
    // Debounced joystick access (single pulse per press)
    bool getJoystickPressed(uint8_t direction) const;
    
    // Debounced switch access
    bool getSwitchState(uint8_t switchIndex) const;
    bool getSwitchChanged(uint8_t switchIndex) const;
    
    // Smoothed potentiometer access
    uint8_t getPotMidiValue(uint8_t potIndex) const;
    bool getPotChanged(uint8_t potIndex) const;
    
    // Idle detection
    uint32_t getTimeSinceLastActivity() const;
    bool isIdle() const;
    
    // Test mode support
    void enableTestMode(bool enable) { testModeEnabled = enable; }
    void dumpTestValues() const;
    
private:
    // Raw input scanner
    InputScanner scanner;
    
    // Debounced button states
    Debouncer buttonDebouncers[BUTTON_COUNT];
    
    // Joystick with rearm timing
    Debouncer joystickDebouncers[4];
    uint32_t joystickRearmTime[4];
    
    // Debounced switch states
    Debouncer switchDebouncers[SWITCH_COUNT];
    
    // Smoothed potentiometer states
    AnalogSmoother potSmoothers[POT_COUNT];
    
    // Activity tracking
    uint32_t lastActivityTime;
    
    // Test mode
    bool testModeEnabled;
    
    /**
     * @brief Update activity timer
     */
    void updateActivity();
    
    /**
     * @brief Process button inputs with debouncing
     */
    void processButtons();
    
    /**
     * @brief Process joystick with debouncing and rearm timing
     */
    void processJoystick();
    
    /**
     * @brief Process switches with debouncing
     */
    void processSwitches();
    
    /**
     * @brief Process potentiometers with smoothing
     */
    void processPotentiometers();
};
