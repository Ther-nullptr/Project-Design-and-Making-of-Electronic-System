#include<SD.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

const int SDIN = 4;
const int blk = 10;
const int sda = 6;
const int rst = 9;
const int cs = 7;
const int dc = 8;
const int sck = 5;

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
uint16_t color =0x00;

void setup()
{
    Serial.begin(9600);
    tft.begin();
     tft.fillScreen(ILI9341_BLACK);
}


void loop()
{
    File img = SD.open("test.txt");
    for(int y =0;y<320;y++)
    {
        for(int x=0;x<240;x++)
        {
            color = img.read();
            Serial.println(color);
            if(color==0xffff)
            tft.drawPixel(x,y,ILI9341_WHITE);
        }
    }
    img.close();
}