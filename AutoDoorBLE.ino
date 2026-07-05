#include <Arduino.h>

#include "Config.h"
#include "ServoControl.h"
#include "Ultrasonic.h"
#include "BleManager.h"
#include "DoorController.h"
#include "WifiManager.h"
#include "WebServerManager.h"

//=====================================================
// 全局对象
//=====================================================
ServoControl servo;
Ultrasonic ultrasonic;
BleManager ble;
DoorController door;
WifiManager wifi;
WebServerManager web;

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
    // BLE（保留，向后兼容）
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

    //=============================
    // WiFi（自动：NVS凭证 → STA / AP）
    //=============================
    wifi.begin();

    //=============================
    // Web Server
    //=============================
    web.begin(&door, &servo, &wifi);

    if (wifi.isConnected())
    {
        Serial.print("Web: http://");
        Serial.print(wifi.getLocalIP());
        Serial.print("  or  http://");
        Serial.print(MDNS_HOSTNAME);
        Serial.println(".local");
    }
    else
    {
        Serial.print("AP Mode, connect to: ");
        Serial.println(AP_SSID);
        Serial.print("Then open: http://");
        Serial.println(wifi.getLocalIP());
    }

    Serial.println("System Ready");
}

//=====================================================
// 主循环
//=====================================================
void loop()
{
    wifi.update();

    ble.update();

    servo.update();

    door.update();
}