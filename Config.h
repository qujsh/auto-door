#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//=====================================================
// 引脚配置
//=====================================================
constexpr uint8_t TRIG_PIN = 2;
constexpr uint8_t ECHO_PIN = 1;
constexpr uint8_t SERVO_PIN = 5;

//=====================================================
// 舵机角度
//=====================================================
constexpr int SERVO_OPEN_ANGLE  = 90;
constexpr int SERVO_CLOSE_ANGLE = 0;

//=====================================================
// 舵机运动参数
//=====================================================
constexpr unsigned long SERVO_UPDATE_INTERVAL = 15;

constexpr int SERVO_OPEN_STEP  = 30;
constexpr int SERVO_CLOSE_STEP = 10;

//=====================================================
// 超声波检测参数
//=====================================================
constexpr float DISTANCE_CHANGE_THRESHOLD = 2.5f;
constexpr float MAX_VALID_DISTANCE = 80.0f;

//=====================================================
// 自动门参数
//=====================================================
constexpr unsigned long PRESENCE_TIMEOUT = 1500;
constexpr unsigned long CLOSE_DELAY      = 2000;
constexpr unsigned long DETECT_DEBOUNCE  = 200;

//=====================================================
// BLE
//=====================================================
#define BLE_DEVICE_NAME          "MyESP32Mini"

#define SERVICE_UUID             "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

#define SERVO_CHAR_UUID          "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define WIFI_SCAN_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b2901"
#define WIFI_CONFIG_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b2902"

//=====================================================
// 门状态
//=====================================================
enum class DoorState
{
    CLOSED,
    OPEN,
    WAIT_CLOSE
};

//=====================================================
// mDNS
//=====================================================
constexpr char MDNS_HOSTNAME[] = "autodoor";

//=====================================================
// Web Server
//=====================================================
constexpr uint16_t WEB_PORT = 80;

//=====================================================
// 调试开关
//=====================================================
constexpr bool DEBUG_DISTANCE = true;
constexpr unsigned long DEBUG_PRINT_INTERVAL = 300;

#endif