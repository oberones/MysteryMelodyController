# Changelog

## [Unreleased]

### Added
- Binary switch representation feature for first 8 switches
  - Individual switches send their own MIDI CC values (CC 20-31)
  - First 8 switches (0-7) additionally generate a binary representation sent as CC 50
  - Binary value represents switch states as bits (switch N = bit N)
  - Value range 0-255 covering all possible combinations of 8 switches
  - Only sends binary CC when the value changes
  - Switches 8-11 do not affect binary representation

### Changed
- Expanded SWITCH_CCS array from 3 to 12 elements to support all 12 switches
- Added SWITCH_BINARY_CC constant (set to CC 50) for binary representation

### Technical Details
- Binary calculation: `binaryValue |= (1 << switchIndex)` for each active switch
- State tracking prevents duplicate MIDI messages
- Debug output shows both individual switch changes and binary representation
- Performance optimized: binary calculation only occurs when switches 0-7 change

## Example Usage
- Switch states: [ON, OFF, ON, OFF, ON, OFF, OFF, OFF] 
- Individual CCs: CC20=127, CC21=0, CC22=127, CC23=0, CC24=127, CC25=0, CC26=0, CC27=0
- Binary CC: CC50=21 (binary: 00010101 = 1+4+16 = 21)