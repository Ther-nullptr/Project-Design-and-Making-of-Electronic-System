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
#include "IRKeyPad.h"

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
    Time t(2021, 7, 24, 23, 59, 50, 7); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
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

void PrintTime(Time &tim) // 打印初始界面
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
    tft.setCursor(10, 290);
    tft.print(date);
    tft.setCursor(F_W - 3 * W_1 - 10, 290);
    tft.print(week);

    // 标题
    tft.setTextSize(3);
    tft.setCursor(75,20);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("START");

    // 信号
    tft.drawRect(5,5,2,4,ILI9341_RED);
    tft.drawRect(9,3,2,6,ILI9341_RED);
    tft.drawRect(13,1,2,8,ILI9341_RED);

    // 钟表
    tft.drawCircle(F_W-6,6,4,ILI9341_RED);
    tft.drawLine(F_W-6,3,F_W-6,6,ILI9341_RED);
    tft.drawLine(F_W-6,6,F_W-3,6,ILI9341_RED);

    // 下标
    tft.setTextSize(1);
    tft.drawLine(0,305,240,305,ILI9341_WHITE);
    tft.setCursor(111,308);
    tft.print("1/4");
    tft.fillTriangle(230,308,230,316,234,312,ILI9341_WHITE);

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

void UI_1() // 一号界面,也是初始界面,显示时间
{
    // 时刻检测时间，但不时刻在屏幕上刷新时间
    static uint16_t last_min = 0;
    Time tim = rtc.time();
    if (tim.min != last_min) // 检测到时间发生改变
    {
        tft.setTextSize(3);
        tft.fillRect(75 + 3 * W_3, 120, 2 * W_3, H_3, ILI9341_BLACK); // 覆盖原有文字
        snprintf(time, sizeof(time), "%02d", tim.min);
        tft.setCursor(75 + 3 * W_3, 120);
        tft.print(time);
        if (tim.min == 0) // 满小时
        {
            tft.setTextSize(3);
            tft.fillRect(75, 120, 2 * W_3, H_3, ILI9341_BLACK);
            snprintf(time, sizeof(time), "%02d", tim.hr);
            tft.setCursor(75, 120);
            tft.print(time);
            if (tim.hr == 0) // 满天
            {
                tft.setTextSize(1);
                tft.fillRect(10, 290, 10 * W_1, H_1, ILI9341_BLACK);
                snprintf(date, sizeof(date), "%04d-%02d-%02d", tim.yr, tim.mon, tim.date);
                tft.setCursor(10, 290);
                tft.print(date);
                getWeek(tim.day);
                tft.fillRect(F_W - 3 * W_1 - 10, 290, 3 * W_1, H_1, ILI9341_BLACK);
                tft.setCursor(F_W - 3 * W_1 - 10, 290);
                tft.print(week);
            }
        }
    }
    last_min = tim.min; // 更新时间
}

void UI_2()
{
    tft.setTextSize(3);
    tft.setCursor(75,20);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("CLOCK");
    tft.setCursor(20,50);
    tft.setTextColor(ILI9341_BLUE);
    tft.print("SET CLOCK:");
    tft.setCursor(20,170);
    tft.setTextColor(ILI9341_GREEN);
    tft.print("MY CLOCK:");
    tft.setCursor(40,200);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("07:00  music 1");

    // 光标
    tft.fillTriangle(10,57,10,65,14,61,ILI9341_WHITE);

    // 信号
    tft.drawRect(5,5,2,4,ILI9341_RED);
    tft.drawRect(9,3,2,6,ILI9341_RED);
    tft.drawRect(13,1,2,8,ILI9341_RED);

    // 钟表
    tft.drawCircle(F_W-6,6,4,ILI9341_RED);
    tft.drawLine(F_W-6,3,F_W-6,6,ILI9341_RED);
    tft.drawLine(F_W-6,6,F_W-3,6,ILI9341_RED);

    // 下标
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.drawLine(0,305,240,305,ILI9341_WHITE);
    tft.setCursor(111,308);
    tft.print("2/4");
    tft.fillTriangle(230,308,230,316,234,312,ILI9341_WHITE);
    tft.fillTriangle(10,308,10,316,6,312,ILI9341_WHITE);


}

void UI_3()
{
    tft.setTextSize(3);
    tft.setCursor(75,20);
    tft.setTextColor(ILI9341_WHITE);
    tft.print("MUSIC");

    // 信号
    tft.drawRect(5,5,2,4,ILI9341_RED);
    tft.drawRect(9,3,2,6,ILI9341_RED);
    tft.drawRect(13,1,2,8,ILI9341_RED);

    // 钟表
    tft.drawCircle(F_W-6,6,4,ILI9341_RED);
    tft.drawLine(F_W-6,3,F_W-6,6,ILI9341_RED);
    tft.drawLine(F_W-6,6,F_W-3,6,ILI9341_RED);

    // 下标
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_WHITE);
    tft.drawLine(0,305,240,305,ILI9341_WHITE);
    tft.setCursor(111,308);
    tft.print("3/4");
    tft.fillTriangle(230,308,230,316,234,312,ILI9341_WHITE);
    tft.fillTriangle(10,308,10,316,6,312,ILI9341_WHITE);

    // 显示歌单
    tft.setTextSize(2);
    tft.setCursor(20,46);
    tft.println("Bad Apple");
    tft.setCursor(20,66);
    tft.println("Two Tigers Like Dancing");

    // 光标
    tft.fillTriangle(10,49,10,57,14,53,ILI9341_WHITE);
}


void loop()
{
    delay(10000);
    tft.fillScreen(ILI9341_BLACK);
    UI_2();
    delay(10000);
    tft.fillScreen(ILI9341_BLACK);
    UI_3();
    delay(10000);

}