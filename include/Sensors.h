/**
 * @file Sensors.h
 * @brief Central aggregator header for all sensor modules
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details Single entry point that includes all sensor definitions and utilities.
 *          Include this file in main.cpp to access all sensors.
 */

#pragma once


// Core system
#include "Core/Config.h"
#include "Core/Types.h"
#include "Core/SensorUtils.h"

// Sensor modules (each self-contained)
#include "Sensors/MAX30105/MAX30105.h"
#include "Sensors/MAX30105/oximeter.h"
#include "Sensors/MAX30105/heartbeat.h"

#include "Sensors/MLX90614/temperature.h"
#include "Sensors/MLX90614/Adafruit_MLX90614.h"

#include "Sensors/MPU6050/imu.h"
#include "Sensors/MPU6050/driver_mpu6050.h"

#include "Sensors/OLED/oled.h"
