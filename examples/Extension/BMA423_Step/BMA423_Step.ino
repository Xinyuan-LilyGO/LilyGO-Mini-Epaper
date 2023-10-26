/*
BMA423 step Test example
Set water-mark level 1 to get interrupt after 20 steps.
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
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>


#include "Arduino.h"
#include <stdio.h>
#include "bma423.h"
#include <Wire.h>


GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

const uint8_t sda = 26;
const uint8_t scl = 25;
#define int_pin 38


struct bma4_dev bma;
struct bma4_accel sens_data;
struct bma4_accel_config accel_conf;
static struct bma4_fifo_frame s_fifoFrame = {0};
static uint8_t s_fifoBuffer[255] = {0};
int8_t rslt;

/* Variable to get the step counter output */
uint32_t step_out = 0;

/* Variable to get the interrupt status  */
uint16_t int_status = 0;

bool BMA_IRQ = false;

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
void LilyGo_logo();
void BMA4_IIC_Configuration();
void BMA4_Accel_Configuration();
void BMA4_INT_Configuration();


void EnterSleep()
{
    //  sensor.shutDown();
//    Wire.end();
    pinMode(25, OUTPUT);
    pinMode(26, OUTPUT);
    digitalWrite(25, HIGH);
    digitalWrite(26, HIGH);
    Serial.println("EnterSleep");
    delay(2000);
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
    /*Turn on power control*/
}


void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    pinMode(int_pin, INPUT_PULLUP);
    attachInterrupt(int_pin, [] {
        // Set interrupt to set irq value to true
        BMA_IRQ = true;
    }, RISING); //Select the interrupt mode according to the actual circuit



    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial


    Wire.begin(sda, scl);

    BMA4_IIC_Configuration();
    BMA4_Accel_Configuration();
    BMA4_INT_Configuration();

    rslt = bma423_step_detector_enable(BMA4_ENABLE, &bma);
    Serial.print("bma423_step_detector_enable"); Serial.println(rslt);

    /* Enable step counter */
    rslt = bma423_feature_enable(BMA423_STEP_CNTR, 1, &bma);
    if (rslt != BMA4_OK) {
        Serial.print(" bma423_feature_enable FALL");
        Serial.println(rslt);
    }


    /* Set water-mark level 1 to get interrupt after 20 steps.
     * Range of step counter interrupt is 0 to 20460(resolution of 20 steps).
     */
    rslt = bma423_step_counter_set_watermark(1, &bma);
    if (rslt != BMA4_OK) {
        Serial.print("bma423_step_counter FALL");    //bma4_error_codes_print_result("bma423_step_counter status", rslt);
        Serial.println(rslt);
    }

    rslt = bma4_set_advance_power_save(BMA4_ENABLE, &bma);
    if (rslt != BMA4_OK) {
        Serial.print(" power save FALL");
        Serial.println(rslt);
    }

    Serial.println("Move/perform the walk/step action with the sensor\n");

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.update();
    delay(1000);
    Serial.println("setup done");
}
uint16_t counter_IRQ = 0;
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
    display.print("step ");
//delay(3000);
//EnterSleep();
    if (BMA_IRQ) {
        BMA_IRQ = false;

        /* Read the interrupt register to check whether step counter interrupt is received. */
        rslt = bma423_read_int_status(&int_status, &bma);
        if (rslt != BMA4_OK) {
            Serial.print("bma423_read_int_status FALL");
            Serial.println(rslt);
        }


        /*    Check if step counter interrupt is triggered */
        if (int_status & BMA423_STEP_CNTR_INT) {
            Serial.println("Step counter interrupt received");

            /* On interrupt, Get step counter output */
            rslt = bma423_step_counter_output(&step_out, &bma);
            if (rslt != BMA4_OK) {
                Serial.print(" bma423_step_counter_output FALL");
                Serial.println(rslt);
            }
            counter_IRQ++;
            Serial.print("step counter out"); Serial.println(step_out);//输出步数
            display.println(step_out);
            display.updateWindow(box_x, box_y, box_w, box_h, true);
        }

    }
    delay(20);


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

void BMA4_INT_Configuration()
{
    struct bma4_int_pin_config intPinCofig;
    intPinCofig.edge_ctrl = BMA4_LEVEL_TRIGGER;
    intPinCofig.lvl = BMA4_ACTIVE_HIGH;
    intPinCofig.input_en = BMA4_INPUT_DISABLE;
    intPinCofig.od = BMA4_PUSH_PULL;//BMA4_OPEN_DRAIN;
    intPinCofig.output_en = BMA4_OUTPUT_ENABLE;
    /* Set the electrical behaviour of interrupt pin1 */
    bma4_set_int_pin_config(&intPinCofig, BMA4_INTR1_MAP, &bma);


    rslt = bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT, 1, &bma);
    Serial.print("bma423_map_interrupt"); Serial.println(rslt);


}
