# SmartAidKit - Embedded Health Monitoring System

Advanced multi-sensor health monitoring system for ESP32-C3 with real-time biometric data collection and display.

## 📦 Project Structure

### `include/` — Header Files (Declarations)

#### `include/Sensors.h`
**Entry point for all sensor modules.** Include this single file in your code to access all sensors.
```cpp
#include "Sensors.h"  // Brings in ALL sensor definitions
```

#### `include/Core/` — Core System Infrastructure
Shared system-wide resources and utilities.

- **`Config.h`**
  - Central configuration hub with all global constants
  - I2C pins: SDA=GPIO8, SCL=GPIO9
  - Sensor settings: LED brightness, sampling rates, buffer sizes
  - Timing intervals: Temperature (2s), Print (1s), OLED (500ms), IMU (20ms)
  - Update here to change sensor behavior globally

- **`Types.h`**
  - Shared data structures for passing sensor data between modules
  - `SensorReadings`: Complete snapshot (all sensor data at once)
  - `IMUData`: Motion data only (accelerometer + gyroscope)
  - `BiometricData`: Heart rate + blood oxygen
  - `TemperatureData`: Temperature readings only

- **`SensorUtils.h`**
  - Helper functions for aggregating sensor data
  - `gatherSensorReadings()`: Get all sensor data at once
  - `gatherIMUData()`: Get motion data only
  - `gatherBiometricData()`: Get heart rate + SpO2 only
  - `gatherTemperatureData()`: Get temperature only

#### `include/Sensors/` — Sensor-Specific Headers
Each sensor is completely self-contained in its own folder.

##### `MAX30105/` — Pulse Oximeter & Heart Rate
Optical sensors for measuring blood oxygen saturation (SpO2) and heart rate.

- **`MAX30105.h`**: Sensor driver (handles I2C communication)
- **`oximeter.h`**: SpO2 calculation with state machine (NoFinger → Filling → Streaming)
- **`heartbeat.h`**: Heart rate detection using Peripheral Beat Amplitude (PBA) algorithm
- **`heartRate.h`**: Low-level beat detection functions
- **`spo2_algorithm.h`**: SpO2 calculation algorithm

##### `MLX90614/` — Non-Contact Thermometer
Infrared temperature measurement without physical contact.

- **`Adafruit_MLX90614.h`**: Adafruit library for sensor hardware
- **`temperature.h`**: Wrapper class with I2C clock management (requires 50kHz)

##### `MPU6050/` — 6-Axis IMU (Accelerometer + Gyroscope)
Motion and orientation tracking.

- **`driver_mpu6050.h`**: Main driver interface
- **`driver_mpu6050_basic.h`**: Simplified "basic" API
- **`driver_mpu6050_interface.h`**: C++ wrapper for Arduino Wire library
- **`driver_mpu6050_code.h`**: Additional algorithm code
- **`imu.h`**: Sensor class with averaging and throttling
- **`Property.h`**: Driver properties and enums

##### `OLED/` — Display Driver
SSD1306 OLED display (128x32 pixels) for showing sensor data.

- **`oled.h`**: SSD1306 driver with graphics functions
  - `printText()`: Display ASCII text
  - `drawHLine()`, `drawVLine()`: Draw lines
  - `printSensorData()`: Display formatted sensor values
  - `printSensorDataStruct()`: Display from SensorReadings struct

---

### `src/` — Implementation Files (Code)

#### `src/main.cpp`
**Program entry point.** Initializes all sensors and runs the main loop.
- Sets up Serial, I2C, and all sensor modules in `setup()`
- Reads sensors, processes data, and updates display in `loop()`
- Manages I2C clock speeds for different sensors (50kHz for temperature, 400kHz default)
- Uses single `#include "Sensors.h"` for all sensor access

#### `src/SensorUtils.cpp`
**Implementations of sensor aggregation functions.**
- `gatherSensorReadings()`: Collects data from all sensors into one struct
- `gatherIMUData()`, `gatherBiometricData()`, `gatherTemperatureData()`: Specialized gather functions
- Accesses global sensor objects (`temp`, `oxygen`, `heartbeat`, `imu`, `display`)

#### `src/Sensors/` — Sensor Implementations
Each sensor has its implementation in its own folder, mirroring `include/Sensors/`.

##### `MAX30105/` — Pulse Oximeter Implementation
- **`MAX30105.cpp`**: I2C communication and register operations
- **`oximeter.cpp`**: SpO2 state machine and rolling window algorithm
- **`heartbeat.cpp`**: Heart rate detection and beat counting
- **`heartRate.cpp`**: Low-level beat detection algorithm (checkForBeat, filtering)
- **`spo2_algorithm.cpp`**: SpO2 calculation math (Maxim's reference algorithm)

##### `MLX90614/` — Temperature Sensor Implementation
- **`Adafruit_MLX90614.cpp`**: Library implementation
- **`temperature.cpp`**: Wrapper class (handles I2C clock switching)

##### `MPU6050/` — IMU Driver Implementation
- **`driver_mpu6050.c`**: Core driver implementation
- **`driver_mpu6050_basic.c`**: Simplified basic API
- **`driver_mpu6050_interface.cpp`**: Arduino Wire adapter (C++ wrapper for C driver)
- **`imu.cpp`**: Sensor class with averaging and throttling

##### `OLED/` — Display Implementation
- **`oled.cpp`**: Font data, SSD1306 initialization, drawing primitives

---

### Configuration Files

#### `platformio.ini`
PlatformIO build configuration for ESP32-C3.
- Specifies board, framework, and serial monitor settings
- PlatformIO automatically compiles all `.cpp` and `.c` files in `src/`

#### `SmartAidKit.code-workspace`
VS Code workspace configuration for the project.

---

### Library Folders

#### `include/README`
Placeholder for Arduino libraries that may be installed.

#### `lib/README`
Placeholder for local library dependencies.

#### `test/README`
Placeholder for unit test files.

---

## 🔌 Hardware Connections (ESP32-C3)

| Component | Function | GPIO Pin |
|-----------|----------|----------|
| SDA (I2C) | Data line | GPIO 8 |
| SCL (I2C) | Clock line | GPIO 9 |
| GND | Ground | GND |
| 3.3V | Power | 3.3V |

### Sensors on I2C Bus

| Sensor | I2C Address | Requires |
|--------|-------------|----------|
| MAX30105 (Oximeter) | 0x57 | 400kHz |
| MLX90614 (Thermometer) | 0x5A | 50kHz |
| MPU6050 (IMU) | 0x68 | 400kHz |
| SSD1306 (OLED Display) | 0x3C | 400kHz |

---

## 🚀 How to Use

### 1. Include All Sensors
```cpp
#include "Sensors.h"  // Single include point
```

### 2. Declare Global Instances
```cpp
MAX30105 sensor;
TempSensor temp;
Oximeter oxygen;
HeartBeat heartbeat;
IMU imu;
OLED display;
```

### 3. Initialize in Setup
```cpp
void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    
    imu.begin();
    oxygen.begin(sensor);
    temp.begin();
    display.begin();
}
```

### 4. Update in Loop
```cpp
void loop() {
    // Read sensors
    imu.update();
    temp.update();
    
    // Drain MAX30105 FIFO
    sensor.check();
    while (sensor.available()) {
        oxygen.process(sensor.getFIFOIR(), sensor.getFIFORed());
        heartbeat.process(sensor.getFIFOIR());
        sensor.nextSample();
    }
    
    // Display data
    SensorReadings readings = gatherSensorReadings();
    display.printSensorDataStruct(readings);
}
```

---

## ⚙️ Configuration Guide

### Change Global Constants
Edit `include/Core/Config.h`:

```cpp
// I2C Pins
constexpr uint8_t I2C_SDA_PIN = 8;
constexpr uint8_t I2C_SCL_PIN = 9;

// Sampling Intervals
constexpr uint32_t TEMP_INTERVAL_MS = 2000;    // Temperature read every 2 seconds
constexpr uint32_t IMU_INTERVAL_MS = 20;       // IMU read every 20ms (50Hz)

// Sensor Settings
constexpr byte MAX30105_LED_BRIGHTNESS = 60;   // 0-255 (brightness)
constexpr int MAX30105_RATE_HZ = 100;          // 100 samples/second
```

### Access Sensor Data
```cpp
// Option 1: Get all data at once
SensorReadings all = gatherSensorReadings();
int hr = all.heartRate;
int spo2 = all.spo2;
float temp = all.objectTemp;

// Option 2: Get specific data types
IMUData motion = gatherIMUData();
float ax = motion.accelX;
float gx = motion.gyroX;

// Option 3: Access sensors directly
int beatAvg = heartbeat.beatAvg();
float objectTemp = temp.objectTemp();
float ax = imu.accelX();
```

---

## 📊 Sensor Specifications

### MAX30105 (Pulse Oximeter)
- **Measurement Range**: SpO2 (50-100%), Heart Rate (40-200 bpm)
- **Sampling Rate**: 100 Hz
- **LED Brightness**: 60mA max
- **Output**: IR + Red LEDs for optical sensing
- **Algorithm**: Rolling 100-sample window with SpO2 lookup table

### MLX90614 (Thermometer)
- **Measurement Range**: -40°C to 125°C (object)
- **Accuracy**: ±0.5°C
- **I2C Speed**: 50kHz (lower than other sensors)
- **Output**: Object temperature + ambient temperature
- **Update Rate**: 2 seconds (throttled in software)

### MPU6050 (IMU)
- **Accelerometer Range**: ±2g to ±16g
- **Gyroscope Range**: ±250 to ±2000 dps
- **Sampling**: 50Hz (20ms interval)
- **Averaging**: 4 samples per output
- **Output**: Ax, Ay, Az (g), Gx, Gy, Gz (dps)

### SSD1306 (OLED)
- **Resolution**: 128×32 pixels
- **Color**: Monochrome (white on black)
- **I2C Address**: 0x3C
- **Update Rate**: 500ms (throttled)

---

## 🛠️ Troubleshooting

### Sensor Not Found
1. Check wiring (SDA, SCL, GND, 3.3V)
2. Verify I2C addresses in datasheet
3. Check Serial output for initialization messages
4. Confirm I2C clock speeds (50kHz for temp, 400kHz for others)

### High Heart Rate / SpO2 Values
- Check finger placement on MAX30105 sensor
- Ensure good optical contact (no air gaps)
- Wait for buffer to fill (first 4 seconds)

### OLED Display Blank
- Check I2C address (0x3C default)
- Verify power and ground connections
- Check contrast setting in initialization

---

## 📝 Code Organization

**Philosophy**: Each sensor is self-contained and independently testable.

- **Core System** (`include/Core/`): Shared types, config, utilities
- **Sensor Modules** (`include/Sensors/{name}/`): All code for one sensor
- **Single Entry Point** (`include/Sensors.h`): Simplifies main.cpp
- **No Circular Dependencies**: Clean dependency graph
- **I2C Bus Managed**: Proper clock switching between sensors

---

## 🚀 Building & Uploading

### Build
```bash
pio run
```

### Build & Upload to Board
```bash
pio run --target upload
```

### Serial Monitor
```bash
pio device monitor
```

---

## 📦 Dependencies

- **Arduino Framework**: Core API and Wire library
- **Adafruit MLX90614**: Thermometer library
- **MAX30105**: SparkFun reference driver
- **MPU6050 LibDriver**: Professional multi-platform driver
- **SSD1306**: Display driver

All dependencies are included in the project.

---

## 📄 License

See individual file headers for licensing information. Most code is:
- **SparkFun**: BSD License (MAX30105, heartbeat, heart rate)
- **Maxim Integrated**: MIT License (SpO2 algorithm)
- **Adafruit**: MIT License (MLX90614)
- **LibDriver**: MIT License (MPU6050)
- **Custom Code**: Project-specific

---

## 🎯 Quick Reference

### Include Patterns
```cpp
// Use this in any file
#include "Sensors.h"              // All sensors at once
#include "Core/Config.h"          // Global constants
#include "Core/Types.h"           // Data structures
#include "Sensors/MAX30105/..."   // Specific sensor
```

### Global Objects (declared in main.cpp)
```cpp
MAX30105 sensor;                  // Sensor hardware
TempSensor temp;                  // Temperature wrapper
Oximeter oxygen;                  // SpO2 calculation
HeartBeat heartbeat;              // Heart rate detection
IMU imu;                          // Motion tracking
OLED display;                     // Display output
```

### Update Sequence (in loop())
```cpp
imu.update();                     // Motion sensors (fast)
temp.update();                    // Temperature (throttled)
sensor.check();                   // MAX30105 FIFO
while (sensor.available()) {      // Process new samples
    oxygen.process(...);
    heartbeat.process(...);
    sensor.nextSample();
}
display.update(...);              // Visual feedback (throttled)
```

---

## 📞 Support

For issues or questions:
1. Check Serial output messages (all modules print status)
2. Review Config.h settings
3. Verify sensor wiring and I2C addresses
4. Check power supply (3.3V, adequate current)

---

**Project**: SmartAidKit - Multi-Sensor Health Monitoring  
**Platform**: ESP32-C3  
**Build System**: PlatformIO  
**Architecture**: Modular, self-contained sensor modules  
