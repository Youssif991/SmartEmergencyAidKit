#pragma once

#include <Adafruit_MLX90614.h>
#include "Config.h"

// Wraps the MLX90614 non-contact thermometer.
// Call begin() once in setup(), then update() every loop iteration.
// Reads are non-blocking — internally throttled to TEMP_INTERVAL_MS.

class TempSensor {
public:
    bool  begin();

    // Should be called every loop(). Only talks to the sensor at TEMP_INTERVAL_MS.
    void  update();

    float objectTemp()  const { return _objectTemp;  }
    float ambientTemp() const { return _ambientTemp; }

private:
    Adafruit_MLX90614 _mlx;
    float    _objectTemp  = 0.0f;
    float    _ambientTemp = 0.0f;
    uint32_t _lastReadMs  = 0;
};
