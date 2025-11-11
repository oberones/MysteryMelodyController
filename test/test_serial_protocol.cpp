#include <unity.h>
#include "serial_portal_protocol.h"

// Test message creation and validation
void test_message_creation() {
    PortalMessage msg(PortalSerialCommand::SET_PROGRAM, 5);
    
    TEST_ASSERT_EQUAL_UINT8(PORTAL_MSG_START_BYTE, msg.startByte);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PortalSerialCommand::SET_PROGRAM), static_cast<uint8_t>(msg.command));
    TEST_ASSERT_EQUAL_UINT8(5, msg.value);
    TEST_ASSERT_EQUAL_UINT8(PORTAL_MSG_END_BYTE, msg.endByte);
    TEST_ASSERT_TRUE(msg.isValid());
}

// Test checksum calculation
void test_checksum_calculation() {
    uint8_t checksum = PortalMessage::calculateChecksum(PortalSerialCommand::SET_BPM, 120);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(PortalSerialCommand::SET_BPM) ^ 120, checksum);
}

// Test message serialization and deserialization
void test_message_serialization() {
    PortalMessage original(PortalSerialCommand::SET_INTENSITY, 200);
    
    uint8_t buffer[5];
    original.toBytes(buffer);
    
    PortalMessage reconstructed = PortalMessage::fromBytes(buffer);
    
    TEST_ASSERT_EQUAL_UINT8(original.startByte, reconstructed.startByte);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(original.command), static_cast<uint8_t>(reconstructed.command));
    TEST_ASSERT_EQUAL_UINT8(original.value, reconstructed.value);
    TEST_ASSERT_EQUAL_UINT8(original.checksum, reconstructed.checksum);
    TEST_ASSERT_EQUAL_UINT8(original.endByte, reconstructed.endByte);
    TEST_ASSERT_TRUE(reconstructed.isValid());
}

// Test invalid message detection
void test_invalid_message_detection() {
    PortalMessage msg(PortalSerialCommand::SET_HUE, 64);
    msg.checksum = 0xFF; // Corrupt checksum
    
    TEST_ASSERT_FALSE(msg.isValid());
}

// Test value mapping functions
void test_value_mapping() {
    // Test BPM mapping (0-255 -> 60-180)
    float bpm_min = SerialPortalProtocol::mapToBpm(0);
    float bpm_max = SerialPortalProtocol::mapToBpm(255);
    float bpm_mid = SerialPortalProtocol::mapToBpm(127);
    
    TEST_ASSERT_EQUAL_FLOAT(60.0f, bpm_min);
    TEST_ASSERT_EQUAL_FLOAT(180.0f, bpm_max);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 120.0f, bpm_mid); // Should be close to 120
    
    // Test normalized mapping (0-255 -> 0.0-1.0)
    float norm_min = SerialPortalProtocol::mapToNormalized(0);
    float norm_max = SerialPortalProtocol::mapToNormalized(255);
    float norm_mid = SerialPortalProtocol::mapToNormalized(127);
    
    TEST_ASSERT_EQUAL_FLOAT(0.0f, norm_min);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, norm_max);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, norm_mid); // Should be close to 0.5
}

// Test LED position mapping
void test_led_position_mapping() {
    uint8_t led_count = 60; // Example LED count
    
    uint8_t pos_min = SerialPortalProtocol::mapToLedPosition(0, led_count);
    uint8_t pos_max = SerialPortalProtocol::mapToLedPosition(255, led_count);
    uint8_t pos_mid = SerialPortalProtocol::mapToLedPosition(127, led_count);
    
    TEST_ASSERT_EQUAL_UINT8(0, pos_min);
    TEST_ASSERT_EQUAL_UINT8(led_count - 1, pos_max);
    TEST_ASSERT_UINT8_WITHIN(2, led_count / 2, pos_mid); // Should be close to middle
}

// Test command name lookup
void test_command_names() {
    TEST_ASSERT_EQUAL_STRING("SET_PROGRAM", SerialPortalProtocol::getCommandName(PortalSerialCommand::SET_PROGRAM));
    TEST_ASSERT_EQUAL_STRING("PING", SerialPortalProtocol::getCommandName(PortalSerialCommand::PING));
    TEST_ASSERT_EQUAL_STRING("ACK", SerialPortalProtocol::getCommandName(PortalSerialCommand::ACK));
}

void setUp(void) {
    // Set up code here
}

void tearDown(void) {
    // Tear down code here
}

void setup() {
    // Initialize serial for test results
    Serial.begin(115200);
    delay(2000);
    
    UNITY_BEGIN();
    RUN_TEST(test_message_creation);
    RUN_TEST(test_checksum_calculation);
    RUN_TEST(test_message_serialization);
    RUN_TEST(test_invalid_message_detection);
    RUN_TEST(test_value_mapping);
    RUN_TEST(test_led_position_mapping);
    RUN_TEST(test_command_names);
    UNITY_END();
}

void loop() {
    // Empty
}
