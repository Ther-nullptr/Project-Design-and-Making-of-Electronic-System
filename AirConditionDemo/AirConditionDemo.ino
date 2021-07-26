#include <Arduino.h>
#include <IRremote.h>

#define GREE2
#define SEND

int khz = 38; // NEC协议常数
const int RECV_PIN = 11;
const int sendpin = 3;
IRrecv irrecv(RECV_PIN);
IRsend irsend(sendpin);
decode_results results;

#ifdef HAIER
const uint16_t Haieropen[] = {2950, 3050, 3000, 4450, 450, 1800, 450, 700, 500, 1800, 400, 750, 450, 700, 450, 1850, 450, 1800, 450, 700, 450, 1800, 450, 750, 450, 700, 450, 700, 450, 750, 450, 700, 450, 750, 400, 1850, 450, 700, 450, 700, 500, 700, 450, 1800, 450, 750, 400, 1850, 450, 700, 450, 700, 500, 700, 450, 700, 450, 750, 450, 1800, 450, 700, 450, 1800, 450, 1800, 450, 750, 450, 750, 400, 1800, 500, 1750, 450, 750, 450, 1800, 450, 1800, 450, 750, 400, 750, 450, 750, 400, 750, 450, 700, 450, 750, 450, 700, 450, 700, 450, 750, 450};
const uint16_t Haierclose[] = {3000, 3050, 3000, 4400, 450, 1800, 450, 750, 450, 1800, 400, 750, 450, 700, 450, 1800, 450, 1800, 450, 700, 450, 1800, 450, 750, 400, 750, 450, 700, 450, 700, 450, 750, 450, 700, 450, 700, 450, 700, 450, 750, 450, 700, 450, 1800, 450, 700, 450, 1800, 450, 700, 450, 750, 400, 750, 450, 700, 450, 700, 450, 1800, 500, 700, 400, 1800, 500, 1800, 450, 1800, 400, 750, 450, 1800, 500, 1750, 450, 700, 450, 1800, 450, 1800, 450, 700, 450, 750, 400, 750, 450, 700, 450, 700, 450, 750, 450, 700, 450, 700, 450, 700, 450};

#endif

#ifdef GREE1
const uint16_t Gree1open[] = {8850, 4450, 650, 1650, 600, 550, 650, 550, 600, 1700, 600, 600, 600, 550, 600, 600, 600, 550, 600, 1700, 600, 600, 600, 550, 600, 1700, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 550, 650, 550, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 1700, 600, 550, 600, 1700, 600, 550, 600, 600, 600, 1700, 550, 600, 600};
const uint16_t Gree1close[] = {8900, 4450, 550, 1750, 550, 600, 550, 650, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 1700, 650, 550, 600, 550, 650, 1650, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 600, 1700, 600, 550, 650, 1650, 600, 600, 550, 600, 600, 1700, 600, 550, 600};
#endif

#ifdef GREE2
const uint16_t Gree2open[] = {8800, 4500, 600, 1650, 600, 600, 600, 600, 550, 600, 600, 600, 600, 600, 600, 1650, 600, 600, 600, 600, 600, 550, 600, 600, 600, 1700, 550, 600, 600, 600, 600, 600, 600, 600, 550, 600, 600, 600, 600, 600, 600, 600, 600, 550, 600, 1700, 600, 600, 550, 1700, 600, 1650, 600, 600, 600, 600, 600, 600, 550, 1700, 600, 600, 600, 1650, 600, 600, 600, 600, 600, 1650, 600, 600, 600};
const uint16_t Gree2close[] = {8850, 4450, 600, 1700, 550, 600, 600, 600, 600, 1650, 600, 600, 600, 600, 600, 1650, 600, 600, 600, 600, 600, 600, 600, 550, 600, 1700, 600, 600, 550, 600, 600, 600, 600, 600, 600, 600, 550, 600, 600, 600, 600, 600, 600, 600, 550, 1700, 600, 600, 600, 1650, 600, 1700, 550, 600, 600, 600, 600, 600, 600, 1650, 600, 600, 600, 1650, 600, 600, 600, 600, 600, 1650, 600, 600, 600};
#endif

void setup()
{
    Serial.begin(9600);
    pinMode(sendpin, OUTPUT);
    digitalWrite(sendpin, HIGH); // 点亮LED测试（需通过手机摄像模式观察） 你可以开始插个led灯来观察led灯是否亮，如果亮就可以换成红外线发射灯了
    delay(3000);                 // 等待3秒
    digitalWrite(sendpin, LOW);  // 结束测试
    irrecv.enableIRIn();         // Start the receiver
}

void dump(decode_results *results)
{
    // Dumps out the decode_results structure.
    // Call this after IRrecv::decode()
    int count = results->rawlen;
    if (results->decode_type == UNKNOWN)
    {
        Serial.print("Unknown encoding: ");
    }
    else if (results->decode_type == NEC)
    {
        Serial.print("Decoded NEC: ");
    }
    else if (results->decode_type == SONY)
    {
        Serial.print("Decoded SONY: ");
    }
    else if (results->decode_type == RC5)
    {
        Serial.print("Decoded RC5: ");
    }
    else if (results->decode_type == RC6)
    {
        Serial.print("Decoded RC6: ");
    }
    else if (results->decode_type == PANASONIC)
    {
        Serial.print("Decoded PANASONIC - Address: ");
        Serial.print(results->address, HEX);
        Serial.print(" Value: ");
    }
    else if (results->decode_type == LG)
    {
        Serial.print("Decoded LG: ");
    }
    else if (results->decode_type == JVC)
    {
        Serial.print("Decoded JVC: ");
    }
    else if (results->decode_type == WHYNTER)
    {
        Serial.print("Decoded Whynter: ");
    }
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
    Serial.print("Raw (");
    Serial.print(count, DEC);
    Serial.print("): ");

    for (int i = 0; i < count; i++)
    {
        if (i & 1)
        {
            Serial.print(results->rawbuf[i] * USECPERTICK, DEC);
        }
        else
        {
            Serial.write(' ');
            Serial.print((unsigned long)results->rawbuf[i] * USECPERTICK, DEC);
        }
        Serial.print(" ");
    }
    Serial.println();
}

void loop()
{

    Serial.println("SendIR");

#ifdef HAIER
    irsend.sendRaw(Haieropen, 99, khz); // 第二个长度为数组长度
    delay(5000);                        // 等待3秒
    irsend.sendRaw(Haierclose, 99, khz);
    delay(5000);
#endif // HAIER

#ifdef GREE1
    irsend.sendRaw(Gree1open, 73, khz); // 第二个长度为数组长度
    delay(5000);                        // 等待3秒
    irsend.sendRaw(Gree1close, 73, khz);
    delay(5000);
#endif // GREE2

#ifdef GREE2
    //irsend.sendRaw(Gree2open, 73, khz); // 第二个长度为数组长度
    if (irrecv.decode(&results))
    {
        Serial.println(results.value, HEX);
        dump(&results);
        irrecv.resume(); // Receive the next value
    }

    delay(5000); // 等待3秒
    /*
    irsend.sendRaw(Gree2close, 73, khz);
    if (irrecv.decode(&results))
    {
        Serial.println(results.value, HEX);
        dump(&results);
        irrecv.resume(); // Receive the next value
    }

    delay(5000);
    */
#endif // GREE2

 
}
