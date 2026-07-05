#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include "DoorController.h"
#include "ServoControl.h"
#include "WifiManager.h"

class WebServerManager
{
public:
    WebServerManager();

    void begin(DoorController *door,
               ServoControl *servo,
               WifiManager *wifi);

private:
    AsyncWebServer *server;

    DoorController *door;
    ServoControl *servo;
    WifiManager *wifi;

    void setupRoutes();

    // 通用
    void handleRoot(AsyncWebServerRequest *request);

    // 控制面板路由（STA 模式）
    void setupControlRoutes();
    void handleStatus(AsyncWebServerRequest *request);
    void handleServo(AsyncWebServerRequest *request);
    void handleMode(AsyncWebServerRequest *request,
                    uint8_t *data,
                    size_t len,
                    size_t index,
                    size_t total);
    void handleCalibrate(AsyncWebServerRequest *request);

    // 配网路由（AP 模式）
    void setupAPRoutes();
    void handleWiFiScan(AsyncWebServerRequest *request);
    void handleWiFiSave(AsyncWebServerRequest *request,
                        uint8_t *data,
                        size_t len,
                        size_t index,
                        size_t total);

    // JSON 构建
    String buildStatusJson();

    const char *doorStateToString(DoorState s);
};

#endif
