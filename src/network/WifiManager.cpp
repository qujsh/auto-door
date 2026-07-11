#include "WifiManager.h"
#include "../config/Config.h"

void WifiManager::begin()
{
    WiFi.mode(WIFI_STA);

    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        Serial.print("WiFi Event=");
        Serial.print(event);
        Serial.print(" reason=");
        Serial.println(info.wifi_sta_disconnected.reason);
    });

    connected = false;
    connecting = false;
    scanning = false;
    connectStartTime = 0;
    lastRetryTime = 0;

    cachedNetworks = "";
    scannedSSIDs.clear();
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
        Serial.print("WiFi: trying ");
        Serial.println(savedSSID);

        tryConnect(savedSSID.c_str(), savedPass.c_str());
    }
    else
    {
        startScan();
    }
}

void WifiManager::update()
{
    unsigned long now = millis();

    //=============================
    // 正在连接中（优先处理，不扫描）
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

            if (MDNS.begin(Config::Network::mdnsHostname))
            {
                Serial.print("mDNS: http://");
                Serial.print(Config::Network::mdnsHostname);
                Serial.println(".local");
            }
        }
        else if (now - connectStartTime >= Config::Network::connectTimeoutMs)
        {
            Serial.print("WiFi status=");
            Serial.println(WiFi.status());

            connecting = false;

            Serial.println("WiFi: timeout");

            WiFi.disconnect();
            delay(100);
            WiFi.mode(WIFI_OFF);
            delay(500);
            WiFi.mode(WIFI_STA);
            delay(500);

            connectStatus = "STATE|FAILED|TIMEOUT";
            statusChanged = true;
        }

        return;
    }

    //=============================
    // 异步扫描：检查完成状态
    //=============================
    if (scanning)
    {
        int n = WiFi.scanComplete();

        if (n == -1)
        {
            return;
        }

        scanning = false;

        if (n == -2)
        {
            Serial.println("WiFi Scan: failed to start");
            scanning = false;
            return;
        }

        processScanResult(n);
        return;
    }

    //=============================
    // 定时触发扫描
    //=============================
    if (now - lastScanTime >= Config::Network::scanIntervalMs)
    {
        startScan();
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

        if (now - lastRetryTime >= Config::Network::reconnectIntervalMs)
        {
            lastRetryTime = now;
            WiFi.reconnect();
        }
    }
}

//=====================================================
// 启动异步扫描
//=====================================================
void WifiManager::startScan()
{
    if (scanning) return;

    WiFi.disconnect();
    delay(200);

    lastScanTime = millis();
    scanning = true;

    Serial.print("WiFi status=");
    Serial.println(WiFi.status());

    connectStatus = "STATE|SCANNING";
    statusChanged = true;

    WiFi.scanNetworks(true);

    Serial.println("WiFi Scan started");
}

//=====================================================
// RSSI → 中文标签
//=====================================================
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

//=====================================================
// 处理异步扫描结果
//=====================================================
void WifiManager::processScanResult(int n)
{
    if (n <= 0)
    {
        Serial.println("WiFi Scan: no networks");

        if (cachedNetworks.length() == 0)
        {
            lastScanTime = millis() - Config::Network::scanIntervalMs + 5000;
        }

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

    Serial.print("Scan count=");
    Serial.println(n);

    for (int i = 0; i < n; i++)
    {
        Serial.print("  ");
        Serial.print("SSID=");
        Serial.print(WiFi.SSID(i));

        Serial.print(" RSSI=");
        Serial.print(WiFi.RSSI(i));

        Serial.print(" CH=");
        Serial.print(WiFi.channel(i));

        Serial.print(" AUTH=");
        Serial.println(WiFi.encryptionType(i));
    }

    cachedNetworks = "";
    scannedSSIDs.clear();

    for (int i = 0; i < n; i++)
    {
        if (i > 0) cachedNetworks += "\r\n";

        int idx = indices[i];

        cachedNetworks += String(i);
        cachedNetworks += "|";
        cachedNetworks += WiFi.SSID(idx);
        cachedNetworks += "|";
        cachedNetworks += rssiToLabel(WiFi.RSSI(idx));

        scannedSSIDs.push_back(WiFi.SSID(idx));
    }

    WiFi.scanDelete();

    Serial.println("WiFi Scan done");
}

//=====================================================
// 连接
//=====================================================
bool WifiManager::tryConnect(const char *ssid,
                             const char *password)
{

    WiFi.disconnect(true);
    delay(1000);


    WiFi.mode(WIFI_STA);
    delay(500);


    WiFi.setSleep(false);


    Serial.println("WiFi.begin");


    WiFi.begin(ssid, password);


    connecting = true;
    connectStartTime = millis();


    return true;
}

//=====================================================
// NVS 保存
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
// 按显示索引获取真实 SSID
//=====================================================
String WifiManager::getSSIDByIndex(int index)
{
    if (index >= 0 && index < (int)scannedSSIDs.size())
    {
        return scannedSSIDs[index];
    }

    return "";
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
