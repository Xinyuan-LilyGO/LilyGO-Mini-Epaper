/*
===============================================================================================================
QMC5883LCompass.h Library Azimuth Example Sketch
Learn more at [https://github.com/mprograms/QMC5883Compas]
===============================================================================================================
v0.3 - June 12, 2019
Written by MRPrograms
Github: [https://github.com/mprograms/]

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
    int a;

    // Read compass values
    compass.read();

    // Return Azimuth reading
    a = compass.getAzimuth();

    Serial.print("A: ");
    Serial.print(a);
    Serial.println();

    delay(250);
}
