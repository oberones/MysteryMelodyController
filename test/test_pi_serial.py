#!/usr/bin/env python3
"""
Test script for Mystery Melody Machine Serial Portal Protocol
Usage: python test_pi_serial.py [serial_port]
"""

import serial
import time
import sys
import struct
from enum import IntEnum

class PortalSerialCommand(IntEnum):
    SET_PROGRAM = 0x01
    SET_BPM = 0x02
    SET_INTENSITY = 0x03
    SET_HUE = 0x04
    SET_BRIGHTNESS = 0x05
    TRIGGER_FLASH = 0x06
    TRIGGER_RIPPLE = 0x07
    PING = 0x10
    RESET = 0x11
    # Response commands
    PONG = 0x20
    ACK = 0x21
    NAK = 0x22
    STATUS = 0x23

PORTAL_MSG_START_BYTE = 0xAA
PORTAL_MSG_END_BYTE = 0x55

def calculate_checksum(command, value):
    """Calculate XOR checksum"""
    return command ^ value

def create_message(command, value):
    """Create a portal message"""
    checksum = calculate_checksum(command, value)
    return struct.pack('BBBBB', 
                      PORTAL_MSG_START_BYTE,
                      command,
                      value,
                      checksum,
                      PORTAL_MSG_END_BYTE)

def parse_response(data):
    """Parse response message"""
    if len(data) < 5:
        return None
    
    start, cmd, val, chk, end = struct.unpack('BBBBB', data[:5])
    
    if start != PORTAL_MSG_START_BYTE or end != PORTAL_MSG_END_BYTE:
        return None
    
    expected_checksum = calculate_checksum(cmd, val)
    if chk != expected_checksum:
        return None
    
    return {
        'command': cmd,
        'value': val,
        'valid': True
    }

def test_communication(serial_port='/dev/ttyACM0', baud_rate=115200):
    """Test serial communication with Teensy"""
    try:
        ser = serial.Serial(serial_port, baud_rate, timeout=1.0)
        print(f"Connected to {serial_port} at {baud_rate} baud")
        time.sleep(2)  # Give Teensy time to initialize
        
        # Test 1: Ping
        print("\n=== Test 1: PING ===")
        ping_msg = create_message(PortalSerialCommand.PING, 0)
        ser.write(ping_msg)
        ser.flush()
        
        response = ser.read(5)
        if response:
            parsed = parse_response(response)
            if parsed and parsed['command'] == PortalSerialCommand.PONG:
                print("✓ PING/PONG successful")
            else:
                print("✗ PING failed - invalid response")
        else:
            print("✗ PING failed - no response")
        
        # Test 2: Set Program
        print("\n=== Test 2: SET_PROGRAM ===")
        for program in [0, 1, 2, 5]:  # Test SPIRAL, PULSE, WAVE, IDLE
            msg = create_message(PortalSerialCommand.SET_PROGRAM, program)
            ser.write(msg)
            ser.flush()
            time.sleep(0.1)
            
            response = ser.read(5)
            if response:
                parsed = parse_response(response)
                if parsed and parsed['command'] == PortalSerialCommand.ACK:
                    print(f"✓ Program {program} set successfully")
                else:
                    print(f"✗ Program {program} failed")
            else:
                print(f"✗ Program {program} - no response")
            
            time.sleep(1)  # Let animation run briefly
        
        # Test 3: Set BPM
        print("\n=== Test 3: SET_BPM ===")
        bpm_values = [85, 127, 170, 200]  # Various BPM values (mapped to 0-255)
        for bpm_val in bpm_values:
            msg = create_message(PortalSerialCommand.SET_BPM, bpm_val)
            ser.write(msg)
            ser.flush()
            time.sleep(0.1)
            
            response = ser.read(5)
            if response:
                parsed = parse_response(response)
                if parsed and parsed['command'] == PortalSerialCommand.ACK:
                    # Calculate actual BPM
                    actual_bpm = 60 + (bpm_val / 255.0) * 120
                    print(f"✓ BPM set to {bpm_val} (≈{actual_bpm:.1f} BPM)")
                else:
                    print(f"✗ BPM {bpm_val} failed")
        
        # Test 4: Flash Effect
        print("\n=== Test 4: TRIGGER_FLASH ===")
        for i in range(3):
            msg = create_message(PortalSerialCommand.TRIGGER_FLASH, 0)
            ser.write(msg)
            ser.flush()
            time.sleep(0.5)
        print("✓ Flash effects triggered")
        
        # Test 5: Ripple Effects
        print("\n=== Test 5: TRIGGER_RIPPLE ===")
        ripple_positions = [0, 64, 127, 191, 255]  # Different positions
        for pos in ripple_positions:
            msg = create_message(PortalSerialCommand.TRIGGER_RIPPLE, pos)
            ser.write(msg)
            ser.flush()
            time.sleep(0.3)
        print("✓ Ripple effects triggered at various positions")
        
        # Test 6: Intensity and Hue
        print("\n=== Test 6: SET_INTENSITY & SET_HUE ===")
        # Sweep intensity
        for intensity in [64, 127, 191, 255, 127]:
            msg = create_message(PortalSerialCommand.SET_INTENSITY, intensity)
            ser.write(msg)
            ser.flush()
            time.sleep(0.2)
        
        # Sweep hue
        for hue in [0, 51, 102, 153, 204, 255]:
            msg = create_message(PortalSerialCommand.SET_HUE, hue)
            ser.write(msg)
            ser.flush()
            time.sleep(0.3)
        print("✓ Intensity and hue sweep completed")
        
        # Test 7: Reset
        print("\n=== Test 7: RESET ===")
        reset_msg = create_message(PortalSerialCommand.RESET, 0)
        ser.write(reset_msg)
        ser.flush()
        
        response = ser.read(5)
        if response:
            parsed = parse_response(response)
            if parsed and parsed['command'] == PortalSerialCommand.ACK:
                print("✓ Reset successful - portal returned to default state")
            else:
                print("✗ Reset failed")
        
        ser.close()
        print("\n=== All tests completed ===")
        
    except serial.SerialException as e:
        print(f"Serial communication error: {e}")
        print("Make sure the Teensy is connected and the correct port is specified")
    except Exception as e:
        print(f"Unexpected error: {e}")

def interactive_mode(serial_port='/dev/ttyACM0', baud_rate=115200):
    """Interactive mode for manual testing"""
    try:
        ser = serial.Serial(serial_port, baud_rate, timeout=1.0)
        print(f"Connected to {serial_port} at {baud_rate} baud")
        print("Interactive Portal Control Mode")
        print("Commands: ping, program <0-9>, bpm <0-255>, flash, ripple <0-255>, quit")
        
        while True:
            cmd = input("\n> ").strip().lower().split()
            
            if not cmd or cmd[0] == 'quit':
                break
            elif cmd[0] == 'ping':
                msg = create_message(PortalSerialCommand.PING, 0)
                ser.write(msg)
                ser.flush()
                print("PING sent")
            elif cmd[0] == 'program' and len(cmd) > 1:
                try:
                    prog = int(cmd[1])
                    if 0 <= prog <= 9:
                        msg = create_message(PortalSerialCommand.SET_PROGRAM, prog)
                        ser.write(msg)
                        ser.flush()
                        print(f"Program {prog} sent")
                    else:
                        print("Program must be 0-9")
                except ValueError:
                    print("Invalid program number")
            elif cmd[0] == 'bpm' and len(cmd) > 1:
                try:
                    bpm = int(cmd[1])
                    if 0 <= bpm <= 255:
                        msg = create_message(PortalSerialCommand.SET_BPM, bpm)
                        ser.write(msg)
                        ser.flush()
                        actual_bpm = 60 + (bpm / 255.0) * 120
                        print(f"BPM {bpm} sent (≈{actual_bpm:.1f} BPM)")
                    else:
                        print("BPM value must be 0-255")
                except ValueError:
                    print("Invalid BPM value")
            elif cmd[0] == 'flash':
                msg = create_message(PortalSerialCommand.TRIGGER_FLASH, 0)
                ser.write(msg)
                ser.flush()
                print("Flash triggered")
            elif cmd[0] == 'ripple' and len(cmd) > 1:
                try:
                    pos = int(cmd[1])
                    if 0 <= pos <= 255:
                        msg = create_message(PortalSerialCommand.TRIGGER_RIPPLE, pos)
                        ser.write(msg)
                        ser.flush()
                        print(f"Ripple triggered at position {pos}")
                    else:
                        print("Position must be 0-255")
                except ValueError:
                    print("Invalid position value")
            else:
                print("Unknown command")
        
        ser.close()
        
    except serial.SerialException as e:
        print(f"Serial communication error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")

if __name__ == "__main__":
    port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyACM0'
    
    if len(sys.argv) > 2 and sys.argv[2] == 'interactive':
        interactive_mode(port)
    else:
        test_communication(port)
