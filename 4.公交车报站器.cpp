#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define PRESS_ADDRESS 0X57

#define CHAR 7

#define GAO_M 0xB8
#define GAO_L 0xDF
#define BEI_M 0xB1
#define BEI_L 0xAE
#define DIAN_M 0xB5
#define DIAN_L 0xEA

#define ZHAN_M 0xD5
#define ZHAN_L 0xBE
#define DAO_M 0xB5
#define DAO_L 0xBD
#define LE_M 0xC1
#define LE_L 0xCB

#define SHUANG_M 0xCB
#define SHUANG_L 0xAB
#define QIAO_M 0xC7
#define QIAO_L 0xC5

#define GUAN_M 0xB9
#define GUAN_L 0xDC
#define ZHUANG_M 0xD7
#define ZHUANG_L 0xAF

byte matrix_led[32];
byte left_matrix_led[16];
byte right_matrix_led[16];
Adafruit_8x16matrix mat_left = Adafruit_8x16matrix();
Adafruit_8x16matrix mat_right = Adafruit_8x16matrix();

uint8_t voice1[17] = {
    0xFD,
    0x00,
    0x0E,
    0x01,
    0x01,
    0xB8,
    0xDF,
    0xB1,
    0xAE,
    0xB5,
    0xEA,
    0xD5,
    0xBE,
    0xB5,
    0xBD,
    0xC1,
    0xCB,
}; // 高碑店站到了
uint8_t voice2[15] = {
    0xFD,
    0x00,
    0x0C,
    0x01,
    0x01,
    0xCB,
    0xAB,
    0xC7,
    0xC5,
    0xD5,
    0xBE,
    0xB5,
    0xBD,
    0xC1,
    0xCB,
}; // 双桥站到了
uint8_t voice3[15] = {
    0xFD,
    0x00,
    0x0C,
    0x01,
    0x01,
    0xB9,
    0xDC,
    0xD7,
    0xAF,
    0xD5,
    0xBE,
    0xB5,
    0xBD,
    0xC1,
    0xCB,
}; // 管庄站到了

int idx = 0;

void setup()
{
    Serial1.begin(115200);
    mat_left.begin(0x70);
    mat_right.begin(0x71);
    pinMode(CHAR, OUTPUT);
    digitalWrite(CHAR, HIGH);
    Wire.begin();
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
}

void loop()
{
    if (ArriveWhere())
    {
        ShowStation();
        AnounceStation();

        delay(1000);
        idx = (idx + 1) % 3;
    }
}

bool ArriveWhere()
{
    bool flag = false;
    Wire.requestFrom(PRESS_ADDRESS, 2);
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read();
        unsigned int low_8 = Wire.read();
        switch (high_8)
        {
        case 128:
            flag = true;
            break;
        }
    }
    return flag;
}

void ShowStation()
{
    int address;
    if (idx == 0)
    {
        address = addr_count(GAO_M, GAO_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);

        address = addr_count(BEI_M, BEI_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);

        address = addr_count(DIAN_M, DIAN_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);
    }
    else if (idx == 1)
    {
        address = addr_count(SHUANG_M, SHUANG_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);

        address = addr_count(QIAO_M, QIAO_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);
    }
    else if (idx == 2)
    {
        address = addr_count(GUAN_M, GUAN_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);

        address = addr_count(ZHUANG_M, ZHUANG_L);
        matrix_addr(address);
        DisplayLED(left_matrix_led, right_matrix_led);
    }
}

void AnounceStation()
{
    if (idx == 0)
        Serial1.write(voice1, 17);
    else if (idx == 1)
        Serial1.write(voice2, 15);
    else if (idx == 2)
        Serial1.write(voice3, 15);
}

int addr_count(int MSB, int LSB)
{
    if (MSB >= 0xA1 && MSB <= 0xa9 && LSB >= 0xA1)
        return ((MSB - 0xA1) * 94 + (LSB - 0xA1)) * 32 + 0x2C9D0;
    else if (MSB >= 0xB0 && MSB <= 0xF7 && LSB >= 0xA1)
        return ((MSB - 0xB0) * 94 + (LSB - 0xA1) + 846) * 32 + 0x2C9D0;
}

void matrix_addr(int addr_countess)
{
    int left = 0;
    int right = 0;

    digitalWrite(CHAR, LOW);
    SPI.transfer(0x03);
    SPI.transfer(addr_countess >> 16);
    SPI.transfer(addr_countess >> 8);
    SPI.transfer(addr_countess);
    for (int i = 0; i < 32; i++)
        matrix_led[i] = SPI.transfer(0x00);
    digitalWrite(CHAR, HIGH);
    for (int j = 0; j < 32; j++)
    {
        if ((j % 2) == 0)
        {
            left_matrix_led[left] = matrix_led[j];
            left++;
        }
        else if ((j % 2) == 1)
        {
            right_matrix_led[right] = matrix_led[j];
            right++;
        }
    }
}

void DisplayLED(byte arr_left[], byte arr_right[])
{
    mat_left.drawBitmap(0, 0, arr_left, 8, 16, LED_ON);
    mat_right.drawBitmap(0, 0, arr_right, 8, 16, LED_ON);
    mat_left.writeDisplay();
    mat_right.writeDisplay();
    delay(500);

    mat_left.clear();
    mat_right.clear();
}