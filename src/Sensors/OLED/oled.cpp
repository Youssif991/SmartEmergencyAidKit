/*
  oled.cpp - Simple SSD1306 0.91" OLED Driver for ESP32

  Adapted from mtmal/OLED-0.91in-Cpp (https://github.com/mtmal/OLED-0.91in-Cpp)
  Original based on Waveshare SSD1306 OLED Module code

  Arduino-compatible implementation for SmartAidKit project.
*/

/**
 * @file oled.cpp
 * @brief Implementation of SSD1306 OLED display driver
 * @author Youssef Mohammed, Youssef Hisham (Integration), mtmal & Waveshare (Original)
 * @date 2026-05-06
 * @version 1.0
 * @details SSD1306 driver with font data, initialization sequence, and drawing primitives.
 *          Supports text and graphics rendering on 128x32 display
 */

#include "../../../include/Sensors/OLED/oled.h"
#include "../../../include/Core/Types.h"

// ── SSD1306 Initialization Commands ─────────────────────────────────────────
static const uint8_t ssd1306_init_sequence[] = {
    0xAE,       // Display OFF (0xAE)
    0x40,       // Set start line address (0x40)
    0xB0,       // Set page address (0xB0 + page)
    0xC8,       // COM output scan direction
    0x81, 0xFF, // Set contrast (0x81, value)
    0xA1,       // Segment remap (0xA1)
    0xA6,       // Display normal/reverse (0xA6 = normal)
    0xA8, 0x1F, // Set multiplex ratio (0xA8, 0x1F for 32-row display)
    0xD3, 0x00, // Set display offset (0xD3, offset)
    0xD5, 0xF0, // Set clock divide ratio (0xD5, ratio)
    0xD9, 0x22, // Set pre-charge period (0xD9, period)
    0xDA, 0x02, // Set COM pins config (0xDA, 0x02 for 32-row)
    0xDB, 0x49, // Set VCOMH deselect level (0xDB, level)
    0x8D, 0x14, // Charge pump enable (0x8D, 0x14)
    0xAF        // Display ON (0xAF)
};

// ── Simple ASCII Font (5x7 pixels) ─────────────────────────────────────────
// Each character is stored as 5 bytes (columns), 7 pixels tall
static const uint8_t ascii_font_5x7[95][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32: space
    {0x00, 0x00, 0x4F, 0x00, 0x00}, // 33: !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 34: "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35: #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36: $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 37: %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 38: &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 39: '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40: (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41: )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42: *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43: +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // 44: ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 45: -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 46: .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 47: /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48: 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49: 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50: 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51: 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52: 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53: 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54: 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55: 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56: 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57: 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 58: :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 59: ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 60: <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 61: =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 62: >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 63: ?
    {0x32, 0x49, 0x59, 0x51, 0x3E}, // 64: @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65: A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66: B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67: C
    {0x7F, 0x41, 0x41, 0x41, 0x3E}, // 68: D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69: E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 70: F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71: G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72: H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73: I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74: J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75: K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76: L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77: M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78: N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79: O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80: P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81: Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 82: R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 83: S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84: T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85: U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86: V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87: W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 88: X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 89: Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 90: Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // 91: [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // 92: backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // 93: ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // 94: ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // 95: _
};

// ── Public Methods ─────────────────────────────────────────────────────────────

bool OLED::begin()
{
    // Clear buffer
    memset(mBuffer, 0, sizeof(mBuffer));

    // Send initialization sequence
    for (size_t i = 0; i < sizeof(ssd1306_init_sequence); i++)
    {
        writeCommand(ssd1306_init_sequence[i]);
    }

    delay(200); // Wait for display to initialize
    clear();
    return true;
}

void OLED::displayOn()
{
    writeCommand(0xAF); // Display ON
}

void OLED::displayOff()
{
    writeCommand(0xAE); // Display OFF
}

void OLED::clear()
{
    memset(mBuffer, 0, sizeof(mBuffer));
    display();
}

void OLED::setPixel(uint8_t x, uint8_t y, bool on)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT)
        return;

    uint16_t index = (y / 8) * OLED_WIDTH + x;
    uint8_t bit = y % 8;

    if (on)
        mBuffer[index] |= (1 << bit);
    else
        mBuffer[index] &= ~(1 << bit);
}

void OLED::drawHLine(uint8_t x, uint8_t y, uint8_t length)
{
    for (uint8_t i = 0; i < length && (x + i) < OLED_WIDTH; i++)
        setPixel(x + i, y, true);
}

void OLED::drawVLine(uint8_t x, uint8_t y, uint8_t length)
{
    for (uint8_t i = 0; i < length && (y + i) < OLED_HEIGHT; i++)
        setPixel(x, y + i, true);
}

void OLED::drawRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    drawHLine(x, y, width);
    drawHLine(x, y + height - 1, width);
    drawVLine(x, y, height);
    drawVLine(x + width - 1, y, height);
}

void OLED::fillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    for (uint8_t row = 0; row < height; row++)
        drawHLine(x, y + row, width);
}

void OLED::printText(const char *text, uint8_t x, uint8_t y)
{
    while (*text && x < OLED_WIDTH)
    {
        drawChar(*text, x, y);
        text++;
        x += 6; // Character width + spacing
    }
}

void OLED::printNumber(int32_t number, uint8_t x, uint8_t y)
{
    char buffer[12];
    snprintf(buffer, sizeof(buffer), "%d", (int)number);
    printText(buffer, x, y);
}

void OLED::printSensorData(float tempObj, float tempAmb, int bpm, int spo2)
{
    clear();
    char line_buf[32];

    // Line 1: Temperatures (Top)
    snprintf(line_buf, sizeof(line_buf), "T:%.1fC A:%.1fC", tempObj, tempAmb);
    printText(line_buf, 0, 0); // y = 0

    // Line 2: HR and SpO2 (Bottom)
    char hr_str[10];
    char o2_str[10];

    if (bpm > 0) snprintf(hr_str, sizeof(hr_str), "%d", bpm);
    else strcpy(hr_str, "--");

    if (spo2 > 50) snprintf(o2_str, sizeof(o2_str), "%d%%", spo2);
    else strcpy(o2_str, "--");

    snprintf(line_buf, sizeof(line_buf), "HR:%s  SpO2:%s", hr_str, o2_str);
    printText(line_buf, 0, 18); // y = 18 (plenty of room for the font height)

    display();
}

void OLED::printSensorDataStruct(const SensorReadings &readings)
{
    printSensorData(readings.objectTemp, readings.ambientTemp,
                    readings.heartRate, readings.spo2);
}

void OLED::display()
{
    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        // Set page address
        writeCommand(0xB0 | page);
        // Set column address low
        writeCommand(0x00);
        // Set column address high
        writeCommand(0x10);

        // Write data for this page
        uint8_t data_buf[OLED_WIDTH + 1];
        data_buf[0] = SSD1306_DATA_MODE;
        memcpy(data_buf + 1, mBuffer + page * OLED_WIDTH, OLED_WIDTH);
        writeData(data_buf, OLED_WIDTH + 1);
    }
}

// ── Private Methods ────────────────────────────────────────────────────────────

void OLED::writeCommand(uint8_t cmd)
{
    Wire.beginTransmission(OLED_ADDRESS);
    Wire.write(SSD1306_CMD_MODE);
    Wire.write(cmd);
    Wire.endTransmission();
}

void OLED::writeData(const uint8_t *data, uint16_t length)
{
    Wire.beginTransmission(OLED_ADDRESS);
    for (uint16_t i = 0; i < length; i++)
        Wire.write(data[i]);
    Wire.endTransmission();
}

void OLED::drawChar(char c, uint8_t x, uint8_t y)
{
    if (c < 32 || c > 126 || x >= OLED_WIDTH)
        return;

    const uint8_t *char_data = ascii_font_5x7[c - 32];

    for (uint8_t col = 0; col < 5; col++)
    {
        uint8_t byte = char_data[col];
        for (uint8_t row = 0; row < 7; row++)
        {
            if (byte & (1 << row))
                setPixel(x + col, y + row, true);
        }
    }
}