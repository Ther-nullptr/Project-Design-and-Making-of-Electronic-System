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
#include <TMRpcm.h> //! 重要! 要在原库中加入 #define SDFAT(.h和.cpp)
#include <DHT.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

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
#define DHTTYPE DHT11

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

// 通信管脚定义
const int rx = A8;
const int tx = A9;

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

// 传感器管脚定义
const int dhtpin = 44;

// 各个模块高电平和低电平的管脚
const int TFThigh = 4;
const int TFTlow = 3;
const int IRhigh = 12;
const int IRlow = 11;
const int SDhigh = 49;
const int SDlow = 47;
const int Audiohigh = 45;
const int Audiolow = 43;
const int LM35high = 41;
const int LM35low = 29;
const int DHThigh = 37;
const int DHTlow = 35;
const int Clockhigh = 33;
const int Clocklow = 31;

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
int colors[][4] = {
    {ILI9341_BLACK, ILI9341_GREEN, ILI9341_RED, ILI9341_BLUE}, // default
    {0x0000, 0xafe5, 0xffe0, 0x07ff},                          // monokai
    {0x6002, 0x4cc0, 0xc800, 0x3334},                          // ubuntu
    {0x0914, 0x0400, 0x8000, 0x0010},                          // powershell
    {0x314c, 0x07f3, 0xc49f, 0xc786}};                         // cyberpunk
uint8_t colorStatus = 1;                                       // 默认为default
int backgroundColor = colors[0][0];
int successColor = colors[0][1];
int warningColor = colors[0][2];
int infoColor = colors[0][3];

// 设定设备状态的状态值
volatile uint8_t status = 1;

// 控制程序第一次是否清屏
bool flag = true;

// 设置音量
uint8_t volume = 3;

// 器件类定义
DS1302 rtc(RST, DAT, CLK);                                                                      // 时钟模块初始化
SdFat SD;                                                                                       // sd卡对象
Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, sda, sck, rst, blk);                            // tft屏
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS); // 4*4键盘
Adafruit_Image img;                                                                             // 所要读取的图片对象
Adafruit_ImageReader reader(SD);                                                                // 图片读取器对象
ImageReturnCode stat;                                                                           // 图片读取状态
keypadEvent e;                                                                                  // 监测按键状态的对象
TMRpcm tmrpcm;                                                                                  // 读取wav文件的对象
DHT dht(dhtpin, DHTTYPE);                                                                       // 温湿度传感器对象
SoftwareSerial espSerial(rx, tx);                                                               // esp8266窗口对象

// 时间字符串
char date[20], time[10], *week;

// 记录天气的字符串
String json = "";
uint8_t weatherStatus[10];

// 文件设置
char *modeNames[] =
    {
        "START",
        "CLOCK",
        "MUSIC",
        "IMAGE",
        "COLOR",
        "WEATHER",
        "ACSET",
        "GAMES"};

const int musicNum = 4;

char *musicPrintNames[] =
    {
        "1.Bad Apple",
        "2.Two Tigers",
        "3.Lost Rivers",
        "4.JOJO"};

char *musicPlayNames[] =
    {
        "badapple.wav",
        "twotigers.wav",
        "lostrivers.wav",
        "jojo.wav"};

const int imageFolderNum = 4;

char *imagePrintNames[] =
    {
        "1.sight",
        "2.mnist",
        "3.cyberpunk",
        "4.comics"};

char *imagePlayNames[] =
    {
        "/sight/",
        "/mnist/",
        "/cyberpunk/",
        "/comics/",
};

char *cityNames[] =
    {
        "beijing",
        "xining",
        "taiyuan",
        "shanghai",
        "chengdu"};

char *imageBaseName = "/weather/";
const int UINum = 8;

/**********************3.初始状态设定**********************/
void setup()
{
    // 初始化管脚
    pinMode(TFThigh, OUTPUT);
    pinMode(TFTlow, OUTPUT);
    pinMode(IRhigh, OUTPUT);
    pinMode(IRlow, OUTPUT);
    pinMode(SDhigh, OUTPUT);
    //pinMode(SDlow, OUTPUT);
    pinMode(Audiohigh, OUTPUT);
    pinMode(Audiolow, OUTPUT);
    //pinMode(LM35high, OUTPUT);
    pinMode(LM35low, OUTPUT);
    pinMode(DHThigh, OUTPUT);
    pinMode(DHTlow, OUTPUT);
    pinMode(Clockhigh, OUTPUT);
    pinMode(Clocklow, OUTPUT);
    // 控制管脚电平高低
    digitalWrite(TFThigh, HIGH);
    digitalWrite(TFTlow, LOW);
    digitalWrite(IRhigh, HIGH);
    digitalWrite(IRlow, LOW);
    digitalWrite(SDhigh, HIGH);
    //digitalWrite(SDlow, LOW);
    digitalWrite(Audiohigh, HIGH);
    digitalWrite(Audiolow, LOW);
    //digitalWrite(LM35high, HIGH);
    digitalWrite(LM35low, LOW);
    digitalWrite(DHThigh, HIGH);
    digitalWrite(DHTlow, LOW);
    digitalWrite(Clockhigh, HIGH);
    digitalWrite(Clocklow, LOW);

    // !以下的内容在正式启用的时候一定要关掉
    rtc.writeProtect(false); //关闭写保护
    rtc.halt(false);         //清除时钟停止标志
    //Time t(2021, 9, 2, 9, 0, 30, 5); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
    //rtc.time(t);                        //向DS1302设置时32*3间数据

    tft.begin();
    colorStatus = EEPROM.read(9);
    if (colorStatus == CLEAR)
    {
        colorStatus = 1;
    }
    int *p = *(colors + colorStatus - 1);
    backgroundColor = p[0];
    successColor = p[1];
    warningColor = p[2];
    infoColor = p[3];
    tft.fillScreen(backgroundColor);

    SD.begin(SDIN);

    Serial.begin(115200);
    espSerial.begin(9600);

    customKeypad.begin();

    tmrpcm.speakerPin = 46; // 初始化音乐播放管脚为46
    tmrpcm.setVolume(volume);

    // 屏幕主状态初始界面设置
    tft.setTextSize(1);
    // 检测闹钟是否开启
    if (EEPROM.read(0) != CLEAR)
    {
        is_clock = true;
    }

    // 开启传感器
    dht.begin();
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
    for (int i = 0; i < UINum; i++)
    {
        if (val == i + 1)
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
        color = successColor;
    }
    else
    {
        color = warningColor;
    }
    tft.drawRect(5, 5, 2, 4, color);
    tft.drawRect(9, 3, 2, 6, color);
    tft.drawRect(13, 1, 2, 8, color);

    // 画钟表,显示当前是否有闹钟工作
    if (is_clock)
    {
        color = successColor;
    }
    else
    {
        color = warningColor;
    }
    tft.drawCircle(F_W - 6, 6, 4, color);
    tft.drawLine(F_W - 6, 3, F_W - 6, 6, color);
    tft.drawLine(F_W - 6, 6, F_W - 3, 6, color);

    // 画下标
    tft.fillTriangle(230, 308, 230, 316, 234, 312, ILI9341_WHITE);
    tft.fillTriangle(10, 308, 10, 316, 6, 312, ILI9341_WHITE);
    tft.drawLine(0, 305, 240, 305, ILI9341_WHITE);
    TextSettings(ILI9341_WHITE, 1, 111, 308);
    tft.print(String(id) + "/" + String(UINum));

    // 画标题
    if (id == 6)
    {
        TextSettings(ILI9341_WHITE, 3, 55, 20);
    }
    else
    {
        TextSettings(ILI9341_WHITE, 3, 75, 20);
    }
    tft.print(modeNames[id - 1]);
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
    @brief   播放图片和视频
    @param   id      界面的标识
    @param   isDelay 是否延迟(即决定播放图片还是音乐)
*/
/**************************************************************************/
void PlayPhoto(uint8_t id)
{
    uint16_t xbegin = 0;
    uint16_t ybegin = 0;
    String before;
    String after(".bmp");
    String filename;
    before = imagePlayNames[id - 1];

    uint16_t num = 1;
    while (1)
    {
        filename = before + num + after;
        stat = reader.drawBMP(filename.c_str(), tft, xbegin, ybegin);
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
        else
        {
            delay(5000);
        }
        num++;
    }
}

/**************************************************************************/
/*!
    @brief   播放城市天气的子界面
    @param   id      城市(游标)序列
*/
/**************************************************************************/
void PlayCityWeather(int id)
{
    PrintBase(6);

    TextSettings(ILI9341_WHITE, 2, 75, 60);
    tft.println(cityNames[id - 1]);

    stat = reader.drawBMP("temp.bmp", tft, 50, 220);
    TextSettings(ILI9341_WHITE, 2, 65, 220);
    tft.print(F("TEMP:"));
    tft.print(weatherStatus[2 * id - 1]);
    tft.print(F(" C"));

    String imageName = imageBaseName + String(weatherStatus[2 * (id - 1)]) + "@2x.bmp";
    stat = reader.drawBMP(imageName.c_str(), tft, 65, 90);

    if (weatherStatus[2 * (id - 1)] >= 10 && weatherStatus[2 * (id - 1)] <= 18)
    {
        TextSettings(warningColor, 2, 20, 270);
        tft.println("Take an umbrella!"); // suggestion when raining.
    }
}

// 存放游戏类函数的命名空间
// 由于游戏类函数较为繁多，所以压缩为一个命名空间
namespace game
{
    int gameData[4][4]; // 储存游戏的数据
    int score = 0;      // 游戏分数
    int highestscore;   // 最高分数

    enum Direction
    {
        up = 1,
        down,
        left,
        right
    };

    int getDataLength() // 统计当前格子中存在的方块数
    {
        int count = 0;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (gameData[i][j] != 0)
                {
                    count++;
                }
            }
        }
        return count;
    }

    void setNum() // 放置新的方块
    {
        randomSeed(analogRead(A7));
        int random1 = int(2 * (random(1, 2)));              // 获取新生成的方块的值(2或4)
        int random2 = int(random(0, 16 - getDataLength())); // 获取生成位置
        // printf("%d",random2);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (gameData[i][j] == 0)
                {
                    if (random2 == 0)
                    {
                        gameData[i][j] = random1;
                    }
                    random2--;
                }
            }
        }
    }

    void updateScore()
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                score += gameData[i][j];
            }
        }
    }

    void printInfo()
    {
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_WHITE);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                tft.fillRect(50 + j * 40, 120 + i * 40, 24, 24, backgroundColor);
                if (gameData[i][j] != 0)
                {
                    tft.setCursor(50 + j * 40, 120 + i * 40);
                    tft.print(gameData[i][j]);
                }
            }
        }
    }

    void debugPrint()
    {
        for(int i = 0;i<4;i++)
        {
            for(int j = 0;j<4;j++)
            {
                Serial.print(gameData[i][j]);
                Serial.print('\t');
            }
            Serial.println();
        }
        Serial.println("===============");
    }

    void move(enum Direction direction)
    {
        switch (direction)
        {
        case game::Direction::up:
            for (int col = 0; col < 4; col++)
            {
                for (int row = 1; row < 4; row++)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = row - 1;
                        while (tmp >= 0)
                        {
                            if (gameData[tmp][col] == 0)
                            {
                                gameData[tmp][col] = gameData[tmp + 1][col];
                                gameData[tmp + 1][col] = 0;
                            }
                            tmp--;
                        }
                    }
                }
            }
            for (int col = 0; col < 4; col++)
            {
                for (int row = 2; row >= 0; row--)
                {
                    if (gameData[row][col] != 0 && gameData[row][col] == gameData[row + 1][col])
                    {
                        gameData[row][col] *= 2;
                        gameData[row + 1][col] = 0;
                    }
                }
            }
            for (int col = 0; col < 4; col++)
            {
                for (int row = 1; row < 4; row++)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = row - 1;
                        while (tmp >= 0)
                        {
                            if (gameData[tmp][col] == 0)
                            {
                                gameData[tmp][col] = gameData[tmp + 1][col];
                                gameData[tmp + 1][col] = 0;
                            }
                            tmp--;
                        }
                    }
                }
            }
            break;
        case game::Direction::down:
            for (int col = 0; col < 4; col++)
            {
                for (int row = 2; row >= 0; row--)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = row + 1;
                        while (tmp < 4)
                        {
                            if (gameData[tmp][col] == 0)
                            {
                                gameData[tmp][col] = gameData[tmp - 1][col];
                                gameData[tmp - 1][col] = 0;
                            }
                            tmp++;
                        }
                    }
                }
            }
            for (int col = 0; col < 4; col++)
            {
                for (int row = 0; row < 3; row++)
                {
                    if (gameData[row + 1][col] != 0 && gameData[row + 1][col] == gameData[row][col])
                    {
                        gameData[row + 1][col] *= 2;
                        gameData[row][col] = 0;
                    }
                }
            }
            for (int col = 0; col < 4; col++)
            {
                for (int row = 2; row >= 0; row--)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = row + 1;
                        while (tmp < 4)
                        {
                            if (gameData[tmp][col] == 0)
                            {
                                gameData[tmp][col] = gameData[tmp - 1][col];
                                gameData[tmp - 1][col] = 0;
                            }
                            tmp++;
                        }
                    }
                }
            }
            break;
        case game::Direction::left:
            for (int row = 0; row < 4; row++)
            {
                for (int col = 1; col < 4; col++)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = col - 1;
                        while (tmp >= 0)
                        {
                            if (gameData[row][tmp] == 0)
                            {
                                gameData[row][tmp] = gameData[row][tmp + 1];
                                gameData[row][tmp + 1] = 0;
                            }
                            tmp--;
                        }
                    }
                }
            }
            for (int row = 0; row < 4; row++)
            {
                for (int col = 2; col >= 0; col--)
                {
                    if (gameData[row][col] != 0 && gameData[row][col] == gameData[row][col + 1])
                    {
                        gameData[row][col] *= 2;
                        gameData[row][col + 1] = 0;
                    }
                }
            }
            for (int row = 0; row < 4; row++)
            {
                for (int col = 1; col < 4; col++)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = col - 1;
                        while (tmp >= 0)
                        {
                            if (gameData[row][tmp] == 0)
                            {
                                gameData[row][tmp] = gameData[row][tmp + 1];
                                gameData[row][tmp + 1] = 0;
                            }
                            tmp--;
                        }
                    }
                }
            }
            break;
        case game::Direction::right:
            for (int row = 0; row < 4; row++)
            {
                for (int col = 2; col >= 0; col--)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = col + 1;
                        while (tmp < 4)
                        {
                            if (gameData[row][tmp] == 0)
                            {
                                gameData[row][tmp] = gameData[row][tmp - 1];
                                gameData[row][tmp - 1] = 0;
                            }
                            tmp++;
                        }
                    }
                }
            }
            for (int row = 0; row < 4; row++)
            {
                for (int col = 0; col < 3; col++)
                {
                    if (gameData[row][col + 1] != 0 && gameData[row][col + 1] == gameData[row][col])
                    {
                        gameData[row][col + 1] *= 2;
                        gameData[row][col] = 0;
                    }
                }
            }
            for (int row = 0; row < 4; row++)
            {
                for (int col = 2; col >= 0; col--)
                {
                    if (gameData[row][col] != 0)
                    {
                        int tmp = col + 1;
                        while (tmp < 4)
                        {
                            if (gameData[row][tmp] == 0)
                            {
                                gameData[row][tmp] = gameData[row][tmp - 1];
                                gameData[row][tmp - 1] = 0;
                            }
                            tmp++;
                        }
                    }
                }
            }
            break;
        }
    }
}

/**********************5.界面类函数**********************/
void UI_1() // 一号界面,也是初始界面,显示时间
{
    PrintBase(1);
    uint8_t alarmHour = EEPROM.read(0);
    uint8_t alarmMinute = EEPROM.read(1);
    uint8_t alarmMusic = EEPROM.read(2);

    uint8_t h = dht.readHumidity();
    uint8_t t = dht.readTemperature();
    stat = reader.drawBMP("temp.bmp", tft, 10, 170);
    TextSettings(successColor, 2, 30, 170);
    tft.print(F("temperature:"));
    tft.print(t);
    tft.print(F("C"));
    stat = reader.drawBMP("humi.bmp", tft, 10, 190);
    TextSettings(infoColor, 2, 30, 190);
    tft.print(F("humidity:"));
    tft.print(h);
    tft.print(F("%"));

    // 时刻检测时间，但不时刻在屏幕上刷新时间
    while (1)
    {
        // 温度显示
        h = dht.readHumidity();
        t = dht.readTemperature();
        static uint8_t last_temp = 0;
        static uint8_t last_humi = 0;
        if (last_temp != t)
        {
            TextSettings(successColor, 2, 172, 170);
            tft.fillRect(172, 170, 24, 18, backgroundColor);
            tft.print(t);
        }
        last_temp = t;
        if (last_humi != h)
        {
            TextSettings(infoColor, 2, 136, 190);
            tft.fillRect(136, 190, 24, 18, backgroundColor);
            tft.print(h);
        }
        last_humi = h;

        // 时间显示
        static uint16_t last_min = 0;
        Time tim = rtc.time();
        if (tim.hr == alarmHour && tim.min == alarmMinute) //触发闹钟响铃1min
        {
            TextSettings(warningColor, 3, 36, 230);
            tft.print("Alarming!");

            if (!tmrpcm.isPlaying())
            {
                tmrpcm.setVolume(volume);
                tmrpcm.play(musicPlayNames[alarmMusic - 1]);
            }

            // TODO 播放音乐
        }
        else if (tim.hr == alarmHour && tim.min == alarmMinute + 1)
        {
            if (tmrpcm.isPlaying())
            {
                tmrpcm.disable();
                tmrpcm.setVolume(0);
            }
            tft.fillRect(36, 230, 210, 25, backgroundColor);
        }
        if (tim.min != last_min) // 检测到时间发生改变
        {
            tft.fillRect(75 + 3 * W_3, 120, 2 * W_3, H_3, backgroundColor); // 覆盖原有文字
            TextSettings(ILI9341_WHITE, 3, 75 + 3 * W_3, 120);
            snprintf(time, sizeof(time), "%02d", tim.min);
            tft.print(time);
            if (tim.min == 0) // 满小时
            {
                tft.fillRect(75, 120, 2 * W_3, H_3, backgroundColor);
                TextSettings(ILI9341_WHITE, 3, 75, 120);
                snprintf(time, sizeof(time), "%02d", tim.hr);
                tft.print(time);
                if (tim.hr == 0) // 满天
                {
                    tft.fillRect(10, 290, 10 * W_1, H_1, backgroundColor);
                    TextSettings(ILI9341_WHITE, 1, 10, 290);
                    snprintf(date, sizeof(date), "%04d-%02d-%02d", tim.yr, tim.mon, tim.date);
                    tft.print(date);
                    getWeek(tim.day);
                    tft.fillRect(F_W - 3 * W_1 - 10, 290, 3 * W_1, H_1, backgroundColor);
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
            if (e.bit.KEY == BACK && e.bit.EVENT == KEY_JUST_PRESSED)
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
    TextSettings(infoColor, 3, 20, 50);
    tft.print(F("SET CLOCK"));

    TextSettings(warningColor, 3, 20, 130);
    tft.print(F("DEL CLOCK"));

    TextSettings(successColor, 3, 20, 210);
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
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            delay(10);
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, backgroundColor);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, backgroundColor);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
            SetClock:;
                PlayCursor(2, cursorPosition, successColor);
                if (cursorPosition == false) // 设定闹钟
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
                                    tft.drawLine(i * 8 + 112, 90, i * 8 + 120, 90, backgroundColor);
                                    // 对闹钟合法性的判定
                                    if (i == 3)
                                    {
                                        if (clockArray[0] * 10 + clockArray[1] > 23 || clockArray[2] * 10 + clockArray[3] > 60)
                                        {
                                            TextSettings(warningColor, 1, 30, 95);
                                            tft.print("illegal time!");
                                            delay(2000);
                                            tft.fillRect(30, 80, 200, 40, backgroundColor);
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
                                tft.drawLine(112, 105, 120, 105, backgroundColor);
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
                                tft.fillRect(30, 110, 200, 7, backgroundColor);
                                TextSettings(successColor, 1, 30, 110);
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
                                tft.fillRect(30, 80, 200, 40, backgroundColor);
                                tft.fillRect(40, 240, 200, 40, backgroundColor);
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
                    TextSettings(warningColor, 1, 20, 160);
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
                                TextSettings(successColor, 1, 20, 180);
                                tft.println(F("Success!"));
                                delay(2000);
                                for (uint8_t i = 0; i < 3; i++)
                                {
                                    EEPROM.write(i, CLEAR);
                                }
                                is_clock = false;
                                tft.fillRect(20, 160, 200, 30, backgroundColor);
                                tft.fillRect(40, 240, 200, 20, backgroundColor);
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
    TextSettings(infoColor, 1, 170, 3);
    tft.print(F("volume:"));
    tft.print(volume);

    // 显示歌单
    TextSettings(ILI9341_WHITE, 2, 20, 46);
    for (int i = 0; i < 4; i++)
    {
        tft.println(musicPrintNames[i]);
        tft.setCursor(20, 66 + i * 20);
    }

    // 光标
    uint8_t cursorPosition = 1;
    PlayCursor(3, cursorPosition, ILI9341_WHITE);
    while (1)
    {
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition--;
                if (cursorPosition == 0)
                {
                    cursorPosition = musicNum;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition++;
                if (cursorPosition == musicNum + 1)
                {
                    cursorPosition = 1;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == LEFT && e.bit.EVENT == KEY_JUST_PRESSED) // 调节音量(但实际上也和音色有关)
            {
                volume--;
                if (volume == 0)
                {
                    volume = 1;
                }
                tmrpcm.volume(0);
                tft.fillRect(212, 1, W_1 + 1, H_1 + 1, backgroundColor);
                TextSettings(infoColor, 1, 212, 3);
                tft.println(volume);
            }
            else if (e.bit.KEY == RIGHT && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                volume++;
                if (volume == 8)
                {
                    volume = 7;
                }
                tmrpcm.volume(1);
                tft.fillRect(212, 1, W_1 + 1, H_1 + 1, backgroundColor);
                TextSettings(infoColor, 1, 212, 3);
                tft.println(volume);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, successColor);
                tft.drawRect(15, 245, 210, 34, successColor);
                TextSettings(ILI9341_WHITE, 3, 20, 250);
                tft.print("Loading...");
                // TODO 播放音乐的操作
                if (tmrpcm.isPlaying()) // 如果正在播放,
                {
                    tmrpcm.disable(); // 就关闭当前音乐
                }
                tmrpcm.setVolume(volume);
                tmrpcm.play(musicPlayNames[cursorPosition - 1]);
                delay(3000);
                tft.fillRect(15, 245, 210, 34, backgroundColor);
            }
            else if (willChangeStatus(e.bit.KEY, 3))
            {
                status = e.bit.KEY;
                if (tmrpcm.isPlaying())
                {
                    tmrpcm.setVolume(0);
                    tmrpcm.stopPlayback();
                    tmrpcm.disable(); //每次跳出该界面都会关闭音乐
                }
                goto Label3;
            }
        }
    }
Label3:;
}

// 播放视频的界面
void UI_4()
{
    PrintBase(4);

    // 显示相册
    TextSettings(ILI9341_WHITE, 2, 20, 46);
    for (int i = 0; i < 4; i++)
    {
        tft.println(imagePrintNames[i]);
        tft.setCursor(20, 66 + i * 20);
    }

    // 光标
    uint8_t cursorPosition = 1;
    PlayCursor(3, cursorPosition, ILI9341_WHITE);
    while (1)
    {
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition--;
                if (cursorPosition == 0)
                {
                    cursorPosition = imageFolderNum;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition++;
                if (cursorPosition == imageFolderNum + 1)
                {
                    cursorPosition = 1;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, successColor);
                tft.drawRect(15, 245, 210, 34, successColor);
                TextSettings(ILI9341_WHITE, 3, 20, 250);
                tft.print(F("Playing..."));
                // 播放"视频"
                tft.fillScreen(backgroundColor);
                PlayPhoto(cursorPosition);
                tft.setCursor(60, 250);
                tft.println("the end");
                delay(5000);
                tft.fillRect(15, 245, 210, 34, backgroundColor);
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

void UI_5()
{
    PrintBase(5);

    // 显示主题
    TextSettings(ILI9341_WHITE, 2, 20, 46);
    tft.println(F("1.Default"));
    tft.setCursor(20, 66);
    tft.println(F("2.Monokai"));
    tft.setCursor(20, 86);
    tft.println(F("3.Ubuntu"));
    tft.setCursor(20, 106);
    tft.println(F("4.Powershell"));
    tft.setCursor(20, 126);
    tft.println(F("5.Cyberpunk"));
    tft.setCursor(20, 170);
    tft.println(F("Now Theme:"));
    tft.setCursor(180, 170);
    tft.println(colorStatus);

    // 光标
    uint8_t cursorPosition = 1;
    PlayCursor(3, cursorPosition, ILI9341_WHITE);

    while (1)
    {
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition--;
                if (cursorPosition == 0)
                {
                    cursorPosition = 5;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, backgroundColor);
                cursorPosition++;
                if (cursorPosition == 6)
                {
                    cursorPosition = 1;
                }
                PlayCursor(3, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(3, cursorPosition, successColor);
                colorStatus = cursorPosition;
                EEPROM.write(9, cursorPosition);
                goto Label5;
            }
            else if (willChangeStatus(e.bit.KEY, 5))
            {
                status = e.bit.KEY;
                goto Label5;
            }
        }
    }
Label5:;
    int *p = *(colors + colorStatus - 1);
    backgroundColor = p[0];
    successColor = p[1];
    warningColor = p[2];
    infoColor = p[3];
}

void UI_6()
{
    PrintBase(6);
    uint8_t cursorPosition = 1;
    bool getStatus = false; // 判断是否接受到了字符串

    // 显示主题
    TextSettings(ILI9341_WHITE, 2, 20, 46);

    while (1)
    {
        // 数据请求
        static int lastTime = millis();
        //Serial.println(espSerial.available());
        if (espSerial.available() > 0)
        {
            json = "{\"0\":";
            json += espSerial.readString();
            json += "}";
            Serial.println(json);
        }

        // 数据解析
        DynamicJsonDocument doc(1024);                           //声明一个JsonDocument对象
        DeserializationError error = deserializeJson(doc, json); //反序列化JSON数据
        if (!error)
        {
            weatherStatus[0] = doc["0"];
            weatherStatus[1] = doc["1"];
            weatherStatus[2] = doc["2"];
            weatherStatus[3] = doc["3"];
            weatherStatus[4] = doc["4"];
            weatherStatus[5] = doc["5"];
            weatherStatus[6] = doc["6"];
            weatherStatus[7] = doc["7"];
            weatherStatus[8] = doc["8"];
            weatherStatus[9] = doc["9"];
        }
        if (weatherStatus[1] == 0)
        {
            getStatus = false;
            TextSettings(warningColor, 3, 20, 160);
            tft.println("loading...");
            customKeypad.tick();
            if (customKeypad.available())
            {
                e = customKeypad.read();
                if (willChangeStatus(e.bit.KEY, 6))
                {
                    status = e.bit.KEY;
                    goto Label6;
                }
            }
        }
        else
        {
            is_Wifi = true;
            if (!getStatus)
            {
                tft.fillRect(20, 160, 200, 25, backgroundColor); // 覆盖原有的提示
                getStatus = true;
            }
            PlayCityWeather(cursorPosition);

            customKeypad.tick();
            if (customKeypad.available())
            {
                e = customKeypad.read();
                if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    cursorPosition--;
                    if (cursorPosition == 0)
                    {
                        cursorPosition = 5;
                    }
                    tft.fillScreen(backgroundColor);
                }
                else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    cursorPosition++;
                    if (cursorPosition == 6)
                    {
                        cursorPosition = 1;
                    }
                    tft.fillScreen(backgroundColor);
                }
                else if (willChangeStatus(e.bit.KEY, 6))
                {
                    status = e.bit.KEY;
                    goto Label6;
                }
            }
        }
        json = "";
    }

Label6:;
}

void UI_7()
{
    PrintBase(7);
    TextSettings(infoColor, 3, 20, 50);
    tft.print(F("SET AC"));

    TextSettings(warningColor, 3, 20, 130);
    tft.print(F("DEL AC"));

    TextSettings(successColor, 3, 20, 210);
    tft.print(F("AC STATUS"));

    TextSettings(ILI9341_WHITE, 2, 40, 240);
    if (EEPROM.read(3) == CLEAR) // 当前物联网系统没有控制空调
    {
        tft.print(F("Closed"));
    }
    else
    {
        TextSettings(infoColor, 2, 40, 240);
        String msg = "goal temp: " + String(EEPROM.read(4)) + " C";
        tft.print(msg);
        TextSettings(successColor, 2, 40, 260);
        msg = "AC temp: " + String(EEPROM.read(4) - 2) + " C";
        tft.print(msg);
    }

    bool cursorPosition = 0;
    PlayCursor(2, cursorPosition, ILI9341_WHITE);
    while (1)
    {
        customKeypad.tick();
        if (customKeypad.available())
        {
            e = customKeypad.read();
            delay(10);
            if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, backgroundColor);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
            {
                PlayCursor(2, cursorPosition, backgroundColor);
                cursorPosition = !cursorPosition;
                PlayCursor(2, cursorPosition, ILI9341_WHITE);
            }
            else if (e.bit.KEY == ENTER && e.bit.EVENT == KEY_JUST_PRESSED)
            {
            SetTemperature:;
                PlayCursor(2, cursorPosition, successColor);
                if (cursorPosition == 0) // 设定控制空调的系统是否打开
                {
                    TextSettings(ILI9341_WHITE, 1, 30, 80);
                    tft.print(F("Set the goal temperature:"));
                    uint8_t tempArray[2];
                    // 设置闹钟时间
                    for (uint8_t i = 0; i < 2; i++)
                    {
                        tft.drawLine(i * 8 + 192, 90, i * 8 + 200, 90, ILI9341_WHITE);
                        while (1)
                        {
                            customKeypad.tick();
                            if (customKeypad.available())
                            {
                                e = customKeypad.read();
                                delay(10);
                                if (e.bit.EVENT == KEY_JUST_PRESSED && IsNumber(e.bit.KEY))
                                {
                                    tempArray[i] = e.bit.KEY;
                                    tft.setCursor(i * 8 + 192, 80);
                                    tft.print(e.bit.KEY);
                                    tft.drawLine(i * 8 + 192, 90, i * 8 + 200, 90, backgroundColor);
                                    // 对温度合法性的判定，此处为便于管理，将温度设置为20~30 C
                                    if (i == 1)
                                    {
                                        if (tempArray[0] * 10 + tempArray[1] < 20 || tempArray[0] * 10 + tempArray[1] > 30)
                                        {
                                            TextSettings(warningColor, 1, 20, 95);
                                            tft.print("illegal temp (must 20 ~ 30 c)");
                                            delay(2000);
                                            tft.fillRect(20, 80, 200, 40, backgroundColor);
                                            goto SetTemperature;
                                        }
                                    }
                                    break;
                                }
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
                                tft.fillRect(30, 110, 200, 7, backgroundColor);
                                TextSettings(successColor, 1, 30, 110);
                                tft.print(F("Success!"));
                                delay(2000);
                                // 将温度信息写入EEPROM
                                uint8_t temp = tempArray[0] * 10 + tempArray[1];
                                EEPROM.write(3, 1);
                                EEPROM.write(4, temp);
                                // 打印温度设置
                                tft.fillRect(30, 80, 200, 40, backgroundColor);
                                tft.fillRect(40, 240, 200, 40, backgroundColor);
                                TextSettings(infoColor, 2, 40, 240);
                                String msg = "goal temp: " + String(EEPROM.read(4)) + " C";
                                tft.print(msg);
                                TextSettings(successColor, 2, 40, 260);
                                msg = "AC temp: " + String(EEPROM.read(4) - 2) + " C";
                                tft.print(msg);
                                Serial.println(temp);
                                // 向esp8266发送数据
                                espSerial.println(temp);
                                break;
                            }
                        }
                    }
                }
                else // 删除闹钟
                {
                    TextSettings(warningColor, 1, 20, 160);
                    tft.println(F("This will shutdown the AC system!"));
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
                                TextSettings(successColor, 1, 20, 180);
                                tft.println(F("Success!"));
                                delay(2000);
                                for (uint8_t i = 3; i < 4; i++)
                                {
                                    EEPROM.write(i, CLEAR);
                                }
                                tft.fillRect(20, 160, 200, 30, backgroundColor);
                                tft.fillRect(40, 240, 200, 40, backgroundColor);
                                // 向esp8266发送数据(当然,一般不会把空调阈值温度设置为0)
                                espSerial.println(0);
                                break;
                            }
                        }
                    }
                }
            }
            else if (willChangeStatus(e.bit.KEY, 7))
            {
                status = e.bit.KEY;
                goto Label7;
            }
        }
    }
Label7:;
}

void UI_8()
{
StartGame:;
    PrintBase(8);
    // 游戏说明
    TextSettings(ILI9341_WHITE, 1, 20, 50);
    tft.print(F("A: up"));
    TextSettings(ILI9341_WHITE, 1, 20, 65);
    tft.print(F("B: down"));
    TextSettings(ILI9341_WHITE, 1, 20, 80);
    tft.print(F("C: left"));
    TextSettings(ILI9341_WHITE, 1, 20, 95);
    tft.print(F("D: right"));

    // 历史记录
    game::highestscore = EEPROM.read(5);
    if (game::highestscore == CLEAR)
    {
        game::highestscore = 0;
    }
    TextSettings(infoColor, 1, 20, 280);
    tft.print(F("score: "));
    TextSettings(successColor, 1, 20, 290);
    tft.print(F("highest score: "));
    tft.print(game::highestscore);

    // 游戏运行界面
    tft.drawRect(40, 110, 160, 160, ILI9341_WHITE);
    while (1)
    {
        while (1)
        {
            customKeypad.tick();
            if (customKeypad.available())
            {
                game::setNum();
                e = customKeypad.read();
                delay(100);
                if (e.bit.KEY == UP && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    game::move(game::Direction::up);
                }
                else if (e.bit.KEY == DOWN && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    game::move(game::Direction::down);
                }
                else if (e.bit.KEY == LEFT && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    game::move(game::Direction::left);
                }
                else if (e.bit.KEY == RIGHT && e.bit.EVENT == KEY_JUST_PRESSED)
                {
                    game::move(game::Direction::right);
                }
                else if (willChangeStatus(e.bit.KEY, 8))
                {
                    status = e.bit.KEY;
                    goto Label8;
                }
                game::debugPrint();
                game::printInfo();
                break;
            }
        }
        // 游戏分数界面
        tft.fillRect(56, 280, 40, 15, backgroundColor);
        TextSettings(infoColor, 1, 56, 280);
        game::updateScore();
        tft.print(game::score);
        delay(100);
        game::score = 0;
        if(game::getDataLength()==16)
        {
            TextSettings(warningColor, 2, 20, 160);
            tft.println("You Lose!");
            tft.fillScreen(backgroundColor);
            goto StartGame;
        }
    }
Label8:;
    memset(game::gameData, 0, sizeof(game::gameData));
    if(game::score > game::highestscore)
    {
        // 更新最高分数记录
        game::highestscore = game::score;
        EEPROM.write(5,game::highestscore);
        game::score = 0;
    }
}

/**************************6.主循环界面**************************/
void loop()
{
    if (status == 1)
    {
        if (!flag)
        {
            tft.fillScreen(backgroundColor);
        }
        flag = false;
        Time tim = rtc.time();
        PrintTime(tim);
        UI_1();
        if (tmrpcm.isPlaying())
        {
            tmrpcm.disable();
        }
    }

    else if (status == 2)
    {
        tft.fillScreen(backgroundColor);
        UI_2();
    }

    else if (status == 3)
    {
        tft.fillScreen(backgroundColor);
        UI_3();
    }

    else if (status == 4)
    {
        tft.fillScreen(backgroundColor);
        UI_4();
    }

    else if (status == 5)
    {
        tft.fillScreen(backgroundColor);
        UI_5();
    }

    else if (status == 6)
    {
        tft.fillScreen(backgroundColor);
        UI_6();
    }

    else if (status == 7)
    {
        tft.fillScreen(backgroundColor);
        UI_7();
    }

    else if(status == 8)
    {
        // 清空游戏数组
        memset(game::gameData, 0, sizeof(game::gameData));
        tft.fillScreen(backgroundColor);
        UI_8();
    }

    else
    {
        Serial.println("a error happens");
        //status = 1;
    }
}
