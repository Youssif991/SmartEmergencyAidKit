/**
 * @file driver_mpu6050_interface.cpp
 * @brief Implementation of MPU6050 driver Arduino Wire interface adapter
 * @author Youssef Mohammed, Youssef Hisham
 * @date 2026-05-06
 * @version 1.0
 * @details C++ wrapper implementation for Arduino Wire library.
 *          Platform-specific I2C functions for ESP32-C3
 */

/*
  driver_mpu6050_interface.cpp
  ----------------------------
  ESP32-C3 / Arduino Wire implementation of the seven platform functions
  declared in driver_mpu6050_interface.h.

  Wire.begin() is called once in main.cpp before any sensor begin() call,
  so the IIC init/deinit stubs here are deliberate no-ops.

  Wiring:
    SDA → GPIO 8  (I2C_SDA_PIN in Config.h)
    SCL → GPIO 9  (I2C_SCL_PIN in Config.h)
    AD0 → GND     → I2C address 0x68 (MPU6050_ADDRESS_AD0_LOW)
*/

#include "../../../include/Sensors/MPU6050/driver_mpu6050_interface.h"

#include <Arduino.h>
#include <Wire.h>
#include <stdarg.h>

// ── I2C init / deinit ─────────────────────────────────────────────────────────
// Wire is owned by main.cpp — nothing to initialise here.

uint8_t mpu6050_interface_iic_init(void)
{
    return 0;
}

uint8_t mpu6050_interface_iic_deinit(void)
{
    return 0;
}

// ── I2C read ──────────────────────────────────────────────────────────────────

uint8_t mpu6050_interface_iic_read(uint8_t addr, uint8_t reg, uint8_t *buf, uint16_t len)
{
    // The driver passes 8-bit addresses (0xD0, 0xD2) with R/W bit.
    // Arduino Wire expects 7-bit addresses, so convert by right-shifting.
    uint8_t i2c_addr = addr >> 1;
    
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg);
    
    // Use full STOP (true) for better compatibility with ESP32 I2C hardware
    if (Wire.endTransmission(true) != 0) 
        return 1;

    // Add sufficient delay for MPU6050 to latch the register pointer
    delayMicroseconds(100); 

    uint16_t received = Wire.requestFrom(i2c_addr, len, true);
    if (received != len)
        return 1;

    for (uint16_t i = 0; i < len; i++)
        buf[i] = (uint8_t)Wire.read();

    return 0;
}

// ── I2C write ─────────────────────────────────────────────────────────────────

uint8_t mpu6050_interface_iic_write(uint8_t addr, uint8_t reg,
                                     uint8_t *buf, uint16_t len)
{
    // The driver passes 8-bit addresses (0xD0, 0xD2) with R/W bit.
    // Arduino Wire expects 7-bit addresses, so convert by right-shifting.
    uint8_t i2c_addr = addr >> 1;
    
    Wire.beginTransmission(i2c_addr);
    Wire.write(reg);
    for (uint16_t i = 0; i < len; i++)
        Wire.write(buf[i]);
    // Send STOP condition (true parameter) to properly close the I2C transaction
    return (Wire.endTransmission(true) == 0) ? 0 : 1;
}

// ── Delay ─────────────────────────────────────────────────────────────────────

void mpu6050_interface_delay_ms(uint32_t ms)
{
    delay(ms);
}

// ── Debug print ───────────────────────────────────────────────────────────────

void mpu6050_interface_debug_print(const char *const fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print(buf);
}

// ── Interrupt callback ────────────────────────────────────────────────────────
// Not used in basic mode.

void mpu6050_interface_receive_callback(uint8_t type)
{
    (void)type;
}
