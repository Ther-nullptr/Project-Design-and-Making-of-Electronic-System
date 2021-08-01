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
#include <Adafruit_Keypad.h>
#include <EEPROM.h>
#include "IRKeyPad.h"
#include "functions.h"

#define DEBUG
#define MEGA
#define KEYPAD

#ifdef UNO
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
#endif // UNO

#ifdef MEGA
// 时钟管脚定义
const int RST = 26;
const int DAT = 24;
const int CLK = 22;

// TFT管脚定义
const int SDIN = 53;
const int sck = 5;
const int sda = 6;
const int cs = 7;
const int dc = 8;
const int rst = 9;
const int blk = 10;

// 红外接收管管脚定义
const int RECV_PIN = A0;

// 按键模块定义
const byte ROWS = 4;                         // rows
const byte COLS = 4;                         // columns
const byte rowPins[ROWS] = {28, 30, 32, 34}; //connect to the row pinouts of the keypad
const byte colPins[COLS] = {36, 38, 40, 42}; //connect to the column pinouts of the keypad
const uint8_t keys[ROWS][COLS] = {
    {1, 2, 3, UP},
    {4, 5, 6, DOWN},
    {7, 8, 9, LEFT},
    {BACK, 0, ENTER, RIGHT}};

#endif // MEGA

// 设定设备状态的bool值
bool is_clock = false; // 是否设定闹钟
bool is_Wifi = false;  // 是否连接wifi

// 设定设备状态的状态值
volatile uint8_t status = 1;

// 控制程序第一次是否清屏
bool flag = true;

DS1302 rtc(RST, DAT, CLK);       // 时钟模块初始化
SdFat SD;                        // sd卡对象
Adafruit_ImageReader reader(SD); // 读取SD卡图片的对象

IRrecv irrecv(RECV_PIN); // 红外接收器
decode_results results;  // 红外信号解码值

Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
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

    SD.begin(SDIN);

    Serial.begin(9600);

    // 屏幕主状态初始界面打印
    Time tim = rtc.time();
    tft.setTextSize(1);
    irrecv.enableIRIn();
    customKeypad.begin();

    if (EEPROM.read(0) != CLEAR)
    {
        is_clock = true;
    }
}

void PrintTime(Time &tim) // 打印初始界面
{
    snprintf(date, sizeof(date), "%04d-%02d-%02d",
             tim.yr, tim.mon, tim.date);
    snprintf(time, sizeof(time), "%02d:%02d",
             tim.hr, tim.min);
    getWeek(tim.day);
    tft.setTextColor(ILI9341_WHITE);
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
    uint8_t alarmHour = EEPROM.read(0);
    uint8_t alarmMinute = EEPROM.read(1);
    uint8_t alarmMusic = EEPROM.read(2);
    // 时刻检测时间，但不时刻在屏幕上刷新时间
    while (1)
    {
        static uint16_t last_min = 0;
        Time tim = rtc.time();
        if (tim.hr == alarmHour && tim.min == alarmMinute) //触发闹钟响铃1min
        {
            tft.setCursor(36, 180);
            tft.setTextColor(ILI9341_RED);
            tft.print("Alarming!");
            // TODO 播放音乐
        }
        else if (tim.hr == alarmHour && tim.min == alarmMinute + 1)
        {
            tft.fillRect(36, 180, 210, 25, ILI9341_BLACK);
        }
        if (tim.min != last_min) // 检测到时间发生改变
        {
            tft.setTextSize(3);
            tft.setTextColor(ILI9341_WHITE);
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

        customKeypad.tick();
        if (customKeypad.available())
        {
            keypadEvent e = customKeypad.read();
            status = e.bit.KEY;
            if (status != 1)
            {
                goto Label1;
            }
        }
    }
Label1:;
}

void UI_2()
{
    PrintBase(2);
    tft.setCursor(20, 50);
    tft.setTextColor(ILI9341_BLUE);
    tft.print(F("SET CLOCK"));

    tft.setCursor(20, 130);
    tft.setTextColor(ILI9341_RED);
    tft.print(F("DEL CLOCK"));

    tft.setCursor(20, 210);
    tft.setTextColor(ILI9341_GREEN);
    tft.print(F("MY CLOCK"));
    tft.setCursor(40, 240);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    if (EEPROM.read(0) == CLEAR) // 没有闹钟
    {
        is_clock = false;
        tft.print(F("No clocks"));
    }
    else
    {
        is_clock = true;
        char clockString[25];
        snprintf(clockString, sizeof(clockString), "%02d:%02d  music %d", EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));
        tft.print(clockString);
    }

    bool cursorPosition = 0;
    PlayCursor(2, cursorPosition, ILI9341_WHITE);
    while (1)
    {
#ifdef IR
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
#endif // IR
        customKeypad.tick();
        if (customKeypad.available())
        {
            keypadEvent e = customKeypad.read();
            delay(10);
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, ILI9341_BLACK);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, ILI9341_BLACK);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, ILI9341_GREEN);
                if (cursorPosition == 0) // 设定闹钟
                {
                    tft.setCursor(30, 80);
                    tft.setTextSize(1);
                    tft.print(F("Set the time:"));
                    uint8_t clockArray[5];
                    // 设置闹钟时间
                    for (int i = 0; i < 4; i++)
                    {
                        tft.drawLine(i * 8 + 112, 90, i * 8 + 120, 90, ILI9341_WHITE);
                        while (1)
                        {
                            customKeypad.tick();
                            if (customKeypad.available())
                            {
                                keypadEvent e = customKeypad.read();
                                delay(10);
                                if (e.bit.EVENT == KEY_JUST_PRESSED)
                                {
                                    clockArray[i] = e.bit.KEY;
                                    tft.setCursor(i * 8 + 112, 80);
                                    tft.print(e.bit.KEY);
                                    tft.drawLine(i * 8 + 112, 90, i * 8 + 120, 90, ILI9341_BLACK);
                                    break;
                                }
                            }
                        }
                    }
                    // 设置闹钟音乐
                    tft.setCursor(30, 95);
                    tft.print(F("Set the music:"));
                    tft.drawLine(112, 105, 120, 105, ILI9341_WHITE);
                    while (1)
                    {
                        customKeypad.tick();
                        if (customKeypad.available())
                        {
                            keypadEvent e = customKeypad.read();
                            delay(10);
                            if (e.bit.EVENT == KEY_JUST_PRESSED)
                            {
                                clockArray[4] = e.bit.KEY;
                                tft.setCursor(112, 95);
                                tft.print(e.bit.KEY);
                                tft.drawLine(112, 105, 120, 105, ILI9341_BLACK);
                                break;
                            }
                        }
                    }
                    tft.setCursor(30, 110);
                    tft.print(F("Press Enter to Continue."));
                    while (1)
                    {
                        customKeypad.tick();
                        if (customKeypad.available())
                        {
                            keypadEvent e = customKeypad.read();
                            delay(10);
                            if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
                            {
                                tft.fillRect(30, 110, 200, 7, ILI9341_BLACK);
                                tft.setCursor(30, 110);
                                tft.setTextColor(ILI9341_GREEN);
                                tft.print(F("Success!"));
                                is_clock = true;
                                // 将闹钟信息写入EEPROM
                                uint8_t hour = min(clockArray[0] * 10 + clockArray[1], 23);
                                uint8_t minute = min(clockArray[2] * 10 + clockArray[3], 59);
                                uint8_t music = clockArray[4];
                                EEPROM.write(0, hour);
                                EEPROM.write(1, minute);
                                EEPROM.write(2, music);
                                delay(2000);
                                tft.fillRect(30, 80, 200, 40, ILI9341_BLACK);
                                break;
                            }
                        }
                    }
                }
                else // 删除闹钟
                {
                    tft.setCursor(20, 160);
                    tft.setTextSize(1);
                    tft.println(F("This will delete all the clocks!"));
                    tft.setCursor(20, 170);
                    tft.println(F("Press Enter to continue."));
                    while (1)
                    {
                        customKeypad.tick();
                        if (customKeypad.available())
                        {
                            keypadEvent e = customKeypad.read();
                            delay(10);
                            if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
                            {
                                for (int i = 0; i < 3; i++)
                                {
                                    EEPROM.write(i, 255);
                                }
                                is_clock = false;
                                tft.fillRect(20, 160, 200, 20, ILI9341_BLACK);
                                break;
                            }
                        }
                    }
                }
            }
            else if (e.bit.KEY == 1 || e.bit.KEY == 3 || e.bit.KEY == 4)
            {
                status = e.bit.KEY;
                goto Label2;
            }
        }
    }
Label2:;
}

void UI_3()
{
    PrintBase(3);

    // 显示歌单
    tft.setTextSize(2);
    tft.setCursor(20, 46);
    tft.println(F("Bad Apple"));
    tft.setCursor(20, 66);
    tft.println(F("Two Tigers"));

    // 光标
    uint8_t cursorPosition = 1;
    PlayCursor(3, cursorPosition, ILI9341_WHITE);
    while (1)
    {
#ifdef IR
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
#endif
        customKeypad.tick();
        if (customKeypad.available())
        {
            keypadEvent e = customKeypad.read();

            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_BLACK);
                cursorPosition--;
                if (cursorPosition == 0)
                {
                    cursorPosition = 2;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_BLACK);
                cursorPosition++;
                if (cursorPosition == 3)
                {
                    cursorPosition = 1;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_GREEN);
                tft.drawRect(15, 245, 210, 34, ILI9341_GREEN);
                tft.setCursor(20, 250);
                tft.setTextSize(3);
                tft.print("Playing...");
                // TODO 播放音乐的操作
                delay(5000);
                tft.fillRect(15, 245, 210, 34, ILI9341_BLACK);
            }
            else if (e.bit.KEY == 1 || e.bit.KEY == 2 || e.bit.KEY == 4)
            {
                status = e.bit.KEY;
                goto Label3;
            }
        }
    }
Label3:;
}

void UI_4()
{
    PrintBase(4);

    // 显示歌单
    tft.setTextSize(2);
    tft.setCursor(20, 46);
    tft.println(F("Bad Apple"));
    tft.setCursor(20, 66);
    tft.println(F("Two Tigers"));

    // 光标
    uint8_t cursorPosition = 1;
    PlayCursor(3, cursorPosition, ILI9341_WHITE);
    while (1)
    {
#ifdef IR
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
#endif
        customKeypad.tick();
        if (customKeypad.available())
        {
            keypadEvent e = customKeypad.read();

            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_BLACK);
                cursorPosition--;
                if (cursorPosition == 0)
                {
                    cursorPosition = 2;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_BLACK);
                cursorPosition++;
                if (cursorPosition == 3)
                {
                    cursorPosition = 1;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, ILI9341_GREEN);
                tft.drawRect(15, 245, 210, 34, ILI9341_GREEN);
                tft.setCursor(20, 250);
                tft.setTextSize(3);
                tft.print("Playing...");
                // 播放"视频"
                tft.fillScreen(ILI9341_BLACK);
                PlayVideo(cursorPosition);
                tft.setCursor(60 , 250);
                tft.println("the end");
                delay(5000);
                tft.fillRect(15, 245, 210, 34, ILI9341_BLACK);
                goto Label4;
            }
            else if (e.bit.KEY == 1 || e.bit.KEY == 2 || e.bit.KEY == 3)
            {
                status = e.bit.KEY;
                goto Label4;
            }
        }
    }
Label4:;
}

void loop()
{
    if (status == 1)
    {
        if (!flag)
        {
            tft.fillScreen(ILI9341_BLACK);
        }
        flag = false;
        Time tim = rtc.time();
        PrintTime(tim);
        UI_1();
    }

    else if (status == 2)
    {
        tft.fillScreen(ILI9341_BLACK);
        UI_2();
    }

    else if (status == 3)
    {
        tft.fillScreen(ILI9341_BLACK);
        UI_3();
    }

    else if (status == 4)
    {
        tft.fillScreen(ILI9341_BLACK);
        UI_4();
    }

    else
    {
        status = 1;
    }
}
