# SmartAidKit - Modular Sensor Architecture

## Folder Structure (Option 3 - Self-Contained Sensors)

```
include/
├─ Core/                          # Core system files
│  ├─ Config.h                    # Global configuration & constants
│  ├─ Types.h                     # Shared data structures (SensorReadings, etc.)
│  └─ SensorUtils.h               # Utility functions for gathering sensor data
│
└─ Sensors/                       # All sensor drivers
   ├─ MAX30105/                   # Pulse oximeter + heart rate sensor
   │  ├─ CMakeLists.txt           # Module definition
   │  ├─ MAX30105.h               # Sensor driver
   │  ├─ oximeter.h               # SpO2 algorithm wrapper
   │  ├─ heartbeat.h              # Heart rate algorithm wrapper
   │  ├─ heartRate.h              # Beat detection algorithm
   │  └─ spo2_algorithm.h         # SpO2 calculation
   │
   ├─ MLX90614/                   # Temperature sensor
   │  ├─ CMakeLists.txt           # Module definition
   │  ├─ Adafruit_MLX90614.h       # Adafruit library header
   │  └─ temperature.h            # Temperature wrapper class
   │
   ├─ MPU6050/                    # IMU (accelerometer + gyroscope)
   │  ├─ CMakeLists.txt           # Module definition
   │  ├─ imu.h                    # IMU wrapper class
   │  ├─ driver_mpu6050.h         # Low-level driver
   │  ├─ driver_mpu6050_basic.h   # Basic interface
   │  ├─ driver_mpu6050_interface.h
   │  ├─ driver_mpu6050_code.h
   │  └─ Property.h               # Helper for MPU6050
   │
   └─ OLED/                       # Display
      ├─ CMakeLists.txt           # Module definition
      └─ oled.h                   # SSD1306 OLED driver

src/
├─ main.cpp                       # Application entry point
├─ SensorUtils.cpp               # Sensor utility implementations
│
└─ Sensors/                       # Implementation files
   ├─ MAX30105/
   │  ├─ MAX30105.cpp
   │  ├─ oximeter.cpp
   │  ├─ heartbeat.cpp
   │  └─ spo2_algorithm.cpp
   │
   ├─ MLX90614/
   │  ├─ Adafruit_MLX90614.cpp
   │  └─ temperature.cpp
   │
   ├─ MPU6050/
   │  ├─ imu.cpp
   │  ├─ driver_mpu6050.c
   │  ├─ driver_mpu6050_basic.c
   │  └─ driver_mpu6050_interface.cpp
   │
   └─ OLED/
      └─ oled.cpp
```

## Benefits of This Structure

### Self-Contained Modules
Each sensor folder is independent and can be:
- Developed separately by different team members
- Tested in isolation
- Replaced or upgraded without affecting other sensors
- Reused in other projects

### Clear Dependencies
Each CMakeLists.txt declares:
- What it depends on (Core files, external libraries)
- What files it includes/compiles
- Any special compilation flags

### Includes Pattern
All includes use relative paths from the module:
- Same folder: `#include "file.h"`
- Core files: `#include "../../Core/Config.h"`
- Other modules: `#include "../MPU6050/imu.h"`

### Scalability
Adding a new sensor is simple:
1. Create `include/Sensors/NewSensor/` folder
2. Create `src/Sensors/NewSensor/` folder
3. Add header and implementation files
4. Create CMakeLists.txt
5. Update main aggregator if needed

## Build with PlatformIO

PlatformIO automatically finds all `.cpp` and `.c` files in `src/`, so no changes needed to `platformio.ini`. Just ensure:
- Include paths are set correctly (already done by PlatformIO)
- All files compile with `-I include` flag (default in PlatformIO)

## Next Steps

1. **Move remaining files** to their sensor folders
2. **Update include paths** in moved files to use relative paths
3. **Create CMakeLists.txt** for each sensor module
4. **Update main.cpp** to use the new structure
5. **Test compilation** with PlatformIO
