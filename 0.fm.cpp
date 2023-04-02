#include <Arduino.h>
#include <Wire.h>

#include <radio.h>
#include <RDA5807M.h>

#define FIX_BAND RADIO_BAND_FM ///< The band that will be tuned by this sketch is FM.
#define FIX_STATION 9740       ///< The station that will be tuned by this sketch is 89.30 MHz.
#define FIX_VOLUME 3           ///< The volume that will be set by this sketch is level 4.

RDA5807M radio; // Create an instance of Class for RDA5807M Chip

/// Setup a FM only radio configuration
/// with some debugging on the Serial port
void setup()
{
    // open the Serial port
    Serial.begin(9600);
    Serial.println("RDA5807M Radio...");
    delay(200);

    // Standard I2C/Wire pins for Arduino UNO:  = SDA:A4, SCL:A5
    // Standard I2C/Wire pins for ESP8266: SDA:D2, SCL:D1
    // Standard I2C/Wire pins for ESP32: SDA:21, SCL:22
    Wire.begin();

    // Enable information to the Serial port
    radio.debugEnable(true);
    radio._wireDebug(false);

    // Initialize the Radio
    if (!radio.initWire(Wire))
    {
        Serial.println("no radio chip found.");
        delay(4000);
        ESP.restart();
    };

    // Set all radio setting to the fixed values.
    radio.setBandFrequency(FIX_BAND, FIX_STATION);
    radio.setVolume(FIX_VOLUME);
    radio.setMono(false);
    radio.setMute(false);
} // setup

/// show the current chip data every 3 seconds.
void loop()
{
    char s[12];
    radio.formatFrequency(s, sizeof(s));
    Serial.print("Station:");
    Serial.println(s);

    Serial.print("Radio:");
    radio.debugRadioInfo();

    Serial.print("Audio:");
    radio.debugAudioInfo();

    delay(3000);
}