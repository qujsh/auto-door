#include "BleManager.h"

BleManager::BleManager()
    : server(nullptr),
      service(nullptr),
      wifiScanChar(nullptr),
      wifiConfigChar(nullptr),
      wifi(nullptr),
      connected(false),
      bleMode(false),
      configIndex(-1)
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
    NimBLEDevice::setMTU(256);

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

    newWiFiConfig.store(false);

    Serial.println("BLE Advertising Started");
}

void BleManager::update()
{
    //=============================
    // 推送 WiFi 连接状态
    //=============================
    if (wifi && wifi->hasStatusChanged() && connected)
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

    if (wifi)
    {
        wifi->startScan();
    }

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
        String list;
        wifi->getScanSnapshot(list, selectionSSIDs);

        if (list.length() == 0)
        {
            characteristic->setValue("\r\n扫描中，请稍后重试...\r\n");
        }
        else
        {
            String header = "\r\n输入格式: 索引+密码\r\n";
            characteristic->setValue((header + list).c_str());
        }

        wifi->startScan();
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
    raw.trim();

    int plusPos = raw.indexOf('+');

    if (plusPos < 0)
    {
        Serial.println("BLE: invalid format, use index+password");
        return;
    }

    configIndex = raw.substring(0, plusPos).toInt();

    configPassword = raw.substring(plusPos + 1);
    configPassword.trim();

    Serial.printf("BLE Config: index=%d pass=%s\n", configIndex, configPassword.c_str());

    if (configIndex < 0 || configIndex >= (int)selectionSSIDs.size())
    {
        configSSID = "";
        Serial.println("BLE: network index is not in the last read snapshot");
    }
    else
    {
        configSSID = selectionSSIDs[configIndex];
    }

    newWiFiConfig.store(true, std::memory_order_release);
}

bool BleManager::hasWiFiConfig(int &index, String &ssid, String &password)
{
    if (newWiFiConfig.exchange(false, std::memory_order_acquire))
    {
        index = configIndex;
        ssid = configSSID;
        password = configPassword;
        return true;
    }

    return false;
}
