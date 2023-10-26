/*
BMA423 direction Test example

This example code is in the public domain.
*/

// According to the board, cancel the corresponding macro definition
// https://www.lilygo.cc/products/mini-e-paper-core , esp32picod4
// #define LILYGO_MINI_EPAPER_ESP32
// esp32s3-fn4r2
// #define LILYGO_MINI_EPAPER_ESP32S3
#if !defined(LILYGO_MINI_EPAPER_ESP32S3)  && !defined(LILYGO_MINI_EPAPER_ESP32)
// 请在草图上方选择对应的目标板子名称,将它取消注释.
#error "Please select the corresponding target board name above the sketch and uncomment it."
#endif

#include <boards.h>
#include <GxEPD.h>
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Arduino.h>
#include <Wire.h>
#include <bma.h>
#include <stdio.h>
#include "bma423.h"

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
const uint8_t sda = 26;
const uint8_t scl = 25;
uint8_t prevRotation;

struct bma4_dev bma;
struct bma4_accel sens_data;
struct bma4_accel_config accel_conf;
BMA423 sensor;
uint32_t last = 0;
uint32_t  complete_refresh = 0;
int rslt;

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
void BMA4_IIC_Configuration();
void BMA4_Accel_Configuration();
uint8_t getDirection();


void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial

    Serial.println("BMA423 Basic Readings Accel Example");
    Wire.begin(sda, scl);
    BMA4_IIC_Configuration();
    BMA4_Accel_Configuration();

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.update();
    delay(1000);
    Serial.println("setup done");
}


void loop()
{
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.fillScreen(GxEPD_WHITE);
    uint16_t box_x = 0;
    uint16_t box_y = 50;
    uint16_t box_w = 170;
    uint16_t box_h = 40;
    uint16_t cursor_y = box_y + 16;
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);

    // Obtain the BMA423 direction,
    // so that the screen orientation is consistent with the sensor
    uint8_t rotation = getDirection();
    if (prevRotation != rotation) {
        prevRotation = rotation;
        switch (rotation) {
        case DIRECTION_DISP_DOWN: {
            Serial.println("DIRECTION_DISP_DOWN");
            display.print("DOWN");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }

        case DIRECTION_DISP_UP: {
            Serial.println("DIRECTION_DISP_UP");
            display.print("UP");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }

        case DIRECTION_BOTTOM_EDGE: {
            Serial.println("DIRECTION_BOTTOM_EDGE");
            display.print("BOTTOM");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }

        case DIRECTION_TOP_EDGE: {
            Serial.println("DIRECTION_TOP_EDGE");
            display.print("TOP");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }

        case DIRECTION_RIGHT_EDGE: {
            Serial.println("DIRECTION_RIGHT_EDGE");
            display.print("RIGHT");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }

        case DIRECTION_LEFT_EDGE: {
            Serial.println("DIRECTION_LEFT_EDGE");
            display.print("LEFT");
            display.updateWindow(box_x, box_y, box_w, box_h, true);
            break;
        }
        default:
            Serial.println("ERROR!!!");
            break;
        }
        delay(500);

    }
}

uint8_t getDirection()
{
    Accel acc;
    if (bma4_read_accel_xyz(&acc, &bma) != BMA4_OK) {
        return 0;
    }
    uint16_t absX = abs(acc.x);
    uint16_t absY = abs(acc.y);
    uint16_t absZ = abs(acc.z);

    if ((absZ > absX) && (absZ > absY)) {
        if (acc.z > 0) {
            return  DIRECTION_DISP_DOWN;
        } else {
            return DIRECTION_DISP_UP;
        }
    } else if ((absY > absX) && (absY > absZ)) {
        if (acc.y > 0) {
            return DIRECTION_BOTTOM_EDGE;
        } else {
            return  DIRECTION_TOP_EDGE;
        }
    } else {
        if (acc.x < 0) {
            return  DIRECTION_RIGHT_EDGE;
        } else {
            return DIRECTION_LEFT_EDGE;
        }
    }
}

// Low-level I2C Communication
// Provided to BMA423_Library communication interface
//
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, (uint8_t)len);
    uint8_t i = 0;
    while (Wire.available()) {
        data[i++] = Wire.read();
    }
    return 0; //Pass
}

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, len);
    return (0 !=  Wire.endTransmission());
}

void BMA4_IIC_Configuration()
{
    bma.chip_id         = BMA423_CHIP_ID;
    bma.dev_addr        = BMA4_I2C_ADDR_PRIMARY;
    bma.interface       = BMA4_I2C_INTERFACE;
    bma.bus_read        = readRegister;
    bma.bus_write       = writeRegister;
    bma.delay           = delay;
    bma.read_write_len  = 8;
    bma.resolution      = 12;
    bma.feature_len     = BMA423_FEATURE_SIZE;

    bma4_set_command_register(0xB6, &bma);
    delay(20);

    /* Sensor initialization */
    rslt = bma423_init(&bma);
    Serial.print("bma423_init"); Serial.println(rslt);


    /* Upload the configuration file to enable the features of the sensor. */
    rslt = bma423_write_config_file(&bma);
    Serial.print("bma423_write_config"); Serial.println(rslt);

}

void BMA4_Accel_Configuration()
{
    accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    accel_conf.range = BMA4_ACCEL_RANGE_2G;
    accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

    /* Set the accel configurations */
    rslt = bma4_set_accel_config(&accel_conf, &bma);
    Serial.print("bma4_set_accel_config"); Serial.println(rslt);

    /* Enable the accelerometer */
    rslt = bma4_set_accel_enable(1, &bma);
    Serial.print("bma4_set_accel_enable"); Serial.println(rslt);
}
