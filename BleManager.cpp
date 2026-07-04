#include "BleManager.h"

BleManager::BleManager()
    : server(nullptr),
      service(nullptr),
      characteristic(nullptr),
      connected(false),
      bleMode(false),
      newCommand(false),
      targetAngle(0)
{
}

void BleManager::begin(const char *deviceName,
                       const char *serviceUUID,
                       const char *characteristicUUID)
{
    NimBLEDevice::init(deviceName);

    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    server = NimBLEDevice::createServer();
    server->setCallbacks(this);

    service = server->createService(serviceUUID);

    characteristic = service->createCharacteristic(
        characteristicUUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );

    characteristic->setCallbacks(this);
    characteristic->setValue("Hello");

    service->start();

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(serviceUUID);
    advertising->setName(deviceName);
    advertising->enableScanResponse(true);

    NimBLEDevice::startAdvertising();

    Serial.println("BLE Advertising Started");
}

void BleManager::update()
{
    // 当前版本无需轮询逻辑（预留扩展）
}

bool BleManager::isConnected() const
{
    return connected;
}

bool BleManager::isBleMode() const
{
    return bleMode;
}

bool BleManager::hasNewCommand(int &angle)
{
    bool temp = newCommand;

    if (temp)
    {
        angle = targetAngle;
        newCommand = false;
    }

    return temp;
}

//=====================================================
// BLE 连接回调
//=====================================================
void BleManager::onConnect(NimBLEServer *server,
                           NimBLEConnInfo &connInfo)
{
    connected = true;
    bleMode = true;

    Serial.println("BLE Connected");
}

//=====================================================
// BLE 断开回调
//=====================================================
void BleManager::onDisconnect(NimBLEServer *server,
                              NimBLEConnInfo &connInfo,
                              int reason)
{
    connected = false;
    bleMode = false;

    Serial.print("BLE Disconnected, reason=");
    Serial.println(reason);

    NimBLEDevice::startAdvertising();
}

//=====================================================
// 读操作
//=====================================================
void BleManager::onRead(NimBLECharacteristic *characteristic,
                         NimBLEConnInfo &connInfo)
{
    Serial.println("BLE Read");
}

//=====================================================
// 写操作（核心）
//=====================================================
void BleManager::onWrite(NimBLECharacteristic *characteristic,
                          NimBLEConnInfo &connInfo)
{
    std::string value = characteristic->getValue();

    if (value.empty())
    {
        return;
    }

    Serial.print("BLE RX: ");

    for (char c : value)
    {
        Serial.print(c);
    }

    Serial.println();

    // 校验并转换角度
    char *endPtr;
    long val = strtol(value.c_str(), &endPtr, 10);

    if (endPtr == value.c_str() || *endPtr != '\0')
    {
        Serial.println("BLE: Invalid angle format, ignored");
        return;
    }

    int angle = (int)val;

    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    targetAngle = angle;
    newCommand = true;

    // 回显
    characteristic->setValue(value);
    characteristic->notify();
}