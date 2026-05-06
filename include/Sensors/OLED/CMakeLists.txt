# OLED Display Module
# 0.91" SSD1306 I2C OLED display (128x32 pixels)
#
# Dependencies:
#   - Wire (I2C)
#   - Core/Types.h (SensorReadings struct)
#
# Provides:
#   - OLED class (Display driver and rendering)
#   - Methods for text, drawing, and sensor data visualization
#
# Source files:
#   - oled.cpp (OLED driver implementation)

target_sources(firmware PRIVATE
  src/Sensors/OLED/oled.cpp
)

target_include_directories(firmware PUBLIC
  include/Sensors/OLED
)
