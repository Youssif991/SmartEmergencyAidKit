#pragma once

/*
  oled.h - Simple SSD1306 0.91" OLED Driver for ESP32

  Adapted from mtmal/OLED-0.91in-Cpp (https://github.com/mtmal/OLED-0.91in-Cpp)
  Original based on Waveshare SSD1306 OLED Module code

  This is a minimal Arduino-compatible driver optimized for the SmartAidKit project.

  Hardware:
    - 0.91" SSD1306 OLED Display (128x32 pixels)
    - I2C address: 0x3C (default)

  Wiring (ESP32):
    - SDA → GPIO 8  (I2C_SDA_PIN in Config.h)
    - SCL → GPIO 9  (I2C_SCL_PIN in Config.h)
    - VCC → 3.3V
    - GND → GND
*/

#include <Arduino.h>
#include <Wire.h>
#include <cstring>

// Forward declaration to avoid circular includes
struct SensorReadings;

// ── OLED Display Dimensions ────────────────────────────────────────────────────
constexpr uint8_t OLED_WIDTH = 128;
constexpr uint8_t OLED_HEIGHT = 32;
constexpr uint8_t OLED_PAGES = OLED_HEIGHT / 8; // 4 pages of 8 pixels each
constexpr uint16_t OLED_BUFFER_SIZE = OLED_WIDTH * OLED_PAGES;

// ── SSD1306 I2C Addresses ──────────────────────────────────────────────────────
constexpr uint8_t OLED_ADDRESS = 0x3C; // 7-bit I2C address

// ── SSD1306 Commands ───────────────────────────────────────────────────────────
constexpr uint8_t SSD1306_CMD_MODE = 0x00;  // Command mode prefix
constexpr uint8_t SSD1306_DATA_MODE = 0x40; // Data mode prefix

class OLED
{
public:
    // ── Lifecycle ──────────────────────────────────────────────────────────────

    OLED() = default;
    virtual ~OLED() = default;

    // Initialize the OLED display
    // Call this once from setup() after Wire.begin()
    bool begin();

    // Turn the display on
    void displayOn();

    // Turn the display off
    void displayOff();

    // Clear the display (fill with black)
    void clear();

    // ── Drawing Operations ────────────────────────────────────────────────────

    // Set pixel at (x, y) to on (true) or off (false)
    void setPixel(uint8_t x, uint8_t y, bool on);

    // Draw a horizontal line from (x, y) for length pixels
    void drawHLine(uint8_t x, uint8_t y, uint8_t length);

    // Draw a vertical line from (x, y) for length pixels
    void drawVLine(uint8_t x, uint8_t y, uint8_t length);

    // Draw a rectangle outline
    void drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    // Draw a filled rectangle
    void fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);

    // ── Text Display ───────────────────────────────────────────────────────────

    // Print text at (x, y) - simple monospace font
    // x: 0-127, y: 0-31 (pixel position)
    void printText(const char *text, uint8_t x, uint8_t y);

    // Print a number at (x, y)
    void printNumber(int32_t number, uint8_t x, uint8_t y);

    // Print sensor readings on screen
    void printSensorData(float tempObj, float tempAmb, int bpm, int spo2);

    // Print sensor readings from a SensorReadings struct
    void printSensorDataStruct(const SensorReadings &readings);

    // Render the buffer to the display
    void display();

private:
    // ── Display Buffer ────────────────────────────────────────────────────────
    uint8_t mBuffer[OLED_BUFFER_SIZE] = {};

    // ── Private Methods ────────────────────────────────────────────────────────

    // Send a single byte as a command
    void writeCommand(uint8_t cmd);

    // Send data bytes to the display
    void writeData(const uint8_t *data, uint16_t length);

    // Set the display cursor position (page and column)
    void setCursor(uint8_t page, uint8_t column);

    // Draw a single ASCII character at (x, y)
    void drawChar(char c, uint8_t x, uint8_t y);
};
