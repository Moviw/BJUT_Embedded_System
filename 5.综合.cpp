#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "RTClib.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <RDA5807M.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// BLE
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// RTC
RTC_DS1307 rtc;

// OLED
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// FM
#define FIX_BAND RADIO_BAND_FM ///< The band that will be tuned by this sketch is FM.
#define MAX_FREQ 10100
#define MIN_FREQ 8760
int station_freq = MIN_FREQ; ///< The station that will be tuned by this sketch is 89.30 MHz.
#define MAX_VOLUME 10
#define MIN_VOLUME 1
int volume = 2; ///< The volume that will be set by this sketch is level 4.

RDA5807M radio; // Create an instance of Class for RDA5807M Chip

// Weather
// const char *ssid = "TP-LINK_AED0";
// const char *password = "xw85705195";
const char *ssid = "Moooooovi";
const char *password = "qpmz2002";
String APIKEY = "63d4f0c383e7dd88ae5e2e769922bcba";
String CityID = "1816670"; // Your City ID
WiFiClient client;
char servername[] = "api.openweathermap.org"; // remote server we will connect to
String result;
StaticJsonDocument<1024> doc;

#define LOOP_WAIT_TIME 200
#define WEATHER_WAIT_TIME 60000        // WEATHER_WAIT_TIME/1000 seconds
int interval_time = WEATHER_WAIT_TIME; // 先让他等于阈值 可以在开始时直接拿到天气数据

// Other
#define btn0 0
#define btn1 1
#define LED_RED 7
#define LED_GREEN 5
#define LED_BLUE 4
#define LIST_MAX_LENGTH 5
int favorite_list[LIST_MAX_LENGTH] = {0};
int idx4list = 0;
int fav_list_length = 0;
int idx4switch = 0;

bool state = 0;      // 0是FM界面 1是天气界面
int update_what = 0; // 0是调整频段 1是调整音量 2是添加/删除收藏夹中的元素 3是在收藏夹中切换

#define PRESS_ADDRESS 0X57 // 按键模块从器件地址

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pChar)
    {
        std::string value = pChar->getValue();

        if (value.length() > 0)
        {
            if (value == "like")
                Serial.println("this is like ");
            else if (value == "dislike")
                Serial.println("this is dislike ");
            else if (value == "update")
                Serial.println("this is update ");
            else
                Serial.println("wrong input");
        }
    }
};

bool isIntInArray(int arr[], int number, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (arr[i] == number)
        {
            return true;
        }
    }
    return false;
}

void draw_FM(int freq_band, int vol)
{
    display.setCursor(0, 0);
    display.clearDisplay();
    display.print("Freq Band :");
    display.println(int(freq_band));
    display.print("Volume :");
    display.println(vol);
    bool liked = isIntInArray(favorite_list, freq_band, LIST_MAX_LENGTH);
    if (liked)
        display.println("like");
    else
        display.println("dislike");
    display.print("now u can change ");
    switch (update_what)
    {
    case 0:
        display.print("freq");
        break;
    case 1:
        display.print("vol");
        break;
    case 2:
        display.print("fav list");
        break;
    case 3:
        display.print("freqin ur fav list");
        break;
    }
    display.display();
}

void draw_weather()
{
    get_weather_info();
    String location = doc["name"];
    String country = doc["sys"]["country"];
    int temperature = doc["main"]["temp"];
    int humidity = doc["main"]["humidity"];
    float pressure = doc["main"]["pressure"];
    float Speed = doc["wind"]["speed"];
    int degree = doc["wind"]["deg"];

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("T: ");
    display.print(temperature);
    display.print((char)247);
    display.print("C     H: ");
    display.print(humidity);
    display.println("%  ");
    display.print("Pressure:");
    display.print(pressure);
    display.println("hpa");
    display.print("WS: ");
    display.print(Speed);
    display.print("m/s  WA: ");
    display.print(degree);
    display.println((char)247);

    display.display();
}

void get_weather_info()
{
    interval_time += LOOP_WAIT_TIME;
    if (interval_time >= WEATHER_WAIT_TIME)
    {
        interval_time = 0;
        if (client.connect(servername, 80))
        {
            client.println("GET /data/2.5/weather?id=" + CityID + "&units=metric&APPID=" + APIKEY);
        }
        else
        {
            Serial.println("connection failed"); // error message if no client connect
            Serial.println();
        }
        // client.connect(servername, 80);

        while (client.connected() && !client.available())
            delay(1); // waits for data
        while (client.connected() || client.available())
        {                           // connected or data available
            char c = client.read(); // gets byte from ethernet buffer
            result = result + c;
        }

        client.stop(); // stop client
        result.replace('[', ' ');
        result.replace(']', ' ');
        char jsonArray[result.length() + 1];
        result.toCharArray(jsonArray, sizeof(jsonArray));
        jsonArray[result.length() + 1] = '\0';
        DeserializationError error = deserializeJson(doc, jsonArray);

        if (error)
        {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            return;
        }
    }
}

void update_freq(bool increase)
{
    if (increase)
    {
        station_freq += 50;
        if (station_freq > MAX_FREQ)
        {
            station_freq = MIN_FREQ;
        }
    }
    else
    {
        station_freq -= 50;
        if (station_freq < MIN_FREQ)
        {
            station_freq = MAX_FREQ;
        }
    }
    radio.setBandFrequency(FIX_BAND, station_freq);
}

void update_fav(bool like)
{
    if (like)
    {
        if (!isIntInArray(favorite_list, station_freq, LIST_MAX_LENGTH))
        {
            favorite_list[idx4list++] = station_freq;
            idx4list = idx4list % LIST_MAX_LENGTH;
            fav_list_length++;
        }
    }
    else
    {
        if (isIntInArray(favorite_list, station_freq, LIST_MAX_LENGTH))
        {
            favorite_list[idx4list--] = 0;
            idx4list = (idx4list + LIST_MAX_LENGTH) % LIST_MAX_LENGTH;
            fav_list_length--;
        }
    }
}

void update_vol(bool increase)
{
    if (increase)
    {
        volume += 1;
        if (volume > MAX_VOLUME)
        {
            volume = MIN_VOLUME;
        }
    }
    else
    {
        volume -= 1;
        if (volume < MIN_VOLUME)
        {
            volume = MAX_VOLUME;
        }
    }
    radio.setVolume(volume);
}

void switch_in_fav_list(bool ascend)
{
    if (ascend)
    {
        station_freq = favorite_list[idx4switch++];
    }
    else
    {
        station_freq = favorite_list[idx4switch--];
    }
    idx4switch = (idx4switch + fav_list_length) % fav_list_length;
    radio.setBandFrequency(FIX_BAND, station_freq);
}

void begin_update()
{
    Wire.requestFrom(PRESS_ADDRESS, 2); // 从16按键请求一个字节
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read(); // 高8位
        unsigned int low_8 = Wire.read();  // 低8位
        if (0 == update_what)
        {
            switch (high_8)
            {
            case 128: // 0
                update_freq(0);
                break;
            case 64: // 1
                update_freq(1);
                break;
            case 32: // 2
                break;
            case 16: // 3
                break;
            case 8: // 4
                break;
            case 4: // 5
                break;
            case 2: // 6
                break;
            case 1: // 7
                break;
            }
        }
        if (1 == update_what)
        {
            switch (high_8)
            {
            case 128: // 0
                update_vol(0);
                break;
            case 64: // 1
                update_vol(1);
                break;
            case 32: // 2
                break;
            case 16: // 3
                break;
            case 8: // 4
                break;
            case 4: // 5
                break;
            case 2: // 6
                break;
            case 1: // 7
                break;
            }
        }
        if (2 == update_what)
        {
            switch (high_8)
            {
            case 128: // 0
                update_fav(0);
                break;
            case 64: // 1
                update_fav(1);
                break;
            case 32: // 2
                break;
            case 16: // 3
                break;
            case 8: // 4
                break;
            case 4: // 5
                break;
            case 2: // 6
                break;
            case 1: // 7
                break;
            }
        }
        if (3 == update_what)
        {
            switch (high_8)
            {
            case 128: // 0
                switch_in_fav_list(0);
                break;
            case 64: // 1
                switch_in_fav_list(1);
                break;
            case 32: // 2
                break;
            case 16: // 3
                break;
            case 8: // 4
                break;
            case 4: // 5
                break;
            case 2: // 6
                break;
            case 1: // 7
                break;
            }
        }
    }
}

void setup()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(btn0, INPUT);
    pinMode(btn1, INPUT);

    Serial.begin(9600);

    // RTC
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1)
            delay(10);
    }

    // OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }

    // FM
    Wire.begin();

    radio.debugEnable(true);
    radio._wireDebug(false);

    if (!radio.initWire(Wire))
    {
        Serial.println("no radio chip found.");
        delay(4000);
        ESP.restart();
    };

    radio.setBandFrequency(FIX_BAND, station_freq);
    radio.setVolume(volume);
    radio.setMono(false);
    radio.setMute(false);

    // Weather
    WiFi.mode(WIFI_STA); //   create wifi station
    WiFi.begin(ssid, password);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3D for 128x64
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.clearDisplay();
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        display.print(".");
        display.display();
    }
}

void loop()
{
    if (0 == state)
    {
        draw_FM(station_freq, volume);
    }
    else if (1 == state)
    {
        draw_weather();
    }
    if (digitalRead(btn0) == LOW)
    {
        state = !state;
    }
    if (digitalRead(btn1) == LOW)
    {
        update_what = (update_what + 1) % 4;
    }
    begin_update();
    delay(LOOP_WAIT_TIME);
}