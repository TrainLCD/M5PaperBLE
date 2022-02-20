#include <M5EPD.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <WiFi.h>

M5EPD_Canvas canvas(&M5.EPD);

void printString(const char *string)
{
  canvas.deleteCanvas();
  canvas.createCanvas(540, 960);
  canvas.setTextSize(5);
  canvas.print(string);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
}

#define SERVICE_UUID "95820a99-7667-45c4-a48e-4fc262955aad"
#define CHARACTERISTIC_UUID "9bccfcd1-492b-4d83-987d-6ef8b0d0e0f5"
#define SIGNAL_ACK "A"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    printString("connect");
  };

  void onDisconnect(BLEServer *pServer)
  {
    printString("disconnect");
    delay(100);
    M5.shutdown(1);
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
    printString("read");
    pCharacteristic->setValue(SIGNAL_ACK);
  }

  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    printString(value.c_str());
  }
};

void setup()
{
  M5.begin(false, false, false, false, false);
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);
  M5.RTC.begin();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  printString("BLE Ready!");

  BLEDevice::init("m5-paper");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_INDICATE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  if (M5.BtnP.wasPressed())
  {
    printString("bye...");
    delay(100);
    M5.shutdown(1);
  }
  M5.update();
  delay(100);
}