#include<SdFat.h>
#include<TMRpcm.h>
#include<SPI.h>

const int SD_ChipSelectPin = 4;
TMRpcm tmrpcm;
SdFat SD;

void setup()
{
    tmrpcm.speakerPin = 9;
    SD.begin(SD_ChipSelectPin);
    tmrpcm.setVolume(2);
    tmrpcm.play("twotigers.wav");
    
}

void loop()
{
   
}