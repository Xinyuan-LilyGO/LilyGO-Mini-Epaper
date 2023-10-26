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
#include <U8g2_for_Adafruit_GFX.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>


GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

void LilyGo_logo(void);

void setup(void)
{
    Serial.begin(115200);
    Serial.println();

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
}


void loop()
{
    u8g2Fonts.setFont(u8g2_font_helvR14_tf);            // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(30, 15);    // start writing at this position
    u8g2Fonts.print("Hello World!");
    display.update();
    delay(10000);
}

void LilyGo_logo(void)
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}

