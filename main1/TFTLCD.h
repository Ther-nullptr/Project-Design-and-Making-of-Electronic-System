#include <Adafruit_GFX.h>         // Core graphics library
#include <Adafruit_ILI9341.h>     // Hardware-specific library
#include <SdFat.h>                // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>    // SPI / QSPI flash library
#include <Adafruit_ImageReader.h> // Image-reading functions

const int SDIN = 4;
const int sck = 5;
const int sda = 6;
const int cs = 7;
const int dc = 8;
const int rst = 9;
const int blk = 10;