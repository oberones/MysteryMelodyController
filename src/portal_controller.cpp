#include "portal_controller.h"
#include "pins.h"
#include <FastLED.h>

PortalController::PortalController() : 
    leds(nullptr),
    currentProgram(PORTAL_AMBIENT),
    bpm(120.0),
    intensity(0.7),
    baseHue(0.0),
    globalBrightness(LED_BRIGHTNESS_MAX),
    activityLevel(0.0),
    frameCount(0),
    lastUpdateTime(0),
    animationPhase(0.0),
    spiralPhase(0.0),
    wavePhase(0.0),
    chaosTimer(0),
    flashActive(false),
    flashIntensity(0),
    bpmPhase(0.0),
    lastBpmUpdate(0)
{
    // Initialize ripples as inactive
    for (int i = 0; i < MAX_RIPPLES; i++) {
        ripples[i].active = false;
    }
}

void PortalController::begin(CRGB* ledArray) {
    leds = ledArray;
    frameCount = 0;
    lastUpdateTime = millis();
    
    Serial.println("Portal Controller initialized with 10 animation programs");
    Serial.printf("Current program: %s\n", 
        currentProgram == PORTAL_AMBIENT ? "AMBIENT" : "UNKNOWN");
}

void PortalController::update() {
    if (!leds) return;
    
    frameCount++;
    uint32_t currentTime = millis();
    float deltaTime = (currentTime - lastUpdateTime) / 1000.0;
    lastUpdateTime = currentTime;
    
    // Update animation phase based on time
    animationPhase += deltaTime;
    
    // Update BPM phase
    if (bpm > 0) {
        float bpmDelta = (currentTime - lastBpmUpdate) / 1000.0;
        bpmPhase += bpmDelta * (bpm / 60.0);
        lastBpmUpdate = currentTime;
    }
    
    // Clear the LED array
    clearLeds();
    
    // Run the current animation program
    switch (currentProgram) {
        case PORTAL_SPIRAL:   updateSpiral(); break;
        case PORTAL_PULSE:    updatePulse(); break;
        case PORTAL_WAVE:     updateWave(); break;
        case PORTAL_CHAOS:    updateChaos(); break;
        case PORTAL_AMBIENT:  updateAmbient(); break;
        case PORTAL_IDLE:     updateIdle(); break;
        case PORTAL_RIPPLE:   updateRipple(); break;
        case PORTAL_RAINBOW:  updateRainbow(); break;
        case PORTAL_PLASMA:   updatePlasma(); break;
        case PORTAL_BREATHE:  updateBreathe(); break;
        default: updateAmbient(); break;
    }
    
    // Apply interaction effects (flash, ripples)
    applyInteractionEffects();
}

void PortalController::setProgram(uint8_t programId) {
    if (programId < PORTAL_PROGRAM_COUNT) {
        currentProgram = programId;
        programTimer = 0;  // Reset program timer
        
        // Reset animation state for new program
        animationPhase = 0.0;
        spiralPhase = 0.0;
        wavePhase = 0.0;
        chaosTimer = 0;
        
        Serial.printf("Portal program changed to: %d\n", programId);
    }
}

void PortalController::setBpm(float newBpm) {
    bpm = constrain(newBpm, 60.0, 180.0);
    lastBpmUpdate = millis();
}

void PortalController::setIntensity(float newIntensity) {
    intensity = constrain(newIntensity, 0.0, 1.0);
}

void PortalController::setBaseHue(float hue) {
    baseHue = fmod(hue, 1.0);
    if (baseHue < 0) baseHue += 1.0;
}

void PortalController::setBrightness(uint8_t brightness) {
    globalBrightness = brightness;
}

void PortalController::triggerFlash() {
    flashActive = true;
    flashTimer = 0;
    flashIntensity = 255;
}

void PortalController::triggerRipple(uint8_t position) {
    // Find an inactive ripple slot
    for (int i = 0; i < MAX_RIPPLES; i++) {
        if (!ripples[i].active) {
            ripples[i].active = true;
            ripples[i].center = position % LED_COUNT;
            ripples[i].radius = 0.0;
            ripples[i].intensity = 200;
            ripples[i].timer = 0;
            break;
        }
    }
}

void PortalController::setActivityLevel(float activity) {
    activityLevel = constrain(activity, 0.0, 1.0);
}

void PortalController::handlePortalCue(uint8_t cueType, uint8_t value) {
    switch (cueType) {
        case PORTAL_CUE_PROGRAM:
            if (value < PORTAL_PROGRAM_COUNT) {
                setProgram(value);
            }
            break;
            
        case PORTAL_CUE_BPM:
            // Map 0-127 to 60-180 BPM
            setBpm(mapFloat(value, 0, 127, 60, 180));
            break;
            
        case PORTAL_CUE_INTENSITY:
            setIntensity(value / 127.0);
            break;
            
        case PORTAL_CUE_HUE:
            setBaseHue(value / 127.0);
            break;
            
        case PORTAL_CUE_BRIGHTNESS:
            setBrightness((uint8_t)mapFloat(value, 0, 127, 0, LED_BRIGHTNESS_MAX));
            break;
            
        case PORTAL_CUE_FLASH:
            if (value >= 64) {  // Trigger on high values
                triggerFlash();
            }
            break;
            
        case PORTAL_CUE_RIPPLE:
            triggerRipple((value * LED_COUNT) / 127);
            break;
    }
}

// ===== ANIMATION IMPLEMENTATIONS =====

void PortalController::updateSpiral() {
    spiralPhase += 0.02 * intensity;
    
    float bpmSpeed = sin(bpmPhase * 2 * PI) * 0.5 + 1.0;  // BPM sync
    
    for (int i = 0; i < LED_COUNT; i++) {
        float angle = (float)i / LED_COUNT * 2 * PI;
        float spiral = sin(angle * 3 + spiralPhase * bpmSpeed);
        
        uint8_t hue = (uint8_t)((baseHue * 255) + (spiral * 60) + (animationPhase * 20)) % 256;
        uint8_t brightness = (uint8_t)(128 + spiral * 100 * intensity);
        
        leds[i] = CHSV(hue, 240, brightness);
    }
}

void PortalController::updatePulse() {
    // Create pulses that emanate from center, synced to BPM
    float pulse = sin(bpmPhase * 2 * PI);
    pulse = max(0.0f, pulse);  // Only positive pulses
    
    for (int i = 0; i < LED_COUNT; i++) {
        // Distance from "center" (position 0)
        int distFromCenter = min(i, LED_COUNT - i);
        float normalizedDist = (float)distFromCenter / (LED_COUNT / 2);
        
        // Create expanding ring effect
        float ring = 1.0 - abs(normalizedDist - (pulse * 0.8));
        ring = max(0.0f, ring * 5.0f);  // Sharpen the ring
        ring = min(1.0f, ring);
        
        uint8_t hue = (uint8_t)((baseHue * 255) + (i * 5)) % 256;
        uint8_t brightness = (uint8_t)(ring * pulse * intensity * 200);
        
        leds[i] = CHSV(hue, 220, brightness);
    }
}

void PortalController::updateWave() {
    wavePhase += 0.05 * intensity;
    
    // Multiple wave frequencies for complexity
    for (int i = 0; i < LED_COUNT; i++) {
        float pos = (float)i / LED_COUNT * 2 * PI;
        
        float wave1 = sin(pos * 2 + wavePhase);
        float wave2 = sin(pos * 3 - wavePhase * 0.7) * 0.5;
        float bpmWave = sin(bpmPhase * 2 * PI) * 0.3;
        
        float combined = (wave1 + wave2 + bpmWave) * intensity;
        
        uint8_t hue = (uint8_t)((baseHue * 255) + (combined * 40) + (i * 3)) % 256;
        uint8_t brightness = (uint8_t)(100 + combined * 100 + activityLevel * 50);
        
        leds[i] = CHSV(hue, 200, max(0, (int)brightness));
    }
}

void PortalController::updateChaos() {
    chaosTimer++;
    
    // Random mutations based on BPM and activity
    if (chaosTimer % (max(1, (int)(20 - intensity * 15))) == 0) {
        for (int i = 0; i < LED_COUNT; i++) {
            if (random8() < (50 + activityLevel * 100)) {
                uint8_t hue = (baseHue * 255) + random8();
                uint8_t sat = 200 + random8(55);
                uint8_t brightness = 80 + random8((uint8_t)(intensity * 150));
                
                leds[i] = CHSV(hue, sat, brightness);
            }
        }
    }
    
    // Fade existing pixels
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].fadeToBlackBy(20 + (uint8_t)(intensity * 30));
    }
}

void PortalController::updateAmbient() {
    // Slow, peaceful color cycling
    for (int i = 0; i < LED_COUNT; i++) {
        float slowPhase = animationPhase * 0.1;
        float positionPhase = (float)i / LED_COUNT * PI * 2;
        
        float wave = sin(slowPhase + positionPhase) * 0.3 + 0.7;
        
        uint8_t hue = (uint8_t)((baseHue * 255) + (slowPhase * 20) + (i * 2)) % 256;
        uint8_t brightness = (uint8_t)(wave * intensity * 120 + 20);
        
        leds[i] = CHSV(hue, 180, brightness);
    }
}

void PortalController::updateIdle() {
    // Very minimal, low brightness ambient mode
    uint8_t idleBrightness = (globalBrightness * IDLE_BRIGHTNESS_CAP_PCT) / 100;
    
    for (int i = 0; i < LED_COUNT; i++) {
        float slowPhase = animationPhase * 0.05;  // Very slow
        float gentle = sin(slowPhase + i * 0.1) * 0.2 + 0.8;
        
        uint8_t hue = (uint8_t)((baseHue * 255) + (slowPhase * 10)) % 256;
        uint8_t brightness = (uint8_t)(gentle * idleBrightness * 0.3);
        
        leds[i] = CHSV(hue, 150, brightness);
    }
}

void PortalController::updateRipple() {
    // Primary ripple animation (can be combined with ripple effects)
    updateAmbient();  // Base ambient pattern
    
    // Add continuous ripples from random positions
    if (random8() < (20 + activityLevel * 50)) {
        triggerRipple(random8(LED_COUNT));
    }
}

void PortalController::updateRainbow() {
    // Smooth rainbow that rotates around the circle
    float rainbowSpeed = 0.01 + (bpm / 1200.0);  // BPM affects speed
    
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t hue = (uint8_t)((animationPhase * rainbowSpeed * 255) + (i * 255 / LED_COUNT)) % 256;
        uint8_t brightness = (uint8_t)(intensity * 180 + 40);
        
        leds[i] = CHSV(hue, 255, brightness);
    }
}

void PortalController::updatePlasma() {
    // Plasma-like flowing colors using multiple sine waves
    for (int i = 0; i < LED_COUNT; i++) {
        float x = (float)i / LED_COUNT;
        
        float plasma1 = sin(x * 10 + animationPhase * 0.3);
        float plasma2 = sin(x * 12 - animationPhase * 0.2);
        float plasma3 = sin((x * 8 + animationPhase * 0.15) * 2);
        float bpmPlasma = sin(bpmPhase * 2 * PI) * 0.4;
        
        float combined = (plasma1 + plasma2 + plasma3 + bpmPlasma) / 4.0;
        
        uint8_t hue = (uint8_t)((combined * 60 + baseHue * 255 + animationPhase * 10)) % 256;
        uint8_t brightness = (uint8_t)(120 + combined * 80 * intensity);
        
        leds[i] = CHSV(hue, 230, max(0, (int)brightness));
    }
}

void PortalController::updateBreathe() {
    // Gentle breathing effect synchronized to BPM
    float breathe = sin(bpmPhase * PI) * 0.5 + 0.5;  // 0-1 breathing cycle
    breathe = smoothstep(0.0, 1.0, breathe);  // Smooth curve
    
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t hue = (uint8_t)((baseHue * 255) + (i * 2)) % 256;
        uint8_t brightness = (uint8_t)(breathe * intensity * 150 + 20);
        
        leds[i] = CHSV(hue, 200, brightness);
    }
}

// ===== HELPER FUNCTIONS =====

void PortalController::clearLeds() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CRGB::Black;
    }
}

void PortalController::applyInteractionEffects() {
    // Apply flash effect
    if (flashActive) {
        if (flashTimer < 100) {  // 100ms flash duration
            uint8_t flashBrightness = mapFloat(flashTimer, 0, 100, flashIntensity, 0);
            
            for (int i = 0; i < LED_COUNT; i++) {
                leds[i] += CRGB(flashBrightness, flashBrightness, flashBrightness);
            }
        } else {
            flashActive = false;
        }
    }
    
    // Apply ripple effects
    for (int r = 0; r < MAX_RIPPLES; r++) {
        if (ripples[r].active) {
            if (ripples[r].timer < 1000) {  // 1 second ripple duration
                ripples[r].radius = mapFloat(ripples[r].timer, 0, 1000, 0, LED_COUNT / 2);
                uint8_t rippleIntensity = mapFloat(ripples[r].timer, 0, 1000, ripples[r].intensity, 0);
                
                // Apply ripple to nearby LEDs
                for (int i = 0; i < LED_COUNT; i++) {
                    int distToCenter = min(abs(i - ripples[r].center), 
                                         LED_COUNT - abs(i - ripples[r].center));
                    
                    if (abs(distToCenter - ripples[r].radius) < 2.0) {
                        uint8_t hue = (uint8_t)((baseHue * 255.0) + 60.0) % 256;
                        leds[i] += CHSV(hue, 255, rippleIntensity / 2);
                    }
                }
            } else {
                ripples[r].active = false;
            }
        }
    }
}

// ===== UTILITY FUNCTIONS =====

uint8_t PortalController::wrapAround(int16_t position) const {
    while (position < 0) position += LED_COUNT;
    return position % LED_COUNT;
}

float PortalController::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float PortalController::smoothstep(float edge0, float edge1, float x) {
    float t = constrain((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

uint8_t PortalController::noise8(uint8_t x, uint8_t y) {
    // Simple pseudo-noise function
    uint16_t n = x + y * 57;
    n = (n << 13) ^ n;
    return (uint8_t)((1.0 - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0) * 128 + 128);
}
