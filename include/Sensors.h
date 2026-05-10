/**
 * @file Sensors.h
 * @brief Central aggregator header for all sensor modules
 * @author Youssef Mohammed, Youssef Hisham, Mahmoud Abdeldayem
 * @date 2026-05-06
 * @version 1.0
 * @details Single entry point that includes all sensor definitions and utilities.
 *          Include this file in main.cpp to access all sensors.
 */

#pragma once

/**
 * @defgroup MAX30105_Group MAX30105 Pulse Oximeter & Heart Rate
 * @brief Modules related to the MAX30105 sensor and biometric algorithms.
 */

/**
 * @defgroup MLX90614_Group MLX90614 Contactless Temperature
 * @brief Modules related to the MLX90614 infrared thermometer.
 */

/**
 * @defgroup MPU6050_Group MPU6050 Inertial Measurement Unit
 * @brief Modules related to motion tracking and orientation.
 */

/**
 * @defgroup OLED_Group SSD1306 OLED Display
 * @brief Modules related to visual data output.
 */


// Core system
#include "Core/Config.h"
#include "Core/Types.h"
#include "Core/SensorUtils.h"

// Sensor modules (each self-contained)
#include "Sensors/MAX30105/MAX30105.h"
#include "Sensors/MAX30105/oximeter.h"
#include "Sensors/MAX30105/heartbeat.h"

#include "Sensors/MLX90614/temperature.h"

#include "Sensors/MPU6050/imu.h"
#include "Sensors/MPU6050/driver_mpu6050.h"

#include "Sensors/OLED/oled.h"