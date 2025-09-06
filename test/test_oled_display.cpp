#include <unity.h>
#include "oled_display.h"

void setUp(void) {
    // Set up test environment
}

void tearDown(void) {
    // Clean up after test
}

void test_oled_display_constructor() {
    OledDisplay display;
    
    // Test initial state
    TEST_ASSERT_EQUAL(OledDisplay::MIDI_LOG, display.getMode());
}

void test_oled_display_mode_switching() {
    OledDisplay display;
    
    // Test mode switching
    display.nextMode();
    TEST_ASSERT_EQUAL(OledDisplay::STATUS, display.getMode());
    
    display.nextMode();
    TEST_ASSERT_EQUAL(OledDisplay::ACTIVITY, display.getMode());
    
    display.nextMode();
    TEST_ASSERT_EQUAL(OledDisplay::INFO, display.getMode());
    
    // Test wrap-around
    display.nextMode();
    TEST_ASSERT_EQUAL(OledDisplay::MIDI_LOG, display.getMode());
    
    // Test previous mode
    display.prevMode();
    TEST_ASSERT_EQUAL(OledDisplay::INFO, display.getMode());
}

void test_oled_display_set_mode() {
    OledDisplay display;
    
    display.setMode(OledDisplay::ACTIVITY);
    TEST_ASSERT_EQUAL(OledDisplay::ACTIVITY, display.getMode());
    
    display.setMode(OledDisplay::INFO);
    TEST_ASSERT_EQUAL(OledDisplay::INFO, display.getMode());
    
    // Test invalid mode (should be ignored)
    display.setMode(OledDisplay::MODE_COUNT);
    TEST_ASSERT_EQUAL(OledDisplay::INFO, display.getMode());
}

void test_oled_midi_logging() {
    OledDisplay display;
    
    // Test MIDI logging functions (these should not crash)
    display.logMidiNoteOn(60, 100, 1);
    display.logMidiNoteOff(60, 0, 1);
    display.logMidiCC(1, 64, 1);
    
    // If we get here without crashing, the test passes
    TEST_ASSERT_TRUE(true);
}

void test_oled_input_status_update() {
    OledDisplay display;
    
    // Test input status updates
    bool buttonStates[10] = {true, false, true, false, false, false, false, false, false, false};
    uint8_t potValues[6] = {0, 32, 64, 96, 127, 50};
    bool switchStates[12] = {true, true, false, false, true, false, false, false, false, false, false, false};
    bool joystickStates[4] = {true, false, false, false};
    
    display.updateInputStatus(buttonStates, potValues, switchStates, joystickStates);
    
    // If we get here without crashing, the test passes
    TEST_ASSERT_TRUE(true);
}

int main() {
    UNITY_BEGIN();
    
    RUN_TEST(test_oled_display_constructor);
    RUN_TEST(test_oled_display_mode_switching);
    RUN_TEST(test_oled_display_set_mode);
    RUN_TEST(test_oled_midi_logging);
    RUN_TEST(test_oled_input_status_update);
    
    return UNITY_END();
}
