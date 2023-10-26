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
#include <SD.h>
#include <FS.h>

#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w

#include GxEPD_BitmapExamples

// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
float cardSize;

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(HSPI);
#endif

#if defined(_GxGDEW0213Z16_H_) || defined(_GxGDEW029Z10_H_) || defined(_GxGDEW027C44_H_) ||defined(_GxGDEW0154Z17_H_) || defined(_GxGDEW0154Z04_H_) || defined(_GxDEPG0290R_H_)
#define _HAS_COLOR_
#endif

void showFont(const char name[], const GFXfont *f);
void drawCornerTest(void);

#ifdef BUTTONS
const uint8_t btns[] = BUTTONS;
#endif


bool setupSDCard(void)
{
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);
    if (!SD.begin(SDCARD_CS, SDSPI)) {
        Serial.println("setupSDCard FAIL");
        return false;
    } else {
        cardSize = SD.cardSize() / (1024 * 1024 * 1024.0);
        Serial.print("setupSDCard PASS . SIZE = ");
        Serial.print(cardSize);
        Serial.println(" G");
        return true;
    }
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

void setup()
{
    bool rlst = false;
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");


#ifdef BUTTONS
    for (int i = 0; i < sizeof(btns) / sizeof(btns[0]); ++i) {
        pinMode(btns[i], INPUT_PULLUP);
    }
#endif

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);

    display.init();
    display.setTextColor(GxEPD_BLACK);

    rlst = setupSDCard();

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
#if defined(_HAS_COLOR_)
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_RED);
#else
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
#endif

#if defined(_HAS_SDCARD_)
    display.setRotation(1);
#if defined(LILYGO_MINI_EPAPER_ESP32) || defined(LILYGO_MINI_EPAPER_ESP32S3)
    display.setCursor(5, display.height() - 15);
#else
    display.setCursor(20, display.height() - 15);
#endif
    String sizeString = "SD:" + String(cardSize) + "G";
    display.println(rlst ? sizeString : "SD:N/A");

    int16_t x1, x2;
    uint16_t w, h;
    String str = GxEPD_BitmapExamplesQ;
    str = str.substring(2, str.lastIndexOf("/"));

    display.getTextBounds(str, 0, 0, &x1, &x2, &w, &h);

    display.setCursor(display.width() - w - 5, display.height() - 15);
    Serial.println(w);
    Serial.println(h);
    display.println(str.c_str());
#endif
    display.update();

    delay(3000);


    testSpeaker();

    testWiFi();

}

void loop()
{
    drawCornerTest();

    int i = 0;
    while (i < 4) {
        display.setRotation(i);
        showFont("FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
        // showFont("FreeMonoBold18pt7b", &FreeMonoBold18pt7b);
        // showFont("FreeMonoBold24pt7b", &FreeMonoBold24pt7b);
        i++;
    }


    display.fillScreen(GxEPD_WHITE);

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
#if defined(_HAS_COLOR_)
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_RED);
#else
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
#endif

    display.update();

    display.powerDown();

    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);

    esp_deep_sleep_start();

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
