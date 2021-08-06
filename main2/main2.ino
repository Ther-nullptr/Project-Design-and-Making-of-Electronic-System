#define BLINKER_WIFI

#include <Blinker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

/****************1.基本常量******************/
char auth[] = "29fa581e93d0"; //从blinker应用上得到的设备密钥
char ssid[] = "YSJJ";         //wifi名
char pswd[] = "wangzy1222";   //wifi密码

const uint8_t kIrLed = 4; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint8_t one_wire_bus = D7;

/****************2.全局对象******************/
// 全局变量
uint8_t temp_read = 0;
uint8_t goalTemp = 28; // 室内想要达到的温度
uint8_t ACTemp = 25;
bool power = false;
uint8_t fanSpeed = 0; // 0为自动,1-3风速依次增加


// 新建组件对象
BlinkerNumber POWER("power");         // blinker需要监视的对象(开关)
BlinkerNumber TEMP("temp");           // blinker需要监视的对象(温度)
BlinkerNumber AC_TEMP("AC_temp");     // blinker需要监视的对象(空调温度)
BlinkerNumber FANSPEED("fanSpeed");   // blinker需要监视的对象(风速)
OneWire oneWire(one_wire_bus);        // 初始连接在单总线上的单总线设备
DallasTemperature sensors(&oneWire);  // 温度控制器对象
IRGreeAC ac(kIrLed);                  // Set the GPIO to be used for sending messages.


/****************3.函数定义******************/
// 心跳包函数,用于将信息发送给app
void heartbeat()
{
    POWER.print(power);
    TEMP.print(temp_read);
    AC_TEMP.print(ACTemp);
    FANSPEED.print(fanSpeed);
}

// 获取空调状态
/*
void printState()
{
    // Display the settings.
    Serial.println("GREE A/C remote is in the following state:");
    Serial.printf("  %s\n", ac.toString().c_str());
    /*
    // Display the encoded IR sequence.
    unsigned char *ir_code = ac.getRaw();
    Serial.print("IR Code: 0x");
    for (uint8_t i = 0; i < kGreeStateLength; i++)
        Serial.printf("%02X", ir_code[i]);
    Serial.println();
    */

// 控制空调,并更新空调状态
void AC_control() 
{
    // 可调节设置
    if (power)
    {
        ac.on();
    }
    else
    {
        ac.off();
    }
    ac.setFan(fanSpeed);
    ac.setTemp(ACTemp); // 16-30C

    // 默认设置
    ac.setSwingVertical(true, kGreeSwingAuto);
    ac.setMode(kGreeCool); // kGreeAuto, kGreeDry, kGreeCool, kGreeFan, kGreeHeat
    ac.setXFan(false);
    ac.setLight(true);
    ac.setSleep(false);
    ac.setTurbo(false);

    // 发送信号
    ac.send();
}

void setup()
{
    Serial.begin(115200);

    // Blinker组件初始化
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachHeartbeat(heartbeat);

    // 传感器初始化
    sensors.begin();

    // 空调状态设置
    ac.begin();
}

void loop()
{
    static unsigned long time = 0;
    Blinker.run();
    sensors.requestTemperatures();
    uint8_t t = sensors.getTempCByIndex(0);
    if (isnan(t))
    {
        BLINKER_LOG("Failed to read from DHT sensor!");
    }
    else
    {
        BLINKER_LOG("Temperature: ", t, " *C");
        temp_read = t;
    }

    if (temp_read - goalTemp >= 4) // 炎热
    {
        power = true;
        ACTemp = goalTemp - 6;
        fanSpeed = 3;
    }
    else if (temp_read - goalTemp >= 3) // 适中
    {
        power = true;
        ACTemp = goalTemp - 4;
        fanSpeed = 2;
    }
    else if (temp_read - goalTemp >= 2) // 适中
    {
        power = true;
        ACTemp = goalTemp - 2;
        fanSpeed = 0;
    }
    else
    {
        power = false;
    }
    delay(1000);
    Blinker.delay(2000);
    if(millis() - time >= 1000*30)
    {
        time = millis();
        AC_control();
    }
}
