#include<DS1302.h>
#include<stdint.h>

const int RST = 1;
const int DAT = 2;
const int CLK = 3;


struct Clockinfo
{
    Time ClockTime; // 设置闹钟的时间
    char music; // 闹钟音乐
};


class Clock
{
public:
    Clock()
    Clock(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
    uint16_t* getTime(); // 获取当前时间
    void setClock(uint16_t [],char); // 设置闹钟和音乐


private:
    DS1302 rtc(RST,DAT,CLK);
    Time ClockTime; // 设置闹钟的时间
};
