#ifndef DOOR_CONTROLLER_H
#define DOOR_CONTROLLER_H

#include <Arduino.h>
#include <Preferences.h>
#include "../devices/TofSensor.h"
#include "../devices/ServoControl.h"
#include "../config/Config.h"

class DoorController
{
public:
    DoorController();

    void loadRuntimeSettings();

    /**
     * 初始化
     */
    void begin(TofSensor *sensor,
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
    void toggleFromButton();

    // ---- 远程标定 ----
    void triggerCalibrate();

    bool saveRuntimeSettings(int initialAngle,
                             int rotationAngle,
                             float distanceThresholdCm);
    int getInitialAngle() const;
    int getRotationAngle() const;
    int getOpenAngle() const;
    float getDistanceThresholdCm() const;

private:

    void updateAutoMode(unsigned long now);

    void setDoorOpen();
    void setDoorClose();

private:

    TofSensor *distanceSensor;
    ServoControl *servo;

    DoorState state;

    float baseline;

    int initialAngle;
    int rotationAngle;
    float distanceThresholdCm;

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
