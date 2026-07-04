#include <Arduino.h>

#include "Config.h"
#include "ServoControl.h"
#include "Ultrasonic.h"
#include "BleManager.h"
#include "DoorController.h"

//=====================================================
// 全局对象
//=====================================================
ServoControl servo;
Ultrasonic ultrasonic;
BleManager ble;
DoorController door;

//=====================================================
// 初始化
//=====================================================
void setup()
{
    Serial.begin(115200);

    delay(500);

    Serial.println("=================================");
    Serial.println("AutoDoorBLE Starting...");
    Serial.println("=================================");

    //=============================
    // Servo
    //=============================
    servo.begin(
        SERVO_PIN,
        SERVO_CLOSE_ANGLE,
        SERVO_UPDATE_INTERVAL,
        SERVO_OPEN_STEP,
        SERVO_CLOSE_STEP
    );

    //=============================
    // Ultrasonic
    //=============================
    ultrasonic.begin(
        TRIG_PIN,
        ECHO_PIN,
        MAX_VALID_DISTANCE
    );

    ultrasonic.calibrate();

    //=============================
    // BLE
    //=============================
    ble.begin(
        BLE_DEVICE_NAME,
        SERVICE_UUID,
        CHARACTERISTIC_UUID
    );

    //=============================
    // Door Controller
    //=============================
    door.begin(
        &ultrasonic,
        &servo,
        &ble
    );

    Serial.println("System Ready");
}

//=====================================================
// 主循环
//=====================================================
void loop()
{
    // 1. BLE更新（目前内部轻量）
    ble.update();

    // 2. 舵机非阻塞更新
    servo.update();

    // 3. 门逻辑（自动 / BLE）
    door.update();
}