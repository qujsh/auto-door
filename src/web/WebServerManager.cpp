#include "WebServerManager.h"
#include "WebPage.h"

WebServerManager::WebServerManager()
    : server(nullptr),
      door(nullptr),
      servo(nullptr),
      wifi(nullptr)
{
}

void WebServerManager::begin(DoorController *doorCtrl,
                             ServoControl *servoCtrl,
                             WifiManager *wifiMgr)
{
    door = doorCtrl;
    servo = servoCtrl;
    wifi = wifiMgr;

    server = new AsyncWebServer(Config::Network::webPort);

    setupRoutes();

    server->begin();

    if (Config::Debug::logStartup) Serial.println("Web Server Started");
}

void WebServerManager::setupRoutes()
{
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleRoot(request);
    });

    server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleStatus(request);
    });

    server->on("/api/servo", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleServo(request);
    });

    server->on(
        "/api/mode",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [this](AsyncWebServerRequest *request,
               uint8_t *data, size_t len,
               size_t index, size_t total) {
            handleMode(request, data, len, index, total);
        });

    server->on("/api/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleCalibrate(request);
    });

    server->on("/api/settings", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleSettings(request);
    });
}

void WebServerManager::handleRoot(AsyncWebServerRequest *request)
{
    if (Config::Debug::logHttp) Serial.println("HTTP GET /");
    request->send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::handleStatus(AsyncWebServerRequest *request)
{
    if (Config::Debug::logHttp) Serial.println("HTTP GET /api/status");
    String json = buildStatusJson();
    request->send(200, "application/json", json);
}

void WebServerManager::handleServo(AsyncWebServerRequest *request)
{
    if (!door->isManualMode())
    {
        if (Config::Debug::logErrors)
            Serial.println("HTTP servo rejected: manual mode required");
        request->send(409, "application/json",
                      "{\"error\":\"manual mode required\"}");
        return;
    }

    if (!request->hasParam("angle"))
    {
        request->send(400, "application/json", "{\"error\":\"missing angle\"}");
        return;
    }

    int angle = request->getParam("angle")->value().toInt();

    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    servo->setTargetAngle(angle);

    request->send(200, "application/json", "{\"ok\":true}");
}

void WebServerManager::handleMode(AsyncWebServerRequest *request,
                                  uint8_t *data,
                                  size_t len,
                                  size_t /*index*/,
                                  size_t /*total*/)
{
    String body;
    body.concat((char *)data, len);

    door->setManualMode(body.indexOf("manual") >= 0);

    request->send(200, "application/json", "{\"ok\":true}");
}

void WebServerManager::handleCalibrate(AsyncWebServerRequest *request)
{
    door->triggerCalibrate();

    char json[64];
    snprintf(json, sizeof(json),
             "{\"ok\":true,\"baseline\":%.2f}",
             door->getBaseline());

    request->send(200, "application/json", json);
}

void WebServerManager::handleSettings(AsyncWebServerRequest *request)
{
    if (!door->isManualMode())
    {
        request->send(409, "application/json",
                      "{\"error\":\"manual mode required\"}");
        return;
    }

    if (!request->hasParam("initial") ||
        !request->hasParam("rotation") ||
        !request->hasParam("threshold"))
    {
        request->send(400, "application/json",
                      "{\"error\":\"missing settings\"}");
        return;
    }

    const int initial = request->getParam("initial")->value().toInt();
    const int rotation = request->getParam("rotation")->value().toInt();
    const float threshold = request->getParam("threshold")->value().toFloat();
    if (!door->saveRuntimeSettings(initial, rotation, threshold))
    {
        request->send(400, "application/json",
                      "{\"error\":\"invalid settings\"}");
        return;
    }

    request->send(200, "application/json", "{\"ok\":true}");
}

String WebServerManager::buildStatusJson()
{
    String s = "{";

    s += "\"distance\":";
    s += String(door->getCurrentDistance(), 2);
    s += ",\"baseline\":";
    s += String(door->getBaseline(), 2);
    s += ",\"diff\":";
    s += String(door->getCurrentDiff(), 2);
    s += ",\"detect\":";
    s += door->getCurrentDetect() ? "true" : "false";
    s += ",\"present\":";
    s += door->getCurrentPresent() ? "true" : "false";
    s += ",\"door\":\"";
    s += doorStateToString(door->getDoorState());
    s += "\",\"servo\":";
    s += String(servo->getCurrentAngle());
    s += ",\"mode\":\"";
    s += door->isManualMode() ? "MANUAL" : "AUTO";
    s += "\",\"initialAngle\":";
    s += String(door->getInitialAngle());
    s += ",\"rotationAngle\":";
    s += String(door->getRotationAngle());
    s += ",\"openAngle\":";
    s += String(door->getOpenAngle());
    s += ",\"distanceThreshold\":";
    s += String(door->getDistanceThresholdCm(), 2);
    s += ",\"wifi\":";
    s += wifi->isConnected() ? "true" : "false";
    s += ",\"ip\":\"";
    s += wifi->getLocalIP();
    s += "\",\"rssi\":";
    s += String(wifi->getRSSI());
    s += "}";

    return s;
}

const char *WebServerManager::doorStateToString(DoorState s)
{
    switch (s)
    {
        case DoorState::Closed:         return "CLOSED";
        case DoorState::Open:           return "OPEN";
        case DoorState::WaitingToClose: return "WAIT_CLOSE";
        default:                    return "UNKNOWN";
    }
}
