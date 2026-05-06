# MLX90614 Temperature Sensor Module
# Non-contact infrared thermometer (Adafruit MLX90614)
#
# Dependencies:
#   - Wire (I2C)
#   - Adafruit_MLX90614 library
#   - Core/Config.h
#   - Core/Types.h
#
# Provides:
#   - TempSensor class (Temperature measurement wrapper)
#
# Source files:
#   - Adafruit_MLX90614.cpp (Adafruit library implementation)
#   - temperature.cpp (TempSensor wrapper)

target_sources(firmware PRIVATE
  src/Sensors/MLX90614/Adafruit_MLX90614.cpp
  src/Sensors/MLX90614/temperature.cpp
)

target_include_directories(firmware PUBLIC
  include/Sensors/MLX90614
)
