/*
    LilyGo Ink Screen Series u8g2Fonts Test
        - Created by Kaibin he
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

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);

const uint32_t partial_update_period_s = 1;
const uint32_t full_update_period_s = 6 * 60 * 60;

uint32_t start_time;
uint32_t next_time;
uint32_t previous_time;
uint32_t previous_full_update;

uint32_t total_seconds = 0;
uint32_t seconds, minutes, hours, days;

void showPartialUpdate();

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    // draw background
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial

    //display.init(115200); // enable diagnostic output on Serial
    display.init(); // disable diagnostic output on Serial
    Serial.println("setup done");
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(0);

    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    display.setFont(&FreeMonoBold12pt7b);

    // partial update to full screen to preset for partial update of box window
    // (this avoids strange background effects)
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1), GxEPD::bm_default | GxEPD::bm_partial_update);
    start_time = next_time = previous_time = previous_full_update = millis();
    display.setRotation(1);
}

void loop()
{
    Serial.println("loop");
    uint32_t actual = millis();
    while (actual < next_time) {
        // the "BlinkWithoutDelay" method works also for overflowed millis
        if ((actual - previous_time) > (partial_update_period_s * 1000)) {
            //Serial.print(actual - previous_time); Serial.print(" > "); Serial.println(partial_update_period_s * 1000);
            break;
        }
        delay(100);
        actual = millis();
    }
    //Serial.print("actual: "); Serial.print(actual); Serial.print(" previous: "); Serial.println(previous_time);
    if ((actual - previous_full_update) > full_update_period_s * 1000) {

        display.update();

        previous_full_update = actual;
    }
    previous_time = actual;
    next_time += uint32_t(partial_update_period_s * 1000);
    total_seconds += partial_update_period_s;
    seconds = total_seconds % 60;
    minutes = (total_seconds / 60) % 60;
    hours = (total_seconds / 3600) % 24;
    days = (total_seconds / 3600) / 24;

    showPartialUpdate();

}

void print02d(uint32_t d)
{
    if (d < 10) display.print("0");
    display.print(d);
}



void showPartialUpdate()
{
    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.fillScreen(GxEPD_WHITE);
    uint16_t box_x = 0;
    uint16_t box_y = 15;
    uint16_t box_w = 170;
    uint16_t box_h = 20;
    uint16_t cursor_y = box_y + 16;
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.print(days); display.print("d "); print02d(hours); display.print(":"); print02d(minutes); display.print(":"); print02d(seconds);
    display.updateWindow(box_x, box_y, box_w, box_h, true);

}
