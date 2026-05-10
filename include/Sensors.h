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
12: 
13: /**
14:  * @defgroup MAX30105_Group MAX30105 Pulse Oximeter & Heart Rate
15:  * @brief Modules related to the MAX30105 sensor and biometric algorithms.
16:  */
17: 
18: /**
19:  * @defgroup MLX90614_Group MLX90614 Contactless Temperature
20:  * @brief Modules related to the MLX90614 infrared thermometer.
21:  */
22: 
23: /**
24:  * @defgroup MPU6050_Group MPU6050 Inertial Measurement Unit
25:  * @brief Modules related to motion tracking and orientation.
26:  */
27: 
28: /**
29:  * @defgroup OLED_Group SSD1306 OLED Display
30:  * @brief Modules related to visual data output.
31:  */
32: 


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