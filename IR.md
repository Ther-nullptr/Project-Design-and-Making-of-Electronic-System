## 编码理论(格力)

基本格式：**起始码+35位数据+连接码+32位数据+结束码**

1-3:模式

4:电源

5-6:风速

7:扫风

8:睡眠

9-12:温度

13-20:定时器

21:加湿

22:灯光

23:负离子

24:节电

之后的数据来源出现冲突,不过应该不影响使用.

## 程序说明

### AirConditionDemo1

主控板:Arduino Uno 

主依赖库:IRremote.h

思路:先通过红外解码程序获取空调遥控器的信号,存为数组,然后通过sendRaw发射红外线.(失败)

这种方法理论上可以控制一切品牌的空调.

### AirConditionDemo2

主控板:Arduino Uno 

主依赖库:IRremote.h

思路:先通过红外解码程序获取空调遥控器的信号,用NEC协议(格力遵循的协议)解码,然后通过sendNEC发射红外线.(失败)

前两种方法失败的一个原因在于,解码后的信息再发射出去,会出现一些不可解释的失真(我测过).

### AirConditionDemo3

主控板:esp8266

主依赖库:IRremoteESP8266.h,ir_Gree.h

思路:顾名思义,这个库封装好了专门操作格力空调的函数.(事实上包括了大部分的主流品牌),操作方法通过读库很容易习得.(成功)

### 备注

* 红外发射灯的功率要大.越大越好.我用的是1W的.
* 红外发射管(3线)的VCC不能接esp8266,因为其电压是3V,驱动不了发射管.
