#include <Arduino.h>

#include "Config.h"
#include "ServoControl.h"
#include "Ultrasonic.h"
#include "BleManager.h"
#include "DoorController.h"
#include "WifiManager.h"
#include "WebServerManager.h"

//=====================================================
// 系统状态
//=====================================================
enum SystemState
{
    CONFIGURING,
    RUNNING
};

//=====================================================
// 全局对象
//=====================================================
ServoControl servo;
Ultrasonic ultrasonic;
BleManager ble;
DoorController door;
WifiManager wifi;
WebServerManager web;

SystemState sysState = CONFIGURING;

//=====================================================
// 根据索引从缓存列表中解析 SSID
//=====================================================
String getSSIDbyIndex(int targetIndex)
{
    String networks = wifi.getCachedNetworks();

    int start = 0;

    for (int i = 0; i <= targetIndex; i++)
    {
        int end = networks.indexOf('\n', start);

        if (i == targetIndex)
        {
            String line;

            if (end < 0)
            {
                line = networks.substring(start);
            }
            else
            {
                line = networks.substring(start, end);
            }

            // 格式: 0|Redmi_E490|良好
            //       p1  p2
            int p1 = line.indexOf('|');
            int p2 = line.indexOf('|', p1 + 1);

            if (p1 >= 0 && p2 > p1)
            {
                return line.substring(p1 + 1, p2);
            }

            return "";
        }

        if (end < 0) break;

        start = end + 1;
    }

    return "";
}

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
        WIFI_SCAN_CHAR_UUID,
        WIFI_CONFIG_CHAR_UUID,
        &wifi
    );

    //=============================
    // Door Controller
    //=============================
    door.begin(
        &ultrasonic,
        &servo
    );

    //=============================
    // WiFi：读 NVS 尝试连接
    //=============================
    wifi.begin();

    //=============================
    // Web Server（始终创建一次）
    //=============================
    web.begin(&door, &servo, &wifi);

    if (wifi.isConnected())
    {
        sysState = RUNNING;

        Serial.print("Web: http://");
        Serial.print(wifi.getLocalIP());
        Serial.print("  or  http://");
        Serial.print(MDNS_HOSTNAME);
        Serial.println(".local");
    }
    else
    {
        sysState = CONFIGURING;

        Serial.println("No WiFi. Use nRF Connect to:");
        Serial.println("  Read WiFiScan -> select index");
        Serial.println("  Write WiFiConfig -> 0+password");
    }

    Serial.println("System Ready");
}

//=====================================================
// 主循环
//=====================================================
void loop()
{
    wifi.update();

    servo.update();

    //=============================
    // BLE 始终运行（配网 / 切换 WiFi）
    //=============================
    ble.update();

    int index;
    String password;

    if (ble.hasWiFiConfig(index, password))
    {
        String ssid = getSSIDbyIndex(index);

        if (ssid.length() > 0)
        {
            Serial.print("Connect via BLE: ");
            Serial.println(ssid);

            wifi.saveCredentials(ssid.c_str(), password.c_str());

            if (sysState == CONFIGURING)
            {
                wifi.tryConnect(ssid.c_str(), password.c_str());
            }
            else
            {
                Serial.println("WiFi updated, restarting...");
                delay(500);
                ESP.restart();
            }
        }
        else
        {
            Serial.println("Invalid WiFi index");
        }
    }

    //=============================
    // CONFIGURING → RUNNING 过渡
    //=============================
    if (sysState == CONFIGURING && wifi.isConnected())
    {
        sysState = RUNNING;

        Serial.print("Web: http://");
        Serial.print(wifi.getLocalIP());
        Serial.print("  or  http://");
        Serial.print(MDNS_HOSTNAME);
        Serial.println(".local");
    }

    //=============================
    // 日常运行
    //=============================
    if (sysState == RUNNING)
    {
        door.update();
    }
}
