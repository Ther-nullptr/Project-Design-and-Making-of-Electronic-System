// Adafruit_ImageReader test for 2.4" TFT FeatherWing. Demonstrates loading
// images from SD card or flash memory to the screen, to RAM, and how to
// query image file dimensions. OPEN THE ARDUINO SERIAL MONITOR WINDOW TO
// START PROGRAM. Requires three BMP files in root directory of SD card or
// flash: purple.bmp, parrot.bmp and wales.bmp.

#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

// Comment out the next line to load from SPI/QSPI flash instead of SD card:
#define USE_SD_CARD

// Pin definitions for 2.4" TFT FeatherWing vary among boards...
#define TFT_CS 9
#define TFT_DC 10

#ifdef UNO
#define SD_CS 4
#else
#define SD_CS 53
#endif

#if defined(USE_SD_CARD)
SdFat SD;                        // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#endif

const int SDIN = 53;
const int blk = 10;
const int sda = 6;
const int rst = 9;
const int cs = 7;
const int dc = 8;
const int sck = 5;

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
Adafruit_Image img; // An image loaded into RAM
int32_t width = 0,  // BMP image dimensions
    height = 0;
ImageReturnCode stat; // Status from image-reading functions

void setup(void)
{
    Serial.begin(9600);
#if !defined(ESP32)
    while (!Serial)
        ; // Wait for Serial Monitor before continuing
#endif
    tft.begin(); // Initialize screen
    // The Adafruit_ImageReader constructor call (above, before setup())
    // accepts an uninitialized SdFat or FatFileSystem object. This MUST
    // BE INITIALIZED before using any of the image reader functions!
    Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
    // SD card is pretty straightforward, a single call...
    if (!SD.begin(SD_CS))
    { // ESP32 requires 25 MHz limit
        Serial.println(F("SD begin() failed"));
        for (;;)
            ; // Fatal error, do not continue
    }
#endif
}

void loop()
{
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("BAD APPLE");
    delay(2000);
    tft.fillScreen(ILI9341_BLACK);

    String before1("v");
    String after(".bmp");

    for (int i = 1; i < 650; i++)
    {
        String number(i);
        String filename = before1 + number + after;
        // 多屏显示,效果不大好
        stat = reader.drawBMP(filename.c_str(), tft, 80,60);
        reader.printStatus(stat); // How'd we do?
    }
    tft.println("the end");

    tft.fillScreen(ILI9341_BLUE);
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("TWO TIGERS LOVE DANCING");
    delay(2000);
    tft.fillScreen(ILI9341_BLACK);

    String before2("T");
    for (int i = 1; i < 650; i++)
    {
        String number(i);
        String filename = before2 + number + after;
        stat = reader.drawBMP(filename.c_str(), tft, 0, 0);
    }
    tft.println("the end");
}
