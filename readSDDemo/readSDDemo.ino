#include<SD.h>
#include<SPI.h>

File file;
void setup()
{
    // 读入文件
    Serial.begin(9600);
    SD.begin(4);
    file = SD.open("test.txt",FILE_READ);
    if(file)
    {
        while(file.available())
        {
            Serial.write(file.read());
        }
        file.close();
    }
    else
    {
        Serial.println("no such file!");
    }
    
    // 写入文件,暂时还没有想到这个函数的用途,但可能有用

    file = SD.open("test2.txt",FILE_WRITE);
    if(file)
    {
        char c[] = "114514";
        file.println(c);
        file.close();
    }
    else
    {
        Serial.println("write error!");
    }
}
void loop()
{
    
}