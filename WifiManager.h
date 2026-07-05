#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>

class WifiManager
{
public:
    WifiManager();

    void begin();

    void update();

    bool isConnected() const;

    bool isAPMode() const;

    String getLocalIP() const;

    int getRSSI() const;

    String scanNetworks();

    bool saveCredentials(const char *ssid,
                         const char *password);

private:
    bool tryConnectSTA(const char *ssid,
                       const char *password);

    void startAPMode();

    void startMDNS();

    DNSServer dnsServer;

    bool connected;
    bool apMode;

    unsigned long lastRetryTime;
};

#endif
