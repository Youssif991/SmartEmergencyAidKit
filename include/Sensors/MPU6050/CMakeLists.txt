# MPU6050 IMU Module
# 6-axis accelerometer + gyroscope (InvenSense MPU6050)
#
# Dependencies:
#   - Wire (I2C)
#   - Core/Config.h
#   - Core/Types.h
#
# Provides:
#   - IMU class (Inertial measurement wrapper)
#   - driver_mpu6050 functions (Low-level I2C driver)
#
# Source files:
#   - imu.cpp (IMU wrapper)
#   - driver_mpu6050.c (Low-level driver - C source)
#   - driver_mpu6050_basic.c (Basic interface)
#   - driver_mpu6050_interface.cpp (I2C interface)

target_sources(firmware PRIVATE
  src/Sensors/MPU6050/imu.cpp
  src/Sensors/MPU6050/driver_mpu6050.c
  src/Sensors/MPU6050/driver_mpu6050_basic.c
  src/Sensors/MPU6050/driver_mpu6050_interface.cpp
)

target_include_directories(firmware PUBLIC
  include/Sensors/MPU6050
)
