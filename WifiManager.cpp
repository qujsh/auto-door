#include "WifiManager.h"
#include "Config.h"

WifiManager::WifiManager()
    : connected(false),
      apMode(false),
      lastRetryTime(0)
{
}

void WifiManager::begin()
{
    //=============================
    // 读取 NVS 中保存的 WiFi 凭证
    //=============================
    Preferences prefs;
    prefs.begin("wifi", true);

    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");

    prefs.end();

    //=============================
    // 有凭证 → 尝试 STA 连接
    //=============================
    if (savedSSID.length() > 0)
    {
        Serial.print("WiFi: trying ");
        Serial.println(savedSSID);

        if (tryConnectSTA(savedSSID.c_str(), savedPass.c_str()))
        {
            startMDNS();
            return;
        }

        Serial.println("WiFi: connection failed, fallback to AP");
    }

    //=============================
    // 无凭证或连接失败 → AP 模式
    //=============================
    startAPMode();
}

void WifiManager::update()
{
    if (apMode)
    {
        dnsServer.processNextRequest();
        return;
    }

    //=============================
    // STA 模式：断线检测 + 自动重连
    //=============================
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!connected)
        {
            connected = true;

            Serial.print("WiFi Reconnected, IP: ");
            Serial.println(WiFi.localIP());
        }
    }
    else
    {
        if (connected)
        {
            connected = false;

            Serial.println("WiFi Disconnected");
        }

        unsigned long now = millis();

        if (now - lastRetryTime >= 30000)
        {
            lastRetryTime = now;
            WiFi.reconnect();
        }
    }
}

//=====================================================
// STA 连接（阻塞 15 秒）
//=====================================================
bool WifiManager::tryConnectSTA(const char *ssid,
                                const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        connected = true;
        apMode = false;

        Serial.print("WiFi Connected, IP: ");
        Serial.println(WiFi.localIP());

        return true;
    }

    return false;
}

//=====================================================
// AP 模式 + Captive Portal
//=====================================================
void WifiManager::startAPMode()
{
    apMode = true;
    connected = false;

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    dnsServer.start(53, "*", WiFi.softAPIP());

    Serial.println("AP Mode: AutoDoor_Setup");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

//=====================================================
// mDNS 广播（http://autodoor.local）
//=====================================================
void WifiManager::startMDNS()
{
    if (MDNS.begin(MDNS_HOSTNAME))
    {
        Serial.print("mDNS: http://");
        Serial.print(MDNS_HOSTNAME);
        Serial.println(".local");
    }
    else
    {
        Serial.println("mDNS: failed");
    }
}

//=====================================================
// 附近 WiFi 扫描
//=====================================================
String WifiManager::scanNetworks()
{
    int n = WiFi.scanNetworks();

    String json = "[";
    for (int i = 0; i < n; i++)
    {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
        json += "}";
    }
    json += "]";

    WiFi.scanDelete();

    return json;
}

//=====================================================
// 保存凭证到 NVS
//=====================================================
bool WifiManager::saveCredentials(const char *ssid,
                                  const char *password)
{
    if (strlen(ssid) == 0) return false;

    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();

    Serial.print("WiFi saved: ");
    Serial.println(ssid);

    return true;
}

//=====================================================
// 状态查询
//=====================================================
bool WifiManager::isConnected() const
{
    return connected;
}

bool WifiManager::isAPMode() const
{
    return apMode;
}

String WifiManager::getLocalIP() const
{
    if (apMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}

int WifiManager::getRSSI() const
{
    return WiFi.RSSI();
}
