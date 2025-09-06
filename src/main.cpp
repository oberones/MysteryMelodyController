#include <Arduino.h>
#include <FastLED.h>
#include "pins.h"
#include "config.h"
#include "robust_input_processor.h"
#include "midi_out.h"
#include "robust_midi_mapper.h"
#include "oled_display.h"

// Forward declarations
void portalStartupSequence();
void updatePortal();
void updateOledInputData();

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

// ===== SETUP FUNCTION =====
void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);  // Give time for serial to initialize
    
    Serial.println("=== Mystery Melody Machine Teensy Firmware ===");
    Serial.println("Phase 2: Robust Input Layer + MIDI + OLED");
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
    Serial.println("Phase 2: Debouncing, smoothing, rate limiting, and OLED logging active");
    Serial.println("OLED controls: Button 0 = next mode, Button 1 = prev mode");
    Serial.println("Entering main loop...");
}

// ===== PORTAL STARTUP SEQUENCE =====
void portalStartupSequence() {
    // Simple startup animation: sweep colors around the ring
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < LED_COUNT; i++) {
            // Clear all LEDs
            FastLED.clear();
            
            // Set current LED to a rotating hue
            uint8_t hue = (i * 255 / LED_COUNT) + (cycle * 85);
            leds[i] = CHSV(hue, 255, 128);
            
            // Set a few trailing LEDs for a comet effect
            for (int j = 1; j <= 3 && i - j >= 0; j++) {
                leds[i - j] = CHSV(hue, 255, 128 / (j + 1));
            }
            
            FastLED.show();
            delay(30);  // 30ms per step
        }
    }
    
    // Fade to black
    for (int brightness = 128; brightness >= 0; brightness -= 4) {
        FastLED.setBrightness(brightness);
        FastLED.show();
        delay(20);
    }
    
    FastLED.setBrightness(LED_BRIGHTNESS_MAX);
    FastLED.clear();
    FastLED.show();
    
    Serial.println("Portal startup sequence complete");
}

// ===== BASIC PORTAL ANIMATION =====
void updatePortal() {
    // Simple breathing effect for Phase 1
    static uint32_t animationPhase = 0;
    animationPhase += 2;
    
    uint8_t brightness = (sin8(animationPhase / 4) / 4) + 32;  // Gentle breathing
    
    for (int i = 0; i < LED_COUNT; i++) {
        // Gentle blue breathing
        leds[i] = CHSV(160, 200, brightness);
    }
    
    FastLED.show();
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
        
        // Handle any incoming MIDI (for future portal cues)
        #ifdef USB_MIDI
        while (usbMIDI.read()) {
            // Placeholder for portal cue handling
        }
        #endif
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
        updatePortal();
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
