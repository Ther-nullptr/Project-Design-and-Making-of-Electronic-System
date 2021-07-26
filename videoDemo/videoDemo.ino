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

SdFat SD;                        // SD card filesystem
Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys

const int SDIN = 4;
const int blk = 10;
const int sda = 6;
const int rst = 9;
const int cs = 7;
const int dc = 8;
const int sck = 5;

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
Adafruit_Image img; // An image loaded into RAM

void setup(void)
{
    SD.begin(SDIN,SD_SCK_MHZ(25));

    ImageReturnCode stat; // Status from image-reading functions
    Serial.begin(9600);
    tft.begin(); // Initialize screen
    //tft.initSPI();

    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0,0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("BAD APPLE");
    tft.fillScreen(ILI9341_BLACK);
    
    

    String before1("");
    String after(".bmp");
    for (int i =1;i<650;i++)
    {
        String number(i);
        String filename = before1+number+after;
        stat = reader.drawBMP(filename.c_str(), tft, 0, 0);
        reader.printStatus(stat); // How'd we do?
    }
/*
    tft.fillScreen(ILI9341_BLUE);
    tft.setCursor(0,0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("TWO TIGERS LOVE DANCING");
    */

    String before2("T");
    for (int i =1;i<650;i++)
    {
        String number(i);
        String filename = before2+number+after;
        stat = reader.drawBMP(filename.c_str(), tft, 0, 0);
    }
    
}

void loop()
{
}
