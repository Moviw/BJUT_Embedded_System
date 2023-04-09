#define BLINKER_BLE

#include <Blinker.h>

#define BUTTON_1 "ButtonKey"

BlinkerButton Button1(BUTTON_1);

void button1_callback(const String &state)
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    BLINKER_LOG("get button state: ", state);

    if (state == BLINKER_CMD_BUTTON_TAP)
    {
        BLINKER_LOG("Button tap!");

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        // Button1.text("Your button name", "describe");
        Button1.print();
    }
    else if (state == BLINKER_CMD_BUTTON_PRESSED)
    {
        BLINKER_LOG("Button pressed!");

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        // Button1.text("Your button name", "describe");
        Button1.print();
    }
    else if (state == BLINKER_CMD_BUTTON_RELEASED)
    {
        BLINKER_LOG("Button released!");

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        // Button1.text("Your button name", "describe");
        Button1.print();
    }
    else if (state == BLINKER_CMD_ON)
    {
        BLINKER_LOG("Toggle on!");

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        // Button1.text("Your button name", "describe");
        Button1.print("on");
    }
    else if (state == BLINKER_CMD_OFF)
    {
        BLINKER_LOG("Toggle off!");

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        // Button1.text("Your button name", "describe");
        Button1.print("off");
    }
    else
    {
        BLINKER_LOG("Get user setting: ", state);

        Button1.icon("icon_1");
        Button1.color("#FFFFFF");
        Button1.text("Your button name or describe");
        Button1.print();
    }
}

void dataRead(const String &data)
{
    BLINKER_LOG("Blinker readString: ", data);

    Blinker.vibrate();

    uint32_t BlinkerTime = millis();

    Blinker.print("millis", BlinkerTime);
}

void setup()
{
    Serial.begin(115200);
    BLINKER_DEBUG.stream(Serial);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Blinker.begin();
    Button1.attach(button1_callback);
    Blinker.attachData(dataRead);
}

void loop()
{
    Blinker.run();
}