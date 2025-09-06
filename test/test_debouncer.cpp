#include <unity.h>
#include "debouncer.h"

// Test basic debouncer functionality
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

void test_debouncer_release_sequence() {
    Debouncer db(5);
    
    // Start with button established as pressed
    db.update(true, 0);
    db.update(true, 10);  // Establish pressed state
    
    // Release at t=20
    bool changed = db.update(false, 20);
    TEST_ASSERT_FALSE(changed);      // Too early
    TEST_ASSERT_TRUE(db.isPressed()); // Should still be pressed
    
    // Still released at t=26ms (enough time)
    changed = db.update(false, 26);
    TEST_ASSERT_TRUE(changed);        // Should register change
    TEST_ASSERT_FALSE(db.isPressed()); // Should be released
    TEST_ASSERT_FALSE(db.justPressed());
    TEST_ASSERT_TRUE(db.justReleased()); // Should be just released
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

void test_debouncer_reset() {
    Debouncer db(5);
    
    // Establish pressed state
    db.update(true, 0);
    db.update(true, 10);
    TEST_ASSERT_TRUE(db.isPressed());
    
    // Reset should clear state
    db.reset();
    TEST_ASSERT_FALSE(db.isPressed());
    TEST_ASSERT_FALSE(db.justPressed());
    TEST_ASSERT_FALSE(db.justReleased());
}

void setUp(void) {
    // Set up code if needed
}

void tearDown(void) {
    // Clean up code if needed
}

void setup() {
    delay(2000); // Wait for serial
    
    UNITY_BEGIN();
    
    RUN_TEST(test_debouncer_initial_state);
    RUN_TEST(test_debouncer_press_sequence);
    RUN_TEST(test_debouncer_release_sequence);
    RUN_TEST(test_debouncer_bounce_immunity);
    RUN_TEST(test_debouncer_reset);
    
    UNITY_END();
}

void loop() {
    // Empty
}
