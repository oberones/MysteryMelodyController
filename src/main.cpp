#include <Arduino.h>
#include <FastLED.h>
#include "pins.h"
#include "config.h"
#include "robust_input_processor.h"
#include "midi_out.h"
#include "robust_midi_mapper.h"
#include "oled_display.h"
#include "portal_controller.h"
#include "portal_cue_handler.h"

// Forward declarations
void portalStartupSequence();
void updateOledInputData();
void handlePortalInteractions();

// ===== GLOBAL VARIABLES =====
CRGB leds[LED_COUNT];
elapsedMicros mainLoopTimer;
elapsedMicros portalFrameTimer;
elapsedMillis blinkTimer;
elapsedMillis testDumpTimer;
elapsedMillis oledUpdateTimer;
bool builtinLedState = false;

// Phase 2: Robust input system modules
RobustInputProcessor inputProcessor;
MidiOut midiOut;
RobustMidiMapper inputMapper(inputProcessor, midiOut);

// OLED Display
OledDisplay oledDisplay;

// Phase 3: Portal Animation System
PortalController portalController;
PortalCueHandler portalCueHandler;

// ===== SETUP FUNCTION =====
void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);  // Give time for serial to initialize
    
    Serial.println("=== Mystery Melody Machine Teensy Firmware ===");
    Serial.println("Phase 3: Portal Animation System");
    Serial.printf("Firmware compiled: %s %s\n", __DATE__, __TIME__);
    #ifdef USB_MIDI
    Serial.println("USB Type: MIDI");
    #else
    Serial.println("USB Type: Serial (Debug Mode)");
    #endif
    
    // Initialize built-in LED for blink test
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW);
    
    // Initialize OLED display first
    Serial.println("Initializing OLED display...");
    if (oledDisplay.begin()) {
        Serial.printf("OLED initialized successfully at I2C address 0x%02X\n", OLED_I2C_ADDRESS);
    } else {
        Serial.println("Warning: OLED initialization failed - continuing without display");
    }
    
    // Wait a moment for the OLED startup message to be visible
    delay(1500);
    
    // Initialize Phase 2 robust input system
    Serial.println("Initializing robust input processor...");
    inputProcessor.begin();
    
    Serial.println("Initializing MIDI output...");
    midiOut.begin();
    
    // Connect MIDI output to OLED for logging
    midiOut.setOledDisplay(&oledDisplay);
    
    Serial.printf("Input mapping: %d buttons, %d pots, %d switches, 4-way joystick\n", 
                  BUTTON_COUNT, POT_COUNT, SWITCH_COUNT);
    Serial.println("Features: debouncing, analog smoothing, change compression, OLED logging");
    
    // Initialize FastLED for portal
    FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS_MAX);
    FastLED.clear();
    FastLED.show();
    Serial.printf("FastLED initialized: %d LEDs on pin %d\n", LED_COUNT, LED_DATA_PIN);
    
    // Initialize Phase 3 Portal Animation System
    Serial.println("Initializing portal controller...");
    portalController.begin(leds);
    portalCueHandler.begin(&portalController);
    
    // Set initial portal program and parameters
    portalController.setProgram(PORTAL_AMBIENT);  // Start with ambient
    portalController.setBpm(120.0);
    portalController.setIntensity(0.7);
    portalController.setBaseHue(0.6);  // Nice blue-purple base
    Serial.println("Portal system ready with 10 animation programs");
    
    // Test MIDI functionality (only if MIDI is available)
    Serial.println("Testing MIDI enumeration...");
    #ifdef USB_MIDI
    midiOut.sendNoteOn(60, 64, MIDI_CHANNEL);  // Test note
    delay(100);
    midiOut.sendNoteOff(60, 0, MIDI_CHANNEL);
    Serial.println("MIDI test note sent (C4)");
    #else
    Serial.println("MIDI not available - debug mode active");
    #endif
    
    // Portal startup sequence
    Serial.println("Starting portal initialization sequence...");
    portalStartupSequence();
    
    // Enable test mode for debugging (can be disabled by setting DEBUG to 0)
    #if DEBUG >= 1
    inputProcessor.enableTestMode(true);
    Serial.println("Test mode enabled - will dump input values every 5 seconds");
    #endif
    
    Serial.println("=== Setup Complete ===");
    Serial.printf("Main loop target: %d Hz\n", SCAN_HZ);
    Serial.printf("Portal target: %d Hz\n", PORTAL_FPS);
    Serial.println("Phase 3: Portal Animation System with 10 programs active");
    Serial.println("OLED controls: Button 0 = next mode, Button 1 = prev mode");
    Serial.println("Portal MIDI CC: 60-66 for program, BPM, intensity, hue, brightness, flash, ripple");
    Serial.println("Entering main loop...");
}

// ===== PORTAL STARTUP SEQUENCE =====
void portalStartupSequence() {
    Serial.println("Starting Portal Animation Showcase...");
    
    // Showcase each animation program briefly
    uint8_t demoPrograms[] = {PORTAL_SPIRAL, PORTAL_PULSE, PORTAL_RAINBOW, PORTAL_WAVE, PORTAL_PLASMA};
    
    for (int p = 0; p < 5; p++) {
        portalController.setProgram(demoPrograms[p]);
        portalController.setIntensity(0.8);
        portalController.setBpm(140);  // Energetic startup tempo
        
        Serial.printf("Demo: %d\n", demoPrograms[p]);
        
        // Run each program for 1 second
        for (int frame = 0; frame < 60; frame++) {  // 60 frames at 60Hz = 1 second
            portalController.update();
            FastLED.show();
            delay(16);  // ~60 FPS
        }
    }
    
    // Final flash sequence
    Serial.println("Startup flash sequence...");
    for (int i = 0; i < 3; i++) {
        portalController.triggerFlash();
        for (int frame = 0; frame < 10; frame++) {
            portalController.update();
            FastLED.show();
            delay(16);
        }
        delay(200);
    }
    
    // Fade to ambient mode
    Serial.println("Transitioning to ambient mode...");
    portalController.setProgram(PORTAL_AMBIENT);
    portalController.setIntensity(0.7);
    portalController.setBpm(120);
    portalController.setBaseHue(0.6);  // Nice blue-purple
    
    Serial.println("Portal startup sequence complete");
}

// ===== PORTAL INTERACTION HANDLING =====
void handlePortalInteractions() {
    // Track input activity for idle detection
    bool hasActivity = false;
    
    // Button press feedback - trigger flash on any button press
    for (int i = 0; i < BUTTON_COUNT; i++) {
        static bool lastButtonStates[10] = {false};
        bool currentState = inputProcessor.getButtonState(i);
        
        if (currentState && !lastButtonStates[i]) {
            // Button just pressed - trigger flash and color change
            portalController.triggerFlash();
            
            // Shift hue slightly on each button press
            float currentHue = (i * 0.1); // Different hue per button
            portalController.setBaseHue(currentHue);
            
            hasActivity = true;
            
            #if DEBUG >= 2
            Serial.printf("Button %d pressed - portal flash + hue shift\n", i);
            #endif
        }
        lastButtonStates[i] = currentState;
    }
    
    // Pot activity feedback - hue rotation based on pot movement
    float totalPotActivity = 0.0;
    for (int i = 0; i < POT_COUNT; i++) {
        if (inputProcessor.getPotChanged(i)) {
            hasActivity = true;
            
            // Get normalized pot value (0.0-1.0)
            float potValue = inputProcessor.getPotMidiValue(i) / 127.0;
            totalPotActivity += potValue;
            
            // Trigger ripple effect at position based on pot
            uint8_t ripplePos = (uint8_t)(potValue * (LED_COUNT - 1));
            portalController.triggerRipple(ripplePos);
        }
    }
    
    // Apply pot-based hue rotation
    if (totalPotActivity > 0.0) {
        float hueShift = fmod(totalPotActivity * 0.2, 1.0);  // Smooth hue changes
        float currentHue = fmod(millis() * 0.0001 + hueShift, 1.0);  // Slow drift + pot influence
        portalController.setBaseHue(currentHue);
        
        // Set activity level for animation intensity
        portalController.setActivityLevel(min(1.0f, totalPotActivity / POT_COUNT));
    }
    
    // Joystick interactions - trigger directional ripples
    for (int dir = 0; dir < 4; dir++) {
        static bool lastJoyStates[4] = {false};
        bool currentState = inputProcessor.getJoystickPressed(dir);
        
        if (currentState && !lastJoyStates[dir]) {
            // Calculate ripple position based on direction
            uint8_t positions[] = {0, LED_COUNT/2, LED_COUNT/4, 3*LED_COUNT/4}; // Up, Down, Left, Right
            portalController.triggerRipple(positions[dir]);
            hasActivity = true;
            
            #if DEBUG >= 2
            Serial.printf("Joystick %s - portal ripple at %d\n", 
                         dir == 0 ? "UP" : dir == 1 ? "DOWN" : dir == 2 ? "LEFT" : "RIGHT",
                         positions[dir]);
            #endif
        }
        lastJoyStates[dir] = currentState;
    }
    
    // Switch changes - program switching (optional)
    if (SWITCH_COUNT > 0) {
        static bool lastSwitchState = false;
        bool currentSwitchState = inputProcessor.getSwitchState(0);  // Use first switch
        
        if (currentSwitchState != lastSwitchState) {
            if (currentSwitchState) {
                // Switch turned on - cycle to next program
                uint8_t nextProgram = (portalController.getCurrentProgram() + 1) % PORTAL_PROGRAM_COUNT;
                portalController.setProgram(nextProgram);
                hasActivity = true;
                
                #if DEBUG >= 1
                Serial.printf("Switch activated - portal program: %d\n", nextProgram);
                #endif
            }
        }
        lastSwitchState = currentSwitchState;
    }
    
    // Update portal cue handler with activity status
    portalCueHandler.setInputActivity(hasActivity);
}

// ===== MAIN LOOP =====
void loop() {
    // Track main loop timing for performance monitoring
    static uint32_t loopStartTime = 0;
    loopStartTime = micros();
    
    // Main scan loop at ~1kHz
    if (mainLoopTimer >= (1000000 / SCAN_HZ)) {
        mainLoopTimer -= (1000000 / SCAN_HZ);
        
        // Phase 2: Robust input processing with debouncing and smoothing
        inputProcessor.update();
        inputMapper.processInputs();
        
        // Handle OLED mode switching with buttons 0 and 1
        static bool button0WasPressed = false;
        static bool button1WasPressed = false;
        
        bool button0IsPressed = inputProcessor.getButtonState(0);
        bool button1IsPressed = inputProcessor.getButtonState(1);
        
        // Button 0: Next mode (on press, not hold)
        if (button0IsPressed && !button0WasPressed) {
            oledDisplay.nextMode();
        }
        button0WasPressed = button0IsPressed;
        
        // Button 1: Previous mode (on press, not hold)
        if (button1IsPressed && !button1WasPressed) {
            oledDisplay.prevMode();
        }
        button1WasPressed = button1IsPressed;
        
        // Update OLED with current input data
        updateOledInputData();
        
        // Phase 3: Handle portal interactions (button presses, pot changes)
        handlePortalInteractions();
        
        // Handle any incoming MIDI (including portal cues)
        #ifdef USB_MIDI
        while (usbMIDI.read()) {
            // Check if it's a portal control CC
            if (usbMIDI.getType() == usbMIDI.ControlChange) {
                portalCueHandler.handleMidiCC(usbMIDI.getData1(), usbMIDI.getData2());
            }
        }
        #endif
        
        // Update portal cue handler (for idle detection, auto-switching)
        portalCueHandler.update();
    }
    
    // OLED display update at ~20Hz (every 50ms)
    if (oledUpdateTimer >= 50) {
        oledUpdateTimer -= 50;
        
        // Update system info for OLED
        uint32_t currentLoopTime = micros() - loopStartTime;
        oledDisplay.updateSystemInfo(currentLoopTime, inputProcessor.isIdle(), millis());
        
        // Update display
        oledDisplay.update();
    }
    
    // Portal animation at ~60Hz
    if (portalFrameTimer >= PORTAL_FRAME_INTERVAL_US) {
        portalFrameTimer -= PORTAL_FRAME_INTERVAL_US;
        
        // Phase 3: Update portal controller (handles all animations)
        portalController.update();
        FastLED.show();
    }
    
    // Built-in LED blink every second to show we're alive
    if (blinkTimer >= 1000) {
        blinkTimer -= 1000;
        builtinLedState = !builtinLedState;
        digitalWrite(BUILTIN_LED_PIN, builtinLedState);
        
        #if DEBUG >= 1
        // Check for idle state
        if (inputProcessor.isIdle()) {
            Serial.printf("Heartbeat - IDLE mode (no activity for %lums)\n", 
                         inputProcessor.getTimeSinceLastActivity());
        } else {
            Serial.printf("Heartbeat - ACTIVE (last activity %lums ago)\n",
                         inputProcessor.getTimeSinceLastActivity());
        }
        #endif
    }
    
    // Test mode: dump input values every 5 seconds
    #if DEBUG >= 1
    if (testDumpTimer >= 5000) {
        testDumpTimer -= 5000;
        inputProcessor.dumpTestValues();
    }
    #endif
}

// ===== OLED INPUT DATA UPDATE =====
void updateOledInputData() {
    // Collect button states
    bool buttonStates[10];
    for (int i = 0; i < BUTTON_COUNT && i < 10; i++) {
        buttonStates[i] = inputProcessor.getButtonState(i);
    }
    
    // Collect pot values (convert to MIDI range 0-127)
    uint8_t potValues[6];
    for (int i = 0; i < POT_COUNT && i < 6; i++) {
        potValues[i] = inputProcessor.getPotMidiValue(i);
    }
    
    // Collect switch states
    bool switchStates[12];
    for (int i = 0; i < SWITCH_COUNT && i < 12; i++) {
        switchStates[i] = inputProcessor.getSwitchState(i);
    }
    
    // Collect joystick states (Up, Down, Left, Right)
    bool joystickStates[4] = {
        inputProcessor.getJoystickPressed(0), // Up
        inputProcessor.getJoystickPressed(1), // Down
        inputProcessor.getJoystickPressed(2), // Left
        inputProcessor.getJoystickPressed(3)  // Right
    };
    
    // Update OLED with current input states
    oledDisplay.updateInputStatus(buttonStates, potValues, switchStates, joystickStates);
    
    // Create activity indicators (simplified for now)
    uint16_t buttonActivity = 0;
    for (int i = 0; i < BUTTON_COUNT && i < 10; i++) {
        if (buttonStates[i]) {
            buttonActivity |= (1 << i);
        }
    }
    
    uint8_t potActivity = 0;
    for (int i = 0; i < POT_COUNT && i < 6; i++) {
        if (inputProcessor.getPotChanged(i)) {
            potActivity |= (1 << i);
        }
    }
    
    uint16_t switchActivity = 0;
    for (int i = 0; i < SWITCH_COUNT && i < 12; i++) {
        if (inputProcessor.getSwitchChanged(i)) {
            switchActivity |= (1 << i);
        }
    }
    
    oledDisplay.setActivity(buttonActivity, potActivity, switchActivity);
}
