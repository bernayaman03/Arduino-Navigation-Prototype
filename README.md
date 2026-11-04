# Arduino Navigation Prototype

A collection of Arduino-based experiments focused on sensor interfacing, distance estimation, speed measurement, and navigation-related control algorithms.

The repository contains six experimental sketches developed using Arduino Uno and Arduino Mega 2560 boards.

## Projects

| Project | Description |
|---|---|
| **DS18B20 Temperature Test** | Reads temperature data from DS18B20 sensors using the OneWire protocol and reports measurements through serial communication. |
| **Omron Sensor Test** | Detects and counts digital sensor activations using edge detection and basic debounce handling. |
| **Distance Calculation Test** | Estimates distance using three independent sensor counters, predefined distance intervals, and a 5% tolerance check to compare measurements. |
| **Speed Calculation Test** | Calculates instantaneous speed, average speed, acceleration, and traveled distance using an E18-D80NK sensor and predefined reflector spacing. |
| **Wheel Speed Test** | Calculates wheel-based speed, acceleration, and distance using a Hall-effect sensor and known wheel circumference. |
| **Navigation Prototype** | Explores the integration of sensor measurements, distance and speed estimation, IMU processing, temperature monitoring, and state-machine-based control. |

## Hardware

- Arduino Uno / Mega 2560
- DS18B20 temperature sensors
- Omron photoelectric sensor
- E18-D80NK reflective sensor
- Hall-effect sensor
- MPU6050 IMU

## Technical Concepts

- Digital sensor interfacing
- Edge detection and debounce
- Time measurements using `micros()` and `millis()`
- Distance estimation from sensor events
- Instantaneous and average speed calculation
- Acceleration calculation
- Multi-sensor measurement consistency checking
- Serial telemetry
- State-machine-based control logic

## Repository Structure

```text
Arduino-Navigation-Prototype/
├── Distance-Calculation-Test/
├── DS18B20-Temperature-Test/
├── Navigation-Prototype/
├── Omron-Sensor-Test/
├── Speed-Calculation-Test/
└── Wheel-Speed-Test/
```

## Development Environment

- Arduino IDE
- Arduino C/C++
- OneWire and DallasTemperature libraries
- MPU6050_tockn library
- Wire (I2C) library

Library requirements and hardware connections depend on the individual sketch.

## Notes

These programs were developed as experimental implementations for testing sensor behavior, measurement methods, and navigation-related algorithms.
