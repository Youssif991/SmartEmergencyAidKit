#pragma once

/*
  driver_mpu6050_interface.h
  --------------------------
  This file is required by driver_mpu6050_basic.h (which does
  #include "driver_mpu6050_interface.h").

  It declares the seven platform functions that the libdriver/mpu6050
  driver calls for all hardware I/O. The implementations live in
  driver_mpu6050_interface.cpp and use Arduino Wire.
*/

#ifndef DRIVER_MPU6050_INTERFACE_H
#define DRIVER_MPU6050_INTERFACE_H

#include "driver_mpu6050.h"   /* provides mpu6050_address_t, mpu6050_handle_t, etc. */
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t mpu6050_interface_iic_init(void);
uint8_t mpu6050_interface_iic_deinit(void);
uint8_t mpu6050_interface_iic_read(uint8_t addr, uint8_t reg,
                                    uint8_t *buf, uint16_t len);
uint8_t mpu6050_interface_iic_write(uint8_t addr, uint8_t reg,
                                     uint8_t *buf, uint16_t len);
void    mpu6050_interface_delay_ms(uint32_t ms);
void    mpu6050_interface_debug_print(const char *const fmt, ...);
void    mpu6050_interface_receive_callback(uint8_t type);

#ifdef __cplusplus
}
#endif

#endif