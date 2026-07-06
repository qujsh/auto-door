#include "WifiManager.h"
#include "Config.h"

static const unsigned long SCAN_INTERVAL = 30000;

void WifiManager::begin()
{
    WiFi.mode(WIFI_STA);

    connected = false;
    connecting = false;
    connectStartTime = 0;
    lastRetryTime = 0;

    cachedNetworks = "";
    lastScanTime = 0;
    connectStatus = "";
    statusChanged = false;

    //=============================
    // 读 NVS → 尝试连接
    //=============================
    Preferences prefs;
    prefs.begin("wifi", true);

    String savedSSID = prefs.getString("ssid", "");
    String savedPass = prefs.getString("pass", "");

    prefs.end();

    if (savedSSID.length() > 0)
    {
        doScan();

        Serial.print("WiFi: trying ");
        Serial.println(savedSSID);

        tryConnect(savedSSID.c_str(), savedPass.c_str());
    }
}

void WifiManager::update()
{
    unsigned long now = millis();

    //=============================
    // 定时 WiFi 扫描
    //=============================
    if (now - lastScanTime >= SCAN_INTERVAL || cachedNetworks == "")
    {
        doScan();
    }

    //=============================
    // 正在连接中
    //=============================
    if (connecting)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            connected = true;
            connecting = false;

            Serial.print("WiFi Connected, IP: ");
            Serial.println(WiFi.localIP());

            connectStatus = "STATE|CONNECTED|";
            connectStatus += WiFi.localIP().toString();
            statusChanged = true;

            if (MDNS.begin(MDNS_HOSTNAME))
            {
                Serial.print("mDNS: http://");
                Serial.print(MDNS_HOSTNAME);
                Serial.println(".local");
            }
        }
        else if (now - connectStartTime >= 15000)
        {
            connecting = false;

            WiFi.disconnect();

            Serial.println("WiFi: timeout");

            connectStatus = "STATE|FAILED|TIMEOUT";
            statusChanged = true;
        }

        return;
    }

    //=============================
    // 已连接 → 断线检测
    //=============================
    if (!connected)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        connected = false;

        Serial.println("WiFi Disconnected");

        connectStatus = "STATE|DISCONNECTED";
        statusChanged = true;

        if (now - lastRetryTime >= 30000)
        {
            lastRetryTime = now;
            WiFi.reconnect();
        }
    }
}

static String rssiToLabel(int rssi)
{
    if (rssi >= -30) return "非常强";
    if (rssi >= -40) return "很强";
    if (rssi >= -50) return "很好";
    if (rssi >= -60) return "良好";
    if (rssi >= -70) return "一般";
    if (rssi >= -80) return "比较差";
    return "很差";
}

void WifiManager::doScan()
{
    lastScanTime = millis();

    int n = WiFi.scanNetworks();

    if (n <= 0)
    {
        cachedNetworks = "";
        Serial.println("WiFi Scan: no networks");
        return;
    }

    // 按信号强度排序
    int indices[n];
    for (int i = 0; i < n; i++) indices[i] = i;

    for (int i = 0; i < n - 1; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
            {
                int t = indices[i];
                indices[i] = indices[j];
                indices[j] = t;
            }
        }
    }

    cachedNetworks = "";

    for (int i = 0; i < n; i++)
    {
        if (i > 0) cachedNetworks += "\n";

        int idx = indices[i];

        cachedNetworks += String(i);
        cachedNetworks += "|";
        cachedNetworks += WiFi.SSID(idx);
        cachedNetworks += "|";
        cachedNetworks += rssiToLabel(WiFi.RSSI(idx));
    }

    WiFi.scanDelete();

    Serial.println("WiFi Scan done");
}

bool WifiManager::tryConnect(const char *ssid,
                             const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    connecting = true;
    connectStartTime = millis();

    connectStatus = "STATE|CONNECTING";
    statusChanged = true;

    return true;
}

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

bool WifiManager::isConnected() const
{
    return connected && WiFi.status() == WL_CONNECTED;
}

bool WifiManager::isConnecting() const
{
    return connecting;
}

String WifiManager::getLocalIP() const
{
    return WiFi.localIP().toString();
}

int WifiManager::getRSSI() const
{
    return WiFi.RSSI();
}

String WifiManager::getCachedNetworks()
{
    return cachedNetworks;
}

String WifiManager::getConnectStatus()
{
    return connectStatus;
}

bool WifiManager::hasStatusChanged()
{
    bool temp = statusChanged;
    statusChanged = false;
    return temp;
}
