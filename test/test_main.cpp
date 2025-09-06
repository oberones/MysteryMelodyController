#include <unity.h>
#include "debouncer.h"
#include "analog_smoother.h"

// ===== DEBOUNCER TESTS =====

void test_debouncer_initial_state() {
    Debouncer db(5); // 5ms debounce
    TEST_ASSERT_FALSE(db.isPressed());
    TEST_ASSERT_FALSE(db.justPressed());
    TEST_ASSERT_FALSE(db.justReleased());
}

void test_debouncer_press_sequence() {
    Debouncer db(5);
    
    // Simulate button press at t=0
    bool changed = db.update(true, 0);
    TEST_ASSERT_FALSE(changed);      // Too early to be stable
    TEST_ASSERT_FALSE(db.isPressed()); // Should still be false
    TEST_ASSERT_FALSE(db.justPressed());
    
    // Still pressed at t=3ms (not enough time)
    changed = db.update(true, 3);
    TEST_ASSERT_FALSE(changed);
    TEST_ASSERT_FALSE(db.isPressed());
    
    // Still pressed at t=6ms (enough time)
    changed = db.update(true, 6);
    TEST_ASSERT_TRUE(changed);       // Should register change
    TEST_ASSERT_TRUE(db.isPressed()); // Should be pressed
    TEST_ASSERT_TRUE(db.justPressed()); // Should be just pressed
    TEST_ASSERT_FALSE(db.justReleased());
}

void test_debouncer_bounce_immunity() {
    Debouncer db(10); // 10ms debounce for this test
    
    // Simulate switch bounce: rapid on-off-on sequence
    db.update(true, 0);   // Initial press
    db.update(false, 2);  // Bounce off
    db.update(true, 4);   // Bounce on
    db.update(false, 6);  // Bounce off
    db.update(true, 8);   // Bounce on
    
    // None of these should register as state changes
    TEST_ASSERT_FALSE(db.isPressed());
    
    // Finally stable at t=15ms
    bool changed = db.update(true, 15);
    TEST_ASSERT_TRUE(changed);
    TEST_ASSERT_TRUE(db.isPressed());
    TEST_ASSERT_TRUE(db.justPressed());
}

// ===== ANALOG SMOOTHER TESTS =====

void test_analog_smoother_initial_state() {
    AnalogSmoother smoother(64, 2, 15); // alpha=64, deadband=2, rateLimit=15ms
    TEST_ASSERT_EQUAL_UINT8(0, smoother.getMidiValue());
    TEST_ASSERT_FALSE(smoother.hasSignificantChange());
}

void test_analog_smoother_reset() {
    AnalogSmoother smoother(64, 2, 15);
    
    // Reset with specific value
    smoother.reset(512); // Mid-range ADC value
    
    // Should map to ~63-64 in MIDI range (512 * 127 / 1023 ≈ 63.5)
    uint8_t initialMidi = smoother.getMidiValue();
    TEST_ASSERT_TRUE(initialMidi >= 63 && initialMidi <= 64);
}

void test_analog_smoother_midi_mapping() {
    AnalogSmoother smoother(255, 1, 1); // Fast response for testing
    
    // Test boundary conditions
    smoother.reset(0);
    smoother.update(0, 0);
    TEST_ASSERT_EQUAL_UINT8(0, smoother.getMidiValue());
    
    smoother.reset(1023);
    smoother.update(1023, 0);
    TEST_ASSERT_EQUAL_UINT8(127, smoother.getMidiValue());
    
    // Test mid-range
    smoother.reset(511);
    smoother.update(511, 0);
    uint8_t midValue = smoother.getMidiValue();
    TEST_ASSERT_TRUE(midValue >= 62 && midValue <= 65); // ~63.5 expected
}

void test_analog_smoother_deadband() {
    AnalogSmoother smoother(64, 5, 10); // deadband=5
    smoother.reset(500); // Start at mid-range
    
    uint8_t startMidi = smoother.getMidiValue();
    
    // Small changes within deadband should not trigger
    smoother.update(505, 20);  // Small increase
    smoother.update(495, 40);  // Small decrease
    
    // Large change should trigger  
    smoother.update(600, 60);  // Significant increase
    TEST_ASSERT_TRUE(smoother.getMidiValue() != startMidi || smoother.hasSignificantChange());
}

// ===== MAIN TEST RUNNER =====

void setUp(void) {
    // Set up code if needed
}

void tearDown(void) {
    // Clean up code if needed
}

void setup() {
    delay(2000); // Wait for serial
    
    UNITY_BEGIN();
    
    // Debouncer tests
    RUN_TEST(test_debouncer_initial_state);
    RUN_TEST(test_debouncer_press_sequence);
    RUN_TEST(test_debouncer_bounce_immunity);
    
    // Analog smoother tests
    RUN_TEST(test_analog_smoother_initial_state);
    RUN_TEST(test_analog_smoother_reset);
    RUN_TEST(test_analog_smoother_midi_mapping);
    RUN_TEST(test_analog_smoother_deadband);
    
    UNITY_END();
}

void loop() {
    // Empty
}
