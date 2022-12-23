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

int wakingMsec = 0;

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLEAdvertising *pAdvertising = NULL;
uint16_t activeConnId = ESP_GATT_IF_NONE;
bool isAdvertising = false;

void printString(const char *string)
{
  M5.EPD.Active();
  canvas.deleteCanvas();
  canvas.createCanvas(540, 960);
  canvas.setTextSize(5);
  canvas.print(string);
  canvas.pushCanvas(0, 0, UPDATE_MODE_DU4);
  M5.update();
  M5.EPD.Sleep();
}

void sleepDeeply()
{
  printString("I'm sleeping deeply. For start this device, Please press right side button!");
  delay(500);
  // 右の釦を押すまでは寝てる
  esp_deep_sleep_start();
}

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    isAdvertising = false;
    activeConnId = pServer->getConnId();
    pAdvertising->stop();
  };

  void onDisconnect(BLEServer *pServer)
  {
    sleepDeeply();
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onRead(BLECharacteristic *pCharacteristic)
  {
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
  M5.TP.SetRotation(90);
  M5.EPD.Clear(true);
  M5.RTC.begin();
  WiFi.mode(WIFI_OFF);

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
  printString("BLE advertising ready! to start advertising, Please push right side button. This device will be sleeping deeply within 30sec.");

  // 右の釦を押すと起きる
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_38, LOW);
}

void loop()
{
  wakingMsec = wakingMsec + 100;

  // 起きてる時に釦が3秒間押されたら寝る
  if (M5.BtnP.pressedFor(3000))
  {
    sleepDeeply();
  }

  if (M5.BtnP.wasPressed())
  {
    // アドバタイズ中ではない&&アプリが接続されていない状態のみアドバタイズを開始できるようにする
    if (isAdvertising == false && activeConnId == ESP_GATT_IF_NONE)
    {
      isAdvertising = true;
      wakingMsec = 0;
      pAdvertising->start();
      printString("BLE advertising started! to stop advertising, Please hold right side button 3s! Also, this device will be sleeping deeply within 1min.");
    }
  }

  // アドバタイズ実施なし無接続で30秒経過 or アドバタイズ実施しても無接続のまま1分経過したら爆睡する
  if (activeConnId == ESP_GATT_IF_NONE)
  {
    if ((isAdvertising == false && wakingMsec == 30 * 1000) ||
        (isAdvertising == true && wakingMsec == 60 * 1000))
    {
      sleepDeeply();
    }
  }

  M5.update();
  delay(100);
}