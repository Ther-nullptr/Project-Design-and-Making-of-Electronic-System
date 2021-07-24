#include <DS1302.h>
// 这个库不能直接下arduino上的(Arduino上的那个不是),需要从github上克隆
// 找到Arduino安装目录下的libraries文件夹,然后 git clone git@github.com:msparks/arduino-ds1302.git

#define INITTIME // 如果需要重新设定时间的话,就把这句话加上,否则就删掉这句话

uint8_t hour = 0;
uint8_t minute = 0; // 设定闹钟时间

const int RST = 2,DAT = 3,CLK = 4;
const int LED = 5; // LED管脚
DS1302 rtc(RST, DAT, CLK); //对应DS1302的RST,DAT,CLK

void initRTCTime(void) //初始化RTC时钟
{
    rtc.writeProtect(false);           //关闭写保护
    rtc.halt(false);                   //清除时钟停止标志
    Time t(2021, 7, 24, 21, 0, 50, 7); //创建时间对象 最后参数位，为星期数据，周日为1，周一为2，周二为3，周四为5以此类推. 直接填写当前时间
    rtc.time(t); //向DS1302设置时间数据
}

void setClock()
{
    hour = 21;
    minute = 1;
} // 这个后续会设置成手动输入的，比如通过调节电位器，获取不同的模拟值来设定时间

void printTime(Time &tim) //打印时间数据
{
    char buf[50];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d Today:%d",
             tim.yr, tim.mon, tim.date,
             tim.hr, tim.min, tim.sec, tim.day - 1);
    //获取星期时，需要做减一计算，由前面的星期映射关系决定的
    Serial.println(buf);
}

void Trigger() //闹钟的触发事件,到时候可以修改,此处为点亮LED灯
{
    digitalWrite(LED,HIGH);
    delay(200);
    digitalWrite(LED,LOW);
    delay(200);
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED,OUTPUT);
    setClock();
    //新模块上电需要设置一次当前时间，
    //下载完成后需屏蔽此函数再次下载，否则每次上电都会初始化时间数据
#ifdef INITTIME
    initRTCTime(); //按照设定时间初始化时间的语句
#endif             //INITTIME
}

void loop()
{
    Time tim = rtc.time();
    printTime(tim);
    if (tim.hr == hour && tim.min == minute)
    {
        Trigger();
    }
    delay(500);//这个可能后续会改成中断函数
}