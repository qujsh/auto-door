#include "BleManager.h"

BleManager::BleManager()
    : server(nullptr),
      service(nullptr),
      wifiScanChar(nullptr),
      wifiConfigChar(nullptr),
      wifi(nullptr),
      connected(false),
      bleMode(false),
      configIndex(-1),
      newWiFiConfig(false)
{
}

void BleManager::begin(const char *deviceName,
                       const char *serviceUUID,
                       const char *wifiScanUUID,
                       const char *wifiConfigUUID,
                       WifiManager *wifiMgr)
{
    wifi = wifiMgr;

    NimBLEDevice::init(deviceName);

    NimBLEDevice::setPower(ESP_PWR_LVL_P9);

    server = NimBLEDevice::createServer();
    server->setCallbacks(this);

    service = server->createService(serviceUUID);

    // ① WiFi Scan（仅 Read）
    wifiScanChar = service->createCharacteristic(
        wifiScanUUID,
        NIMBLE_PROPERTY::READ
    );

    wifiScanChar->setCallbacks(this);

    // ② WiFi Config（Write + Notify）
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
// 读操作：WiFi Scan
//=====================================================
void BleManager::onRead(NimBLECharacteristic *characteristic,
                         NimBLEConnInfo &connInfo)
{
    Serial.println("BLE Read");

    if (characteristic == wifiScanChar && wifi)
    {
        String header = "\n输入格式: 索引+密码\n";
        String list   = wifi->getCachedNetworks();

        characteristic->setValue((header + list).c_str());
    }
}

//=====================================================
// 写操作：WiFi Config
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

    if (characteristic == wifiConfigChar)
    {
        parseWiFiConfig(value.c_str());
    }
}

//=====================================================
// 解析 WiFi Config: "0+123456"
//=====================================================
void BleManager::parseWiFiConfig(const char *data)
{
    String raw(data);

    int plusPos = raw.indexOf('+');

    if (plusPos < 0)
    {
        Serial.println("BLE: invalid format, use index+password");
        return;
    }

    configIndex = raw.substring(0, plusPos).toInt();
    configPassword = raw.substring(plusPos + 1);
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
