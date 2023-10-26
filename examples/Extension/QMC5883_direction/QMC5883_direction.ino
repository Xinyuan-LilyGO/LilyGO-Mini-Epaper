/*
===============================================================================================================
QMC5883LCompass.h Library Direction Example Sketch
Learn more at [https://github.com/mprograms/QMC5883Compas]

This example shows how to get Compass direction.

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
int calibrationData[3][2];
bool changed = false;

const char bearings[16][3] =  {
    {' ', ' ', 'W'},
    {'W', 'S', 'W'},
    {' ', 'S', 'W'},
    {'S', 'S', 'W'},
    {' ', ' ', 'S'},
    {'S', 'S', 'E'},
    {' ', 'S', 'E'},
    {'E', 'S', 'E'},
    {' ', ' ', 'E'},
    {'E', 'N', 'E'},
    {' ', 'N', 'E'},
    {'N', 'N', 'E'},
    {' ', ' ', 'N'},
    {'N', 'N', 'W'},
    {' ', 'N', 'W'},
    {'W', 'N', 'W'},
};



void setup()
{
    Serial.begin(115200);
    Wire.begin(IIC_SDA, IIC_SCL);
    compass.init();
    //compass.setCalibration(-268, 1680, 0, 2005, 0, 2052);
}

void loop()
{


    compass.read();

    int x, y, z;
    // Return XYZ readings
    x = compass.getX();
    y = compass.getY();
    z = compass.getZ();

    changed = false;

    if (x < calibrationData[0][0]) {
        calibrationData[0][0] = x;
        changed = true;
    }
    if (x > calibrationData[0][1]) {
        calibrationData[0][1] = x;
        changed = true;
    }

    if (y < calibrationData[1][0]) {
        calibrationData[1][0] = y;
        changed = true;
    }
    if (y > calibrationData[1][1]) {
        calibrationData[1][1] = y;
        changed = true;
    }

    if (z < calibrationData[2][0]) {
        calibrationData[2][0] = z;
        changed = true;
    }
    if (z > calibrationData[2][1]) {
        calibrationData[2][1] = z;
        changed = true;
    }

    compass.setCalibration(calibrationData[0][0], calibrationData[0][1], calibrationData[1][0], calibrationData[1][1], calibrationData[2][0], calibrationData[2][1]);



    int a = compass.getAzimuth();
    int b = compass.getBearing(a);



    char myArray[3];

    myArray[0] = bearings[b][0];
    myArray[1] = bearings[b][1];
    myArray[2] = bearings[b][2];

    Serial.print(myArray[0]);
    Serial.print(myArray[1]);
    Serial.print(myArray[2]);
    Serial.println();

    delay(250);
}
