# Code Organization Migration Guide - Option 3

This guide explains how to complete the migration from the flat structure to the self-contained sensor modules.

## Current Status ✅

✅ Folder structure created
✅ Core files moved to `include/Core/`
✅ CMakeLists.txt templates created for each sensor
✅ Example: `include/Sensors/MAX30105/oximeter.h` updated with new include paths

## Files Still to Migrate

### MAX30105 Sensor (Pulse Oximeter + Heart Rate)

**Headers** (from `include/` to `include/Sensors/MAX30105/`):
- [ ] `MAX30105.h` → keep as-is (uses Wire.h only)
- [ ] `heartbeat.h` → already has #include "heartRate.h" (update to relative path)
- [ ] `heartRate.h` → keep as-is (uses only Arduino.h)
- [ ] `spo2_algorithm.h` → keep as-is

**Implementation** (from `src/` to `src/Sensors/MAX30105/`):
- [ ] `MAX30105.cpp`
- [ ] `oximeter.cpp`
- [ ] `heartbeat.cpp`
- [ ] `spo2_algorithm.cpp`
- [ ] `heartRate.cpp`

### MLX90614 Temperature Sensor

**Headers** (from `include/` to `include/Sensors/MLX90614/`):
- [ ] `Adafruit_MLX90614.h`
- [ ] `temperature.h` → Update: `#include "../../Core/Config.h"` (instead of `"Config.h"`)

**Implementation** (from `src/` to `src/Sensors/MLX90614/`):
- [ ] `Adafruit_MLX90614.cpp`
- [ ] `temperature.cpp`

### MPU6050 IMU Sensor

**Headers** (from `include/` to `include/Sensors/MPU6050/`):
- [ ] `driver_mpu6050.h`
- [ ] `driver_mpu6050_basic.h`
- [ ] `driver_mpu6050_interface.h`
- [ ] `driver_mpu6050_code.h`
- [ ] `imu.h` → Update: `#include "../../Core/Config.h"` (instead of `"Config.h"`)
- [ ] `Property.h`

**Implementation** (from `src/` to `src/Sensors/MPU6050/`):
- [ ] `driver_mpu6050.c`
- [ ] `driver_mpu6050_basic.c`
- [ ] `driver_mpu6050_interface.cpp`
- [ ] `imu.cpp`

### OLED Display

**Headers** (from `include/` to `include/Sensors/OLED/`):
- [ ] `oled.h` → Update: `#include "../../Core/Types.h"` (forward declare removed)

**Implementation** (from `src/` to `src/Sensors/OLED/`):
- [ ] `oled.cpp`

## Include Path Update Reference

When moving files, update these include statements:

| Old Path | New Path | Reason |
|----------|----------|--------|
| `#include "Config.h"` | `#include "../../Core/Config.h"` | Config moved to Core/ |
| `#include "Types.h"` | `#include "../../Core/Types.h"` | Types moved to Core/ |
| `#include "SensorUtils.h"` | `#include "../../Core/SensorUtils.h"` | SensorUtils moved to Core/ |
| `#include "MAX30105.h"` | (same - same folder) | Both in MAX30105/ |
| `#include "temperature.h"` | (same - same folder) | Both in MLX90614/ |
| `#include "imu.h"` | (same - same folder) | Both in MPU6050/ |

## Updated Aggregator Header

Update `include/Sensors.h` to import from new locations:

```cpp
#pragma once

// Core system files
#include "Core/Config.h"
#include "Core/Types.h"
#include "Core/SensorUtils.h"

// Sensor modules
#include "Sensors/MAX30105/MAX30105.h"
#include "Sensors/MAX30105/oximeter.h"
#include "Sensors/MAX30105/heartbeat.h"

#include "Sensors/MLX90614/Adafruit_MLX90614.h"
#include "Sensors/MLX90614/temperature.h"

#include "Sensors/MPU6050/imu.h"
#include "Sensors/MPU6050/driver_mpu6050.h"

#include "Sensors/OLED/oled.h"
```

## Testing After Migration

1. **Verify compilation**:
   ```bash
   cd SmartAidKit
   pio run
   ```

2. **Check all includes resolve**:
   - No "file not found" errors
   - No circular dependencies

3. **Verify functionality**:
   - Upload to device
   - Test each sensor works

## Benefits After Migration

✨ **Clean code organization**:
- Each sensor is self-contained
- Easy to find sensor-related code
- Clear module boundaries

✨ **Team collaboration**:
- Different team members can work on different sensors
- Minimal merge conflicts

✨ **Reusability**:
- Can extract a sensor folder for use in other projects
- Clear dependencies documented in CMakeLists.txt

✨ **Scaling**:
- Adding new sensors is straightforward
- Copy template folder structure
- Fill in files and update aggregator

## Notes

- **PlatformIO compatibility**: No changes needed to `platformio.ini`. PlatformIO automatically compiles all `.cpp` and `.c` files in `src/`.
- **Include paths**: All relative paths use `../` to go up directory levels.
- **No breaking changes**: `main.cpp` continues to use `#include "Sensors.h"` as before.
