#define BLINKER_WIFI

#include <Blinker.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

/****************1.基本常量******************/
const char auth[] = "29fa581e93d0";     //从blinker应用上得到的设备密钥
const char ssid[] = "Honor 10";         //wifi名
const char pswd[] = "12345678";         //wifi密码
const char host[] = "api.seniverse.com";//API地址

const int httpPort = 80;

const uint8_t kIrLed = 4; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
const uint8_t rx = D6;
const uint8_t tx = D7;
const uint8_t one_wire_bus = D5;

//心知天气HTTP请求所需信息
const String key = "S61QcIDfu7zNuIXPj"; // 私钥
const String location = "beijing";      // 城市
const String locationList[5] = {"beijing", "xining", "taiyuan", "shanghai", "chengdu"};
String weatherList; 

/****************2.全局对象******************/
// 全局变量
uint8_t temp_read = 0;
uint8_t goalTemp = 28; // 室内想要达到的温度
uint8_t ACTemp = 25; // 默认的空调温度
bool power = false; // 空调是否开启
bool underControl = true; // IoT是否控制空调
uint8_t fanSpeed = 0; // 0为自动,1-3风速依次增加

// 新建组件对象
BlinkerNumber POWER("power");         // blinker需要监视的对象(开关)
BlinkerNumber TEMP("temp");           // blinker需要监视的对象(温度)
BlinkerNumber AC_TEMP("AC_temp");     // blinker需要监视的对象(空调温度)
BlinkerNumber FANSPEED("fanSpeed");   // blinker需要监视的对象(风速)
OneWire oneWire(one_wire_bus);        // 初始连接在单总线上的单总线设备
DallasTemperature sensors(&oneWire);  // 温度控制器对象
IRGreeAC ac(kIrLed);                  // Set the GPIO to be used for sending messages.
SoftwareSerial arduinoSerial(rx,tx);        // 串口通信对象

// 天气对象定义

/****************3.函数定义******************/
// 心跳包函数,用于将信息发送给app
void heartbeat()
{
    POWER.print(power);
    TEMP.print(temp_read);
    AC_TEMP.print(ACTemp);
    FANSPEED.print(fanSpeed);
}

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
    arduinoSerial.begin(9600);
    //BLINKER_DEBUG.stream(Serial);

    // Blinker组件初始化
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachHeartbeat(heartbeat);

    // 传感器初始化
    sensors.begin();

    // 空调状态设置
    ac.begin();

    // wifi组件初始化
    WiFi.mode(WIFI_STA); 
    WiFi.begin(ssid,pswd);
    while(WiFi.status()!=WL_CONNECTED)
    {
        delay(100);
        // Serial.print(".");
    }
    //Serial.println(WiFi.localIP());
}

void loop()
{
    static unsigned long time = 0;
    Blinker.run();

    // 建立心知天气API当前天气请求资源地址
    //weatherList = "{";
    for (int i = 0; i < 5; i++)
    {
        String reqRes = "/v3/weather/now.json?key=" + key +
                        +"&location=" + locationList[i] + "&language=en&unit=c";
        weatherList += httpRequest(reqRes,i);
    }
    Serial.println(weatherList);
    arduinoSerial.println(weatherList);
    weatherList="";

    sensors.requestTemperatures();
    uint8_t t = sensors.getTempCByIndex(0);
    if (isnan(t))
    {
        //BLINKER_LOG("Failed to read from DHT sensor!");
    }
    else
    {
        //BLINKER_LOG("Temperature: ", t, " *C");
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
    delay(15000);
    Blinker.delay(2000);
    if(millis() - time >= 1000*30)
    {
        time = millis();
        AC_control();
    }

    // 读取收到的数据
    if(arduinoSerial.available()>0)
    {
        // 读取控制温度的信息
        uint8_t tmp = arduinoSerial.read();
        Serial.println(tmp);
        if(tmp == 0) // IoT控制空调关闭
        {
            underControl = false;
        }
        else // IoT控制空调开启
        {
            underControl = true;
            goalTemp = tmp;
        }
    }
}

// 向心知天气服务器服务器请求信息并对信息进行解析
String httpRequest(String reqRes,int i)
{
    WiFiClient client;
    String json;

    // 建立http请求信息
    String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +
                         "Host: " + host + "\r\n" +
                         "Connection: close\r\n\r\n";
    /*
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.print(host);
    */
    // 尝试连接服务器
    if (client.connect(host, httpPort))
    {
        //Serial.println(" Success!");

        // 向服务器发送http请求信息
        client.print(httpRequest);
        //Serial.println("Sending request: ");
        //Serial.println(httpRequest);

        // 获取并显示服务器响应状态行
        String status_response = client.readStringUntil('\n');
        //Serial.print("status_response: ");
        //Serial.println(status_response);

        // 使用find跳过HTTP响应头
        if (client.find("\r\n\r\n"))
        {
            //Serial.println("Found Header End. Start Parsing.");
        }
        // 利用ArduinoJson库解析心知天气响应信息

        DynamicJsonDocument doc(1024);

        deserializeJson(doc, client);

        JsonObject results_0 = doc["results"][0];

        JsonObject results_0_daily = results_0["now"];
        String results_0_daily_code = results_0_daily["code"];          
        String results_0_daily_temperature = results_0_daily["temperature"];               

        // 组装为json字符串准备发送
        if(i!=0)
        {
            json = "\""+String(i*2)+"\":"+results_0_daily_code+",\""+String(i*2+1)+"\":"+results_0_daily_temperature;
        }
        else
        {
            json = results_0_daily_code+",\""+String(i*2+1)+"\":"+results_0_daily_temperature;
        }
        if(i!=4)
        {
            json+=",";
        }
        Serial.println(json);

        Serial.println(F("======Weather Now======="));
        Serial.print(F("weather: "));
        Serial.println(results_0_daily_code);
        Serial.print(F("temp: "));
        Serial.println(results_0_daily_temperature);
        Serial.println(F("========================"));
    }
    else
    {
        //Serial.println(" connection failed!");
    }
    //断开客户端与服务器连接工作
    client.stop();
    return json;
}

