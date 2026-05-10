# SmartAidKit - Embedded Health Monitoring System
1: 
2: /**
3:  * @mainpage SmartAidKit Documentation
4:  * 
5:  * @section overview_sec Overview
6:  * Advanced multi-sensor health monitoring system for ESP32-C3 with real-time biometric data collection and display.
7:  */
8: 

Advanced multi-sensor health monitoring system for ESP32-C3 with real-time biometric data collection and display.

##  Project Structure

This project uses **Doxygen** to automatically generate detailed code documentation from source comments. You can generate HTML docs by running `doxygen Doxyfile` and opening `html/index.html`.

Below is a visual overview of the project structure.

```text
📁 SmartAidKit
├── 📁 include/                 # Header Files (Declarations)
│   ├── 📄 Sensors.h            # Entry point for all sensor modules
│   ├── 📁 Core/                # Shared system-wide infrastructure
│   │   ├── 📄 Config.h         # Global constants, I2C pins, sampling intervals
│   │   ├── 📄 Types.h          # Shared data structures for sensor data
│   │   └── 📄 SensorUtils.h    # Helper functions for data aggregation
│   └── 📁 Sensors/             # Sensor-Specific Headers
│       ├── 📁 MAX30105/        # Pulse Oximeter & Heart Rate
│       │   ├── 📄 MAX30105.h   # Sensor driver & I2C communication
│       │   ├── 📄 heartbeat.h  # FFT-based heart rate algorithm
│       │   ├── 📄 heartRate.h  # Low-level beat detection
│       │   ├── 📄 oximeter.h   # SpO2 calculation & state machine
│       │   └── 📄 spo2_algorithm.h # Maxim's SpO2 math reference
│       ├── 📁 MLX90614/        # Non-Contact Thermometer
│       │   ├── 📄 Adafruit_MLX90614.h
│       │   └── 📄 temperature.h# Wrapper class managing 50kHz I2C clock
│       ├── 📁 MPU6050/         # 6-Axis IMU (Motion Tracking)
│       │   ├── 📄 driver_mpu6050.h
│       │   ├── 📄 driver_mpu6050_basic.h
│       │   ├── 📄 driver_mpu6050_interface.h
│       │   ├── 📄 driver_mpu6050_code.h
│       │   ├── 📄 imu.h        # Sensor wrapper class (throttling & averaging)
│       │   └── 📄 Property.h
│       └── 📁 OLED/            # SSD1306 Display Driver
│           └── 📄 oled.h       # Graphics primitives and formatting
├── 📁 src/                     # Implementation Files (Source Code)
│   ├── 📄 main.cpp             # Setup, main loop, and sensor initialization
│   ├── 📄 SensorUtils.cpp      # Sensor aggregation implementations
│   └── 📁 Sensors/             # Sensor implementations mirroring include/
│       ├── 📁 MAX30105/        # MAX30105 (.cpp files)
│       ├── 📁 MLX90614/        # MLX90614 (.cpp files)
│       ├── 📁 MPU6050/         # MPU6050 (.c/.cpp files)
│       └── 📁 OLED/            # OLED (.cpp files)
├── 📁 lib/                     # Local library dependencies
├── 📁 test/                    # Unit testing files
├── 📄 Doxyfile                 # Doxygen configuration file
├── 📄 platformio.ini           # PlatformIO configuration
└── 📄 SmartAidKit.code-workspace # VS Code Workspace
```

###  File & Folder Details

####  Core System (`Core/`)
- **`Config.h`**: Central configuration hub with all global constants. Update here to change sensor behavior globally (e.g. I2C pins, sensor settings, timing intervals).
- **`Types.h`**: Shared data structures (`SensorReadings`, `IMUData`, `BiometricData`) for passing sensor data between modules.
- **`SensorUtils.h` / `.cpp`**: Helper functions (`gatherSensorReadings()`, `gatherIMUData()`) for aggregating data from multiple sensors.

####  Pulse Oximeter & Heart Rate (`Sensors/MAX30105/`)
- **@ref MAX30105**: Sensor driver for handling I2C communication and register operations.
- **@ref Oximeter**: SpO2 calculation with a state machine (NoFinger → Filling → Streaming).
- **@ref HeartBeat**: Heart rate detection using an advanced FFT (Fast Fourier Transform) algorithm.
- **`heartRate`**: Low-level beat detection algorithm.
- **`spo2_algorithm`**: SpO2 calculation math (Maxim's reference algorithm).

####  Non-Contact Thermometer (`Sensors/MLX90614/`)
- **@ref TempSensor**: Wrapper class handling proper I2C clock switching (requires 50kHz, unlike other 400kHz sensors).

####  6-Axis IMU (`Sensors/MPU6050/`)
- **@ref IMU**: Wrapper class providing motion and orientation tracking with built-in averaging and throttling.

#### Display Driver (`Sensors/OLED/`)
- **@ref OLED**: SSD1306 display driver for formatting and printing data (`printText()`, `printSensorDataStruct()`).

####  Configuration Files
- **`Doxyfile`**: Configuration for generating Doxygen documentation.
- **`platformio.ini`**: PlatformIO build configuration for ESP32-C3.
- **`SmartAidKit.code-workspace`**: VS Code workspace settings.

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

##  How to Use

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

## Configuration Guide

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

##  Sensor Specifications

### MAX30105 (Pulse Oximeter)
- **Measurement Range**: SpO2 (50-100%), Heart Rate (40-200 bpm)
- **Sampling Rate**: 100 Hz
- **LED Brightness**: 60mA max
- **Output**: IR + Red LEDs for optical sensing
- **Algorithm**: Rolling 100-sample window for SpO2, and a 512-sample sliding window FFT with parabolic interpolation and median filtering for Heart Rate

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

##  Troubleshooting

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

##  Code Organization

**Philosophy**: Each sensor is self-contained and independently testable.

- **Core System** (`include/Core/`): Shared types, config, utilities
- **Sensor Modules** (`include/Sensors/{name}/`): All code for one sensor
- **Single Entry Point** (`include/Sensors.h`): Simplifies main.cpp
- **No Circular Dependencies**: Clean dependency graph
- **I2C Bus Managed**: Proper clock switching between sensors

---

##  Building & Uploading

### Build
```bash
pio run
```

### Build & Upload to Board
```bash
pio run --target upload
```

---
275: 
276: ## 📖 Documentation
277: 
278: Detailed API documentation is generated using **Doxygen**.
279: 
280: ### Generating Documentation
281: 
282: To generate the HTML documentation locally:
283: 
284: ```bash
285: doxygen Doxyfile
286: ```
287: 
288: After generation, open `html/index.html` in your browser.
289: 
290: ### Documentation Features
291: - **Class Hierarchy**: Visual representation of sensor classes.
292: - **Module Grouping**: Sensors are grouped by manufacturer and function.
293: - **Call Graphs**: Automated visualization of function dependencies.
294: - **Search**: Real-time search for API members.
295: 
296: ---
297: 
298: ##  Serial Monitor
```bash
pio device monitor
```

---

##  Dependencies

- **Arduino Framework**: Core API and Wire library
- **Adafruit MLX90614**: Thermometer library
- **MAX30105**: SparkFun reference driver
- **MPU6050 LibDriver**: Professional multi-platform driver
- **SSD1306**: Display driver

All dependencies are included in the project.

---

##  License

See individual file headers for licensing information. Most code is:
- **SparkFun**: BSD License (MAX30105, heartbeat, heart rate)
- **Maxim Integrated**: MIT License (SpO2 algorithm)
- **Adafruit**: MIT License (MLX90614)
- **LibDriver**: MIT License (MPU6050)
- **Custom Code**: Project-specific

---

##  Quick Reference

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

##  Support

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
