/*
* 此文件主要负责时钟的显示,设置和TFTLCD的控制
* 
*/
#include <Arduino.h>
#include <DS1302.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

// 定义每一种字号的大小
#define H_1 8
#define W_1 6
#define H_2 16
#define W_2 12
#define H_3 24
#define W_3 18

// 定义屏幕宽度和高度
#define F_H 320
#define F_W 240

// 时钟管脚定义
const int RST = 1;
const int DAT = 2;
const int CLK = 3;

// TFT管脚定义
const int SDIN = 4;
const int sck = 5;
const int sda = 6;
const int cs = 7;
const int dc = 8;
const int rst = 9;
const int blk = 10;

DS1302 rtc(RST, DAT, CLK);       // 时钟模块初始化
SdFat SD;                        // sd卡对象
Adafruit_ImageReader reader(SD); // 读取SD卡图片的对象

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
Adafruit_Image img;   // An image loaded into RAM
ImageReturnCode stat; // Status from image-reading functions

char date[20], time[10], *week;

void setup()
{
    // !以下的内容在正式启用的时候一定要关掉
    rtc.writeProtect(false);           //关闭写保护
    rtc.halt(false);                   //清除时钟停止标志
    Time t(2021, 7, 24, 21, 0, 50, 7); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
    rtc.time(t);                       //向DS1302设置时间数据

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

    SD.begin(SDIN, SD_SCK_MHZ(25));

    // 屏幕主状态初始界面打印
    Time tim = rtc.time();
    tft.setTextSize(1);
    PrintTime(tim);
    /*
    tft.setCursor(150,230);
    tft.setTextSize(1);
    tft.print("1/3");
    tft.drawTriangle(230,300,238,300,234,315,ILI9341_WHITE);
    */
}

void PrintTime(Time &tim)
{
    snprintf(date, sizeof(date), "%04d-%02d-%02d",
             tim.yr, tim.mon, tim.date);
    snprintf(time, sizeof(time), "%02d:%02d",
             tim.hr, tim.min);
    getWeek(tim.day);
    tft.setTextSize(3);
    tft.setCursor(75, 120);
    tft.print(time);
    tft.setTextSize(1);
    tft.setCursor(10, 280);
    tft.print(date);
    tft.setCursor(F_W - 3 * W_1 - 10, 280);
    tft.print(week);
}

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

void loop()
{
    // 时刻检测时间，但不时刻在屏幕上刷新时间
    static uint16_t last_min = 0;
    Time tim = rtc.time();
    if (tim.min != last_min) // 检测到时间发生改变
    {
        tft.setTextSize(3);
        tft.fillRect(75 + 3 * W_3, 120, 2 * W_3, H_3, ILI9341_BLACK); // 覆盖原有文字
        snprintf(time, sizeof(time), "%02d", tim.min);
        tft.setCursor(75+ 3 * W_3,120);
        tft.print(time);
        if (tim.min == 0) // 满小时
        {
            tft.setTextSize(3);
            tft.fillRect(75, 120, 2 * W_3, H_3, ILI9341_BLACK);
            snprintf(time, sizeof(time), "%02d", tim.hr);
            tft.setCursor(75,120);
            tft.print(time);
            if (tim.hr == 0) // 满天
            {
                tft.setTextSize(1);
                tft.fillRect(10, 280, 5 * W_1, H_1, ILI9341_BLACK);
                snprintf(date, sizeof(date), "%04d-%02d-%02d", tim.yr, tim.mon, tim.date);
                tft.setCursor(10,280);
                tft.print(date);
                getWeek(tim.day);
                tft.fillRect(F_W - 3 * W_1 - 10, 280, 3 * W_1, H_1, ILI9341_BLACK);
                tft.setCursor(F_W - 3 * W_1 - 10, 280);
                tft.print(week);
            }
        }
    }
    last_min = tim.min; // 更新时间
}