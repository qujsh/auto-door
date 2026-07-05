#include "BleManager.h"

BleManager::BleManager()
    : server(nullptr),
      service(nullptr),
      servoChar(nullptr),
      wifiScanChar(nullptr),
      wifiConfigChar(nullptr),
      wifi(nullptr),
      servo(nullptr),
      connected(false),
      bleMode(false),
      newCommand(false),
      targetAngle(0),
      configIndex(-1),
      newWiFiConfig(false)
{
}

void BleManager::begin(const char *deviceName,
                       const char *serviceUUID,
                       const char *servoUUID,
                       const char *wifiScanUUID,
                       const char *wifiConfigUUID,
                       WifiManager *wifiMgr,
                       ServoControl *servoCtrl)
{
    wifi = wifiMgr;
    servo = servoCtrl;

    NimBLEDevice::init(deviceName);

    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    server = NimBLEDevice::createServer();
    server->setCallbacks(this);

    service = server->createService(serviceUUID);

    // ① Servo（兼容旧协议）
    servoChar = service->createCharacteristic(
        servoUUID,
        NIMBLE_PROPERTY::READ |
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );

    servoChar->setCallbacks(this);
    servoChar->setValue("0");

    // ② WiFi Scan（仅 Read）
    wifiScanChar = service->createCharacteristic(
        wifiScanUUID,
        NIMBLE_PROPERTY::READ
    );

    wifiScanChar->setCallbacks(this);

    // ③ WiFi Config（Write + Notify）
    wifiConfigChar = service->createCharacteristic(
        wifiConfigUUID,
        NIMBLE_PROPERTY::WRITE |
        NIMBLE_PROPERTY::NOTIFY
    );

    wifiConfigChar->setCallbacks(this);

    service->start();

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();

    advertising->addServiceUUID(serviceUUID);
    advertising->setName(deviceName);
    advertising->enableScanResponse(true);

    NimBLEDevice::startAdvertising();

    newWiFiConfig = false;

    Serial.println("BLE Advertising Started");
}

void BleManager::update()
{
    //=============================
    // 推送 WiFi 连接状态
    //=============================
    if (wifi && wifi->hasStatusChanged())
    {
        String status = wifi->getConnectStatus();

        if (wifiConfigChar)
        {
            wifiConfigChar->setValue(status.c_str());
            wifiConfigChar->notify();

            Serial.print("BLE Notify: ");
            Serial.println(status);
        }
    }
}

void BleManager::stop()
{
    NimBLEDevice::stopAdvertising();

    Serial.println("BLE Stopped");
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
// 读操作：按 UUID 分发
//=====================================================
void BleManager::onRead(NimBLECharacteristic *characteristic,
                         NimBLEConnInfo &connInfo)
{
    Serial.println("BLE Read");

    if (characteristic == wifiScanChar && wifi)
    {
        String list = wifi->getCachedNetworks();
        characteristic->setValue(list.c_str());
    }

    if (characteristic == servoChar && servo)
    {
        characteristic->setValue(String(servo->getCurrentAngle()).c_str());
    }
}

//=====================================================
// 写操作：按 UUID 分发
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

    //=============================
    // WiFi Config Characteristic
    //=============================
    if (characteristic == wifiConfigChar)
    {
        parseWiFiConfig(value.c_str());
        return;
    }

    //=============================
    // Servo Characteristic（数字角度）
    //=============================
    if (characteristic == servoChar)
    {
        char *endPtr;
        long val = strtol(value.c_str(), &endPtr, 10);

        if (endPtr == value.c_str() || *endPtr != '\0')
        {
            Serial.println("BLE: Invalid angle, ignored");
            return;
        }

        int angle = (int)val;

        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        targetAngle = angle;
        newCommand = true;

        characteristic->setValue(value);
        characteristic->notify();
    }
}

//=====================================================
// 解析 WiFi Config: "CFG|index|password"
//=====================================================
void BleManager::parseWiFiConfig(const char *data)
{
    String raw(data);

    // 检查 CFG| 前缀
    if (!raw.startsWith("CFG|"))
    {
        Serial.println("BLE: invalid WiFi config format");
        return;
    }

    // 跳过 "CFG|"
    int pos1 = raw.indexOf('|');

    if (pos1 < 0) return;

    int pos2 = raw.indexOf('|', pos1 + 1);

    if (pos2 < 0)
    {
        Serial.println("BLE: no password separator");
        return;
    }

    String idxStr = raw.substring(pos1 + 1, pos2);

    configPassword = raw.substring(pos2 + 1);
    configIndex = idxStr.toInt();

    newWiFiConfig = true;

    Serial.print("BLE Config: index=");
    Serial.print(configIndex);
    Serial.print(" pass=");
    Serial.println(configPassword);
}

bool BleManager::hasWiFiConfig(int &index, String &password)
{
    bool temp = newWiFiConfig;

    if (temp)
    {
        index = configIndex;
        password = configPassword;
        newWiFiConfig = false;
    }

    return temp;
}
