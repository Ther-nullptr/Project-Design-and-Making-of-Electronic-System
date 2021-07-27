#include <IRremote.h>

IRsend irsend;
int IR_RECEIVE_PIN = 11;
IRrecv IrReceiver(IR_RECEIVE_PIN);

void setup()
{
    Serial.begin(115200);
    IrReceiver.enableIRIn();  // Start the receiver
}

void loop()
{
    for (int i = 0; i < 3; i++)
    {
        irsend.sendNEC(0x8010040A, 32);
        delay(100);
    }
    delay(2000); //5 second delay between each signal burst
    for (int i = 0; i < 3; i++)
    {
        irsend.sendNEC(0x9010040A, 32);
        delay(100);
    }
    delay(2000);
}