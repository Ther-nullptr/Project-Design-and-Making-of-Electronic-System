#include <Arduino.h>
#include <IRremote.h>

IRsend irsend;
int khz = 38; // NEC协议常数
int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

const uint16_t open[] = {8850, 4450, 650, 1650, 600, 550, 650, 550, 600, 1700, 600, 600, 600, 550, 600, 600, 600, 550, 600, 1700, 600, 600, 600, 550, 600, 1700, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 550, 650, 550, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 1700, 600, 550, 600, 1700, 600, 550, 600, 600, 600, 1700, 550, 600, 600};
const uint16_t close[] = {8900, 4450, 550, 1750, 550, 600, 550, 650, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 1700, 650, 550, 600, 550, 650, 1650, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 600, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 600, 600, 600, 600, 600, 550, 600, 600, 600, 550, 600, 1700, 600, 550, 650, 1650, 600, 600, 550, 600, 600, 1700, 600, 550, 600};
const int sendpin = 3;

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
    /*
     Serial.print("SendIR: ");
     irsend.sendRaw(open, 73, khz);
     delay(3000); // 等待3秒
     irsend.sendRaw(close, 73, khz);
     delay(3000);
     */

    if (irrecv.decode(&results))
    {
        Serial.println(results.value, HEX);
        dump(&results);
        irrecv.resume(); // Receive the next value
    }
}
