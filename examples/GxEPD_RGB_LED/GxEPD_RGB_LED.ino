// According to the board, cancel the corresponding macro definition
// https://www.lilygo.cc/products/mini-e-paper-core , esp32picod4
// #define LILYGO_MINI_EPAPER_ESP32
// esp32s3-fn4r2
// #define LILYGO_MINI_EPAPER_ESP32S3

#if !defined(LILYGO_MINI_EPAPER_ESP32S3)  && !defined(LILYGO_MINI_EPAPER_ESP32)
// 请在草图上方选择对应的目标板子名称,将它取消注释.
#error "Please select the corresponding target board name above the sketch and uncomment it."
#endif

#include <FunctionalInterrupt.h>
#include <SD.h>
#include <FS.h>
#define _HAS_SDCARD_


bool rlst = false;
// Set up the rgb led names
uint8_t ledR = 25;
uint8_t ledG = 32;
uint8_t ledB = 33;

uint8_t ledArray[3] = {1, 2, 3}; // three led channels

const bool invert = true; // set true if common anode, false if common cathode

uint8_t color = 0;          // a value from 0 to 255 representing the hue
uint32_t R, G, B;           // the Red Green and Blue color components
uint8_t brightness = 255;  // 255 is maximum brightness, but can be changed.  Might need 256 for common anode to fully turn off.




#include <Arduino.h>
#include <boards.h>
#include <GxEPD.h>
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include GxEPD_BitmapExamples
#include <U8g2_for_Adafruit_GFX.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <WiFi.h>

GxIO_Class io(SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
SPIClass SDSPI(HSPI);
#endif



bool button0_flag = 0;
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
            if (PIN == BUTTON_3) button0_flag = 1;
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

Button button0(BUTTON_3);
Button button1(BUTTON_1);
Button button2(BUTTON_2);


bool start = 0;//led start
void LilyGo_logo(void);
void button0_callback(void);
void button1_callback(void);
void button2_callback(void);
void hueToRGB(uint8_t hue, uint8_t brightness);

bool setupSDCard(void)
{
#if defined(_HAS_SDCARD_) && !defined(_USE_SHARED_SPI_BUS_)
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    return SD.begin(SDCARD_CS, SDSPI);
#elif defined(_HAS_SDCARD_)
    return SD.begin(SDCARD_CS);
#endif
    return false;
}
void setup(void)
{
    Serial.begin(115200);


    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);


    ledcAttachPin(ledR, 1); // assign RGB led pins to channels
    ledcAttachPin(ledG, 2);
    ledcAttachPin(ledB, 3);

    // Initialize channels
    // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
    // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
    ledcSetup(1, 12000, 8); // 12 kHz PWM, 8-bit resolution
    ledcSetup(2, 12000, 8);
    ledcSetup(3, 12000, 8);

    rlst = setupSDCard();

    SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial
    u8g2Fonts.begin(display);
    display.setTextColor(GxEPD_BLACK);
    u8g2Fonts.setFontMode(1);                           // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(1);                      // left to right (this is default)
    u8g2Fonts.setForegroundColor(GxEPD_BLACK);          // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE);          // apply Adafruit GFX color
    u8g2Fonts.setFont(u8g2_font_helvR12_tf);            // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
    LilyGo_logo();
    Serial.println("setup done");
}


void loop()
{
    button0.checkPressed();
    button1.checkPressed();
    button2.checkPressed();


    if (button0_flag == 1)button0_callback();
    else if (button1_flag == 1)button1_callback();
    else if (button2_flag == 1)button2_callback();

}



void hueToRGB(uint8_t hue, uint8_t brightness)
{
    uint16_t scaledHue = (hue * 6);
    uint8_t segment = scaledHue / 256; // segment 0 to 5 around the
    // color wheel
    uint16_t segmentOffset =
        scaledHue - (segment * 256); // position within the segment

    uint8_t complement = 0;
    uint16_t prev = (brightness * ( 255 -  segmentOffset)) / 256;
    uint16_t next = (brightness *  segmentOffset) / 256;

    if (invert) {
        brightness = 255 - brightness;
        complement = 255;
        prev = 255 - prev;
        next = 255 - next;
    }

    switch (segment ) {
    case 0:      // red
        R = brightness;
        G = next;
        B = complement;
        break;
    case 1:     // yellow
        R = prev;
        G = brightness;
        B = complement;
        break;
    case 2:     // green
        R = complement;
        G = brightness;
        B = next;
        break;
    case 3:    // cyan
        R = complement;
        G = prev;
        B = brightness;
        break;
    case 4:    // blue
        R = next;
        G = complement;
        B = brightness;
        break;
    case 5:      // magenta
    default:
        R = brightness;
        G = complement;
        B = prev;
        break;
    }
}


//button0 callback function
void button0_callback(void)
{
    // red
    ledcWrite(1, 0);
    ledcWrite(2, 255);
    ledcWrite(3, 0);

    button0_flag = 0;//Clear flag bit

}

//button1 callback function
void button1_callback(void)
{
    //green
    ledcWrite(1, 255);
    ledcWrite(2, 0);
    ledcWrite(3, 0);

    button1_flag = 0;//Clear flag bit
}

//button2 callback function
void button2_callback(void)
{
    //blue
    ledcWrite(1, 0);
    ledcWrite(2, 0);
    ledcWrite(3, 255);

    button2_flag = 0;//Clear flag bit

}




void LilyGo_logo(void)
{

    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);

#if defined(_HAS_COLOR_)
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_RED);
#else
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
#endif

#if defined(_HAS_SDCARD_)
    display.setRotation(1);
    display.setTextColor(GxEPD_BLACK);
    display.setFont();
#if defined(LILYGO_MINI_EPAPER_ESP32)
    display.setCursor(0, display.height() - 15);
#else
    display.setCursor(20, display.height() - 15);
#endif
    String sizeString = "SD:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    display.println(rlst ? sizeString : "SD:N/A");

    int16_t x1, x2;
    uint16_t w, h;
    String str = GxEPD_BitmapExamplesQ;
    str = str.substring(2, str.lastIndexOf("/"));
    display.getTextBounds(str, 0, 0, &x1, &x2, &w, &h);
    display.setCursor(display.width() - w, display.height() - 15);
    display.println(str);
#endif

    display.update();
}
