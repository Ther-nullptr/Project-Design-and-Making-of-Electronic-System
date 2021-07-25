
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.

#define sclk 13
#define mosi 11
#define cs 10
#define dc 8
#define rst 5

Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 8, 11, 13, 5, 12);

void setup()
{
    Serial.begin(9600);
    Serial.println("ILI9341 Test!");
    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
}

void loop(void)
{
    unsigned long start = micros();
    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("Welcome to our class");
    tft.setCursor(0, 50);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.println("AnalogRead:");
    tft.setCursor(150, 50);
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.setTextSize(2);
    if (analogRead(A0) <= 9)
    {
        tft.println(String("000") + String(analogRead(A0)));
    }
    else if (analogRead(A0) >= 10 && analogRead(A0) <= 99)
    {
        tft.println(String("00") + String(analogRead(A0)));
    }
    else if (analogRead(A0) >= 100 && analogRead(A0) <= 999)
    {
        tft.println(String("0") + String(analogRead(A0)));
    }
    else if (analogRead(A0) >= 1000)
    {
        tft.println(analogRead(A0));
    }
    tft.setCursor(0, 100);
    tft.setTextColor(ILI9341_BLUE);
    tft.setTextSize(2);
    tft.println("Hello World");
    tft.setCursor(0, 150);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(3);
    tft.println("YwRobot TFT");
    tft.fillCircle(80, 250, 10, ILI9341_WHITE);
    tft.drawCircle(80, 250, 15, ILI9341_MAGENTA);
    tft.drawCircle(80, 250, 16, ILI9341_MAGENTA);
    tft.drawCircle(80, 250, 21, ILI9341_WHITE);
    tft.drawCircle(80, 250, 20, ILI9341_WHITE);
    tft.drawCircle(80, 250, 23, ILI9341_BLUE);
    tft.drawCircle(80, 250, 24, ILI9341_BLUE);
    tft.drawCircle(80, 250, 27, ILI9341_GREEN);
    tft.drawCircle(80, 250, 28, ILI9341_GREEN);
    tft.drawCircle(80, 250, 31, ILI9341_YELLOW);
    tft.drawCircle(80, 250, 32, ILI9341_YELLOW);
    tft.drawCircle(80, 250, 35, ILI9341_RED);
    tft.drawCircle(80, 250, 36, ILI9341_RED);
    tft.drawCircle(80, 250, 37, ILI9341_BLUE);
    tft.drawCircle(80, 250, 38, ILI9341_BLUE);
    tft.drawCircle(80, 250, 39, ILI9341_GREEN);
    tft.drawCircle(80, 250, 40, ILI9341_GREEN);
    tft.drawCircle(80, 250, 41, ILI9341_YELLOW);
    tft.drawCircle(80, 250, 42, ILI9341_YELLOW);
    tft.drawCircle(80, 250, 43, ILI9341_RED);
    tft.drawCircle(80, 250, 44, ILI9341_RED);
    tft.drawCircle(80, 250, 45, ILI9341_WHITE);
    tft.drawCircle(80, 250, 46, ILI9341_MAGENTA);
    tft.drawCircle(80, 250, 47, ILI9341_MAGENTA);
    tft.drawCircle(80, 250, 48, ILI9341_WHITE);
    tft.drawCircle(80, 250, 49, ILI9341_WHITE);
    tft.drawCircle(80, 250, 50, ILI9341_BLUE);
    tft.drawCircle(80, 250, 51, ILI9341_BLUE);
    tft.drawCircle(80, 250, 52, ILI9341_GREEN);
    tft.drawCircle(80, 250, 53, ILI9341_GREEN);
    return micros() - start;
}