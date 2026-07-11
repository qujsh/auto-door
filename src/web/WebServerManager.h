#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <ESPAsyncWebServer.h>
#include "../control/DoorController.h"
#include "../devices/ServoControl.h"
#include "../network/WifiManager.h"

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

    void handleRoot(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleServo(AsyncWebServerRequest *request);
    void handleMode(AsyncWebServerRequest *request,
                    uint8_t *data,
                    size_t len,
                    size_t index,
                    size_t total);
    void handleCalibrate(AsyncWebServerRequest *request);

    String buildStatusJson();
    const char *doorStateToString(DoorState s);
};

#endif
