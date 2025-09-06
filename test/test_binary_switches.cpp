#include <unity.h>
#include "config.h"

// Test the binary representation logic directly
uint8_t calculateBinaryRepresentation(bool switches[8]) {
    uint8_t binaryValue = 0;
    
    for (int i = 0; i < 8; i++) {
        if (switches[i]) {
            binaryValue |= (1 << i);
        }
    }
    
    return binaryValue;
}

void test_binary_calculation_all_off() {
    bool switches[8] = {false, false, false, false, false, false, false, false};
    uint8_t result = calculateBinaryRepresentation(switches);
    TEST_ASSERT_EQUAL_UINT8(0, result);
}

void test_binary_calculation_single_bits() {
    // Test each individual bit
    for (int i = 0; i < 8; i++) {
        bool switches[8] = {false, false, false, false, false, false, false, false};
        switches[i] = true;
        
        uint8_t result = calculateBinaryRepresentation(switches);
        uint8_t expected = (1 << i);
        
        TEST_ASSERT_EQUAL_UINT8(expected, result);
    }
}

void test_binary_calculation_multiple_bits() {
    // Test pattern: switches 0, 2, 4 on (binary: 00010101 = 21)
    bool switches[8] = {true, false, true, false, true, false, false, false};
    uint8_t result = calculateBinaryRepresentation(switches);
    uint8_t expected = (1 << 0) | (1 << 2) | (1 << 4); // 21
    TEST_ASSERT_EQUAL_UINT8(expected, result);
}

void test_binary_calculation_all_on() {
    bool switches[8] = {true, true, true, true, true, true, true, true};
    uint8_t result = calculateBinaryRepresentation(switches);
    TEST_ASSERT_EQUAL_UINT8(255, result); // 0b11111111
}

void test_binary_calculation_alternating_pattern() {
    // Pattern: 10101010 = 170
    bool switches[8] = {false, true, false, true, false, true, false, true};
    uint8_t result = calculateBinaryRepresentation(switches);
    uint8_t expected = (1 << 1) | (1 << 3) | (1 << 5) | (1 << 7); // 170
    TEST_ASSERT_EQUAL_UINT8(expected, result);
}

void test_switch_ccs_array_size() {
    // Verify that SWITCH_CCS array has the expected size
    constexpr size_t expected_size = 12;
    size_t actual_size = sizeof(SWITCH_CCS) / sizeof(SWITCH_CCS[0]);
    TEST_ASSERT_EQUAL_UINT32(expected_size, actual_size);
}

void test_switch_binary_cc_defined() {
    // Verify that SWITCH_BINARY_CC is defined and has a reasonable value
    TEST_ASSERT_GREATER_THAN_UINT8(0, SWITCH_BINARY_CC);
    TEST_ASSERT_LESS_THAN_UINT8(128, SWITCH_BINARY_CC);
}

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_binary_calculation_all_off);
    RUN_TEST(test_binary_calculation_single_bits);
    RUN_TEST(test_binary_calculation_multiple_bits);
    RUN_TEST(test_binary_calculation_all_on);
    RUN_TEST(test_binary_calculation_alternating_pattern);
    RUN_TEST(test_switch_ccs_array_size);
    RUN_TEST(test_switch_binary_cc_defined);
    
    return UNITY_END();
}
