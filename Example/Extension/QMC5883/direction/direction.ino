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

	const char bearings[12][3] =  {
		{' ', ' ', 'N'},
		{'N', 'N', 'E'},
		{' ', 'N', 'E'},
		{' ', ' ', 'E'},
		{'E', 'S', 'E'},
		{' ', 'S', 'E'},
		{' ', ' ', 'S'},
		{'S', 'S', 'W'},
		{' ', 'S', 'W'},
		{' ', ' ', 'W'},
		{'W', 'N', 'W'},
		{' ', 'N', 'W'},
	};

void setup() {
  Serial.begin(115200);
  Wire.begin(IIC_SDA, IIC_SCL);
  compass.init();
  compass.setCalibration(-268, 1680, 0, 2005, 0, 2052);
}

void loop() {


  compass.read();
  
  byte a = compass.getAzimuth();
  byte b = compass.getBearing(a);

  Serial.print(bearings[b][0]);
  Serial.print(bearings[b][1]);
  Serial.print(bearings[b][2]);
  Serial.println();
  
  delay(250);
}
