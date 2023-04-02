#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();

        if (value.length() > 0)
        {
            if (value == "like")
            {
                Serial.println("this is like ");
            }
            else if (value == "dislike")
            {
                Serial.println("this is dislike ");
            }
            else if (value == "dislike")
            {
                Serial.println("this is dislike ");
            }
            else if (value == "update")
            {
                Serial.println("this is update ");
            }
            else
            {
                Serial.println("wrong input");
            }
        }
    }
};

void setup()
{
    Serial.begin(9600);

    BLEDevice::init("Mooovi");
    BLEServer *pServer = BLEDevice::createServer();

    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    pCharacteristic->setValue("Hello World");
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void loop()
{
    delay(2000);
}