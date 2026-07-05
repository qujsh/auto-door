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
#define BLE_DEVICE_NAME          "AutoDoor"

#define SERVICE_UUID             "a550d001-0000-a550-d001-a550d0010001"

#define WIFI_SCAN_CHAR_UUID      "a550d001-0000-a550-d001-a550d001a001"
#define WIFI_CONFIG_CHAR_UUID    "a550d001-0000-a550-d001-a550d001a002"

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