#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

class BleManager : public NimBLEServerCallbacks,
                   public NimBLECharacteristicCallbacks
{
public:

    BleManager();

    /**
     * 初始化BLE
     */
    void begin(const char *deviceName,
               const char *serviceUUID,
               const char *characteristicUUID);

    /**
     * loop中调用（目前预留）
     */
    void update();

    /**
     * 是否已连接
     */
    bool isConnected() const;

    /**
     * 当前是否进入BLE控制模式
     */
    bool isBleMode() const;

    /**
     * 消费新命令：若收到新指令则返回 true，同时通过 angle 返回目标角度
     */
    bool hasNewCommand(int &angle);

private:

    //--------------------------------------------------
    // NimBLE 回调
    //--------------------------------------------------

    void onConnect(NimBLEServer *server,
                   NimBLEConnInfo &connInfo) override;

    void onDisconnect(NimBLEServer *server,
                      NimBLEConnInfo &connInfo,
                      int reason) override;

    void onRead(NimBLECharacteristic *characteristic,
                NimBLEConnInfo &connInfo) override;

    void onWrite(NimBLECharacteristic *characteristic,
                 NimBLEConnInfo &connInfo) override;

private:

    NimBLEServer *server;
    NimBLEService *service;
    NimBLECharacteristic *characteristic;

    volatile bool connected;

    volatile bool bleMode;

    volatile bool newCommand;

    volatile int targetAngle;
};

#endif