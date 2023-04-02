#include <Wire.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PRESS_ADDRESS 0X57  // 按键模块从器件地址

char *password;                      // 用户输入的密码
char *true_pwd;                      // 真正的密码
const String admin_pwd = "qpmz2002"; // 管理员密码
#define MAX 20                       // 密码最大长度
int idx4pwd = 0;                     // password的下标
int idx4true_pwd = 0;                // true_pwd的下标

#define check_btn 0 // 确认密码按键
#define reset_btn 1 // 重置密码按键
#define LED_RED 7
#define LED_GREEN 5
#define LED_BLUE 4

void main_proc();
void Blink(int);
void input_pwd();
void input_true_pwd();
void change_true_pwd();

void setup()
{
    password = (char *)malloc(sizeof(char) * MAX);
    memset(password, 0, MAX);
    true_pwd = (char *)malloc(sizeof(char) * MAX);
    strcpy(true_pwd, "1234");
    Wire.begin(); // 加入I2C总线
    Serial.begin(115200);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    pinMode(check_btn, INPUT);
    pinMode(reset_btn, INPUT);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
}

void loop()
{
    main_proc();
}

void main_proc()
{
    input_pwd();
    display.clearDisplay();

    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.println("input is : ");
    display.println(password);

    if (digitalRead(check_btn) == LOW)
    {
        if (!strcmp(true_pwd, password))
        {
            Serial.println("Correct!");
            display.println("UnLock!");
            Blink(1);
            memset(password, 0, MAX);
            idx4pwd = 0;
            change_true_pwd();
        }
        else
        {
            Serial.println("Wrong!");
            display.println("Wrong!");
            Blink(0);
            memset(password, 0, MAX);
            idx4pwd = 0;
        }
    }
    if (wanna_changed_by_admin())
    {
        change_true_pwd();
    }
    display.display();
    delay(400);
}

void input_pwd()
{
    Wire.requestFrom(PRESS_ADDRESS, 2); // 从16按键请求一个字节
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read(); // 高8位
        unsigned int low_8 = Wire.read();  // 低8位
        switch (high_8)
        {
        case 128:
            password[idx4pwd] = 48;
            idx4pwd++;
            break;
        case 64:
            password[idx4pwd] = 49;
            idx4pwd++;
            break;
        case 32:
            password[idx4pwd] = 50;
            idx4pwd++;
            break;
        case 16:
            password[idx4pwd] = 51;
            idx4pwd++;
            break;
        case 8:
            password[idx4pwd] = 52;
            idx4pwd++;
            break;
        case 4:
            password[idx4pwd] = 53;
            idx4pwd++;
            break;
        case 2:
            password[idx4pwd] = 54;
            idx4pwd++;
            break;
        case 1:
            password[idx4pwd] = 55;
            idx4pwd++;
            break;
        }
        switch (low_8)
        {
        case 128:
            password[idx4pwd] = 56;
            idx4pwd++;
            break;
        case 64:
            password[idx4pwd] = 57;
            idx4pwd++;
            break;
        case 32:
            password[idx4pwd] = 65;
            idx4pwd++;
            break;
        case 16:
            password[idx4pwd] = 66;
            idx4pwd++;
            break;
        case 8:
            password[idx4pwd] = 67;
            idx4pwd++;
            break;
        case 4:
            password[idx4pwd] = 68;
            idx4pwd++;
            break;
        case 2:
            password[idx4pwd] = 69;
            idx4pwd++;
            break;
        case 1:
            password[idx4pwd] = 70;
            idx4pwd++;
            break;
        }
    }
}

void input_true_pwd()
{
    Wire.requestFrom(PRESS_ADDRESS, 2); // 从16按键请求一个字节
    while (Wire.available())
    {
        unsigned int high_8 = Wire.read(); // 高8位
        unsigned int low_8 = Wire.read();  // 低8位
        switch (high_8)
        {
        case 128:
            true_pwd[idx4true_pwd] = 48;
            idx4true_pwd++;
            break;
        case 64:
            true_pwd[idx4true_pwd] = 49;
            idx4true_pwd++;
            break;
        case 32:
            true_pwd[idx4true_pwd] = 50;
            idx4true_pwd++;
            break;
        case 16:
            true_pwd[idx4true_pwd] = 51;
            idx4true_pwd++;
            break;
        case 8:
            true_pwd[idx4true_pwd] = 52;
            idx4true_pwd++;
            break;
        case 4:
            true_pwd[idx4true_pwd] = 53;
            idx4true_pwd++;
            break;
        case 2:
            true_pwd[idx4true_pwd] = 54;
            idx4true_pwd++;
            break;
        case 1:
            true_pwd[idx4true_pwd] = 55;
            idx4true_pwd++;
            break;
        }
        switch (low_8)
        {
        case 128:
            true_pwd[idx4true_pwd] = 56;
            idx4true_pwd++;
            break;
        case 64:
            true_pwd[idx4true_pwd] = 57;
            idx4true_pwd++;
            break;
        case 32:
            true_pwd[idx4true_pwd] = 65;
            idx4true_pwd++;
            break;
        case 16:
            true_pwd[idx4true_pwd] = 66;
            idx4true_pwd++;
            break;
        case 8:
            true_pwd[idx4true_pwd] = 67;
            idx4true_pwd++;
            break;
        case 4:
            true_pwd[idx4true_pwd] = 68;
            idx4true_pwd++;
            break;
        case 2:
            true_pwd[idx4true_pwd] = 69;
            idx4true_pwd++;
            break;
        case 1:
            true_pwd[idx4true_pwd] = 70;
            idx4true_pwd++;
            break;
        }
    }
}

void change_true_pwd()
{
    idx4true_pwd = 0;
    memset(true_pwd, 0, MAX);
    while (true)
    {
        input_true_pwd();
        display.clearDisplay();
        Blink(2);
        display.setTextSize(1);              // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.setCursor(0, 0);             // Start at top-left corner
        display.println("now change your pwd");
        display.println(true_pwd);
        if (digitalRead(reset_btn) == LOW)
        {
            if (strlen(true_pwd) >= 4)
            {
                display.println("changed successfully!");
                return;
            }
            else
            {
                display.println("must longer than 4");
                idx4true_pwd = 0;
                memset(true_pwd, 0, MAX);
            }
        }
        display.display();
    }
}

bool wanna_changed_by_admin()
{
    if (Serial.available())
    {
        return (Serial.readString() == admin_pwd);
    }
}

void Blink(int signal)
{
    int temp;
    if (1 == signal)
    {
        temp = LED_GREEN;
    }
    else if (0 == signal)
    {
        temp = LED_RED;
    }
    else if (2 == signal)
    {
        temp = LED_BLUE;
    }
    for (int i = 0; i < 6; i++)
    {
        digitalWrite(temp, HIGH);
        delay(100);
        digitalWrite(temp, LOW);
        delay(66);
    }
}