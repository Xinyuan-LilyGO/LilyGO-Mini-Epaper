// According to the board, cancel the corresponding macro definition
// https://www.lilygo.cc/products/mini-e-paper-core , esp32picod4
// #define LILYGO_MINI_EPAPER_ESP32
// esp32s3-fn4r2
// #define LILYGO_MINI_EPAPER_ESP32S3
#if !defined(LILYGO_MINI_EPAPER_ESP32S3)  && !defined(LILYGO_MINI_EPAPER_ESP32)
// 请在草图上方选择对应的目标板子名称,将它取消注释.
#error "Please select the corresponding target board name above the sketch and uncomment it."
#endif

#include GxEPD_BitmapExamples
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include <U8g2_for_Adafruit_GFX.h>
#include <boards.h>
#include <GxEPD.h>
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include "pcf8563.h"
PCF8563_Class rtc;



GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void LilyGo_logo();
void showtime();

void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");


    Wire.begin(SDA, SCL);
    rtc.begin();
    rtc.setDateTime(2020, 12, 31, 23, 59, 50);//set time . year,month,day,huor,min,second

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial
    u8g2Fonts.begin(display);
    display.setTextColor(GxEPD_BLACK);
    u8g2Fonts.setFontMode(1);                           // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(1);                      // left to right (this is default)
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);          // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);          // apply Adafruit GFX color

    LilyGo_logo();
    Serial.println("setup done");

    RTC_Date t = rtc.getDateTime();
}

uint32_t last = 0;
void loop()
{

    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
    Serial.println(rtc.getDayOfWeek(28, 6, 2021));

    if (millis() - last > 1000) {
        showtime();
        last = millis();
    }
}

void print02d(uint32_t d)
{
    if (d < 10) display.print("0");
    display.print(d);
}

void showtime()
{
    RTC_Date t = rtc.getDateTime();

    display.setRotation(1);
    display.setFont(&FreeMonoBold9pt7b);
    display.fillScreen(GxEPD_WHITE);
    uint16_t box_x = 0;
    uint16_t box_y = 0;
    uint16_t box_w = 170;
    uint16_t box_h = 40;
    uint16_t cursor_y = box_y + 16;
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y);
    display.print(t.year); display.print("/"); display.print(t.month); display.print("/"); display.print(t.day);
    display.setCursor(box_x, cursor_y + 16);
    print02d(t.hour); display.print(":"); print02d(t.minute); display.print(":"); print02d(t.second);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void LilyGo_logo(void)
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    // partial update to full screen to preset for partial update of box window
    // (this avoids strange background effects)
    display.drawExampleBitmap(BitmapExample1, sizeof(BitmapExample1), GxEPD::bm_default | GxEPD::bm_partial_update);
}
