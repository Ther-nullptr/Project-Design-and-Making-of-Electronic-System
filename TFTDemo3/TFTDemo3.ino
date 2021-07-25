
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "3.h"
#define sclk 13
#define mosi 11
#define cs 10
#define dc 8
#define rst 5

Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 8, 11, 13, 5, 12);

void setup()
{
    tft.begin();
    
}

const uint8_t image_data_3 = {0,0,0};

void loop()
{
    tft.fillScreen(ILI9341_BLACK);
    tft.drawGrayscaleBitmap(1,1,image_data_3,320,240);
    

}