#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <vector>

class WifiManager
{
public:
    void begin();

    void update();

    bool isConnected() const;

    String getLocalIP() const;

    int getRSSI() const;

    bool tryConnect(const char *ssid,
                    const char *password);

    bool saveCredentials(const char *ssid,
                         const char *password);

    bool isConnecting() const;

    String getCachedNetworks();

    String getSSIDByIndex(int index);

    String getConnectStatus();

    bool hasStatusChanged();

private:
    void startScan();

    void processScanResult(int n);

    bool connected;
    bool connecting;
    bool scanning;

    unsigned long connectStartTime;
    unsigned long lastRetryTime;

    String cachedNetworks;
    unsigned long lastScanTime;

    std::vector<String> scannedSSIDs;

    String connectStatus;
    bool statusChanged;
};

#endif
