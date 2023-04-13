#define BLINKER_BLE
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
#include <Blinker.h>
#include <vector>
#include "EEPROM.h"
int address = 0;

// Blinker
BlinkerButton frequency_up_button("FreqUp");
BlinkerButton frequency_down_button("FreqDown");
BlinkerButton volume_up_button("VolUp");
BlinkerButton volume_down_button("VolDown");
BlinkerButton like_button("like");
BlinkerButton dislike_button("dislike");
BlinkerButton list_forward_button("ListForward");
BlinkerButton list_backward_button("ListBackward");

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
int volume = 1; ///< The volume that will be set by this sketch is level 4.

RDA5807M radio; // Create an instance of Class for RDA5807M Chip

// Weather
const char *ssid = "Moooooovi";
const char *password = "qpmz2002";
String APIKEY = "Your own API-KEY";
String CityID = "1816670"; // Beijing ID (Don't CHANGE)
WiFiClient client;
char servername[] = "api.openweathermap.org"; // remote server we will connect to
String result;
StaticJsonDocument<1024> doc;

// Online Time
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"

#define LOOP_WAIT_TIME 200
#define WEATHER_WAIT_TIME 60000                         // WEATHER_WAIT_TIME/1000 seconds
int interval_time = WEATHER_WAIT_TIME - LOOP_WAIT_TIME; // 先让他等于阈值 可以在开始时直接拿到天气数据

// Other
#define btn0 0
#define btn1 1
#define LED_RED 7
#define LED_GREEN 5
#define LED_BLUE 4

std::vector<int> favorite_list;
std::vector<int>::iterator pr;
std::vector<int>::iterator br = favorite_list.begin();

bool state = 0;      // 0是FM界面 1是天气界面
int update_what = 0; // 0是调整频段 1是调整音量 2是添加/删除收藏夹中的元素 3是在收藏夹中切换

#define PRESS_ADDRESS 0X57 // 按键模块从器件地址

/**
 * 该函数检查向量中是否存在整数，如果找到则返回其迭代器。
 *
 * @param elem 参数 elem 是对 std::vector<int> 的迭代器的引用。用于存放指向vector中符合一定条件的元素的迭代器。
 *
 * @return
 * 函数“IsIntInArray”返回一个布尔值。如果在“favorite_list”向量中找到整数“station_freq”，它将返回“true”，并将“elem”迭代器设置为指向向量中与该整数匹配的元素。如果在向量中找不到整数，则函数返回“false”。
 */
bool IsIntInArray(std::vector<int>::iterator &elem)
{
    for (std::vector<int>::iterator it = favorite_list.begin(); it != favorite_list.end(); it++)
    {
        if (station_freq == *it)
        {
            elem = it;
            return true;
        }
    }
    return false;
}

void draw_FM()
{
    display.setCursor(0, 0);
    display.clearDisplay();
    setClock();
    display.print("Freq Band :");
    display.println(int(station_freq));
    display.print("Volume :");
    display.println(volume);

    bool liked = IsIntInArray(pr);
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
        display.print("freq in ur fav list");
        break;
    }
    display.display();
}

void get_weather_info()
{
    if (client.connect(servername, 80))
    {
        client.println("GET /data/2.5/weather?id=" + CityID + "&units=metric&APPID=" + APIKEY);
    }
    else
    {
        Serial.println("connection failed"); // error message if no client connect
        Serial.println();
    }

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

void draw_weather()
{
    if (interval_time >= WEATHER_WAIT_TIME)
    {
        interval_time = 0;
        get_weather_info();
    }

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

void update_freq(bool increase)
{
    if (increase)
    {
        station_freq += 10;
        if (station_freq > MAX_FREQ)
        {
            station_freq = MIN_FREQ;
        }
    }
    else
    {
        station_freq -= 10;
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
        favorite_list.push_back(station_freq);
        sort(favorite_list.begin(), favorite_list.end());
        br = favorite_list.begin();
        SaveData();
    }
    else
    {
        favorite_list.erase(pr);
        SaveData();
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
    if (!favorite_list.empty())
    {
        if (ascend)
        {
            pr = br;
            pr++;
            if (pr != favorite_list.end())
                br = pr;
            else
                pr = br;
        }
        else
        {
            pr = br;
            if (pr != favorite_list.begin())
            {
                pr--;
                br = pr;
            }
        }
        radio.setBandFrequency(FIX_BAND, *pr);
        station_freq = *pr;
    }
    else
    {
        Serial.println("Empty FavList");
    }
}

void BeginUpdate()
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
            }
        }
    }
}

// 蓝牙模块不会在传入函数的同时传入参数, 只能多此一举
void freq_up(const String &state)
{
    update_freq(true);
}
void freq_down(const String &state)
{
    update_freq(false);
}
void volume_up(const String &state)
{
    update_vol(true);
}
void volume_down(const String &state)
{
    update_vol(false);
}
void like(const String &state)
{
    update_fav(true);
}
void dislike(const String &state)
{
    update_fav(false);
}
void list_forward(const String &state)
{
    switch_in_fav_list(true);
}
void list_backward(const String &state)
{
    switch_in_fav_list(false);
}

void dataRead(const String &data)
{
    Serial.println(data);
    int new_frequency = 0;

    bool isValidFrequency = true;

    for (char c : data)
    {
        if (!isdigit(c))
        {
            isValidFrequency = false;
            break;
        }
    }

    const char *freq_str = data.c_str();
    if (isValidFrequency)
    {
        new_frequency = atoi(freq_str);
        if (new_frequency >= MIN_FREQ && new_frequency <= MAX_FREQ)
        {
            if (IsIntInArray(pr))
            {
                station_freq = new_frequency;
                favorite_list.push_back(station_freq);
                sort(favorite_list.begin(), favorite_list.end());
                br = favorite_list.begin();
                SaveData();
            }
            else
            {
                Serial.println("Not valid because current frequency is not in favorite list!");
            }
        }
        else
            Serial.println("Not valid because beyond minmax value!");
    }
}

void setClock()
{
    struct tm dt;
    if (!getLocalTime(&dt))
    {
        // 如果获取失败，就开启联网模式，获取时间
        Serial.println("Failed to obtain time");
        WiFi.mode(WIFI_STA); // 开启网络
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
        return;
    }
    display.println(&dt, "%F %T %A"); // 格式化输出:2021-10-24 23:00:44 Sunday
}

void SaveData()
{
    if (!favorite_list.empty())
    {
        for (std::vector<int>::iterator it = favorite_list.begin(); it != favorite_list.end(); it++)
        {
            EEPROM.writeInt(address, *it);
            address += sizeof(int);
        }
        address = 10 * sizeof(int);
        EEPROM.writeInt(address, favorite_list.size());
        address = 0;
    }
    else
    {
        address = 10 * sizeof(int);
        EEPROM.writeInt(address, -1);
        address = 0;
    }
    EEPROM.commit();
}

void LoadData()
{
    int size = EEPROM.readInt(10 * sizeof(int));
    if (size > 0 && size <= 5)
    {
        for (int i = 0; i < size; i++)
        {
            favorite_list.push_back(EEPROM.readInt(i * sizeof(int)));
        }
        br = favorite_list.begin();
    }
}

void setup()
{
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(btn0, INPUT);
    pinMode(btn1, INPUT);

    Serial.begin(115200);

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

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        display.print(".");
        display.display();
    }
    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    setClock();

    // RTC
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        while (1)
            delay(10);
    }
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3D for 128x64
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.clearDisplay();

    // EEPROM
    if (!EEPROM.begin(1000))
    {
        Serial.println("Failed to initialise EEPROM");
        Serial.println("Restarting...");
        delay(1000);
        ESP.restart();
    }

    // Blinker
    BLINKER_DEBUG.stream(Serial);
    Blinker.begin();
    Blinker.attachData(dataRead);
    frequency_up_button.attach(freq_up);
    frequency_down_button.attach(freq_down);
    volume_up_button.attach(volume_up);
    volume_down_button.attach(volume_down);
    like_button.attach(like);
    dislike_button.attach(dislike);
    list_forward_button.attach(list_forward);
    list_backward_button.attach(list_backward);
    LoadData();
}

void loop()
{
    if (0 == state)
    {
        draw_FM();
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
    BeginUpdate();
    Blinker.run();
    interval_time += LOOP_WAIT_TIME;
    delay(LOOP_WAIT_TIME);
}