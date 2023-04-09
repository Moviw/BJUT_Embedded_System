#define BLINKER_BLE
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
#include <Blinker.h>

// BLE
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Blinker
BlinkerButton frequency_up_button("frequency_up");
BlinkerButton frequency_down_button("frequency_down");
BlinkerButton volume_up_button("volume_up");
BlinkerButton volume_down_button("volume_down");
BlinkerButton like_button("like");
BlinkerButton dislike_button("dislike");
BlinkerButton list_forward_button("list_forward");
BlinkerButton list_backward_button("list_backward");

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
String APIKEY = "enter your OWN API-KEY";
String CityID = "1816670"; // Your City ID
WiFiClient client;
char servername[] = "api.openweathermap.org"; // remote server we will connect to
String result;
StaticJsonDocument<1024> doc;

// Online Time
#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"

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

int FindIndex(int arr[], int target, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (arr[i] == target)
        {
            return i;
        }
    }
    return -1; // 未找到
}

int findMinDiffIndex(int arr[], int target, int size)
{
    int *diffArr = new int[size];
    for (int i = 0; i < size; i++)
    {
        diffArr[i] = abs(arr[i] - target);
    }
    int minIndex = 0;
    for (int i = 1; i < size; i++)
    {
        if (diffArr[i] < diffArr[minIndex])
        {
            minIndex = i;
        }
    }
    delete[] diffArr;
    return minIndex;
}

void draw_FM(int freq_band, int vol)
{
    display.setCursor(0, 0);
    display.clearDisplay();
    setClock();
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
        display.print("freq in ur fav list");
        break;
    }
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
            favorite_list[--idx4list] = 0;
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
    bool flag = favorite_list[(idx4switch - 1 + fav_list_length) % fav_list_length] == station_freq; // deprecated
    if (ascend)
    {
        station_freq = favorite_list[idx4switch++];
    }
    else
    {
        station_freq = favorite_list[--idx4switch];
    }
    idx4switch = (idx4switch + fav_list_length) % fav_list_length;
    radio.setBandFrequency(FIX_BAND, station_freq);
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
            if (isIntInArray(favorite_list, station_freq, LIST_MAX_LENGTH))
            {
                int idx = FindIndex(favorite_list, station_freq, LIST_MAX_LENGTH);
                favorite_list[idx] = new_frequency;
                station_freq = new_frequency;
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

void DisplayFavList()
{
    for (size_t i = 0; i < LIST_MAX_LENGTH; i++)
    {
        Serial.printf("%d, ", favorite_list[i]);
    }
    Serial.println();
    Serial.printf("list index : %d\n", idx4list);
    Serial.printf("list switch index : %d\n", idx4switch);
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
    BeginUpdate();
    // DisplayFavList();
    Blinker.run();
    delay(LOOP_WAIT_TIME);
}