
<h1 align = "center">🌟LilyGO Mini Epaper🌟</h1>

### English | [中文](docs/details_cn.md) 

--------------------------------------


<h2 align = "left">Quick start ⚡:</h2>

#### USE PlatformIO

1. Install[VSCODE](https://code.visualstudio.com/)and[Python](https://www.python.org/)
2. Search for the PlatformIO plug-in in the VSCODE extension and install it.
3. After the installation is complete and the reload is completed, there will be a small house icon in the lower left corner. Click to display the Platformio IDE home page
4. Go to file - > Open folder - > Select the LilyGO-Mini-Epaper folder and click the (√) symbol in the lower left corner to compile (→) for upload.




#### USE Arduino IDE


1. Copy all the folders in the lib directory to `"C:\User\<YourName>\Documents\Arduino\libraries"`
2. At the top of the sketch, define the model of the board and screen to be used 
3. Select `ESP32 Pico Kit` in the development board, and keep the other options as default
4. If you do not find the ESP32 series in the development board, then you should see the following installation method [How to install ESP32 in ArduinoIDE](https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md)
5. For the version of the board you purchased, please check the product link below 
6. When you think there is a problem with your board, you can pull the binary file corresponding to your screen model in the firmware directory, follow the `SETTING.png` settings in the directory, download it to the board, and then observe the execution. 




<h2 align = "left">Product link📷:</h2>

|     Name     |                            Product  Link                             |
| :----------: | :------------------------------------------------------------------: |
| [T5 V1.02]() | [Product link](https://pt.aliexpress.com/item/1005002857956100.html) |

<h3 align = "left">PinOut :</h3>

![](image/MINI1.02CORE.jpg)


<h2 align = "left">Example description:</h2>

```
examples
├── Extension                        # Expansion Board Example
├── GxEPD_Example                    # GxEPD Example
├── GxEPD_Example2                   # GxEPD Example2
├── GxEPD_Hello_world                # EPD  Hello world example 
├── GxEPD_RGB_LED                    # RGB LED example 
├── GxEPD_SD_Example                 # 1.02" SD example 
├── Hello_World_U8G2_Fonts           # EPD  U8G2 Fonts Hello world example 
├──Partial_UpdateTest                # Partial refresh example 
└──Integration 
```


