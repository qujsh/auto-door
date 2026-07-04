#ifndef DOOR_CONTROLLER_H
#define DOOR_CONTROLLER_H

#include <Arduino.h>
#include "Ultrasonic.h"
#include "ServoControl.h"
#include "BleManager.h"
#include "Config.h"

class DoorController
{
public:
    DoorController();

    /**
     * 初始化
     */
    void begin(Ultrasonic *ultra,
               ServoControl *servo,
               BleManager *ble);

    /**
     * 主逻辑循环（loop调用）
     */
    void update();

private:

    void updateAutoMode(unsigned long now);

    void updateBleMode();

    void setDoorOpen();
    void setDoorClose();

private:

    Ultrasonic *ultrasonic;
    ServoControl *servo;
    BleManager *ble;

    DoorState state;

    float baseline;

    unsigned long lastSeenTime;
    unsigned long closeStartTime;

    bool previousBleMode;

    bool detecting;
    unsigned long detectStartTime;
};

#endif