/*
===============================================================================================================
QMC5883LCompass.h Library XYZ Example Sketch
Learn more at [https://github.com/mprograms/QMC5883LCompass]

This example shows how to get the XYZ values from the sensor.

===============================================================================================================
Release under the GNU General Public License v3
[https://www.gnu.org/licenses/gpl-3.0.en.html]
===============================================================================================================
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

#define E_PAPER_DISPLAY   //Displayed in the e - paper
#include <QMC5883LCompass.h>
QMC5883LCompass compass;

#if defined(E_PAPER_DISPLAY)
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

#else
#endif /*E_PAPER_DISPLAY*/
uint32_t last = 0;
uint32_t  complete_refresh = 0;

void setup()
{
    Serial.begin(115200);
    Wire.begin(IIC_SDA, IIC_SCL);
    compass.init();

#if defined(E_PAPER_DISPLAY)
    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);
    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.update();
    delay(1000);
#else
#endif /*E_PAPER_DISPLAY*/
    Serial.println("setup done");
}

void loop()
{

    if (millis() - last > 3000) {
        int x, y, z;
        // Read compass values
        compass.read();

        // Return XYZ readings
        x = compass.getX();
        y = compass.getY();
        z = compass.getZ();

        Serial.print("X: ");
        Serial.print(x);
        Serial.print(" Y: ");
        Serial.print(y);
        Serial.print(" Z: ");
        Serial.print(z);
        Serial.println();

#if defined(E_PAPER_DISPLAY)
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
        if (complete_refresh > 10) { //Use full brush after 100 rounds
            display.update();
            complete_refresh = 0;
        }
        last = millis();
#else


#endif
    }
}
