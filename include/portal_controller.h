#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "pins.h"

// Portal Control Interface
class PortalController {
public:
    PortalController();
    
    // Core control methods
    void begin(CRGB* ledArray);
    void update();
    
    // Program control
    void setProgram(uint8_t programId);
    uint8_t getCurrentProgram() const { return currentProgram; }
    
    // Animation parameters
    void setBpm(float bpm);
    void setIntensity(float intensity);    // 0.0-1.0
    void setBaseHue(float hue);           // 0.0-1.0 hue shift
    void setBrightness(uint8_t brightness); // 0-255
    
    // Interaction feedback
    void triggerFlash();                  // Button press feedback
    void triggerRipple(uint8_t position); // Positional effect
    void setActivityLevel(float activity); // 0.0-1.0 based on pot/input activity
    
    // State queries
    bool isIdle() const { return currentProgram == PORTAL_IDLE; }
    uint32_t getFrameCount() const { return frameCount; }
    
    // Portal cue handling (hooks for MIDI CC)
    void handlePortalCue(uint8_t cueType, uint8_t value);
    
private:
    // LED array reference
    CRGB* leds;
    
    // Current state
    uint8_t currentProgram;
    float bpm;
    float intensity;
    float baseHue;
    uint8_t globalBrightness;
    float activityLevel;
    
    // Timing
    uint32_t frameCount;
    uint32_t lastUpdateTime;
    elapsedMillis programTimer;
    
    // Animation state
    float animationPhase;
    float spiralPhase;
    float wavePhase;
    uint8_t chaosTimer;
    
    // Interaction effects
    bool flashActive;
    elapsedMillis flashTimer;
    uint8_t flashIntensity;
    
    struct Ripple {
        bool active;
        uint8_t center;
        float radius;
        uint8_t intensity;
        elapsedMillis timer;
    };
    static constexpr uint8_t MAX_RIPPLES = 3;
    Ripple ripples[MAX_RIPPLES];
    
    // BPM sync
    float bpmPhase;
    uint32_t lastBpmUpdate;
    
    // Animation implementations
    void updateSpiral();
    void updatePulse();
    void updateWave();
    void updateChaos();
    void updateAmbient();
    void updateIdle();
    void updateRipple();
    void updateRainbow();
    void updatePlasma();
    void updateBreathe();
    
    // Helper functions
    void clearLeds();
    void applyInteractionEffects();
    CRGB getColorAtPosition(uint8_t position, uint8_t hue, uint8_t sat, uint8_t val);
    uint8_t beat8(uint8_t beatsPerMinute);
    float smoothstep(float edge0, float edge1, float x);
    uint8_t noise8(uint8_t x, uint8_t y = 0);
    
    // Utility functions
    uint8_t wrapAround(int16_t position) const;
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

// Portal Cue Types (for MIDI CC control)
enum PortalCueType {
    PORTAL_CUE_PROGRAM = 0,      // Switch animation program
    PORTAL_CUE_BPM = 1,          // Set BPM (0-127 -> 60-180 BPM)
    PORTAL_CUE_INTENSITY = 2,    // Set intensity level
    PORTAL_CUE_HUE = 3,          // Set base hue
    PORTAL_CUE_BRIGHTNESS = 4,   // Set brightness
    PORTAL_CUE_FLASH = 5,        // Trigger flash effect
    PORTAL_CUE_RIPPLE = 6        // Trigger ripple at position
};

// MIDI CC assignments for portal control
constexpr uint8_t PORTAL_PROGRAM_CC = 60;     // CC 60: Program select (0-9)
constexpr uint8_t PORTAL_BPM_CC = 61;         // CC 61: BPM (0-127 -> 60-180)
constexpr uint8_t PORTAL_INTENSITY_CC = 62;   // CC 62: Intensity (0-127)
constexpr uint8_t PORTAL_HUE_CC = 63;         // CC 63: Base hue (0-127)
constexpr uint8_t PORTAL_BRIGHTNESS_CC = 64;  // CC 64: Brightness (0-127)
constexpr uint8_t PORTAL_FLASH_CC = 65;       // CC 65: Flash trigger (127 = trigger)
constexpr uint8_t PORTAL_RIPPLE_CC = 66;      // CC 66: Ripple position (0-44)
