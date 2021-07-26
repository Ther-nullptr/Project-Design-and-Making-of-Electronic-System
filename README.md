# 多功能智能家居(暂定名)

## 0.写在前面

这些东西只是想法,后续可以修改补充.

## 1.基本思路(暂定)

主体部分示例图如下:

![example.jpg](https://i.loli.net/2021/07/22/7PlwGWkz1CNtcET.jpg)

左为TFTLCD,右上为LCD1,右下为LCD2.

### 时钟

利用DS1302或DS3231时钟模块,可以实现不断电计时,以实时响应时间,并打印在TFTLCD上.

### 闹钟

同样利用时钟模块,可以设定闹钟的响铃时间,在特定时段触发.

### 温湿度

利用温湿度传感器(如DHT11等),实时测量环境温湿度,显示在LCD1上.

### 红外遥控

利用红外发射灯,通过发射特定的信号,使得其能代替遥控器:当检测到室内温度较低时或较高时,可以发射红外信号开关空调.

> 有一个问题:各种空调的红外控制信号不同,但只要摸清楚红外遥控的机制,应该问题不大吧.

### (考虑中)播放视频

此项目有一定的娱乐功能.SD卡可以储存一些图片或视频,可以在TFTLCD上播放.

### (考虑中)播放音乐

利用音频模块,可以在播放视频或闹铃时同步播放音乐.

### 物联网

利用之前提到的物联网平台和ESP8266wifi模块,将关键状态上传至网络,通过公众号就可以远程查看.

### (考虑中)按钮

通过按钮实现TFTLCD不同界面的切换.

### (考虑中)人体红外识别,语音识别

用对应的模块实现.

## 2.器材需求

* **时钟模块 (我有一块DS1302)**
* **TFTLCD 2.4寸**
* **温湿度传感器 (有一块)**
* **红外发射 (我再看看买什么合适)**
* LCD (应该都有吧)
* 合适的按钮
* SD卡及其适配器
* **ESP8266wifi(好像发了??)**
* 音乐模块(扬声器),语音识别模块,人体红外识别模块等.

## 日志

### 7.22

立项

### 7.23

购买设备.

考虑到项目的功能比较繁杂,可能会用到两片单片机,下将项目分为两个相对独立的模块.(这也是不采用大屏显示的原因之一吧,因为将所有IO集中在一个大屏上的话,IO不够用)

* 单片机1:
  1. 连接温湿度传感器和LCD1,显示温度
  2. 根据温度控制红外灯的亮灭,以控制空调开关(考虑到Arduino可怜的内存,可能只录两条指令:开机和关机)
  3. 连接WIFI模块,向物联网发送状态
* 单片机2:
  1. 连接时钟模块进行计时
  2. 连接TFTLCD,显示时间(年月日时分秒星期和闹钟设置)
  3. 连接几个按钮,设定闹钟
  4. 连接LCD2,可以显示播放视频的状态,进度等(瞎想的)

> 注:LCD可以使用I2C接口,这样只需要4个引脚.

#### 计划

首先需要分块测试,这样既有利于分布式开发,也能给到时后组装提供方便.

* 温湿度模块:需要传感器和LCD,(每隔一段时间将状况显示到LCD上)
* 红外遥控模块:接受温度,返回是否发射红外线
* Wi-Fi模块:探索该模块的功能
* 物联网:了解物联网平台和Arduino之间的通信机制
* 时钟模块:先看看这个模块能不能进行计时,再看看如何设计时序逻辑和UI以设定闹钟
* 视频模块:先简单测试如何读取SD卡中的图片资料

### 7.24

#### SD1302模块测试

可以正常打印出时间,并根据设定时间触发"闹钟".当然这个模块后续会改,比如设置定时器中断函数,减少访问时间的频次等.

#### 红外模块测试

还未完全完成.

### 7.25

#### 时钟模块测试

时钟模块在上纽扣电池之后可以做到断电走时.

#### TFT模块测试

TFT模块理论上可以做到播放视频--BAD APPLE(黑白)和TWO TIGERS LIKE DANCING(彩色).

但仍有不足之处:播放帧率十分低--1fps,效果远不如b站上的视频,还经常卡bug.

如果后期仍得不到较好解决,将考虑放弃这一功能,换为其他可视化功能.

样例中附带一个将mp4转换为bmp的python脚本.

#### 红外模块测试

### 7.26

#### TFT模块测试

TFT显示帧率的问题,需要从硬件上入手(据说理论可以达到60fps?),这个就需要研究一下TFT和UTFT的一些设置,特别是arduino库文件.

#### 红外模块测试

编码和解码可能都有一定的问题.不知道是否与硬件有关.需要对esp8266进行一些研究,esp8266的红外控制库更加强大.

#### 其他

开始构思控制架构与运行逻辑.为解决遗留问题,需要多看相关书籍.