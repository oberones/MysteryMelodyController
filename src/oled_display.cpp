#include "oled_display.h"

// Note names for MIDI display
const char* OledDisplay::NOTE_NAMES[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

OledDisplay::OledDisplay() : 
    display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1),
    initialized(false),
    currentMode(MIDI_LOG),
    lastUpdate(0),
    modeDisplayTime(0),
    midiLogIndex(0),
    buttonActivity(0),
    potActivity(0),
    switchActivity(0),
    loopTimeUs(0),
    isIdle(false),
    uptime(0)
{
    // Initialize MIDI log
    for (int i = 0; i < MIDI_LOG_SIZE; i++) {
        midiLog[i].valid = false;
        midiLog[i].timestamp = 0;
        midiLog[i].text[0] = '\0';
    }
    
    // Initialize input state arrays
    for (int i = 0; i < 10; i++) buttonStates[i] = false;
    for (int i = 0; i < 6; i++) potValues[i] = 0;
    for (int i = 0; i < 12; i++) switchStates[i] = false;
    for (int i = 0; i < 4; i++) joystickStates[i] = false;
}

bool OledDisplay::begin() {
    // Initialize I2C
    Wire.begin();
    
    // Initialize display with I2C address
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        #if DEBUG >= 1
        Serial.println("OLED: Failed to initialize SSD1306 display");
        #endif
        return false;
    }
    
    // Initial display setup
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    // Show startup message
    display.println("Mystery Melody");
    display.println("Machine");
    display.println("");
    display.println("OLED Initialized");
    display.display();
    
    initialized = true;
    modeDisplayTime = millis();
    
    #if DEBUG >= 1
    Serial.printf("OLED: Initialized %dx%d display at 0x%02X\n", 
                  OLED_WIDTH, OLED_HEIGHT, OLED_I2C_ADDRESS);
    #endif
    
    return true;
}

void OledDisplay::update() {
    if (!initialized) return;
    
    // Update every 50ms (20Hz) to avoid flickering
    uint32_t now = millis();
    if (now - lastUpdate < 50) return;
    lastUpdate = now;
    
    display.clearDisplay();
    
    // Draw current mode content
    switch (currentMode) {
        case MIDI_LOG:
            drawMidiLog();
            break;
        case STATUS:
            drawStatus();
            break;
        case ACTIVITY:
            drawActivity();
            break;
        case INFO:
            drawInfo();
            break;
        case MODE_COUNT:
        default:
            // Invalid mode, reset to default
            currentMode = MIDI_LOG;
            drawMidiLog();
            break;
    }
    
    display.display();
}

void OledDisplay::nextMode() {
    currentMode = static_cast<DisplayMode>((currentMode + 1) % MODE_COUNT);
    modeDisplayTime = millis();
    
    #if DEBUG >= 2
    Serial.printf("OLED: Switched to mode %d\n", currentMode);
    #endif
}

void OledDisplay::prevMode() {
    currentMode = static_cast<DisplayMode>((currentMode + MODE_COUNT - 1) % MODE_COUNT);
    modeDisplayTime = millis();
    
    #if DEBUG >= 2
    Serial.printf("OLED: Switched to mode %d\n", currentMode);
    #endif
}

void OledDisplay::setMode(DisplayMode mode) {
    if (mode < MODE_COUNT) {
        currentMode = mode;
        modeDisplayTime = millis();
    }
}

void OledDisplay::logMidiNoteOn(uint8_t note, uint8_t velocity, uint8_t channel) {
    const char* noteName = getNoteNameFromMidi(note);
    int octave = (note / 12) - 1;
    
    char message[32];
    snprintf(message, sizeof(message), "NoteOn %s%d V%d Ch%d", 
             noteName, octave, velocity, channel);
    
    addMidiLogEntry(message);
}

void OledDisplay::logMidiNoteOff(uint8_t note, uint8_t velocity, uint8_t channel) {
    const char* noteName = getNoteNameFromMidi(note);
    int octave = (note / 12) - 1;
    
    char message[32];
    snprintf(message, sizeof(message), "NoteOff %s%d V%d Ch%d", 
             noteName, octave, velocity, channel);
    
    addMidiLogEntry(message);
}

void OledDisplay::logMidiCC(uint8_t controller, uint8_t value, uint8_t channel) {
    char message[32];
    snprintf(message, sizeof(message), "CC%d=%d Ch%d", 
             controller, value, channel);
    
    addMidiLogEntry(message);
}

void OledDisplay::updateInputStatus(const bool* buttonStates, const uint8_t* potValues, 
                                   const bool* switchStates, const bool* joystickStates) {
    if (buttonStates) {
        for (int i = 0; i < 10; i++) {
            this->buttonStates[i] = buttonStates[i];
        }
    }
    
    if (potValues) {
        for (int i = 0; i < 6; i++) {
            this->potValues[i] = potValues[i];
        }
    }
    
    if (switchStates) {
        for (int i = 0; i < 12; i++) {
            this->switchStates[i] = switchStates[i];
        }
    }
    
    if (joystickStates) {
        for (int i = 0; i < 4; i++) {
            this->joystickStates[i] = joystickStates[i];
        }
    }
}

void OledDisplay::setActivity(uint16_t buttonActivity, uint8_t potActivity, uint16_t switchActivity) {
    this->buttonActivity = buttonActivity;
    this->potActivity = potActivity;
    this->switchActivity = switchActivity;
}

void OledDisplay::updateSystemInfo(uint32_t loopTimeUs, bool isIdle, uint32_t uptime) {
    this->loopTimeUs = loopTimeUs;
    this->isIdle = isIdle;
    this->uptime = uptime;
}

void OledDisplay::drawMidiLog() {
    drawHeader();
    
    // Display recent MIDI messages, newest first
    int y = 16;
    int displayed = 0;
    
    for (int i = 0; i < MIDI_LOG_SIZE && displayed < 6; i++) {
        int index = (midiLogIndex - 1 - i + MIDI_LOG_SIZE) % MIDI_LOG_SIZE;
        
        if (midiLog[index].valid) {
            display.setCursor(0, y);
            
            // Show relative timestamp
            uint32_t age = millis() - midiLog[index].timestamp;
            if (age < 10000) {
                display.printf("%ds %s", age / 1000, midiLog[index].text);
            } else {
                display.print(midiLog[index].text);
            }
            
            y += 8;
            displayed++;
        }
    }
    
    // Show message if no MIDI activity
    if (displayed == 0) {
        display.setCursor(0, 28);
        display.println("No MIDI activity");
        display.println("yet...");
    }
}

void OledDisplay::drawStatus() {
    drawHeader();
    
    // Show button states
    display.setCursor(0, 16);
    display.print("Btns:");
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            display.setCursor(0, 24);
            display.print("     ");
        }
        display.print(buttonStates[i] ? "1" : "0");
    }
    
    // Show pot values
    display.setCursor(0, 32);
    display.printf("Pots: %d %d %d %d", 
                   potValues[0], potValues[1], potValues[2], potValues[3]);
    
    // Show switch states
    display.setCursor(0, 40);
    display.print("Switches:");
    display.setCursor(0, 48);
    for (int i = 0; i < 12; i++) {
        display.print(switchStates[i] ? "1" : "0");
        if (i == 7) {
            display.setCursor(0, 56);
        }
    }
    
    // Show joystick
    display.setCursor(80, 48);
    display.printf("Joy:%s%s%s%s", 
                   joystickStates[0] ? "U" : "",
                   joystickStates[1] ? "D" : "",
                   joystickStates[2] ? "L" : "",
                   joystickStates[3] ? "R" : "");
}

void OledDisplay::drawActivity() {
    drawHeader();
    
    // Visual representation of input activity
    int y = 20;
    
    // Button activity bars
    display.setCursor(0, y);
    display.print("Buttons:");
    y += 10;
    
    for (int i = 0; i < 10; i++) {
        int x = i * 12;
        int height = buttonStates[i] ? 8 : (buttonActivity & (1 << i)) ? 4 : 1;
        display.fillRect(x, y, 8, height, SSD1306_WHITE);
    }
    y += 12;
    
    // Pot activity bars
    display.setCursor(0, y);
    display.print("Pots:");
    y += 10;
    
    for (int i = 0; i < 4; i++) {
        int x = i * 30;
        int height = map(potValues[i], 0, 127, 1, 12);
        display.fillRect(x, y, 8, height, SSD1306_WHITE);
        
        // Activity indicator
        if (potActivity & (1 << i)) {
            display.drawRect(x - 1, y - 1, 10, height + 2, SSD1306_WHITE);
        }
    }
}

void OledDisplay::drawInfo() {
    drawHeader();
    
    // System information
    display.setCursor(0, 16);
    display.printf("Loop: %luus", loopTimeUs);
    
    display.setCursor(0, 24);
    display.printf("State: %s", isIdle ? "IDLE" : "ACTIVE");
    
    display.setCursor(0, 32);
    uint32_t uptimeSeconds = uptime / 1000;
    uint32_t minutes = uptimeSeconds / 60;
    uint32_t seconds = uptimeSeconds % 60;
    display.printf("Uptime: %lum%lus", minutes, seconds);
    
    display.setCursor(0, 40);
    display.printf("Mode: %d/%d", currentMode + 1, MODE_COUNT);
    
    display.setCursor(0, 48);
    display.print("MIDI Log Mode");
    
    #ifdef USB_MIDI
    display.setCursor(0, 56);
    display.print("USB: MIDI");
    #else
    display.setCursor(0, 56);
    display.print("USB: Serial");
    #endif
}

void OledDisplay::addMidiLogEntry(const char* message) {
    strncpy(midiLog[midiLogIndex].text, message, sizeof(midiLog[midiLogIndex].text) - 1);
    midiLog[midiLogIndex].text[sizeof(midiLog[midiLogIndex].text) - 1] = '\0';
    midiLog[midiLogIndex].timestamp = millis();
    midiLog[midiLogIndex].valid = true;
    
    midiLogIndex = (midiLogIndex + 1) % MIDI_LOG_SIZE;
}

void OledDisplay::drawHeader() {
    // Mode indicator at top
    const char* modeNames[] = {"MIDI LOG", "STATUS", "ACTIVITY", "INFO"};
    
    display.setCursor(0, 0);
    display.print(modeNames[currentMode]);
    
    // Show mode switch indicator if recently changed
    if (millis() - modeDisplayTime < 2000) {
        display.setCursor(80, 0);
        display.printf("(%d/%d)", currentMode + 1, MODE_COUNT);
    }
    
    // Draw horizontal line
    display.drawLine(0, 12, OLED_WIDTH - 1, 12, SSD1306_WHITE);
}

const char* OledDisplay::getNoteNameFromMidi(uint8_t note) {
    return NOTE_NAMES[note % 12];
}
