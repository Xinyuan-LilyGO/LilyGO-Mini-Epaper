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


#define LILYGO_T5_V102
#include <FunctionalInterrupt.h>
#include "battery_index.h"


#include <boards.h>
#include <GxEPD.h>

#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w


#include GxEPD_BitmapExamples
#include <U8g2_for_Adafruit_GFX.h>
// FreeFonts from Adafruit_GFX

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMonoOblique12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBoldOblique12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBoldItalic12pt7b.h>
#include <Fonts/FreeSerifItalic12pt7b.h>


#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>



#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "time.h"
#include "pcf8563.h"
PCF8563_Class rtc;
#define IIC_SDA 26
#define IIC_SCL 25
#define   TIME_ZONE  8

//const char* ssid = "REPLACE_WITH_YOUR_SSID";
//const char* password = "REPLACE_WITH_YOUR_PASSWORD";
const char *ssid       = "********";
const char *password   = "********";


// Your Domain name with URL path or IP address with path
//String openWeatherMapApiKey = "REPLACE_WITH_YOUR_OPEN_WEATHER_MAP_API_KEY";
// Example:
String openWeatherMapApiKey = "************************";

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
bool check_button1(uint8_t pin);

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

    button3.checkPressed();

    if (button3_flag == 1)button3_callback();
    else if (check_button1(BUTTON_1))button1_callback();
    else if (check_button1(BUTTON_2))button2_callback();
    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_HM));
    Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD));

}

bool check_button1(uint8_t pin)
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
    EnterSleep();
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

    u8g2Fonts.setCursor(50, 10);
    u8g2Fonts.setFont(u8g2_font_timB12_tr );
    u8g2Fonts.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD));
    /*
    u8g2Fonts.print(timeinfo.tm_year + 1900);
    u8g2Fonts.print("/");
    u8g2Fonts.print(timeinfo.tm_mon + 1);
    u8g2Fonts.print("/");
    u8g2Fonts.println(timeinfo.tm_mday);*/


    u8g2Fonts.setCursor(35, 30);
    u8g2Fonts.setFont(u8g2_font_helvR10_te);
    u8g2Fonts.println(&timeinfo, "%a");


}

void display_Battery()
{

    int sensorValue = analogRead(35);
    float voltage = sensorValue * (3.3 / 4096);
    Serial.println(sensorValue);
    Serial.println(voltage);
    delay(100);
    voltage = (voltage*2) - 3.0;
    int ADC_Int = (int)(voltage / 0.083);

    if (ADC_Int > 12) ADC_Int = 12;
    else if (ADC_Int < 0) ADC_Int = 0;
    // print out the value you read:
    Serial.println(sensorValue);
    Serial.println(voltage);
    display.setRotation(0);
    display.drawExampleBitmap(BMP[ADC_Int], 65, 0, 16, 8, GxEPD_BLACK);



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
