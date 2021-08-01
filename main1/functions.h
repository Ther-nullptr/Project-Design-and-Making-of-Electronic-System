#pragma once
#include "IRKeyPad.h"
#include <Adafruit_ILI9341.h>

/* 一些与主运行逻辑关系不大的函数将会被放到这里*/

extern char *week;
extern Adafruit_ILI9341 tft;
extern bool is_clock;
extern bool is_Wifi;
extern ImageReturnCode stat;
extern SdFat SD; 
extern Adafruit_Keypad customKeypad;
extern Adafruit_Image img;
extern Adafruit_ImageReader reader;

void getWeek(uint16_t num)
{
    switch (num)
    {
    case 7:
        week = "Sat";
        break;

    case 1:
        week = "Sun";
        break;

    case 2:
        week = "Mon";
        break;

    case 3:
        week = "Tue";
        break;

    case 4:
        week = "Wed";
        break;

    case 5:
        week = "Thu";
        break;

    case 6:
        week = "Fri";
        break;
    }
}

void PrintBase(uint8_t id) // 打印每个界面的共性物
{
    int color;
    // 信号
    if (is_Wifi)
    {
        color = ILI9341_GREEN;
    }
    else
    {
        color = ILI9341_RED;
    }
    tft.drawRect(5, 5, 2, 4, color);
    tft.drawRect(9, 3, 2, 6, color);
    tft.drawRect(13, 1, 2, 8, color);

    // 钟表
    if (is_clock)
    {
        color = ILI9341_GREEN;
    }
    else
    {
        color = ILI9341_RED;
    }
    tft.drawCircle(F_W - 6, 6, 4, color);
    tft.drawLine(F_W - 6, 3, F_W - 6, 6, color);
    tft.drawLine(F_W - 6, 6, F_W - 3, 6, color);

    // 下标
    tft.fillTriangle(230, 308, 230, 316, 234, 312, ILI9341_WHITE);
    tft.fillTriangle(10, 308, 10, 316, 6, 312, ILI9341_WHITE);

    tft.setTextSize(1);
    tft.drawLine(0, 305, 240, 305, ILI9341_WHITE);
    tft.setCursor(111, 308);
    tft.setTextColor(ILI9341_WHITE);
    switch (id)
    {
    case 1:
        tft.print("1/4");
        break;

    case 2:
        tft.print("2/4");
        break;

    case 3:
        tft.print("3/4");
        break;

    case 4:
        tft.print("4/4");
        break;
    }

    tft.setTextSize(3);
    tft.setCursor(75, 20);
    tft.setTextColor(ILI9341_WHITE);
    switch (id)
    {
    case 1:
        tft.print(F("START"));
        break;

    case 2:
        tft.print(F("CLOCK"));
        break;

    case 3:
        tft.print(F("MUSIC"));
        break;

    case 4:
        tft.print(F("VIDEO"));
        break;
    }
}

void PlayCursor(uint8_t status, int i, int color)
{
    if (status == 2)
    {
        tft.fillTriangle(10, 58 + i * 80, 10, 66 + i * 80, 14, 62 + i * 80, color);
    }
    else if (status == 3)
    {
        tft.fillTriangle(10, 29 + i * 20, 10, 37 + i * 20, 14, 33 + i * 20, color);
    }
}

void PlayVideo(uint8_t label)
{
    String before;
    String after(".bmp");
    String filename;

    switch (label)
    {
    case 1:
        before = "";
        break;

    case 2:
        before = "T";
        break;
    }
    int num = 1;
    while (1)
    {
        filename = before + num + after;
        stat = reader.drawBMP(filename.c_str(), tft, 0, 0);
        if (stat != IMAGE_SUCCESS)
        {
            break;
        }
        // TODO 此处应有更好的优化方法:把esc设置为中断
        customKeypad.tick();
        if (customKeypad.available())
        {
            keypadEvent e = customKeypad.read();
            if (e.bit.KEY == BACK && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                break;
            }
        }
        num++;
    }
}
