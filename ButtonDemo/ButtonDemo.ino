const int readPin = 7;
void setup()
{
    pinMode(readPin, INPUT);
    Serial.begin(9600);
}

void loop()
{
    int a = digitalRead(readPin);
    Serial.println(a);
}