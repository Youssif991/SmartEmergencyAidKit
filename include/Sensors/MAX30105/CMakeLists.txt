# MAX30105 Sensor Module
# Pulse oximeter + Heart rate sensor (SparkFun MAX30105)
#
# Dependencies:
#   - Wire (I2C)
#   - Core/Config.h
#   - Core/Types.h
#
# Provides:
#   - Oximeter class (SpO2 measurement)
#   - HeartBeat class (Heart rate via PBA algorithm)
#   - MAX30105 driver class (Sensor low-level control)
#
# Source files:
#   - MAX30105.cpp (sensor driver)
#   - oximeter.cpp (SpO2 wrapper)
#   - heartbeat.cpp (Heart rate wrapper)
#   - spo2_algorithm.cpp (SpO2 calculation algorithm)
#   - heartRate.cpp (Beat detection algorithm)

target_sources(firmware PRIVATE
  src/Sensors/MAX30105/MAX30105.cpp
  src/Sensors/MAX30105/oximeter.cpp
  src/Sensors/MAX30105/heartbeat.cpp
  src/Sensors/MAX30105/spo2_algorithm.cpp
  src/Sensors/MAX30105/heartRate.cpp
)

target_include_directories(firmware PUBLIC
  include/Sensors/MAX30105
)
