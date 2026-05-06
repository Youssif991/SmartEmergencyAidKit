# SmartAidKit - Code Organization Summary

## ✅ Completed (Phase 1)

### Folder Structure
✅ Created hierarchical folder organization:
```
include/
├─ Core/                    (New) Central system files
│  ├─ Config.h
│  ├─ Types.h
│  └─ SensorUtils.h
└─ Sensors/                 (New) Self-contained modules
   ├─ MAX30105/
   ├─ MLX90614/
   ├─ MPU6050/
   └─ OLED/

src/
├─ main.cpp
├─ SensorUtils.cpp
└─ Sensors/                 (New) Implementation files
   ├─ MAX30105/
   ├─ MLX90614/
   ├─ MPU6050/
   └─ OLED/
```

### Core System Refactoring
✅ Created `include/Core/` with:
- `Config.h` — Global configuration and constants
- `Types.h` — Shared data structures (SensorReadings, IMUData, etc.)
- `SensorUtils.h` — Helper functions for data aggregation

✅ Created `src/SensorUtils.cpp` — Implementation of utility functions

### New Data Structures (Types.h)
```cpp
struct SensorReadings { ... }    // All sensors in one struct
struct IMUData { ... }           // Just motion data
struct BiometricData { ... }     // Just HR + SpO2
struct TemperatureData { ... }   // Just temperature
```

### Module Templates
✅ Created CMakeLists.txt for each sensor showing:
- Dependencies (what each module needs)
- Source files (what it compiles)
- Include directories (what it provides)

### Documentation
✅ Created `ARCHITECTURE.md` — Full structure overview
✅ Created `MIGRATION_GUIDE.md` — Step-by-step migration instructions

### Updated Aggregator
✅ Updated `include/Sensors.h` with new include paths pointing to Core/ and Sensors/ folders

---

## 📋 Remaining Tasks (Phase 2)

### Step 1: Copy Header Files to Sensor Folders
Copy each header to its sensor folder. Update any `#include "Config.h"` to `#include "../../Core/Config.h"`:

**MAX30105 module:**
- Copy `include/MAX30105.h` → `include/Sensors/MAX30105/`
- Copy `include/oximeter.h` → `include/Sensors/MAX30105/` *(already done with updated includes)*
- Copy `include/heartbeat.h` → `include/Sensors/MAX30105/`
- Copy `include/heartRate.h` → `include/Sensors/MAX30105/`
- Copy `include/spo2_algorithm.h` → `include/Sensors/MAX30105/`

**MLX90614 module:**
- Copy `include/temperature.h` → `include/Sensors/MLX90614/`
- Update: `#include "Config.h"` → `#include "../../Core/Config.h"`
- Copy `include/Adafruit_MLX90614.h` → `include/Sensors/MLX90614/`

**MPU6050 module:**
- Copy `include/imu.h` → `include/Sensors/MPU6050/`
- Update: `#include "Config.h"` → `#include "../../Core/Config.h"`
- Copy all `driver_mpu6050*` headers → `include/Sensors/MPU6050/`
- Copy `include/Property.h` → `include/Sensors/MPU6050/`

**OLED module:**
- Copy `include/oled.h` → `include/Sensors/OLED/`

### Step 2: Copy Implementation Files
Move source files to match header locations:

**MAX30105:**
- `src/MAX30105.cpp` → `src/Sensors/MAX30105/`
- `src/oximeter.cpp` → `src/Sensors/MAX30105/`
- `src/heartbeat.cpp` → `src/Sensors/MAX30105/`
- `src/spo2_algorithm.cpp` → `src/Sensors/MAX30105/`
- `src/heartRate.cpp` → `src/Sensors/MAX30105/`

**MLX90614:**
- `src/temperature.cpp` → `src/Sensors/MLX90614/`
- `src/Adafruit_MLX90614.cpp` → `src/Sensors/MLX90614/`

**MPU6050:**
- `src/imu.cpp` → `src/Sensors/MPU6050/`
- `src/driver_mpu6050*.c` → `src/Sensors/MPU6050/`
- `src/driver_mpu6050_interface.cpp` → `src/Sensors/MPU6050/`

**OLED:**
- `src/oled.cpp` → `src/Sensors/OLED/`

### Step 3: Update Include Paths
In moved `.cpp` files, update any old include statements:

```cpp
// OLD                              // NEW
#include "Config.h"             →  #include "../../Core/Config.h"
#include "Types.h"              →  #include "../../Core/Types.h"
#include "SensorUtils.h"        →  #include "../../Core/SensorUtils.h"
#include "MAX30105.h"           →  #include "MAX30105.h"  (same folder)
#include "temperature.h"        →  #include "temperature.h"  (same folder)
```

### Step 4: Test Compilation
```bash
cd SmartAidKit
pio run
```

Expected result: ✅ Clean compilation with no errors

### Step 5: Cleanup (Optional)
Once everything compiles and works:
- Delete old header files from `include/` root
- Delete old source files from `src/` root
- Keep `include/Sensors.h` at root level (it's the aggregator)

---

## 📊 Why This Structure (Option 3)?

| Aspect | Benefit |
|--------|---------|
| **Modularity** | Each sensor is self-contained, easy to understand |
| **Reusability** | Can copy a sensor folder to another project |
| **Scalability** | Adding sensors just means copying the template structure |
| **Documentation** | CMakeLists.txt files document dependencies |
| **Team work** | Different people can work on different sensors independently |
| **Testing** | Can test sensor modules independently if needed |

---

## Current Code Example

Your `main.cpp` continues to work exactly the same:

```cpp
#include "Sensors.h"  // Pulls in everything from the new structure

MAX30105 sensor;
TempSensor temp;
Oximeter oxygen;
HeartBeat heartbeat;
IMU imu;
OLED display;

void setup() { /* ... */ }
void loop() { 
  SensorReadings readings = gatherSensorReadings();  // Uses new Types.h
  display.printSensorDataStruct(readings);           // Uses new Sensors.h
  /* ... */ 
}
```

**No breaking changes** — Your code logic stays the same, just better organized!

---

## Quick Reference: New Include Pattern

### In main.cpp
```cpp
#include "Sensors.h"  // One-line import of everything
```

### In sensor implementations (if modifying them)
```cpp
#include "../../Core/Config.h"    // Go up 2 levels to Core
#include "oximeter.h"             // Same folder - no path needed
```

---

## Support Structure for Future Growth

When adding a **new sensor** (e.g., pressure sensor):

1. Create folder: `include/Sensors/BMP280/`
2. Copy template CMakeLists.txt from another sensor
3. Add headers: `bmp280.h`, `pressure.h`
4. Add sources: `src/Sensors/BMP280/bmp280.cpp`, etc.
5. Update `include/Sensors.h` to include the new headers
6. Update `MIGRATION_GUIDE.md` if needed

---

## Files to Keep at Root

These should stay in the root `include/` directory:
- ✅ `Sensors.h` — Aggregator (central entry point)
- ❌ `README` — Old, can be cleaned up
- ❌ All sensor headers/implementation — Moved to subfolders

---

**Next**: Follow MIGRATION_GUIDE.md to move the remaining files and test compilation!
