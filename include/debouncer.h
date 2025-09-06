#pragma once

#include <Arduino.h>
#include "config.h"

/**
 * @brief Time-based debouncer for digital inputs
 * 
 * Uses configurable debounce window to eliminate switch bounce.
 * Call update() every scan cycle with current input state.
 * 
 * Phase 2: Robust Input Layer
 */
class Debouncer {
public:
    /**
     * @brief Constructor with debounce time
     * @param debounceMs Minimum stable time required (typically 5-10ms)
     */
    explicit Debouncer(uint8_t debounceMs = DEBOUNCE_MS);
    
    /**
     * @brief Update debouncer with current input state
     * @param currentState Raw input reading (true = active)
     * @param timestampMs Current system time in milliseconds
     * @return true if stable state changed since last update
     */
    bool update(bool currentState, uint32_t timestampMs);
    
    /**
     * @brief Get current stable state
     * @return true if input is stably active
     */
    bool isPressed() const { return stableState; }
    
    /**
     * @brief Check if just pressed (rising edge)
     * @return true if transitioned from false to true this update
     */
    bool justPressed() const { return stableState && stateChanged; }
    
    /**
     * @brief Check if just released (falling edge)
     * @return true if transitioned from true to false this update
     */
    bool justReleased() const { return !stableState && stateChanged; }
    
    /**
     * @brief Reset debouncer state
     */
    void reset();
    
private:
    uint8_t debounceMs;
    bool rawState;
    bool stableState;
    bool stateChanged;
    uint32_t lastChangeTime;
};
