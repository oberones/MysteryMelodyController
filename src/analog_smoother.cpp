#include "analog_smoother.h"

AnalogSmoother::AnalogSmoother(uint8_t alpha, uint8_t deadband, uint8_t rateLimitMs)
    : alpha(alpha)
    , deadband(deadband)
    , rateLimitMs(rateLimitMs)
    , filteredValue(0)
    , midiValue(0)
    , lastSentMidi(0)
    , lastSendTime(0)
    , significantChange(false)
    , forceSend(false)
{
}

bool AnalogSmoother::update(uint16_t rawValue, uint32_t timestampMs) {
    significantChange = false;
    
    // Apply EMA filter using fixed-point arithmetic
    // filtered = filtered + alpha * (raw - filtered)
    // Using: filtered += (raw - filtered) * alpha / 256
    int32_t error = (int32_t)rawValue - (int32_t)filteredValue;
    filteredValue += (error * alpha) >> 8;  // Divide by 256
    
    // Map to MIDI range
    uint8_t newMidiValue = mapToMidi(filteredValue);
    
    // Check for significant change (exceeds deadband)
    uint8_t deltaFromLast = abs((int16_t)newMidiValue - (int16_t)lastSentMidi);
    if (deltaFromLast >= deadband) {
        significantChange = true;
    }
    
    // Update current MIDI value
    midiValue = newMidiValue;
    
    // Determine if we should send the update
    bool shouldSend = false;
    uint32_t timeSinceLastSend = timestampMs - lastSendTime;
    
    if (forceSend) {
        // Forced send (for large changes)
        shouldSend = true;
        forceSend = false;
    } else if (significantChange && (timeSinceLastSend >= rateLimitMs)) {
        // Significant change and enough time has passed
        shouldSend = true;
    } else if (deltaFromLast >= 8) {
        // Large change overrides rate limit
        shouldSend = true;
        forceNextSend();  // Set flag for next update
    }
    
    if (shouldSend) {
        lastSentMidi = midiValue;
        lastSendTime = timestampMs;
        return true;
    }
    
    return false;
}

uint8_t AnalogSmoother::mapToMidi(uint16_t value) const {
    // Map 0-1023 to 0-127
    // Using: (value * 127) / 1023
    // Optimized: (value >> 3) ensures we stay in 0-127 range
    return min(127, (value * 127) / 1023);
}

void AnalogSmoother::reset(uint16_t initialValue) {
    filteredValue = initialValue;
    midiValue = mapToMidi(initialValue);
    lastSentMidi = midiValue;
    lastSendTime = 0;
    significantChange = false;
    forceSend = false;
}
