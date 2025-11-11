#pragma once

#include <Arduino.h>
#include "config.h"

/**
 * @brief Serial Portal Communication Protocol
 * 
 * Binary protocol for Raspberry Pi to Teensy portal control messages.
 * Replaces MIDI CC-based communication for better performance and reliability.
 * 
 * Message Format:
 * [START_BYTE][MSG_TYPE][VALUE][CHECKSUM][END_BYTE]
 * 
 * - START_BYTE: 0xAA (message start marker)
 * - MSG_TYPE: Command type (see PortalSerialCommand enum)
 * - VALUE: 8-bit parameter value (0-255)
 * - CHECKSUM: Simple XOR checksum of MSG_TYPE and VALUE
 * - END_BYTE: 0x55 (message end marker)
 * 
 * Total message size: 5 bytes
 */

// Serial portal command types (replaces MIDI CC numbers)
enum class PortalSerialCommand : uint8_t {
    SET_PROGRAM = 0x01,      // Switch animation program (0-9)
    SET_BPM = 0x02,          // Set BPM (60-180, mapped from 0-255)
    SET_INTENSITY = 0x03,    // Set intensity (0-255 -> 0.0-1.0)
    SET_HUE = 0x04,          // Set base hue (0-255 -> 0.0-1.0)
    SET_BRIGHTNESS = 0x05,   // Set brightness (0-255)
    TRIGGER_FLASH = 0x06,    // Trigger flash effect (value ignored)
    TRIGGER_RIPPLE = 0x07,   // Trigger ripple at position (0-255 -> LED position)
    
    // System commands
    PING = 0x10,             // Ping/keepalive (responds with PONG)
    RESET = 0x11,            // Reset to default state
    
    // Response commands (Teensy -> Pi)
    PONG = 0x20,             // Response to PING
    ACK = 0x21,              // Command acknowledged
    NAK = 0x22,              // Command rejected/invalid
    STATUS = 0x23            // Status report
};

// Portal message structure
struct PortalMessage {
    uint8_t startByte;
    PortalSerialCommand command;
    uint8_t value;
    uint8_t checksum;
    uint8_t endByte;
    
    // Constructor
    PortalMessage() : startByte(PORTAL_MSG_START_BYTE), 
                     command(PortalSerialCommand::PING), 
                     value(0), 
                     checksum(0), 
                     endByte(PORTAL_MSG_END_BYTE) {}
    
    PortalMessage(PortalSerialCommand cmd, uint8_t val) : 
                     startByte(PORTAL_MSG_START_BYTE),
                     command(cmd),
                     value(val),
                     checksum(calculateChecksum(cmd, val)),
                     endByte(PORTAL_MSG_END_BYTE) {}
    
    // Calculate simple XOR checksum
    static uint8_t calculateChecksum(PortalSerialCommand cmd, uint8_t val) {
        return static_cast<uint8_t>(cmd) ^ val;
    }
    
    // Validate message integrity
    bool isValid() const {
        return (startByte == PORTAL_MSG_START_BYTE) &&
               (endByte == PORTAL_MSG_END_BYTE) &&
               (checksum == calculateChecksum(command, value));
    }
    
    // Convert to byte array for transmission
    void toBytes(uint8_t* buffer) const {
        buffer[0] = startByte;
        buffer[1] = static_cast<uint8_t>(command);
        buffer[2] = value;
        buffer[3] = checksum;
        buffer[4] = endByte;
    }
    
    // Create from byte array
    static PortalMessage fromBytes(const uint8_t* buffer) {
        PortalMessage msg;
        msg.startByte = buffer[0];
        msg.command = static_cast<PortalSerialCommand>(buffer[1]);
        msg.value = buffer[2];
        msg.checksum = buffer[3];
        msg.endByte = buffer[4];
        return msg;
    }
};

// Protocol utility functions
namespace SerialPortalProtocol {
    
    // Map values between different ranges
    inline float mapToFloat(uint8_t value, float min_val, float max_val) {
        return min_val + (value / 255.0f) * (max_val - min_val);
    }
    
    // Map BPM from 0-255 to 60-180
    inline float mapToBpm(uint8_t value) {
        return mapToFloat(value, 60.0f, 180.0f);
    }
    
    // Map intensity/hue from 0-255 to 0.0-1.0
    inline float mapToNormalized(uint8_t value) {
        return value / 255.0f;
    }
    
    // Map LED position from 0-255 to LED count
    inline uint8_t mapToLedPosition(uint8_t value, uint8_t ledCount) {
        return (value * (ledCount - 1)) / 255;
    }
    
    // Command name lookup for debugging
    inline const char* getCommandName(PortalSerialCommand cmd) {
        switch (cmd) {
            case PortalSerialCommand::SET_PROGRAM: return "SET_PROGRAM";
            case PortalSerialCommand::SET_BPM: return "SET_BPM";
            case PortalSerialCommand::SET_INTENSITY: return "SET_INTENSITY";
            case PortalSerialCommand::SET_HUE: return "SET_HUE";
            case PortalSerialCommand::SET_BRIGHTNESS: return "SET_BRIGHTNESS";
            case PortalSerialCommand::TRIGGER_FLASH: return "TRIGGER_FLASH";
            case PortalSerialCommand::TRIGGER_RIPPLE: return "TRIGGER_RIPPLE";
            case PortalSerialCommand::PING: return "PING";
            case PortalSerialCommand::RESET: return "RESET";
            case PortalSerialCommand::PONG: return "PONG";
            case PortalSerialCommand::ACK: return "ACK";
            case PortalSerialCommand::NAK: return "NAK";
            case PortalSerialCommand::STATUS: return "STATUS";
            default: return "UNKNOWN";
        }
    }
}
