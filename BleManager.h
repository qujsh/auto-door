#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "WifiManager.h"

class BleManager : public NimBLEServerCallbacks,
                   public NimBLECharacteristicCallbacks
{
public:

    BleManager();

    void begin(const char *deviceName,
               const char *serviceUUID,
               const char *wifiScanUUID,
               const char *wifiConfigUUID,
               WifiManager *wifiMgr);

    void update();

    bool isConnected() const;

    bool isBleMode() const;

    void stop();

    bool hasWiFiConfig(int &index, String &password);

private:

    void onConnect(NimBLEServer *server,
                   NimBLEConnInfo &connInfo) override;

    void onDisconnect(NimBLEServer *server,
                      NimBLEConnInfo &connInfo,
                      int reason) override;

    void onRead(NimBLECharacteristic *characteristic,
                NimBLEConnInfo &connInfo) override;

    void onWrite(NimBLECharacteristic *characteristic,
                 NimBLEConnInfo &connInfo) override;

    void parseWiFiConfig(const char *data);

private:

    NimBLEServer *server;
    NimBLEService *service;
    NimBLECharacteristic *wifiScanChar;
    NimBLECharacteristic *wifiConfigChar;

    WifiManager *wifi;

    volatile bool connected;
    volatile bool bleMode;

    int configIndex;
    String configPassword;
    volatile bool newWiFiConfig;
};

#endif
