#include "DoorController.h"

DoorController::DoorController()
    : distanceSensor(nullptr),
      servo(nullptr),
      state(DoorState::Closed),
      baseline(0),
      lastSeenTime(0),
      closeStartTime(0),
      manualMode(false),
      detecting(false),
      detectStartTime(0),
      currentDistance(-1),
      currentDiff(0),
      currentDetect(false),
      currentPresent(false),
      lastPrintTime(0)
{
}

void DoorController::begin(TofSensor *sensor,
                           ServoControl *servoCtrl)
{
    distanceSensor = sensor;
    servo = servoCtrl;

    baseline = distanceSensor->getBaseline();

    state = DoorState::Closed;

    lastSeenTime = 0;
    closeStartTime = 0;

    manualMode = false;
    detecting = false;
    detectStartTime = 0;

    servo->setTargetAngle(Config::Servo::closedAngle);
}

void DoorController::update()
{
    if (manualMode)
    {
        return;
    }

    updateAutoMode(millis());
}

//=====================================================
// 自动模式核心逻辑（复刻你的原始代码）
//=====================================================
void DoorController::updateAutoMode(unsigned long now)
{
    float d = distanceSensor->readDistance();

    currentDistance = d;

    if (d < 0)
    {
        return;
    }

    float diff = baseline - d;

    currentDiff = diff;

    bool detect = (diff >= Config::Tof::changeThresholdCm);

    currentDetect = detect;

    //=============================
    // 防抖：连续检测持续 DETECT_DEBOUNCE ms 后才确认有人
    //=============================
    if (detect)
    {
        if (!detecting)
        {
            detecting = true;
            detectStartTime = now;
        }

        if (now - detectStartTime >= Config::Door::detectionDebounceMs)
        {
            lastSeenTime = now;
        }
    }
    else
    {
        detecting = false;
    }

    bool isPresent = (now - lastSeenTime) < Config::Door::presenceTimeoutMs;

    currentPresent = isPresent;

    if (Config::Debug::logDoor)
    {
        if (now - lastPrintTime >= Config::Debug::printIntervalMs)
        {
            lastPrintTime = now;

            Serial.print("Distance=");
            Serial.print(currentDistance, 2);
            Serial.print("cm  Baseline=");
            Serial.print(baseline, 2);
            Serial.print("cm  Diff=");
            Serial.print(currentDiff, 2);
            Serial.print("cm  Detect=");
            Serial.print(currentDetect ? "YES" : "NO");
            Serial.print("  Present=");
            Serial.print(currentPresent ? "YES" : "NO");
            Serial.print("  Door=");

            switch (state)
            {
                case DoorState::Closed:
                    Serial.print("CLOSED");
                    break;
                case DoorState::Open:
                    Serial.print("OPEN");
                    break;
                case DoorState::WaitingToClose:
                    Serial.print("WAIT_CLOSE");
                    break;
            }

            Serial.print("  Servo=");
            Serial.print(servo->getCurrentAngle());
            Serial.println("°");
        }
    }

    //=============================
    // 状态机
    //=============================
    switch (state)
    {
        case DoorState::Closed:
        {
            if (isPresent)
            {
                setDoorOpen();
                state = DoorState::Open;
            }
        }
        break;

        case DoorState::Open:
        {
            if (!isPresent)
            {
                state = DoorState::WaitingToClose;
                closeStartTime = now;
            }
        }
        break;

        case DoorState::WaitingToClose:
        {
            if (isPresent)
            {
                state = DoorState::Open;
            }
            else if (now - closeStartTime >= Config::Door::closeDelayMs)
            {
                setDoorClose();
                state = DoorState::Closed;
            }
        }
        break;
    }
}

//=====================================================
// 开门
//=====================================================
void DoorController::setDoorOpen()
{
    servo->setTargetAngle(Config::Servo::openAngle);
}

//=====================================================
// 关门
//=====================================================
void DoorController::setDoorClose()
{
    servo->setTargetAngle(Config::Servo::closedAngle);
}

//=====================================================
// 状态查询（Web API）
//=====================================================
float DoorController::getCurrentDistance() const
{
    return currentDistance;
}

float DoorController::getBaseline() const
{
    return baseline;
}

float DoorController::getCurrentDiff() const
{
    return currentDiff;
}

bool DoorController::getCurrentDetect() const
{
    return currentDetect;
}

bool DoorController::getCurrentPresent() const
{
    return currentPresent;
}

DoorState DoorController::getDoorState() const
{
    return state;
}

//=====================================================
// Manual 模式
//=====================================================
void DoorController::setManualMode(bool manual)
{
    manualMode = manual;
}

bool DoorController::isManualMode() const
{
    return manualMode;
}

//=====================================================
// 远程标定
//=====================================================
void DoorController::triggerCalibrate()
{
    distanceSensor->calibrate();

    baseline = distanceSensor->getBaseline();

    state = DoorState::Closed;

    detecting = false;
}
