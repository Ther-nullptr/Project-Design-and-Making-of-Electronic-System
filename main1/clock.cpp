#include"clock.h"
Clock::Clock()
{
    rtc.writeProtect(false);           
    rtc.halt(false);                   
    Time t(1970, 1, 1, 0, 0, 0, 0);
    rtc.time(t); 
}

Clock::Clock(uint16_t year,uint16_t month,uint16_t day,uint16_t hour,uint16_t minute,uint16_t second,uint16_t week)
{
    rtc.writeProtect(false);           
    rtc.halt(false);                   
    Time t(year,month,day,hour,minute,second,week);
    rtc.time(t); 
}

uint16_t* Clock::getTime()
{
    uint16_t buf[20];
    Time tmp = rtc.time();
    snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d%02d%d",
             tmp.yr, tmp.mon, tmp.date,
             tmp.hr, tmp.min, tmp.sec, tmp.day - 1);
    return buf;
}

void Clock::setClock() // 前后端分离,这里选择时间和音乐的操作均交给TFT类来实现
{
    
}