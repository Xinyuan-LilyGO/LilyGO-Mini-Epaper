
<h1 align = "center">🌟LilyGO Mini Epaper🌟</h1>

### English | [中文](docs/details_cn.md) 

--------------------------------------


<h2 align = "left">Quick start ⚡:</h2>


1. Install the following two dependencies
     - [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
     - please use this branch [lewisxhe/fork/GxEPD](https://github.com/lewisxhe/GxEPD),Please note that referencing the wrong branch will cause the program to fail to compile
2. Copy all the folders in the lib directory to `"C:\User\<YourName>\Documents\Arduino\libraries"`
3. At the top of the sketch, define the model of the board and screen to be used 
5. Select `ESP32 Dev Module` in the development board, and keep the other options as default
6. If you do not find the ESP32 series in the development board, then you should see the following installation method [How to install ESP32 in ArduinoIDE](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
7. For the version of the board you purchased, please check the product link below 
8. When you think there is a problem with your board, you can pull the binary file corresponding to your screen model in the firmware directory, follow the `SETTING.png` settings in the directory, download it to the board, and then observe the execution. 

<h2 align = "left">Product link📷:</h2>

|     Name     |                            Product  Link                             |
| :----------: | :------------------------------------------------------------------: |
| [T5 V1.02]() | [Product link](https://pt.aliexpress.com/item/1005002857956100.html) |

<h2 align = "left">Example description:</h2>

```
examples
├── Extension                        # Expansion Board Example
├── GxEPD_Hello_world                # EPD  Hello world example 
├── GxEPD_RGB_LED                    # RGB LED example 
├── GxEPD_SD_Example                 # 1.02" SD example 
├── Hello_World_U8G2_Fonts           # EPD  U8G2 Fonts Hello world example 
└── Partial_UpdateTest               # Partial refresh example 
```


