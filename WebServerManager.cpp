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

    server = new AsyncWebServer(WEB_PORT);

    setupRoutes();

    server->begin();

    if (!wifi->isAPMode())
    {
        Serial.println("Web Server Started");
    }
}

void WebServerManager::setupRoutes()
{
    if (wifi->isAPMode())
    {
        setupAPRoutes();
    }
    else
    {
        setupControlRoutes();
    }
}

//=====================================================
// STA 模式：控制面板
//=====================================================
void WebServerManager::setupControlRoutes()
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

//=====================================================
// AP 模式：配网 + Captive Portal
//=====================================================
void WebServerManager::setupAPRoutes()
{
    // WiFi 扫描
    server->on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleWiFiScan(request);
    });

    // WiFi 保存
    server->on(
        "/api/wifi/save",
        HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        nullptr,
        [this](AsyncWebServerRequest *request,
               uint8_t *data, size_t len,
               size_t index, size_t total) {
            handleWiFiSave(request, data, len, index, total);
        });

    // Captive Portal：所有其他请求返回配网页面
    server->onNotFound([this](AsyncWebServerRequest *request) {
        handleRoot(request);
    });
}

//=====================================================
// 控制页面
//=====================================================
void WebServerManager::handleRoot(AsyncWebServerRequest *request)
{
    if (wifi->isAPMode())
    {
        request->send_P(200, "text/html", WIFI_SETUP_HTML);
    }
    else
    {
        request->send_P(200, "text/html", INDEX_HTML);
    }
}

//=====================================================
// 状态查询
//=====================================================
void WebServerManager::handleStatus(AsyncWebServerRequest *request)
{
    String json = buildStatusJson();
    request->send(200, "application/json", json);
}

//=====================================================
// 舵机控制
//=====================================================
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

//=====================================================
// 模式切换
//=====================================================
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

//=====================================================
// 远程标定
//=====================================================
void WebServerManager::handleCalibrate(AsyncWebServerRequest *request)
{
    door->triggerCalibrate();

    char json[64];
    snprintf(json, sizeof(json),
             "{\"ok\":true,\"baseline\":%.2f}",
             door->getBaseline());

    request->send(200, "application/json", json);
}

//=====================================================
// WiFi 扫描
//=====================================================
void WebServerManager::handleWiFiScan(AsyncWebServerRequest *request)
{
    String json = wifi->scanNetworks();
    request->send(200, "application/json", json);
}

//=====================================================
// WiFi 凭证保存
//=====================================================
void WebServerManager::handleWiFiSave(AsyncWebServerRequest *request,
                                      uint8_t *data,
                                      size_t len,
                                      size_t /*index*/,
                                      size_t /*total*/)
{
    String body;
    body.concat((char *)data, len);

    int ssidStart = body.indexOf("\"ssid\":\"");
    int passStart = body.indexOf("\"pass\":\"");

    if (ssidStart < 0)
    {
        request->send(400, "application/json", "{\"error\":\"missing ssid\"}");
        return;
    }

    ssidStart += 8;
    int ssidEnd = body.indexOf("\"", ssidStart);

    String ssid = body.substring(ssidStart, ssidEnd);

    String password;

    if (passStart >= 0)
    {
        passStart += 8;
        int passEnd = body.indexOf("\"", passStart);
        password = body.substring(passStart, passEnd);
    }

    wifi->saveCredentials(ssid.c_str(), password.c_str());

    request->send(200, "application/json", "{\"ok\":true}");

    delay(500);
    ESP.restart();
}

//=====================================================
// JSON 构建
//=====================================================
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
        case DoorState::CLOSED:     return "CLOSED";
        case DoorState::OPEN:       return "OPEN";
        case DoorState::WAIT_CLOSE: return "WAIT_CLOSE";
        default:                    return "UNKNOWN";
    }
}
