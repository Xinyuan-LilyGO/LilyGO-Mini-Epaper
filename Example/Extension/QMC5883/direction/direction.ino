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
#define IIC_SDA 26
#define IIC_SCL 25
#include <QMC5883LCompass.h>

QMC5883LCompass compass;

void setup() {
  Serial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);
  compass.init();
  compass.setCalibration(-1461, 218, -1240, 885, 0, 1468);
}

void loop() {
  compass.read();
  
  byte a = compass.getAzimuth();

  char myArray[3];
  compass.getDirection(myArray, a);
  
  Serial.print(myArray[0]);
  Serial.print(myArray[1]);
  Serial.print(myArray[2]);
  Serial.println();
  
  delay(250);
}
