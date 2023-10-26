
// According to the board, cancel the corresponding macro definition
// https://www.lilygo.cc/products/mini-e-paper-core , esp32picod4
// #define LILYGO_MINI_EPAPER_ESP32
// esp32s3-fn4r2
// #define LILYGO_MINI_EPAPER_ESP32S3
#if !defined(LILYGO_MINI_EPAPER_ESP32S3)  && !defined(LILYGO_MINI_EPAPER_ESP32)
// 请在草图上方选择对应的目标板子名称,将它取消注释.
#error "Please select the corresponding target board name above the sketch and uncomment it."
#endif


#define SEND_PWM_BY_TIMER

#include <Arduino.h>
#include <GxEPD.h>
#include <boards.h>
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
// include the library
#include <RadioLib.h>
// #include "PinDefinitionsAndMore.h" //Define macros for input and output pin etc.
#include <IRremote.hpp>


#ifdef LILYGO_MINI_EPAPER_ESP32
#define CC1101_SCLK     26
#define CC1101_MISO     38
#define CC1101_MOSI     23
#define CC1101_CS       25
#define CC1101_GDO0     32
#define CC1101_GDO2     37
#define IR_TX_PIN        12
#define IR_RECEIVE_PIN   18
#else
#define CC1101_SCLK     8
#define CC1101_MISO     6
#define CC1101_MOSI     17
#define CC1101_CS       7
#define CC1101_GDO0     5
#define CC1101_GDO2     16
#define IR_TX_PIN        21
#define IR_RECEIVE_PIN   18
#endif

CC1101 radio = new Module(CC1101_CS, CC1101_GDO0, RADIOLIB_NC, CC1101_GDO2);
SPIClass EPD_SPI(HSPI);

GxIO_Class io(EPD_SPI,  EPD_CS, EPD_DC,  EPD_RSET);
GxEPD_Class display(io, EPD_RSET, EPD_BUSY);
void LilyGo_logo();
// flag to indicate that a packet was received
volatile bool receivedFlag = false;
// disable interrupt when it's not needed
volatile bool enableInterrupt = true;
int ir_receiver_flag = 0;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void)
{
    // check if the interrupt is enabled
    if (!enableInterrupt) {
        return;
    }

    // we got a packet, set the flag
    receivedFlag = true;
}
void setup(void)
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");


    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);


    EPD_SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial

    LilyGo_logo();

    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMonoBold9pt7b);


    SPI.begin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    // initialize CC1101 with default settings
    Serial.print(F("[CC1101] Initializing ... "));
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(0, 20);
        display.println("CC1101 failed");
        display.update();
        delay(1000);
        while (true);
    }

    // you can also change the settings at runtime
    // and check if the configuration was changed successfully

    // set carrier frequency to 433.5 MHz
    if (radio.setFrequency(868.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
        Serial.println(F("[CC1101] Selected frequency is invalid for this module!"));
        while (true);
    }


    // set allowed frequency deviation to 10.0 kHz  设置允许频率偏差为10.0 kHz
    if (radio.setFrequencyDeviation(10.0) == RADIOLIB_ERR_INVALID_FREQUENCY_DEVIATION) {
        Serial.println(F("[CC1101] Selected frequency deviation is invalid for this module!"));
        while (true);
    }

    // set output power to 5 dBm
    if (radio.setOutputPower(10) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
        Serial.println(F("[CC1101] Selected output power is invalid for this module!"));
        while (true);
    }

    // set the function that will be called
    // when new packet is received
    radio.setGdo0Action(setFlag);

    // start listening for packets
    Serial.print(F("[CC1101] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true);
    }


    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, false);
    // Serial.print(F("Ready to receive IR signals of protocols: "));
    // printActiveIRProtocols(&Serial);
    // Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));

    Serial.println("setup done");
}



void loop()
{

    if (receivedFlag) {
        // disable the interrupt service routine while
        // processing the data
        enableInterrupt = false;

        // reset flag
        receivedFlag = false;

        // you can read received data as an Arduino String
        String str;
        int state = radio.readData(str);
        // you can also read received data as byte array
        /*
          byte byteArr[8];
          int state = radio.readData(byteArr, 8);
        */

        if (state == RADIOLIB_ERR_NONE) {
            // packet was successfully received
            Serial.println(F("[CC1101] Received packet!"));

            // print data of the packet
            Serial.print(F("[CC1101] Data:\t\t"));
            Serial.println(str);

            // print RSSI (Received Signal Strength Indicator)
            // of the last received packet
            Serial.print(F("[CC1101] RSSI:\t\t"));
            Serial.print(radio.getRSSI());
            Serial.println(F(" dBm"));

            display.fillScreen(GxEPD_WHITE);
            display.setCursor(0, 20);
            display.print("cc1101:");
            display.println(str);
            display.print("RSSI:");
            display.println(radio.getRSSI());
            display.update();
            vTaskDelay(200);
        } else {
            // some other error occurred
            Serial.print(F("failed, code "));
            Serial.println(state);
        }

        // put module back to listen mode
        radio.startReceive();

        // we're ready to receive more packets,
        // enable interrupt service routine
        enableInterrupt = true;
    }

    if (IrReceiver.decode()) {
        IrReceiver.resume(); // Enable receiving of the next value
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(0, 20);
        display.print("IR:");
        display.println(ir_receiver_flag);
        display.update();
        ir_receiver_flag++;
    }
    delay(20);
}




void LilyGo_logo(void)
{
    display.setRotation(2);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}

