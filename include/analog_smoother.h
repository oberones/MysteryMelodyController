#pragma once

#include <Arduino.h>
#include "config.h"

/**
 * @brief Exponential Moving Average (EMA) filter for analog inputs
 * 
 * Provides noise reduction and change compression for potentiometers.
 * Uses fixed-point arithmetic to avoid floating point in hot path.
 * 
 * Phase 2: Robust Input Layer
 */
class AnalogSmoother {
public:
    /**
     * @brief Constructor with smoothing parameters
     * @param alpha Smoothing factor (0-255, where 64 ≈ 0.25)
     * @param deadband Minimum change to register (typically 2)
     * @param rateLimitMs Minimum time between output changes (typically 15ms)
     */
    explicit AnalogSmoother(uint8_t alpha = 64, uint8_t deadband = POT_DEADBAND, 
                           uint8_t rateLimitMs = POT_RATE_LIMIT_MS);
    
    /**
     * @brief Update filter with new input value
     * @param rawValue Raw 10-bit ADC reading (0-1023)
     * @param timestampMs Current system time in milliseconds
     * @return true if filtered output changed and should be sent
     */
    bool update(uint16_t rawValue, uint32_t timestampMs);
    
    /**
     * @brief Get current filtered value in MIDI range
     * @return Filtered value mapped to 0-127
     */
    uint8_t getMidiValue() const { return midiValue; }
    
    /**
     * @brief Get raw filtered value (0-1023)
     * @return Raw filtered value before MIDI mapping
     */
    uint16_t getRawFiltered() const { return filteredValue; }
    
    /**
     * @brief Check if value has changed significantly
     * @return true if change exceeds deadband threshold
     */
    bool hasSignificantChange() const { return significantChange; }
    
    /**
     * @brief Force next update to send regardless of rate limit
     * Useful for large threshold changes
     */
    void forceNextSend() { forceSend = true; }
    
    /**
     * @brief Reset filter state
     * @param initialValue Starting value (default: 0)
     */
    void reset(uint16_t initialValue = 0);
    
private:
    // Filter parameters
    uint8_t alpha;          // EMA alpha (fixed point: 0-255)
    uint8_t deadband;       // Minimum change threshold
    uint8_t rateLimitMs;    // Rate limiting interval
    
    // Filter state
    uint16_t filteredValue; // Current filtered value (0-1023)
    uint8_t midiValue;      // Current MIDI value (0-127)
    uint8_t lastSentMidi;   // Last sent MIDI value
    uint32_t lastSendTime;  // Last time value was sent
    bool significantChange; // Flag for significant change
    bool forceSend;         // Force next send flag
    
    /**
     * @brief Map 10-bit value to 7-bit MIDI range
     * @param value Input value (0-1023)
     * @return MIDI value (0-127)
     */
    uint8_t mapToMidi(uint16_t value) const;
};
