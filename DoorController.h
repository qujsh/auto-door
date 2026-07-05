#ifndef DOOR_CONTROLLER_H
#define DOOR_CONTROLLER_H

#include <Arduino.h>
#include "Ultrasonic.h"
#include "ServoControl.h"
#include "Config.h"

class DoorController
{
public:
    DoorController();

    /**
     * 初始化
     */
    void begin(Ultrasonic *ultra,
               ServoControl *servo);

    /**
     * 主逻辑循环（loop调用）
     */
    void update();

    // ---- 状态查询（Web API 使用）----
    float getCurrentDistance() const;
    float getBaseline() const;
    float getCurrentDiff() const;
    bool getCurrentDetect() const;
    bool getCurrentPresent() const;
    DoorState getDoorState() const;

    // ---- Manual 模式 ----
    void setManualMode(bool manual);
    bool isManualMode() const;

    // ---- 远程标定 ----
    void triggerCalibrate();

private:

    void updateAutoMode(unsigned long now);

    void setDoorOpen();
    void setDoorClose();

private:

    Ultrasonic *ultrasonic;
    ServoControl *servo;

    DoorState state;

    float baseline;

    unsigned long lastSeenTime;
    unsigned long closeStartTime;

    bool manualMode;

    bool detecting;
    unsigned long detectStartTime;

    float currentDistance;
    float currentDiff;
    bool currentDetect;
    bool currentPresent;
    unsigned long lastPrintTime;
};

#endif