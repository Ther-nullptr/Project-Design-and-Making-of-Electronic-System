/*
* 此文件主要负责时钟的显示,设置和TFTLCD的控制
* 
*/
#include <Arduino.h>
#include <IRremote.h>
#include <DS1302.h>
#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions
#include "IRKeyPad.h"
#include "functions.h"

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

// 红外接收管管脚定义
const int RECV_PIN = A0;

// 设定设备状态的bool值
bool is_clock = false; // 是否设定闹钟
bool is_Wifi = false;  // 是否连接wifi

// 设定设备状态的状态值
uint8_t status = 1;

// 控制程序第一次是否清屏
bool flag = true;

DS1302 rtc(RST, DAT, CLK);       // 时钟模块初始化
SdFat SD;                        // sd卡对象
Adafruit_ImageReader reader(SD); // 读取SD卡图片的对象
IRrecv irrecv(RECV_PIN);         // 红外接收器
decode_results results;          // 红外信号解码值

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
Adafruit_Image img;
ImageReturnCode stat;

char date[20], time[10], *week;

void setup()
{
    // !以下的内容在正式启用的时候一定要关掉
    rtc.writeProtect(false);            //关闭写保护
    rtc.halt(false);                    //清除时钟停止标志
    Time t(2021, 7, 24, 23, 59, 50, 7); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
    rtc.time(t);                        //向DS1302设置时间数据

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

    SD.begin(SDIN, SD_SCK_MHZ(25));

    // 屏幕主状态初始界面打印
    Time tim = rtc.time();
    tft.setTextSize(1);
    irrecv.enableIRIn();
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
}

void UI_1() // 一号界面,也是初始界面,显示时间
{
    PrintBase(1);
    // 时刻检测时间，但不时刻在屏幕上刷新时间
    while (1)
    {
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

        if (irrecv.available())
        {
            if (irrecv.decode(&results))
            {
                if (results.value == _2)
                {
                    status = 2;
                }
                else if (results.value == _3)
                {
                    status = 3;
                }
                irrecv.resume();
                break;
            }
        }
    }
}

void UI_2()
{
    PrintBase(2);
    tft.setCursor(20, 50);
    tft.setTextColor(ILI9341_BLUE);
    tft.print(F("SET CLOCK:"));
    tft.setCursor(20, 170);
    tft.setTextColor(ILI9341_GREEN);
    tft.print(F("MY CLOCK:"));
    tft.setCursor(40, 200);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(F("07:00  music 1"));

    while (1)
    {
        if (irrecv.available())
        {
            if (irrecv.decode(&results))
            {
                if (results.value == ENTER) // 设定闹钟
                {

                    tft.setTextSize(3);
                    tft.setCursor(40, 90);
                    tft.print("00:00");
                    for (int i = 0; i < 4; i++)
                    {
                        irrecv.resume();
                        tft.drawLine(84 + i * W_3, 70, 84 + (i + 1) * W_3, 70, ILI9341_WHITE);

                        while (!irrecv.decode(&results))
                            ; // 一直等待直到收到红外信号
                        // 找对应的字母值
                        tft.drawLine(84 + i * W_3, 70, 84 + (i + 1) * W_3, 70, ILI9341_BLACK); // 覆盖原有的白线
                    }
                }
                else
                {
                    if (results.value == _1)
                    {
                        status = 1;
                    }
                    else if (results.value == _3)
                    {
                        status = 3;
                    }
                    irrecv.resume();
                    break;
                }
            }
        }
    }
}

void UI_3()
{
    PrintBase(3);

    // 显示歌单
    tft.setTextSize(2);
    tft.setCursor(20, 46);
    tft.println(F("Bad Apple"));
    tft.setCursor(20, 66);
    tft.println(F("Two Tigers Like Dancing"));

    // 光标
    tft.fillTriangle(10, 49, 10, 57, 14, 53, ILI9341_WHITE);

    while (1)
    {
        if (irrecv.available())
        {
            if (irrecv.decode(&results))
            {
                if (results.value == _1)
                {
                    status = 1;
                }
                else if (results.value == _2)
                {
                    status = 2;
                }
                irrecv.resume();
                break;
            }
        }
    }
}

void loop()
{
    switch (status)
    {
    case 1:
        if (!flag)
        {
            tft.fillScreen(ILI9341_BLACK);
        }
        flag = false;
        Time tim = rtc.time();
        PrintTime(tim);
        UI_1();
        break;

    case 2:
        tft.fillScreen(ILI9341_BLACK);
        UI_2();
        break;

    case 3:
        tft.fillScreen(ILI9341_BLACK);
        UI_3();
        break;
    }
}