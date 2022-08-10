/*
    LilyGo Ink Screen Series Test
        - Created by Lewis he
*/

// According to the board, cancel the corresponding macro definition
// #define LILYGO_T5_V213
// #define LILYGO_T5_V22
// #define LILYGO_T5_V24
// #define LILYGO_T5_V28
#define LILYGO_T5_V102
// #define LILYGO_T5_V266
// #define LILYGO_EPD_DISPLAY_102      //Depend  https://github.com/adafruit/Adafruit_NeoPixel
// #define LILYGO_EPD_DISPLAY_154

#define PWR_ON              27

#include <boards.h>
#include <GxEPD.h>
#include <SD.h>
#include <FS.h>
#include <AceButton.h>    //https://github.com/bxparks/AceButton
using namespace         ace_button;

#if defined(LILYGO_T5_V102) || defined(LILYGO_EPD_DISPLAY_102)
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include <Adafruit_NeoPixel.h>             //Depend  https://github.com/adafruit/Adafruit_NeoPixel
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

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>


#include "font_prazo_7.h"
#include "font_prazo_9.h"
#include "font_prazo_12.h"
#include "font_prazo_16.h"
#include "font_prazo_18.h"
#include "font_prazo_22.h"
#include "font_prazo_24.h"

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);


AceButton btn1(BUTTON_1, HIGH);
#ifdef BUTTON_2
AceButton btn2(BUTTON_2, HIGH);
#endif
#ifdef BUTTON_3
AceButton btn3((uint8_t) BUTTON_3, HIGH);
#endif

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(VSPI);
#endif

#if defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_) ||defined(_GxGDEW0154Z17_H_) || defined(_GxGDEW0154Z04_H_) || defined(_GxDEPG0290R_H_)
#define _HAS_COLOR_
#endif

#if defined(LILYGO_EPD_DISPLAY_102)
Adafruit_NeoPixel strip(RGB_STRIP_COUNT, RGB_STRIP_PIN, NEO_GRBW + NEO_KHZ800);
#endif /*LILYGO_EPD_DISPLAY_102*/

bool rlst = false;
void showFont(const char name[], const GFXfont *f);
void drawCornerTest(void);
void GxepdPage0();
void GxepdPage1();
void GxepdPage2();
void EnterSleep();

bool setupSDCard(void)
{
#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    return SD.begin(SDCARD_CS, SDSPI);
#elif defined(_HAS_SDCARD_)
    return SD.begin(SDCARD_CS);
#endif
    return false;
}


void testSpeaker()
{
#if defined(_HAS_SPEAKER_)
#ifdef _HAS_PWR_CTRL_
    pinMode(SPK_POWER_EN, OUTPUT);
    digitalWrite(SPK_POWER_EN, HIGH);
#endif
    ledcSetup(LEDC_CHANNEL_0, 1000, 8);
    ledcAttachPin(SPERKER_PIN, LEDC_CHANNEL_0);
    int i = 3;
    while (i--) {
        ledcWriteTone(LEDC_CHANNEL_0, 1000);
        delay(200);
        ledcWriteTone(LEDC_CHANNEL_0, 0);
    }
#ifdef _HAS_PWR_CTRL_
    pinMode(SPK_POWER_EN, INPUT);
#endif
    ledcDetachPin(SPERKER_PIN);
#endif
}

void testWiFi()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();

    Serial.println("scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(")");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            delay(10);
        }
    }
    Serial.println("");
}


static void aceButtonHandleEventCb(AceButton *b, uint8_t event, uint8_t state)
{
    Serial.printf("Pin:%d event:%u state:%u\n", b->getPin(), event, state);


#ifdef LILYGO_T5_V102
    if (event != AceButton::kEventReleased && event != AceButton::kEventLongPressed) {
        return;
    }
#else
    if (event != AceButton::kEventClicked && event != AceButton::kEventLongPressed) {
        return;
    }
#endif

    switch (b->getPin()) {
    case BUTTON_1:
        GxepdPage0();
        /*
        #ifdef LILYGO_T5_V102
            event == AceButton::kEventLongPressed ? EnterSleep() : GxepdPage0();
        #else
            event == AceButton::kEventClicked ? GxepdPage0() : EnterSleep();
        #endif*/
        break;
#ifdef BUTTON_2
    case BUTTON_2:
        GxepdPage1();
        break;
#endif
#ifdef BUTTON_3
    case BUTTON_3:
        GxepdPage2();
        break;
#endif
    default:
        break;
    }
}

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");


    /*Turn on power control*/
    pinMode(PWR_ON, OUTPUT);
    digitalWrite(PWR_ON, HIGH);


//
//#if defined(LILYGO_EPD_DISPLAY_102)
//    pinMode(EPD_POWER_ENABLE, OUTPUT);
//    digitalWrite(EPD_POWER_ENABLE, HIGH);
//    delay(50);
//    // strip test
//    strip.begin();
//    strip.show();
//    strip.setBrightness(200);
//    int i = 0;
//    while (i < 5) {
//        uint32_t color[] = {0xFF0000, 0x00FF00, 0x0000FF, 0x000000};
//        strip.setPixelColor(0, color[i]);
//        strip.show();
//        delay(1000);
//        i++;
//    }
//    strip.setPixelColor(0, 0);
//    strip.show();
//#endif /*LILYGO_EPD_DISPLAY_102*/

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

    display.init();
    display.setTextColor(GxEPD_BLACK);


    pinMode(BUTTON_1, INPUT);
    btn1.init(BUTTON_1);
    ButtonConfig *buttonConfig = btn1.getButtonConfig();
    buttonConfig->setEventHandler(aceButtonHandleEventCb);
#ifdef LILYGO_T5_V102
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
#else
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
#endif
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);

#ifdef BUTTON_2
    pinMode(BUTTON_2, INPUT);
    btn2.init(BUTTON_2);
    buttonConfig = btn2.getButtonConfig();
    buttonConfig->setEventHandler(aceButtonHandleEventCb);
#ifdef LILYGO_T5_V102
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
#else
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
#endif
#endif
     testSpeaker();

     testWiFi();

    rlst = setupSDCard();

    GxepdPage0();
}

void loop()
{

    btn1.check();
#ifdef BUTTON_2
    btn2.check();
#endif
#ifdef BUTTON_3
    btn3.check();
#endif

}
void GxepdPage0()
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);

#if defined(_HAS_COLOR_)
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_RED);
#else
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
#endif

#if defined(_HAS_SDCARD_)
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setFont();
#if defined(LILYGO_T5_V102)
    display.setCursor(0, display.height() - 15);
#else
    display.setCursor(20, display.height() - 15);
#endif
    String sizeString = "SD:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    display.println(rlst ? sizeString : "SD:N/A");

    int16_t x1, x2;
    uint16_t w, h;
    String str = GxEPD_BitmapExamplesQ;
    str = str.substring(2, str.lastIndexOf("/"));
    display.getTextBounds(str, 0, 0, &x1, &x2, &w, &h);
    display.setCursor(display.width() - w, display.height() - 15);
    display.println(str);
#endif

    display.update();
}

void showFont(const char name[], const GFXfont *f)
{
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
    display.update();
    delay(5000);
}
void EnterSleep()
{
    Serial.println("EnterSleep");
    delay(2000);
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
    /*Turn on power control*/
}

void drawCornerTest()
{
    display.drawCornerTest();
    delay(5000);
    uint8_t rotation = display.getRotation();
    for (uint16_t r = 0; r < 4; r++) {
        display.setRotation(r);
        display.fillScreen(GxEPD_WHITE);
        display.fillRect(0, 0, 8, 8, GxEPD_BLACK);
        display.fillRect(display.width() - 18, 0, 16, 16, GxEPD_BLACK);
        display.fillRect(display.width() - 25, display.height() - 25, 24, 24, GxEPD_BLACK);
        display.fillRect(0, display.height() - 33, 32, 32, GxEPD_BLACK);
        display.update();
        delay(5000);
    }
    display.setRotation(rotation); // restore
}

void GxepdPage1()
{
    Serial.println("GxepdPage1");
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    delay(1000);/*
    display.fillTriangle(0, display.height() - 1,
                         display.width(), 0,
                         display.width() - 1, display.height() - 1
                         , GxEPD_BLACK);*/

    int16_t x1, y1;
    uint16_t w, h;
    display.drawExampleBitmap(gImage_1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_WHITE);
    String str = "Product By LilyGo";

    delay(1000);
    display.update();

    Serial.println("GxepdPage1 done");

    //EnterSleep();
}



void GxepdPage2()
{
    Serial.println("GxepdPage2");
    int16_t x1, y1;
    uint16_t w, h;
    uint16_t box_w = display.width() - 20, box_h = display.height() / 2 + 20;
    uint16_t box_x = 10, box_y = 10;
    String str = "LilyGo";
    display.setRotation(1);
    display.fillScreen(GxEPD_WHITE);

    uint16_t  offset_y1 = 0,
              offset_y2 = 0,
              offset_y3 = 0,
              offset_y4 = 0;
#if   defined(_GxGDEW0213M21_H_) || defined(_GxGDEW0213Z16_H_)
    const GFXfont *font1 = &prazo_Regular_29pt7b;
    const GFXfont *font2 = &prazo_Regular_212pt7b;
    offset_y1 = 20;
    offset_y2 = 40;
    offset_y3 = 60;
    offset_y4 = 25;

#elif defined(_GxGDGDEW0102T4_H_)
    const GFXfont *font1 = &prazo_Regular_27pt7b;
    const GFXfont *font2 = &prazo_Regular_29pt7b;
    offset_y1 = 15;
    offset_y2 = 30;
    offset_y3 = 45;
    offset_y4 = 20;
#elif defined(_GxDEPG0290B_H_) || defined(_GxGDEM0213B74_H_) || defined(_GxDEPG0290R_H_) || defined(_GxGDEW029Z10_H_) ||defined(_GxGDEH029A1_H_) || defined(_GxDEPG0213BN_H_) || defined(_GxGDEH0213B73_H_) || defined(_GxGDEH0213B72_H_) || defined(_GxGDE0213B1_H_) || defined(_GxDEPG0266BN_H_)
    const GFXfont *font1 = &prazo_Regular_212pt7b;
    const GFXfont *font2 = &prazo_Regular_218pt7b;
    offset_y1 = 25;
    offset_y2 = 50;
    offset_y3 = 75;
    offset_y4 = 30;
#elif defined(_GxGDEW027W3_H_) || defined(_GxGDEW027C44_H_) || defined(_GxDEPG0750BN_H_)
    const GFXfont *font1 = &prazo_Regular_216pt7b;
    const GFXfont *font2 = &prazo_Regular_224pt7b;
    offset_y1 = 42;
    offset_y2 = 75;
    offset_y3 = 110;
    offset_y4 = 45;
#elif defined(_GxDEPG0150BN_H_)
    const GFXfont *font1 = &prazo_Regular_216pt7b;
    const GFXfont *font2 = &prazo_Regular_224pt7b;
    offset_y1 = 42;
    offset_y2 = 80;
    offset_y3 = 120;
    offset_y4 = 50;

#endif
    display.setTextColor(GxEPD_BLACK);
    display.setFont(font1);
    display.setCursor(5, offset_y1);
    display.print(GxEPD_ProductID"E-Paper");
    display.setCursor(5, offset_y2);
    display.print(display.width());
    display.print("x");
    display.print(display.height());


    display.setTextColor(GxEPD_BLACK);
    display.setFont(font1);
    str = GxEPD_BitmapExamplesQ;
    str = str.substring(2, str.lastIndexOf("/"));
    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(5, offset_y3);
    display.println(str);

    str = "LilyGo";
    box_x = 0;
    box_w = display.width();
    box_y = display.height() - display.height() / 3;
    box_h = box_y;
    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
    display.setTextColor(
#ifdef _HAS_COLOR_
        GxEPD_RED
#else
        GxEPD_WHITE
#endif
    );
    display.setFont(font2);
    display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((box_w / 2) - (w / 2 ), box_y  + offset_y4);
    display.print(str);
    display.update();
    display.setFont();
    Serial.println("GxepdPage2 done");
}
