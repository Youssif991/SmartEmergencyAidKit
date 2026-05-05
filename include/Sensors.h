#pragma once

/*
  Sensors.h — Aggregator header for all sensor modules.
  
  Include this single file in main.cpp instead of including each sensor
  module individually. This centralizes sensor dependencies and makes it
  easier to manage the module list as the project grows.
*/

#include "Config.h"
#include "Types.h"
#include "MAX30105.h"
#include "temperature.h"
#include "oximeter.h"
#include "heartbeat.h"
#include "imu.h"
#include "oled.h"
#include "SensorUtils.h"
