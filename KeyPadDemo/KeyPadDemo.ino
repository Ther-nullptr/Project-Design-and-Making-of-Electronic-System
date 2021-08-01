#include "Adafruit_Keypad.h"

#define MEGA

const byte ROWS = 4; // rows
const byte COLS = 4; // columns
//define the symbols on the buttons of the keypads
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

#ifdef UNO
byte rowPins[ROWS] = {2, 3, 4, 5};   //connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 9, 10, 11}; //connect to the column pinouts of the keypad
#endif                               // UNO

#ifdef MEGA
byte rowPins[ROWS] = {28, 30, 32, 34}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {36, 38, 40, 42}; //connect to the column pinouts of the keypad
#endif

//initialize an instance of class NewKeypad
Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup()
{
    Serial.begin(9600);
    customKeypad.begin();
}

void loop()
{
    static bool flag = 0;
    // put your main code here, to run repeatedly:
    if (flag)
    {
        customKeypad.tick();
    }
    flag = !flag;

    if (customKeypad.available())
    {
        keypadEvent e = customKeypad.read();
        Serial.println((char)e.bit.KEY);
        /*
        if (e.bit.EVENT == KEY_JUST_PRESSED)
            Serial.println(" pressed");
        else if (e.bit.EVENT == KEY_JUST_RELEASED)
            Serial.println(" released");
        */
    }

    delay(10);
}