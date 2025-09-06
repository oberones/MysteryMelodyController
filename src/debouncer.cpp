#include "debouncer.h"

Debouncer::Debouncer(uint8_t debounceMs) 
    : debounceMs(debounceMs)
    , rawState(false)
    , stableState(false)
    , stateChanged(false)
    , lastChangeTime(0)
{
}

bool Debouncer::update(bool currentState, uint32_t timestampMs) {
    stateChanged = false;
    
    // Check if raw state has changed
    if (currentState != rawState) {
        rawState = currentState;
        lastChangeTime = timestampMs;
    }
    
    // Check if enough time has passed for stable state change
    if ((timestampMs - lastChangeTime) >= debounceMs) {
        if (rawState != stableState) {
            stableState = rawState;
            stateChanged = true;
        }
    }
    
    return stateChanged;
}

void Debouncer::reset() {
    rawState = false;
    stableState = false;
    stateChanged = false;
    lastChangeTime = 0;
}
