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
#include <TMRpcm.h>               //! 重要! 要在原库中加入 #define SDFAT(.h和.cpp)
#include "IRKeyPad.h"

/**********************0.宏定义**********************/
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
#define CLEAR 255

// 定义键盘上的其他数字
#define UP 11
#define DOWN 12
#define LEFT 13
#define RIGHT 14
#define ENTER 15
#define BACK 16

/**********************1.管脚定义**********************/
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

// sd管脚定义
const int SDIN = 53;
// 50:miso 51:mosi 52:sck

// TFT管脚定义
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

/**********************2.状态与全局类定义**********************/

// 设定设备状态的bool值
bool is_clock = false; // 是否设定闹钟
bool is_Wifi = false;  // 是否连接wifi

// 设定系统颜色


// 设定设备状态的状态值
volatile uint8_t status = 1;
const uint8_t idList[] = {1, 2, 3, 4};

// 控制程序第一次是否清屏
bool flag = true;

// 器件类定义
DS1302 rtc(RST, DAT, CLK);                                                                      // 时钟模块初始化
SdFat SD;                                                                                       // sd卡对象
IRrecv irrecv(RECV_PIN);                                                                        // 红外接收器
decode_results results;                                                                         // 红外信号解码值
Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);                            // tft屏
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // 4*4键盘
Adafruit_Image img;                                                                             // 所要读取的图片对象
Adafruit_ImageReader reader(SD);                                                                // 图片读取器对象
ImageReturnCode stat;                                                                           // 图片读取状态
keypadEvent e;                                                                                  // 监测按键状态的对象
TMRpcm tmrpcm;                                                                                  // 读取wav文件的对象

// 时间字符串
char date[20], time[10], *week;

/**********************3.初始状态设定**********************/
void setup()
{
    // !以下的内容在正式启用的时候一定要关掉
    rtc.writeProtect(false);            //关闭写保护
    rtc.halt(false);                    //清除时钟停止标志
    Time t(2021, 7, 24, 23, 59, 50, 7); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
    rtc.time(t);                        //向DS1302设置时32*3间数据

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

    SD.begin(SDIN);

    Serial.begin(9600);

    irrecv.enableIRIn();

    customKeypad.begin();

    tmrpcm.speakerPin = 46; // 初始化音乐播放管脚为11
    tmrpcm.setVolume(3);

    // 屏幕主状态初始界面设置
    tft.setTextSize(1);
    // 检测闹钟是否开启
    if (EEPROM.read(0) != CLEAR)
    {
        is_clock = true;
    }
}

/**********************4.功能类函数**********************/
/**************************************************************************/
/*!
    @brief   判断是否应该切换界面
    @param   val 键盘输入值
    @param   id  当前所在页面
    @return  true/false 是否切换界面
*/
/**************************************************************************/
bool willChangeStatus(uint8_t val, uint8_t id)
{
    // 1.按键值与当前界面相同
    if (val == id)
    {
        return false;
    }

    // 2.按键值与当前界面不同且合法
    for (int i = 0; i < 4; i++)
    {
        if (val == idList[i])
        {
            return true;
        }
    }

    // 3.按键值与当前界面不同且非法
    return false;
}

/**************************************************************************/
/*!
    @brief   判断键盘输入值是否为数字
    @param   val 键盘输入值
    @return  true/false 是否为数字
*/
/**************************************************************************/
bool IsNumber(uint8_t val)
{
    for (uint8_t i = 0; i < 10; i++)
    {
        if (val == i)
        {
            return true;
        }
    }
    return false;
}

/**************************************************************************/
/*!
    @brief   将打印文字时的设置压缩为一条语句,既便于阅读,也免于遗漏
*/
/**************************************************************************/
void TextSettings(int color, uint8_t fontsize, uint16_t x, uint16_t y)
{
    tft.setTextColor(color);
    tft.setTextSize(fontsize);
    tft.setCursor(x, y);
}

/**************************************************************************/
/*!
    @brief   将时钟模块代表星期的数字转为字符串
*/
/**************************************************************************/
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

/**************************************************************************/
/*!
    @brief   在首次加载界面时打印时间
    @param   tim   时间对象
*/
/**************************************************************************/
void PrintTime(Time &tim) // 打印初始界面
{
    snprintf(date, sizeof(date), "%04d-%02d-%02d",
             tim.yr, tim.mon, tim.date);
    snprintf(time, sizeof(time), "%02d:%02d",
             tim.hr, tim.min);
    getWeek(tim.day);
    TextSettings(ILI9341_WHITE, 3, 75, 120);
    tft.print(time);
    TextSettings(ILI9341_WHITE, 1, 10, 290);
    tft.print(date);
    TextSettings(ILI9341_WHITE, 1, F_W - 3 * W_1 - 10, 290);
    tft.print(week);
}

/**************************************************************************/
/*!
    @brief   打印每个页面的基础内容,除非界面改变,否则不会刷新
    @param   id   界面的标识
*/
/**************************************************************************/
void PrintBase(uint8_t id) // 打印每个界面的共性物
{
    int color;
    // 画信号
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

    // 画钟表,显示当前是否有闹钟工作
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

    // 画下标
    tft.fillTriangle(230, 308, 230, 316, 234, 312, ILI9341_WHITE);
    tft.fillTriangle(10, 308, 10, 316, 6, 312, ILI9341_WHITE);
    tft.drawLine(0, 305, 240, 305, ILI9341_WHITE);
    TextSettings(ILI9341_WHITE, 1, 111, 308);
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

    // 画标题
    TextSettings(ILI9341_WHITE, 3, 75, 20);
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
        tft.print(F("IMAGE"));
        break;
    }
}

/**************************************************************************/
/*!
    @brief   打印光标
    @param   id    界面的标识
    @param   i     光标位置
    @param   color 光标颜色
*/
/**************************************************************************/
void PlayCursor(uint8_t id, uint8_t i, int color)
{
    if (id == 2)
    {
        tft.fillTriangle(10, 58 + i * 80, 10, 66 + i * 80, 14, 62 + i * 80, color);
    }
    else if (id == 3 || id == 4)
    {
        tft.fillTriangle(10, 29 + i * 20, 10, 37 + i * 20, 14, 33 + i * 20, color);
    }
}

/**************************************************************************/
/*!
    @brief   播放视频(图片)
    @param   id    界面的标识
    @param   i     光标位置
    @param   color 光标颜色
*/
/**************************************************************************/
void PlayVideo(uint8_t id)
{
    String before;
    String after(".bmp");
    String filename;

    switch (id)
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
        if (stat != IMAGE_SUCCESS) // 若获取图片失败
        {
            break;
        }
        // TODO 此处应有更好的优化方法:把esc设置为中断
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            if (e.bit.KEY == BACK && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                break;
            }
        }
        num++;
    }
}

/**********************5.界面类函数**********************/
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
            TextSettings(ILI9341_RED, 3, 36, 180);
            tft.print("Alarming!");
            if (!tmrpcm.isPlaying())
            {
                if (alarmMusic == 1)
                {
                    tmrpcm.play("demo.wav");
                }
                else if (alarmMusic == 2)
                {
                    tmrpcm.play("twotigers.wav");
                }
            }
            // TODO 播放音乐
        }
        else if (tim.hr == alarmHour && tim.min == alarmMinute + 1)
        {
            if (tmrpcm.isPlaying())
            {
                tmrpcm.disable();
            }
            tft.fillRect(36, 180, 210, 25, ILI9341_BLACK);
        }
        if (tim.min != last_min) // 检测到时间发生改变
        {
            tft.fillRect(75 + 3 * W_3, 120, 2 * W_3, H_3, ILI9341_BLACK); // 覆盖原有文字
            TextSettings(ILI9341_WHITE, 3, 75 + 3 * W_3, 120);
            snprintf(time, sizeof(time), "%02d", tim.min);
            tft.print(time);
            if (tim.min == 0) // 满小时
            {
                tft.fillRect(75, 120, 2 * W_3, H_3, ILI9341_BLACK);
                TextSettings(ILI9341_WHITE, 3, 75, 120);
                snprintf(time, sizeof(time), "%02d", tim.hr);
                tft.print(time);
                if (tim.hr == 0) // 满天
                {
                    tft.fillRect(10, 290, 10 * W_1, H_1, ILI9341_BLACK);
                    TextSettings(ILI9341_WHITE, 1, 10, 290);
                    snprintf(date, sizeof(date), "%04d-%02d-%02d", tim.yr, tim.mon, tim.date);
                    tft.print(date);
                    getWeek(tim.day);
                    tft.fillRect(F_W - 3 * W_1 - 10, 290, 3 * W_1, H_1, ILI9341_BLACK);
                    TextSettings(ILI9341_WHITE, 1, F_W - 3 * W_1 - 10, 290);
                    tft.print(week);
                }
            }
        }
        last_min = tim.min; // 更新时间

        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            status = e.bit.KEY;
            if(e.bit.KEY == BACK && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                tmrpcm.disable(); // 按下back键即可关闭闹钟
            }
            else if (willChangeStatus(status, 1))
            {
                goto Label1;
            }
        }
    }
Label1:;
}

void UI_2() // 二号界面,闹钟的设置,删除和展示
{
    PrintBase(2);
    TextSettings(ILI9341_BLUE, 3, 20, 50);
    tft.print(F("SET CLOCK"));

    TextSettings(ILI9341_RED, 3, 20, 130);
    tft.print(F("DEL CLOCK"));

    TextSettings(ILI9341_GREEN, 3, 20, 210);
    tft.print(F("MY CLOCK"));

    TextSettings(ILI9341_WHITE, 2, 40, 240);
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
            e = customKeypad.read();
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
            SetClock:;
                PlayCursor(2, cursorPosition, ILI9341_GREEN);
                if (cursorPosition == 0) // 设定闹钟
                {
                    TextSettings(ILI9341_WHITE, 1, 30, 80);
                    tft.print(F("Set the time:"));
                    uint8_t clockArray[5];
                    // 设置闹钟时间
                    for (uint8_t i = 0; i < 4; i++)
                    {
                        tft.drawLine(i * 8 + 112, 90, i * 8 + 120, 90, ILI9341_WHITE);
                        while (1)
                        {
                            customKeypad.tick();
                            if (customKeypad.available())
                            {
                                e = customKeypad.read();
                                delay(10);
                                if (e.bit.EVENT == KEY_JUST_PRESSED && IsNumber(e.bit.KEY))
                                {
                                    clockArray[i] = e.bit.KEY;
                                    tft.setCursor(i * 8 + 112, 80);
                                    tft.print(e.bit.KEY);
                                    tft.drawLine(i * 8 + 112, 90, i * 8 + 120, 90, ILI9341_BLACK);
                                    // 对闹钟合法性的判定
                                    if (i == 3)
                                    {
                                        if (clockArray[0] * 10 + clockArray[1] > 23 || clockArray[2] * 10 + clockArray[3] > 60)
                                        {
                                            TextSettings(ILI9341_RED, 1, 30, 95);
                                            tft.print("illegal time!");
                                            delay(2000);
                                            tft.fillRect(30, 80, 200, 40, ILI9341_BLACK);
                                            goto SetClock;
                                        }
                                    }
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
                            e = customKeypad.read();
                            delay(10);
                            if (e.bit.EVENT == KEY_JUST_PRESSED && IsNumber(e.bit.KEY))
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
                            e = customKeypad.read();
                            delay(10);
                            if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
                            {
                                tft.fillRect(30, 110, 200, 7, ILI9341_BLACK);
                                TextSettings(ILI9341_GREEN, 1, 30, 110);
                                tft.print(F("Success!"));
                                delay(2000);
                                // 将闹钟信息写入EEPROM
                                uint8_t hour = clockArray[0] * 10 + clockArray[1];
                                uint8_t minute = clockArray[2] * 10 + clockArray[3];
                                uint8_t music = clockArray[4];
                                EEPROM.write(0, hour);
                                EEPROM.write(1, minute);
                                EEPROM.write(2, music);
                                // 打印闹钟时间
                                is_clock = true;
                                char clockString[25];
                                tft.fillRect(30, 80, 200, 40, ILI9341_BLACK);
                                tft.fillRect(40, 240, 200, 40, ILI9341_BLACK);
                                TextSettings(ILI9341_WHITE, 2, 40, 240);
                                snprintf(clockString, sizeof(clockString), "%02d:%02d  music %d", hour, minute, music);
                                tft.print(clockString);
                                break;
                            }
                        }
                    }
                }
                else // 删除闹钟
                {
                    TextSettings(ILI9341_RED, 1, 20, 160);
                    tft.println(F("This will delete all the clocks!"));
                    tft.setCursor(20, 170);
                    tft.println(F("Press Enter to continue."));
                    while (1)
                    {
                        customKeypad.tick();
                        if (customKeypad.available())
                        {
                            e = customKeypad.read();
                            delay(10);
                            if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
                            {
                                TextSettings(ILI9341_GREEN, 1, 20, 180);
                                tft.println(F("Success!"));
                                delay(2000);
                                for (uint8_t i = 0; i < 3; i++)
                                {
                                    EEPROM.write(i, 255);
                                }
                                is_clock = false;
                                tft.fillRect(20, 160, 200, 30, ILI9341_BLACK);
                                tft.fillRect(40, 240, 200, 20, ILI9341_BLACK);
                                break;
                            }
                        }
                    }
                }
            }
            else if (willChangeStatus(e.bit.KEY, 2))
            {
                status = e.bit.KEY;
                goto Label2;
            }
        }
    }
Label2:;
}

// 播放音乐的界面
void UI_3()
{
    PrintBase(3);

    // 显示歌单
    TextSettings(ILI9341_WHITE, 2, 20, 46);
    tft.println(F("1.Bad Apple"));
    tft.setCursor(20, 66);
    tft.println(F("2.Two Tigers"));

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
            e = customKeypad.read();
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
                TextSettings(ILI9341_WHITE, 3, 20, 250);
                tft.print("Loading...");
                // TODO 播放音乐的操作
                if(tmrpcm.isPlaying()) // 如果正在播放,
                {
                    tmrpcm.disable(); // 就关闭当前音乐
                }
                if (cursorPosition == 1)
                {
                    tmrpcm.play("demo.wav");
                }
                else if (cursorPosition == 2)
                {
                    tmrpcm.play("twotigers.wav");
                }
                delay(5000);
                tft.fillRect(15, 245, 210, 34, ILI9341_BLACK);
            }
            else if (willChangeStatus(e.bit.KEY, 3))
            {
                status = e.bit.KEY;
                goto Label3;
            }
        }
    }
Label3:;
tmrpcm.disable();//每次跳出该界面都会关闭音乐
}

// 播放视频的界面
void UI_4()
{
    PrintBase(4);

    // 显示歌单
    TextSettings(ILI9341_WHITE, 2, 20, 46);
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
            e = customKeypad.read();
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
                TextSettings(ILI9341_WHITE, 3, 20, 250);
                tft.print(F("Playing..."));
                // 播放"视频"
                tft.fillScreen(ILI9341_BLACK);
                PlayVideo(cursorPosition);
                tft.setCursor(60, 250);
                tft.println("the end");
                delay(5000);
                tft.fillRect(15, 245, 210, 34, ILI9341_BLACK);
                goto Label4;
            }
            else if (willChangeStatus(e.bit.KEY, 4))
            {
                status = e.bit.KEY;
                goto Label4;
            }
        }
    }
Label4:;
}

/**************************6.主循环界面**************************/
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
