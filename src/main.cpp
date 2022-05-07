/*
Weather_clock Test example

You need to fill in the information correctly in the code.
const char *ssid       = "********";
const char *password   = "********";
String openWeatherMapApiKey = "************************";
String city = "****";
String countryCode = "**";

 This example code is in the public domain.
 */


#define TIME_ZONE  8


#define LILYGO_T5_V102
#include <FunctionalInterrupt.h>
#include "battery_index.h"
#include <boards.h>
#include <GxEPD.h>
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w

#include GxEPD_BitmapExamples
#include <U8g2_for_Adafruit_GFX.h>
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include "Arduino.h"
#include <stdio.h>
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "time.h"
#include "pcf8563.h"
#include "bma423.h"
#include <QMC5883LCompass.h>


const char *ssid = "REPLACE_WITH_YOUR_SSID";
const char *password = "REPLACE_WITH_YOUR_PASSWORD";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "REPLACE_WITH_YOUR_OPEN_WEATHER_MAP_API_KEY";

// Replace with your country code and city
String city = "Shenzhen";
String countryCode = "CN";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;
uint32_t    last = 0;
uint32_t    update_flag = 0;
uint32_t    wifi_flag = 0;

const char *ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600 * (TIME_ZONE - 4);
const int   daylightOffset_sec = 3600 * (TIME_ZONE - 4);

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

struct tm timeinfo;
String current_weather;

PCF8563_Class rtc;

QMC5883LCompass compass;
int calibrationData[3][2];
bool changed = false;
char myArray[3];
uint32_t   QMC_last = 0;


struct bma4_dev bma;
struct bma4_accel sens_data;
struct bma4_accel_config accel_conf;
int8_t rslt;

/* Variable to get the step counter output */
uint32_t step_out = 0;

/* Variable to get the interrupt status  */
uint16_t int_status = 0;

bool BMA_IRQ = false;
uint16_t counter_IRQ = 0;

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);
void LilyGo_logo();
void BMA4_IIC_Configuration();
void BMA4_Accel_Configuration();
void BMA4_INT_Configuration();
void BMA_setup();
void QMC_setup();
void QMC_main();

void LilyGo_logo();
bool printLocalTime();
void display_time();
void display_Battery();
void get_weather();
String httpGETRequest(const char *serverName);
void display_weather();
void display_wifi();
void EnterSleep();


bool button3_flag = 0;
bool button1_flag = 0;
bool button2_flag = 0;

class Button
{
public:

    Button(uint8_t reqPin) : PIN(reqPin)
    {
        pinMode(PIN, INPUT_PULLUP);
        attachInterrupt(PIN, std::bind(&Button::isr, this), FALLING);
    };
    ~Button()
    {
        detachInterrupt(PIN);
    }

    void IRAM_ATTR isr()
    {
        numberKeyPresses += 1;
        pressed = true;
    }


    void checkPressed()
    {
        if (pressed) {
            delay(300);//flutter-free
            if (PIN == BUTTON_3) button3_flag = 1;
            else if (PIN == BUTTON_1) button1_flag = 1;
            else if (PIN == BUTTON_2) button2_flag = 1;
            pressed = false;
        }
    }

private:
    const uint8_t PIN;
    volatile uint32_t numberKeyPresses;
    volatile bool pressed;
};

Button button3(BUTTON_3);

void button3_callback(void);
void button1_callback(void);
void button2_callback(void);
bool check_button(uint8_t pin);

void setup(void)
{

    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial
    u8g2Fonts.begin(display);
    display.setTextColor(GxEPD_BLACK);
    u8g2Fonts.setFontMode(1);                           // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(1);                      // left to right (this is default)
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);          // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);          // apply Adafruit GFX color


    Wire.begin(IIC_SDA, IIC_SCL);
    rtc.begin();
    BMA_setup();
    QMC_setup();


    //connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" CONNECTED");

    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


    while (!printLocalTime()); //get time


    get_weather();

    //disconnect WiFi as it's no longer needed
    //  WiFi.disconnect(true);
    // WiFi.mode(WIFI_OFF);
    WiFi.disconnect();


    pinMode(POWER_ENABLE, OUTPUT);
    digitalWrite(POWER_ENABLE, HIGH);

    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);

    LilyGo_logo();
    display_time();
    display_Battery();
    display_weather();
    display.update();
    Serial.println("setup done");

}

void loop()
{

    if (millis() - last > 10000) {
        display_time();
        display_Battery();
        display_weather();
        display_wifi();
        if (update_flag == 6) {
            display.update();
            update_flag = 0;
        }
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);//display.update();////display.update();
        update_flag++;
        last = millis();
    }

    if (millis() - QMC_last > 1000) {
        QMC_main();
        display.fillRect(105, 55, 10, 30, GxEPD_WHITE);
        u8g2Fonts.setFont(u8g2_font_timB08_tr);
        u8g2Fonts.setCursor(105, 55);
        u8g2Fonts.print(myArray[0]);
        u8g2Fonts.print(myArray[1]);
        u8g2Fonts.print(myArray[2]);
        display.updateWindow(105, 55, 10, 30, true);
        QMC_last = millis();
    }

    if (BMA_IRQ) {
        BMA_IRQ = false;

        /* Read the interrupt register to check whether step counter interrupt is received. */
        rslt = bma423_read_int_status(&int_status, &bma);
        if (rslt != BMA4_OK) {
            Serial.print("bma423_read_int_status FALL");
            Serial.println(rslt);
        }


        /*    Check if step counter interrupt is triggered */
        if (int_status & BMA423_STEP_CNTR_INT) {
            Serial.println("Step counter interrupt received");

            /* On interrupt, Get step counter output */
            rslt = bma423_step_counter_output(&step_out, &bma);
            if (rslt != BMA4_OK) {
                Serial.print(" bma423_step_counter_output FALL");
                Serial.println(rslt);
            }
            counter_IRQ++;
            Serial.print("step counter out"); Serial.println(step_out);//输出步数
            //display.println(step_out);
            // display.updateWindow(box_x, box_y, box_w, box_h, true);

        }

    }

    button3.checkPressed();

    if (button3_flag == 1)button3_callback();
    else if (check_button(BUTTON_1))button1_callback();
    else if (check_button(BUTTON_2))button2_callback();
    //Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_HM));
    //Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD));

}


bool check_button(uint8_t pin)
{
    if (digitalRead(pin) == 0)   {
        delay(200);
        //Wait for release
        while (!digitalRead(pin));
        return true;
    } else return false;

}

//button3 callback function
void button3_callback(void)
{
    if (wifi_flag == 0) {

        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_2x_t); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("...");
        Serial.printf("Connecting to %s ", ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println(" CONNECTED");

        display.setRotation(3);
        u8g2Fonts.setFont(u8g2_font_helvR10_te); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("P");
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
        //init and get the time
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        printLocalTime();

        //get weather date
        get_weather();
        wifi_flag = 1;
    } else {
        //disconnect WiFi as it's no longer needed
        WiFi.disconnect();
        u8g2Fonts.setFont(u8g2_font_open_iconic_other_2x_t); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("F");
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
        wifi_flag = 0;
    }

    button3_flag = 0;//Clear flag bit
}

//button1 callback function
void button1_callback(void)
{
    Serial.print("button1_callback");

    /*
        display.setRotation(3);
        display.fillScreen(GxEPD_WHITE);
        u8g2Fonts.setFont(u8g2_font_helvR10_te  );
        u8g2Fonts.setCursor(10, 0);    // start writing at this position
        u8g2Fonts.print("EnterSleep");
        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_8x_t); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(30, 10);    // start writing at this position  u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("N");
        display.update();
        u8g2Fonts.print("button1_callback");



        button1_flag = 0;//Clear flag bit
        EnterSleep();*/
}

//button2 callback function
void button2_callback(void)
{
    Serial.print("button2_callback");
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(30, 0);    // start writing at this position
    u8g2Fonts.print("button2_callback");
    display.update();

    button2_flag = 0;//Clear flag bit
}

void display_wifi()
{
    if (wifi_flag == 1) {//show  wifi icon
        display.setRotation(3);
        u8g2Fonts.setFont(u8g2_font_open_iconic_embedded_2x_t); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("P");
    } else { //show  wifi icon
        display.setRotation(3);
        u8g2Fonts.setFont(u8g2_font_open_iconic_other_2x_t); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        u8g2Fonts.setCursor(110, 5);
        u8g2Fonts.print("F");

    }

}

void display_Battery()
{

    int sensorValue = analogRead(35);
    float voltage = sensorValue * (3.3 / 4096);
    delay(100);
    voltage = (voltage * 2) - 3.0;
    int ADC_Int = (int)(voltage / 0.083);

    if (ADC_Int > 12) ADC_Int = 12;
    else if (ADC_Int < 0) ADC_Int = 0;
    // print out the value you read:
    //Serial.println(sensorValue);
    //Serial.println(voltage);
    display.setRotation(0);
    display.drawExampleBitmap(BMP[ADC_Int], 65, 0, 16, 8, GxEPD_BLACK);



}


/*
 __          ________       _______ _    _ ______ _____
 \ \        / /  ____|   /\|__   __| |  | |  ____|  __ \
  \ \  /\  / /| |__     /  \  | |  | |__| | |__  | |__) |
   \ \/  \/ / |  __|   / /\ \ | |  |  __  |  __| |  _  /
    \  /\  /  | |____ / ____ \| |  | |  | | |____| | \ \
     \/  \/   |______/_/    \_\_|  |_|  |_|______|_|  \_\
*/

void display_weather()
{
    get_weather();
    display.setRotation(3);
    u8g2Fonts.setFont(u8g2_font_open_iconic_weather_2x_t ); //  weather iconic, select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    //display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(10, 5);
    if   (current_weather == "Clear")  u8g2Fonts.print("E");//clear
    else if (current_weather == "Clouds")u8g2Fonts.print("A");//Clouds
    else if (current_weather == "Rain")  u8g2Fonts.print("C"); //Rain
    u8g2Fonts.setFont(u8g2_font_helvR10_te  );
    u8g2Fonts.setCursor(10, 25);
    u8g2Fonts.print(current_weather);
}

void get_weather()
{
    if (WiFi.status() == WL_CONNECTED) {
        String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

        jsonBuffer = httpGETRequest(serverPath.c_str());
        Serial.println(jsonBuffer);
        JSONVar myObject = JSON.parse(jsonBuffer);

        while (JSON.typeof(myObject) == "undefined");
        // JSON.typeof(jsonVar) can be used to get the type of the var
        if (JSON.typeof(myObject) == "undefined") {
            Serial.println("Parsing input failed!");
            return;
        }

        Serial.print("JSON object = ");
        Serial.println(myObject);
        /*Serial.print("Temperature: ");
        Serial.println(myObject["main"]["temp"]);
        Serial.print("Pressure: ");
        Serial.println(myObject["main"]["pressure"]);
        Serial.print("Humidity: ");
        Serial.println(myObject["main"]["humidity"]);
        Serial.print("Wind Speed: ");
        Serial.println(myObject["wind"]["speed"]);
        Serial.print("weather main: ");
        Serial.println(myObject["weather"][0]["main"]);
        Serial.print("weather icon: ");
        Serial.println(myObject["weather"][0]["icon"]);
        */
        current_weather = myObject["weather"][0]["main"];

    } else {
        return;
        //Serial.println("WiFi Disconnected");
    }



}

String httpGETRequest(const char *serverName)
{
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin(serverName);

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();

    return payload;
}


/*
  _______ _____ __  __ ______
 |__   __|_   _|  \/  |  ____|
    | |    | | | \  / | |__
    | |    | | | |\/| |  __|
    | |   _| |_| |  | | |____
    |_|  |_____|_|  |_|______|
 */
void display_time()
{
    //printLocalTime();
    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_HM));
    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD));

    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_fub20_tf );
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setCursor(80, 5);
    // u8g2Fonts.println(&timeinfo, "%H:%M");
    u8g2Fonts.println(rtc.formatDateTime(PCF_TIMEFORMAT_HM));

    u8g2Fonts.setCursor(50 + 10, 10);
    u8g2Fonts.setFont(u8g2_font_timB12_tr );
    u8g2Fonts.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD));
    /*
    u8g2Fonts.print(timeinfo.tm_year + 1900);
    u8g2Fonts.print("/");
    u8g2Fonts.print(timeinfo.tm_mon + 1);
    u8g2Fonts.print("/");
    u8g2Fonts.println(timeinfo.tm_mday);*/

    u8g2Fonts.setCursor(35 + 10, 30);
    u8g2Fonts.setFont(u8g2_font_helvR10_te);
    u8g2Fonts.println(&timeinfo, "%a");

    display.setRotation(0);
    display.drawExampleBitmap(Walk_icon, 20, 85, 16, 18, GxEPD_BLACK);

    //Output the number
    display.setRotation(3);
    u8g2Fonts.setCursor(28, 35);
    u8g2Fonts.println(step_out);

    // display "↑"
    u8g2Fonts.setFont(u8g2_font_cu12_t_symbols);
    u8g2Fonts.drawGlyph(103, 48, 8593);

}

bool printLocalTime()
{
    int restart_flag = 0;
    int last = millis();
    int while_flag = 0;

    while (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        //WIFI iconic
        display.setRotation(3);
        u8g2Fonts.setFont(u8g2_font_helvR10_te); // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
        display.fillScreen(GxEPD_WHITE);
        u8g2Fonts.setCursor(90, 5);
        u8g2Fonts.print("updating...");
        display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);
        //return false;
        if (millis() - last > 10000) {
            ESP.restart();
        }
    }

    rtc.getDayOfWeek(timeinfo.tm_mday,  timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    rtc.setDateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    return true;


}


//QMC
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

void QMC_setup()
{

    compass.init();
    compass.setCalibration(-268, 1680, 0, 2005, 0, 2052);
}

void QMC_main()
{

    compass.read();

    // Return XYZ readings
    int x = compass.getX();
    int y = compass.getY();
    int z = compass.getZ();

    Serial.print("X: ");
    Serial.print(x);
    Serial.print(" Y: ");
    Serial.print(y);
    Serial.print(" Z: ");
    Serial.print(z);
    Serial.println();

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


    myArray[0] = bearings[b][0];
    myArray[1] = bearings[b][1];
    myArray[2] = bearings[b][2];
    /*
      Serial.print(myArray[0]);
      Serial.print(myArray[1]);
      Serial.print(myArray[2]);
      Serial.println();
    */


}

//BMP
void bma_irq()
{
    // Set interrupt to set irq value to true
    BMA_IRQ = true;
}

void BMA_setup()
{
    pinMode(BMP_INT1, INPUT_PULLUP);
    attachInterrupt(BMP_INT1, bma_irq, RISING); //Select the interrupt mode according to the actual circuit



    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial



    BMA4_IIC_Configuration();
    BMA4_Accel_Configuration();
    BMA4_INT_Configuration();

    rslt = bma423_step_detector_enable(BMA4_ENABLE, &bma);
    Serial.print("bma423_step_detector_enable"); Serial.println(rslt);

    /* Enable step counter */
    rslt = bma423_feature_enable(BMA423_STEP_CNTR, 1, &bma);
    if (rslt != BMA4_OK) {
        Serial.print(" bma423_feature_enable FALL");
        Serial.println(rslt);
    }


    /* Set water-mark level 1 to get interrupt after 20 steps.
     * Range of step counter interrupt is 0 to 20460(resolution of 20 steps).
     */
    rslt = bma423_step_counter_set_watermark(1, &bma);
    if (rslt != BMA4_OK) {
        Serial.print("bma423_step_counter FALL");    //bma4_error_codes_print_result("bma423_step_counter status", rslt);
        Serial.println(rslt);
    }

    rslt = bma4_set_advance_power_save(BMA4_ENABLE, &bma);
    if (rslt != BMA4_OK) {
        Serial.print(" power save FALL");
        Serial.println(rslt);
    }

    Serial.println("Move/perform the walk/step action with the sensor\n");

}


// Low-level I2C Communication
// Provided to BMA423_Library communication interface
//
uint16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)address, (uint8_t)len);
    uint8_t i = 0;
    while (Wire.available()) {
        data[i++] = Wire.read();
    }
    return 0; //Pass
}

uint16_t writeRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len)
{
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data, len);
    return (0 !=  Wire.endTransmission());
}


void BMA4_IIC_Configuration()
{
    bma.chip_id         = BMA423_CHIP_ID;
    bma.dev_addr        = BMA4_I2C_ADDR_PRIMARY;
    bma.interface       = BMA4_I2C_INTERFACE;
    bma.bus_read        = readRegister;
    bma.bus_write       = writeRegister;
    bma.delay           = delay;
    bma.read_write_len  = 8;
    bma.resolution      = 12;
    bma.feature_len     = BMA423_FEATURE_SIZE;

    bma4_set_command_register(0xB6, &bma);
    delay(20);

    /* Sensor initialization */
    rslt = bma423_init(&bma);
    Serial.print("bma423_init"); Serial.println(rslt);


    /* Upload the configuration file to enable the features of the sensor. */
    rslt = bma423_write_config_file(&bma);
    Serial.print("bma423_write_config"); Serial.println(rslt);

}

void BMA4_Accel_Configuration()
{
    accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    accel_conf.range = BMA4_ACCEL_RANGE_2G;
    accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

    /* Set the accel configurations */
    rslt = bma4_set_accel_config(&accel_conf, &bma);
    Serial.print("bma4_set_accel_config"); Serial.println(rslt);

    /* Enable the accelerometer */
    rslt = bma4_set_accel_enable(1, &bma);
    Serial.print("bma4_set_accel_enable"); Serial.println(rslt);
}

void BMA4_INT_Configuration()
{
    struct bma4_int_pin_config intPinCofig;
    intPinCofig.edge_ctrl = BMA4_LEVEL_TRIGGER;
    intPinCofig.lvl = BMA4_ACTIVE_HIGH;
    intPinCofig.input_en = BMA4_INPUT_DISABLE;
    intPinCofig.od = BMA4_PUSH_PULL;//BMA4_OPEN_DRAIN;
    intPinCofig.output_en = BMA4_OUTPUT_ENABLE;
    /* Set the electrical behaviour of interrupt pin1 */
    bma4_set_int_pin_config(&intPinCofig, BMA4_INTR1_MAP, &bma);


    rslt = bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_STEP_CNTR_INT, 1, &bma);
    Serial.print("bma423_map_interrupt"); Serial.println(rslt);


}


void EnterSleep()
{
    Serial.println("EnterSleep");
    delay(2000);
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_1)), ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
    /*Turn on power control*/
}

void LilyGo_logo(void)
{
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
    display.fillScreen(GxEPD_WHITE);

}


