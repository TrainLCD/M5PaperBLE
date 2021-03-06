#include <M5EPD.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <WiFi.h>

M5EPD_Canvas canvas(&M5.EPD);

#define BLE_DEVICE_NAME "da5f6dec"
#define SERVICE_UUID "95820a99-7667-45c4-a48e-4fc262955aad"
#define CHARACTERISTIC_UUID "9bccfcd1-492b-4d83-987d-6ef8b0d0e0f5"
#define SIGNAL_ACK "A"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLEAdvertising *pAdvertising = NULL;
uint16_t activeConnId = ESP_GATT_IF_NONE;
bool isAdvertising = false;

void printString(const char *string)
{
  canvas.deleteCanvas();
  canvas.createCanvas(540, 960);
  canvas.setTextSize(5);
  canvas.print(string);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  M5.update();
}

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    printString("connected");
    isAdvertising = false;
    activeConnId = pServer->getConnId();
    pAdvertising->stop();
  };

  void onDisconnect(BLEServer *pServer)
  {
    printString("disconnect");
    delay(100);
    // どうせシャットダウンするので初期化は行わない
    // activeConnId = ESP_GATT_IF_NONE;
    // pAdvertising->start();
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
  M5.begin();
  M5.EPD.SetRotation(90);
  M5.EPD.Clear(true);
  M5.RTC.begin();
  WiFi.mode(WIFI_OFF);

  printString("WELCOME!");

  BLEDevice::init(BLE_DEVICE_NAME);
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
  pAdvertising = pServer->getAdvertising();
  printString("BLE advertising ready! to start advertising, Please push the right side button.");

  // 右の釦を押す
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, LOW);
  // 右の釦を押すまでは寝てる
  esp_light_sleep_start();
}

void loop()
{
  if (pAdvertising != nullptr)
  {
    // アドバタイズ中ではない&&アプリが接続されていない状態のみアドバタイズを再開できるようにする
    if (isAdvertising == false && activeConnId == ESP_GATT_IF_NONE)
    {
      pAdvertising->start();
      isAdvertising = true;
      printString("Advertising started! to stop advertising, Please reset the power supply.");
    }
  }
}