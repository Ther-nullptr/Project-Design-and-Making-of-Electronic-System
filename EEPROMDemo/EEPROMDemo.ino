#include <EEPROM.h>

// 记录要写入EEPROM的数据
const byte clockTime_Hour = 6;
const byte clockTime_Minute = 7;
const byte clockMusic = 1;
const byte clear = 60; // 擦除值

// 记录EEPROM的地址
const int Hour_ADDR = 0;
const int Minute_ADDR = 1;
const int Music_ADDR = 2;

void setClock()
{
    EEPROM.write(Hour_ADDR, clockTime_Hour);
    EEPROM.write(Minute_ADDR, clockTime_Minute);
    EEPROM.write(Music_ADDR, clockMusic);
}

void deleteClock()
{
    EEPROM.write(Hour_ADDR, clear);
    EEPROM.write(Minute_ADDR, clear);
    EEPROM.write(Music_ADDR, clear);
}

void getClock()
{
    byte hour, minute, music;
    hour = EEPROM.read(Hour_ADDR);
    minute = EEPROM.read(Minute_ADDR);
    music = EEPROM.read(Music_ADDR);
    Serial.println(hour);
    Serial.println(minute);
    Serial.println(music);
}

void setup()
{
    /*
    Serial.begin(115200);
    byte clockTime = EEPROM.read(clockTime_ADDR); // 读入数据,若之前没有写入数据应该为255
    Serial.println(clockTime);

    EEPROM.write(clockTime_ADDR,clockTime_EEPROM); // 写入数据
    clockTime = EEPROM.read(clockTime_ADDR);
    Serial.println(clockTime);

    EEPROM.update(clockTime_ADDR,clockTime_EEPROM2); // 更新数据
    clockTime = EEPROM.read(clockTime_ADDR);
    Serial.println(clockTime);

    EEPROM.write(clockTime_ADDR,0);
    clockTime = EEPROM.read(clockTime_ADDR);
    Serial.println(clockTime);
    */
    Serial.begin(115200);
    setClock();
    getClock();
    deleteClock();
    getClock();
}

void loop()
{
}