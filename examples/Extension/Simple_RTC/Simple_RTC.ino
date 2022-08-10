/*
    LilyGo Ink Screen Series u8g2Fonts Test
        - Created by Lewis he
*/

// According to the board, cancel the corresponding macro definition
// #define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY_102
// #define LILYGO_EPD_DISPLAY_154
#define LILYGO_T5_V102

#include <boards.h>
#include <GxEPD.h>

#if defined(LILYGO_T5_V102) || defined(LILYGO_EPD_DISPLAY_102)
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#elif defined(LILYGO_T5_V266)
#include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
#elif defined(LILYGO_T5_V213)
#include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
#else
// #include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
// #include <GxGDEW0154Z04/GxGDEW0154Z04.h>  // 1.54" b/w/r 200x200
// #include <GxGDEW0154Z17/GxGDEW0154Z17.h>  // 1.54" b/w/r 152x152
// #include <GxGDEH0154D67/GxGDEH0154D67.h>  // 1.54" b/w
// #include <GxDEPG0150BN/GxDEPG0150BN.h>    // 1.51" b/w   form DKE GROUP
// #include <GxDEPG0266BN/GxDEPG0266BN.h>    // 2.66" b/w   form DKE GROUP
// #include <GxDEPG0290R/GxDEPG0290R.h>      // 2.9" b/w/r  form DKE GROUP
// #include <GxDEPG0290B/GxDEPG0290B.h>      // 2.9" b/w    form DKE GROUP
// #include <GxGDEW029Z10/GxGDEW029Z10.h>    // 2.9" b/w/r  form GoodDisplay
// #include <GxGDEW0213Z16/GxGDEW0213Z16.h>  // 2.13" b/w/r form GoodDisplay
// #include <GxGDE0213B1/GxGDE0213B1.h>      // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEH0213B72/GxGDEH0213B72.h>  // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEH0213B73/GxGDEH0213B73.h>  // 2.13" b/w  old panel , form GoodDisplay
// #include <GxGDEM0213B74/GxGDEM0213B74.h>  // 2.13" b/w  form GoodDisplay 4-color
// #include <GxGDEW0213M21/GxGDEW0213M21.h>  // 2.13"  b/w Ultra wide temperature , form GoodDisplay
// #include <GxDEPG0213BN/GxDEPG0213BN.h>    // 2.13" b/w  form DKE GROUP
// #include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w   form GoodDisplay
// #include <GxGDEW027C44/GxGDEW027C44.h>    // 2.7" b/w/r form GoodDisplay
// #include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w   form GoodDisplay
// #include <GxDEPG0750BN/GxDEPG0750BN.h>    // 7.5" b/w   form DKE GROUP
#endif

#include GxEPD_BitmapExamples
#include <U8g2_for_Adafruit_GFX.h>

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include "pcf8563.h"
PCF8563_Class rtc;

#define SDA 26
#define SCL 25

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

#if defined(LILYGO_EPD_DISPLAY_102)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
#endif /*LILYGO_EPD_DISPLAY_102*/
#if defined(LILYGO_T5_V102)
    pinMode(POWER_ENABLE, OUTPUT);
    digitalWrite(POWER_ENABLE, HIGH);
#endif /*LILYGO_T5_V102*/

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

uint32_t last=0;
void loop()
{
    
    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
    Serial.println(rtc.getDayOfWeek(28,6, 2021));

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
    display.print(t.year); display.print("/");display.print(t.month); display.print("/");display.print(t.day);
    display.setCursor(box_x, cursor_y+16);
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
