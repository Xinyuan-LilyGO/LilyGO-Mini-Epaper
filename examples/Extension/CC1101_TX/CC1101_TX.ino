
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

#include <AceButton.h>
#include <GxEPD.h>
#include <boards.h>
#include <GxGDGDEW0102T4/GxGDGDEW0102T4.h> //1.02" b/w
#include GxEPD_BitmapExamples
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Arduino.h>
#include <RadioLib.h>
#include <IRremote.hpp>

using namespace ace_button;


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

// Physical pin numbers attached to the buttons.
const int BUTTON1_PIN = 0;
AceButton button1(BUTTON1_PIN);
// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton *, uint8_t, uint8_t);

bool cc1101_send_flag = true; //用来切换模式（IR发送或者cc1101发送）



void LilyGo_logo();

// save transmission state between loops
int transmissionState = RADIOLIB_ERR_NONE;

/*
 * Set up the data to be sent.
 * For most protocols, the data is build up with a constant 8 (or 16 byte) address
 * and a variable 8 bit command.
 * There are exceptions like Sony and Denon, which have 5 bit address.
 */
uint16_t sAddress = 0x00;
uint8_t sCommand = 0x34;
uint8_t sRepeats = 0;

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is transmitted by the module
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
    // we sent a packet, set the flag
    transmittedFlag = true;
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println("setup");

    pinMode(EPD_POWER_ENABLE, OUTPUT);
    digitalWrite(EPD_POWER_ENABLE, HIGH);

    EPD_SPI.begin(EPD_SCLK, EPD_MISO, EPD_MOSI);
    display.init(); // enable diagnostic output on Serial

    LilyGo_logo();


    display.setRotation(3);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono9pt7b);


    // Buttons use the built-in pull up register.
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    // Configure the ButtonConfig with the event handler, and enable all higher
    // level events.
    ButtonConfig *buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureClick);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);



    SPI.begin(CC1101_SCLK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
    // initialize CC1101 with default settings
    Serial.print(F("[CC1101] Initializing ... "));
    int state = radio.begin();
    // int   state = radio.begin(434.0, 32.0, 60.0, 250.0, 7, 32);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println(F("success!"));
    } else {
        Serial.print(F("failed, code "));
        Serial.println(state);
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(0, 20);
        display.print("CC1101 fail");
        display.update();
        while (true);
    }

    // you can also change the settings at runtime
    // and check if the configuration was changed successfully

    // set carrier frequency to 433.5 MHz
    if (radio.setFrequency(868.0) == RADIOLIB_ERR_INVALID_FREQUENCY) {
        Serial.println(F("[CC1101] Selected frequency is invalid for this module!"));
        while (true);
    }

    // set allowed frequency deviation to 10.0 kHz
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
    // when packet transmission is finished
    radio.setGdo0Action(setFlag);

    // start transmitting the first packet
    Serial.print(F("[CC1101] Sending first packet ... "));

    // you can transmit C-string or Arduino string up to
    // 64 characters long
    transmissionState = radio.startTransmit("Hello World!");



    /*
     * The IR library setup. That's all!
     */
    IrSender.begin(IR_TX_PIN); // Start with IR_SEND_PIN as send pin and if NO_LED_FEEDBACK_CODE is NOT defined, enable feedback LED at default feedback LED pin

    Serial.print(F("Ready to send IR signals at pin "));
    Serial.println(IR_TX_PIN);

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 20);
    display.print("CC TX mode");
    display.update();

    Serial.println("setup done");
}

uint32_t lastSend = 0;


void loopSender()
{
    if (cc1101_send_flag) {
        // check if the previous transmission finished
        if (transmittedFlag) {
            // disable the interrupt service routine while
            // processing the data
            enableInterrupt = false;

            // reset flag
            transmittedFlag = false;

            if (transmissionState == RADIOLIB_ERR_NONE) {
                // packet was successfully sent
                Serial.println(F("transmission finished!"));

                // NOTE: when using interrupt-driven transmit method,
                //       it is not possible to automatically measure
                //       transmission data rate using getDataRate()
            } else {
                Serial.print(F("failed, code "));
                Serial.println(transmissionState);
            }
            // wait a second before transmitting again
            // delay(2000);  // one tick delay (15ms) in between reads for stability

            // send another one

            // you can transmit C-string or Arduino string up to
            // 256 characters long

            static char send_str[10];
            static  int send_num = 0;
            sprintf(send_str, "%d", send_num);
            // you can transmit C-string or Arduino string up to 63 characters long
            // int state = radio.transmit(send_str);
            send_num++;
            Serial.print(F("[CC1101] Sending another packet ... "));
            Serial.println(send_str);
            transmissionState = radio.startTransmit(send_str);

            // you can also transmit byte array up to 256 bytes long
            /*
              byte byteArr[] = {0x01, 0x23, 0x45, 0x67,
                                0x89, 0xAB, 0xCD, 0xEF};
              int state = radio.startTransmit(byteArr, 8);
            */
            // we're ready to send more packets,
            // enable interrupt service routine
            enableInterrupt = true;
        }

        // delay(2000);  // one tick delay (15ms) in between reads for stability
    } else {

        Serial.println();
        Serial.print(F("Send now: address=0x"));
        Serial.print(sAddress, HEX);
        Serial.print(F(" command=0x"));
        Serial.print(sCommand, HEX);
        Serial.print(F(" repeats="));
        Serial.print(sRepeats);
        Serial.println();

        Serial.println(F("Send NEC with 16 bit address"));
        Serial.flush();

        // Results for the first loop to: Protocol=NEC Address=0x102 Command=0x34 Raw-Data=0xCB340102 (32 bits)
        IrSender.sendNEC(sAddress, sCommand, sRepeats);

        /*
         * If you cannot avoid to send a raw value directly like e.g. 0xCB340102 you must use sendNECRaw()
         */
        //    Serial.println(F("Send NECRaw 0xCB340102"));
        //    IrSender.sendNECRaw(0xCB340102, sRepeats);
        /*
         * Increment send values
         * Also increment address just for demonstration, which normally makes no sense
         */
        // sAddress += 0x0101;
        sCommand += 0x11;
        sRepeats++;
        // clip repeats at 4
        if (sRepeats > 4) {
            sRepeats = 4;
        }
    }
}



void loop()
{
    button1.check();
    if (millis() > lastSend) {
        loopSender();
        lastSend = millis() + 2000;

    }
}

// The event handler for both buttons.
void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState)
{

    // Control the LED only for the Pressed and Released events of Button 1.
    // Notice that if the MCU is rebooted while the button is pressed down, no
    // event is triggered and the LED remains off.
    switch (eventType) {
    case AceButton::kEventPressed: {
        Serial.println(F("kEventPressed"));
        if (cc1101_send_flag) {
            display.fillScreen(GxEPD_WHITE);
            display.fillScreen(GxEPD_WHITE);
            display.setCursor(0, 20);
            display.print("IR TX mode");
            display.update();
            cc1101_send_flag = false;
            Serial.println("false");
        } else {
            display.fillScreen(GxEPD_WHITE);
            display.setCursor(0, 20);
            display.print("CC TX mode");
            display.update();
            cc1101_send_flag = true;
            Serial.println("cc1101_send_flag = true;");
        }
    }
    break;
    default: break;
    }
}


void LilyGo_logo(void)
{
    display.setRotation(2);
    display.fillScreen(GxEPD_WHITE);
    display.drawExampleBitmap(BitmapExample1, 0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, GxEPD_BLACK);
    display.update();
}



