#include "AutoDoorApp.h"

#include <Arduino.h>

#include "../config/Config.h"

void AutoDoorApp::begin()
{
    Serial.begin(Config::Serial::baudRate);
    const unsigned long serialWaitStart = millis();
    while (!Serial &&
           millis() - serialWaitStart < Config::Serial::readyTimeoutMs)
    {
        delay(10);
    }
    delay(Config::Serial::startupDelayMs);

    Serial.println("=================================");
    Serial.println("AutoDoorBLE starting...");
    Serial.println("=================================");

    servo_.begin(Config::Pins::servo,
                 Config::Servo::closedAngle,
                 Config::Servo::updateIntervalMs,
                 Config::Servo::openStep,
                 Config::Servo::closeStep);

    if (!tofSensor_.begin(Config::Pins::i2cSda,
                          Config::Pins::i2cScl,
                          Config::Tof::address,
                          Config::Tof::maxValidDistanceMm))
    {
        while (true)
        {
            delay(1000);
        }
    }

    wifi_.begin();
    ble_.begin(Config::Ble::deviceName,
               Config::Ble::serviceUuid,
               Config::Ble::wifiScanCharacteristicUuid,
               Config::Ble::wifiConfigCharacteristicUuid,
               &wifi_);

    tofSensor_.calibrate();
    door_.begin(&tofSensor_, &servo_);

    if (wifi_.isConnected())
    {
        enterRunningState();
    }
    else
    {
        Serial.println("No WiFi connection. Configure it over BLE.");
    }

    Serial.println("System ready");
}

void AutoDoorApp::update()
{
    wifi_.update();
    servo_.update();
    ble_.update();
    handleWiFiConfig();

    if (state_ == State::Configuring && wifi_.isConnected())
    {
        enterRunningState();
    }

    if (state_ == State::Running)
    {
        door_.update();
    }
}

void AutoDoorApp::handleWiFiConfig()
{
    int networkIndex = -1;
    String ssid;
    String password;
    if (!ble_.hasWiFiConfig(networkIndex, ssid, password))
    {
        return;
    }

    if (ssid.isEmpty())
    {
        Serial.println("BLE WiFi configuration contains an invalid network index");
        return;
    }

    wifi_.saveCredentials(ssid.c_str(), password.c_str());
    if (state_ == State::Configuring)
    {
        wifi_.tryConnect(ssid.c_str(), password.c_str());
        return;
    }

    wifi_.tryConnect(ssid.c_str(), password.c_str());
}

void AutoDoorApp::enterRunningState()
{
    state_ = State::Running;
    web_.begin(&door_, &servo_, &wifi_);

    Serial.print("Web: http://");
    Serial.print(wifi_.getLocalIP());
    Serial.print(" or http://");
    Serial.print(Config::Network::mdnsHostname);
    Serial.println(".local");
}
