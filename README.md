# Kinetic Light CHOP Plugin

A TouchDesigner CHOP plugin for controlling kinetic light installations with multiple DMX-controlled motors. This plugin enables precise control of motor positions and lighting effects through a unified interface.

## Overview

The Kinetic Light CHOP plugin is designed to control a three-motor kinetic light system where motors can be independently positioned using height, roll, pitch, and yaw parameters. The system supports three different types of motors:
- 62CH Motor (for main control and lighting)
- 9CH Motors  (for auxiliary positioning)

### Features

- Real-time motor position calculation based on roll, pitch, and yaw inputs
- DMX value mapping with configurable ranges
- Support for different motor types (62CH,10CH and 9CH)
- Calibration settings for motor heights and DMX output ranges
- Speed control for motor movements
- Additional DMX channel control for lighting effects

## Installation

1. Build the plugin using your preferred C++ compiler (ensure it supports C++11 or later)
2. Copy the compiled DLL/plugin file to your TouchDesigner project's plugin directory
3. In TouchDesigner, create a new CHOP and select "Kinetic Light" from the operator list

## Input Parameters

The plugin accepts the following inputs:

### CHOP Inputs
1. Height (Channel 0)
2. Roll (Channel 1)
3. Pitch (Channel 2)
4. Yaw (Channel 3)
5. Speed (Channel 4) - Optional
6. Additional DMX Values (Channel 5) - Optional

### Configuration Parameters

#### Base Configuration
- **Base Size**: Size of the triangular base (default: 1.0, range: 0.1-5.0)
- **Min Height**: Minimum height limit (default: 0.5, range: 0.0-1.0)
- **Max Height**: Maximum height limit (default: 3.0, range: 1.0-10.0)

#### Angle Limits
- **Min/Max Pitch**: Pitch angle limits (default: 0°, range: ±90°)
- **Min/Max Roll**: Roll angle limits (default: 0°, range: ±90°)
- **Min/Max Yaw**: Yaw angle limits (default: 0°, range: ±90°)

#### Motor Calibration
- **Motor Calibration Min Height**: Physical minimum height of motors
- **Motor Calibration Max Height**: Physical maximum height of motors
- **Motor Calibration Min DMX Out**: Minimum DMX value (0-255)
- **Motor Calibration Max DMX Out**: Maximum DMX value (0-255)

## Output Channels

The plugin outputs 80 DMX channels total:

### First Motor (62CH)
- Channels 1-62
  - CH1: Lifting height
  - CH2: Fine-tuning
  - CH3: Speed
  - CH4-CH62: Additional lighting control

### Second Motor (9CH)
- Channels 63-71
  - CH63: Lifting height
  - CH64: Fine-tuning
  - CH65: Speed

### Third Motor (9CH)
- Channels 72-80
  - CH72: Lifting height
  - CH73: Fine-tuning
  - CH74: Speed

## Usage Example

1. Create a new Kinetic Light CHOP in your TouchDesigner network
2. Connect the required input channels (height, roll, pitch, yaw)
3. Configure the base parameters according to your physical setup
4. Set the calibration values based on your motor specifications
5. Connect the output to your DMX interface

```python
# Example TouchDesigner network setup
null1 = op('null1')  # Height input
null2 = op('null2')  # Roll input
null3 = op('null3')  # Pitch input
null4 = op('null4')  # Yaw input

kinetic_light = op('kinetic_light')
kinetic_light.inputConnectors[0].connect(null1)
kinetic_light.inputConnectors[1].connect(null2)
kinetic_light.inputConnectors[2].connect(null3)
kinetic_light.inputConnectors[3].connect(null4)
```

## Technical Details

### Motor Control Logic

The plugin uses a mathematical model to calculate motor positions based on the desired orientation:
1. Positions motors in an equilateral triangle configuration
2. Applies rotation matrices for roll, pitch, and yaw transformations
3. Converts calculated heights to DMX values using calibration parameters

### DMX Mapping

Height values are mapped to DMX values using the following formula:
```cpp
scaled_value = (height - min_height) / (max_height - min_height) * (max_dmx - min_dmx) + min_dmx
```

## Error Handling

The plugin includes robust error handling for:
- Invalid height ranges
- Out-of-range DMX values
- Calibration parameter validation
- Motor position calculations

## Performance Considerations

- The plugin cooks every frame when requested
- Supports timeslicing for efficient processing
- Minimal memory footprint with smart pointer usage
- Efficient matrix calculations for position updates

## Contributing

Feel free to submit issues and pull requests for:
- Bug fixes
- Feature enhancements
- Documentation improvements
- Performance optimizations

## Author

Kavinda Madhubhashana (kmkavinda6@gmail.com)



## Version History

- 1.0.0: Initial release
  - Basic motor control
  - DMX output
  - Calibration settings