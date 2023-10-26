/*
===============================================================================================================
QMC5883LCompass.h Library Bearing Example Sketch
Learn more at [https://github.com/mprograms/QMC5883Compas]

This example shows how to get the range that the current bearing is in. You can use this to roll
your very own direction output.

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
#include <boards.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

void setup()
{
    Serial.begin(115200);
    Wire.begin(IIC_SDA, IIC_SCL);
    compass.init();
}

void loop()
{
    compass.read();

    byte a = compass.getAzimuth();
    // Output here will be a value from 0 - 15 based on the direction of the bearing / azimuth.
    byte b = compass.getBearing(a);

    Serial.print("B: ");
    Serial.print(b);
    Serial.println();

    delay(250);
}
