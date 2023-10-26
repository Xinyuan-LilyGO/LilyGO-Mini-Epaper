/*
BMA423 accel Test example

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

struct bma4_dev bma;
struct bma4_accel sens_data;
struct bma4_accel_config accel_conf;
BMA423 sensor;
uint32_t last = 0;
uint32_t  complete_refresh = 0;
int rslt;
float x, y, z;


/* Earth's gravity in m/s^2 */
#define GRAVITY_EARTH       (9.80665f)

/*! Macro that holds the total number of accel x,y and z axes sample counts to be printed */
#define ACCEL_SAMPLE_COUNT  UINT8_C(100)

static float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width);

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
void BMA4_IIC_Configuration();
void BMA4_Accel_Configuration();


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
    if (millis() - last > 1000) {
        /* Read the accel data */
        rslt = bma4_read_accel_xyz(&sens_data, &bma);

        /* Converting lsb to meters per seconds square for 12 bit accelerometer at 2G range */
        x = lsb_to_ms2(sens_data.x, 2, bma.resolution);
        y = lsb_to_ms2(sens_data.y, 2, bma.resolution);
        z = lsb_to_ms2(sens_data.z, 2, bma.resolution);

        /* Print the data in m/s2 */
        printf("%.2f, %.2f, %.2f\r\n", x, y, z);
        //Serial.println
        display.setRotation(1);
        display.setFont(&FreeMonoBold9pt7b);
        display.fillScreen(GxEPD_WHITE);
        uint16_t box_x = 0;
        uint16_t box_y = 0;
        uint16_t box_w = 128;
        uint16_t box_h = 70;
        uint16_t cursor_y = box_y + 16;
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print("X:"); display.println(x);
        display.print("Y:"); display.println(y);
        display.print("Z:"); display.println(z);
        display.updateWindow(box_x, box_y, box_w, box_h, true);
        complete_refresh++;
        if (complete_refresh > 100) { //Use full brush after 100 rounds
            display.update();
            complete_refresh = 0;
        }
        last = millis();

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


/*! @brief Converts raw sensor values(LSB) to meters per seconds square.
 *
 *  @param[in] val      : Raw sensor value.
 *  @param[in] g_range  : Accel Range selected (2G, 4G, 8G, 16G).
 *  @param[in] bit_width    : Resolution of the sensor.
 *
 *  @return Accel values in meters per second square.
 *
 */
static float lsb_to_ms2(int16_t val, float g_range, uint8_t bit_width)
{
    float half_scale = (float)(1 << bit_width) / 2.0f;

    return GRAVITY_EARTH * val * g_range / half_scale;
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
