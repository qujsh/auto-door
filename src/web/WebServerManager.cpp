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

    Serial.println("Web Server Started");
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
}

void WebServerManager::handleRoot(AsyncWebServerRequest *request)
{
    Serial.println("HTTP GET /");
    request->send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::handleStatus(AsyncWebServerRequest *request)
{
    Serial.println("HTTP GET /api/status");
    String json = buildStatusJson();
    request->send(200, "application/json", json);
}

void WebServerManager::handleServo(AsyncWebServerRequest *request)
{
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
    s += "\",\"wifi\":";
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
