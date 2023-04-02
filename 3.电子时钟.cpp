#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RST -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#define PRESS_ADDRESS 0X57
#define EEPROM_ADDRESS 0x50
#define ALARM_HOUR_ADDR 1
#define ALARM_MINUTE_ADDR 2

RTC_DS1307 rtc;
DateTime now;

#define BUZ 0

byte hour_alarm = 0;
byte minute_alarm = 0;
bool need_alarm = false;
int state = 0;
DateTime DT_alarm;
int change_what = 0;
int set_hour, set_minute, set_second;

char buf_time[9];
char buf_day[11];
char daysOfWeek[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

#define SET_HOUR 0
#define SET_MINUTE 1
#define SET_SECOND 2

void setup()
{
    pinMode(BUZ, OUTPUT);
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }

    if (!rtc.begin())
    {
        Serial.println(F("RTC failed"));
        for (;;)
            ;
    }

    if (!rtc.isrunning())
    {
        Serial.println("RTC is NOT running, let's set the time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    display.setTextColor(WHITE);
    LoadAlarm();
}

void loop()
{
    if (0 == state)
    {
        OledDisplay();
        SwitchFunc();
        delay(900);
    }
    else if (1 == state)
    {
        SetTime();
    }
    else if (2 == state)
    {
        SetAlarm();
    }

    checkAlarm();
}

void SwitchFunc()
{
    Wire.requestFrom(PRESS_ADDRESS, 2);
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read();
        unsigned int low_8 = Wire.read();
        switch (high_8)
        {
        case 128:
            Serial.println("设置时钟");
            set_hour = now.hour();
            set_minute = now.minute();
            set_second = now.second();
            state = 1;
            break;
        case 64:
            Serial.println("设置闹钟");
            state = 2;
            break;
        case 32:
            ToggleAlarm();
            break;
        }
    }
}

void OledDisplay()
{
    now = rtc.now();
    snprintf(buf_day, sizeof(buf_day), "%04d-%02d-%02d", now.year(), now.month(), now.day());
    snprintf(buf_time, sizeof(buf_time), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println("Show Time");
    display.setTextSize(1);
    display.print(buf_day);
    display.print(F(" "));
    display.println(daysOfWeek[now.dayOfTheWeek()]);
    display.setTextSize(1);
    display.println(buf_time);
    display.setTextSize(1);
    if (need_alarm)
    {
        display.println("Alarm ON");
    }
    else
        display.println("Alarm OFF");
    display.display();
}

void SetTime()
{
    snprintf(buf_time, sizeof(buf_time), "%02d:%02d:%02d", set_hour, set_minute, set_second);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println("ChangeTime");
    display.setTextSize(1);
    display.println(buf_time);
    if (change_what == SET_HOUR)
    {
        display.println("Now Change Hour");
    }
    else if (change_what == SET_MINUTE)
    {
        display.println("Now Change Minute");
    }
    else if (change_what == SET_SECOND)
    {
        display.println("Now Change Second");
    }
    display.display();

    Wire.requestFrom(PRESS_ADDRESS, 2);
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read();
        unsigned int low_8 = Wire.read();
        switch (high_8)
        {
        case 128:
            state = 0;
            rtc.adjust(DateTime(now.year(), now.month(), now.day(), set_hour, set_minute, set_second));
            break;
        case 64:
            change_what = (change_what + 1) % 3;
            break;
        case 32:
            switch (change_what)
            {
            case SET_HOUR:
                set_hour = (set_hour + 1) % 24;
                break;
            case SET_MINUTE:
                set_minute = (set_minute + 1) % 60;
                break;
            case SET_SECOND:
                set_second = (set_second + 1) % 60;
                break;
            }
            break;
        }
    }
    delay(500);
}

void SetAlarm()
{
    Wire.requestFrom(PRESS_ADDRESS, 2);
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read();
        unsigned int low_8 = Wire.read();
        if (64 == high_8)
        {
            state = 0;
        }
        else if (high_8 == 128)
        {
            hour_alarm = (hour_alarm + 1) % 24;
        }
        else if (high_8 == 32)
        {
            minute_alarm = (minute_alarm + 5) % 60;
        }
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println("Set Alarm");
    display.setTextSize(1);
    display.print("Hour : ");
    display.println(hour_alarm);
    display.print("Minute : ");
    display.println(minute_alarm);
    display.display();

    SaveAlarm();
    delay(500);
}

void ToggleAlarm()
{
    need_alarm = !need_alarm;
    if (need_alarm)
    {
        Serial.println("Alarm ON");
    }
    else
    {
        Serial.println("Alarm OFF");
    }
}

void checkAlarm()
{
    if (need_alarm && now.hour() == hour_alarm && now.minute() == minute_alarm)
    {
        tone(BUZ, 300);
        digitalWrite(BUZ, HIGH);
    }
    else
    {
        noTone(BUZ);
        digitalWrite(BUZ, LOW);
    }
}

void SaveAlarm()
{
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(ALARM_HOUR_ADDR >> 8));   // 高位地址
    Wire.write((int)(ALARM_HOUR_ADDR & 0xFF)); // 低位地址
    Wire.write(hour_alarm);
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(ALARM_MINUTE_ADDR >> 8));   // 高位地址
    Wire.write((int)(ALARM_MINUTE_ADDR & 0xFF)); // 低位地址
    Wire.write(minute_alarm);
    Wire.endTransmission();
    delay(5);
}

void LoadAlarm()
{
    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(ALARM_HOUR_ADDR >> 8));   // 高位地址
    Wire.write((int)(ALARM_HOUR_ADDR & 0xFF)); // 低位地址
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDRESS, 1);
    if (Wire.available())
    {
        hour_alarm = Wire.read();
        if (hour_alarm < 0 || hour_alarm > 23)
        {
            hour_alarm = 0;
        }
    }

    Wire.beginTransmission(EEPROM_ADDRESS);
    Wire.write((int)(ALARM_MINUTE_ADDR >> 8));   // 高位地址
    Wire.write((int)(ALARM_MINUTE_ADDR & 0xFF)); // 低位地址
    Wire.endTransmission();
    Wire.requestFrom(EEPROM_ADDRESS, 1);
    if (Wire.available())
    {
        minute_alarm = Wire.read();
        if (minute_alarm < 0 || minute_alarm > 59)
        {
            minute_alarm = 0;
        }
    }
}