#include "DoorController.h"

DoorController::DoorController()
    : distanceSensor(nullptr),
      servo(nullptr),
      state(DoorState::Closed),
      baseline(0),
      initialAngle(Config::Servo::closedAngle),
      rotationAngle(Config::Servo::openAngle - Config::Servo::closedAngle),
      distanceThresholdCm(Config::Tof::changeThresholdCm),
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

    servo->setTargetAngle(initialAngle);
}

void DoorController::loadRuntimeSettings()
{
    Preferences preferences;
    preferences.begin("doorcfg", true);
    initialAngle = constrain(preferences.getInt("initial", Config::Servo::closedAngle), 0, 180);
    rotationAngle = constrain(
        preferences.getInt("rotation", Config::Servo::openAngle - Config::Servo::closedAngle),
        -180, 180);
    distanceThresholdCm = preferences.getFloat("threshold", Config::Tof::changeThresholdCm);
    preferences.end();
    if (distanceThresholdCm < 0.1F || distanceThresholdCm > 200.0F)
        distanceThresholdCm = Config::Tof::changeThresholdCm;
    if (initialAngle + rotationAngle < 0 || initialAngle + rotationAngle > 180)
        rotationAngle = constrain(rotationAngle, -initialAngle, 180 - initialAngle);
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

    bool detect = (diff >= distanceThresholdCm);

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
    servo->setTargetAngle(getOpenAngle());
}

//=====================================================
// 关门
//=====================================================
void DoorController::setDoorClose()
{
    servo->setTargetAngle(initialAngle);
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

void DoorController::toggleFromButton()
{
    manualMode = true;
    if (state == DoorState::Closed)
    {
        setDoorOpen();
        state = DoorState::Open;
    }
    else
    {
        setDoorClose();
        state = DoorState::Closed;
    }
    detecting = false;
    currentDetect = false;
    currentPresent = false;
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

bool DoorController::saveRuntimeSettings(int newInitialAngle,
                                         int newRotationAngle,
                                         float newDistanceThresholdCm)
{
    if (!manualMode ||
        newInitialAngle < 0 || newInitialAngle > 180 ||
        newRotationAngle < -180 || newRotationAngle > 180 ||
        newInitialAngle + newRotationAngle < 0 ||
        newInitialAngle + newRotationAngle > 180 ||
        newDistanceThresholdCm < 0.1F || newDistanceThresholdCm > 200.0F)
    {
        return false;
    }

    initialAngle = newInitialAngle;
    rotationAngle = newRotationAngle;
    distanceThresholdCm = newDistanceThresholdCm;
    state = DoorState::Closed;
    detecting = false;
    currentDetect = false;
    currentPresent = false;
    servo->setTargetAngle(initialAngle);

    Preferences preferences;
    preferences.begin("doorcfg", false);
    preferences.putInt("initial", initialAngle);
    preferences.putInt("rotation", rotationAngle);
    preferences.putFloat("threshold", distanceThresholdCm);
    preferences.end();
    return true;
}

int DoorController::getInitialAngle() const
{
    return initialAngle;
}

int DoorController::getRotationAngle() const
{
    return rotationAngle;
}

int DoorController::getOpenAngle() const
{
    return constrain(initialAngle + rotationAngle, 0, 180);
}

float DoorController::getDistanceThresholdCm() const
{
    return distanceThresholdCm;
}
