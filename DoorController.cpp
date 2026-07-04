#include "DoorController.h"

DoorController::DoorController()
    : ultrasonic(nullptr),
      servo(nullptr),
      ble(nullptr),
      state(DoorState::CLOSED),
      baseline(0),
      lastSeenTime(0),
      closeStartTime(0),
      previousBleMode(false),
      detecting(false),
      detectStartTime(0)
{
}

void DoorController::begin(Ultrasonic *ultra,
                           ServoControl *servoCtrl,
                           BleManager *bleMgr)
{
    ultrasonic = ultra;
    servo = servoCtrl;
    ble = bleMgr;

    baseline = ultrasonic->getBaseline();

    state = DoorState::CLOSED;

    lastSeenTime = millis();
    closeStartTime = 0;

    previousBleMode = false;
    detecting = false;
    detectStartTime = 0;

    servo->setTargetAngle(SERVO_CLOSE_ANGLE);
}

void DoorController::update()
{
    unsigned long now = millis();

    bool isBle = ble->isBleMode();

    //=============================
    // BLE → AUTO 切换：强制关门并重置状态
    //=============================
    if (previousBleMode && !isBle)
    {
        state = DoorState::CLOSED;
        servo->setTargetAngle(SERVO_CLOSE_ANGLE);
        lastSeenTime = now;
        closeStartTime = 0;
        detecting = false;

        Serial.println("BLE Disconnected -> AUTO mode, door closed");
    }

    previousBleMode = isBle;

    //=============================
    // BLE 模式：直接接管舵机
    //=============================
    if (isBle)
    {
        updateBleMode();
        return;
    }

    //=============================
    // 自动模式
    //=============================
    updateAutoMode(now);
}

//=====================================================
// BLE模式：手机直接控制舵机
//=====================================================
void DoorController::updateBleMode()
{
    int angle;

    if (ble->hasNewCommand(angle))
    {
        servo->setTargetAngle(angle);
    }
}

//=====================================================
// 自动模式核心逻辑（复刻你的原始代码）
//=====================================================
void DoorController::updateAutoMode(unsigned long now)
{
    float d = ultrasonic->readDistance();

    if (d < 0)
    {
        return;
    }

    float diff = baseline - d;

    bool detect = (diff >= DISTANCE_CHANGE_THRESHOLD);

    if (detect)
    {
        Serial.print("DIST: baseline=");
        Serial.print(baseline);
        Serial.print(" current=");
        Serial.print(d);
        Serial.print(" diff=");
        Serial.println(diff);
    }

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

        if (now - detectStartTime >= DETECT_DEBOUNCE)
        {
            lastSeenTime = now;
        }
    }
    else
    {
        detecting = false;
    }

    bool isPresent = (now - lastSeenTime) < PRESENCE_TIMEOUT;

    //=============================
    // 状态机
    //=============================
    switch (state)
    {
        case DoorState::CLOSED:
        {
            if (isPresent)
            {
                setDoorOpen();
                state = DoorState::OPEN;
            }
        }
        break;

        case DoorState::OPEN:
        {
            if (!isPresent)
            {
                state = DoorState::WAIT_CLOSE;
                closeStartTime = now;
            }
        }
        break;

        case DoorState::WAIT_CLOSE:
        {
            if (isPresent)
            {
                state = DoorState::OPEN;
            }
            else if (now - closeStartTime >= CLOSE_DELAY)
            {
                setDoorClose();
                state = DoorState::CLOSED;
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
    servo->setTargetAngle(SERVO_OPEN_ANGLE);
}

//=====================================================
// 关门
//=====================================================
void DoorController::setDoorClose()
{
    servo->setTargetAngle(SERVO_CLOSE_ANGLE);
}