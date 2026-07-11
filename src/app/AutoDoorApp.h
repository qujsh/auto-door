#pragma once

#include "../network/BleManager.h"
#include "../control/DoorController.h"
#include "../devices/ServoControl.h"
#include "../devices/TofSensor.h"
#include "../web/WebServerManager.h"
#include "../network/WifiManager.h"

class AutoDoorApp
{
public:
    void begin();
    void update();

private:
    enum class State
    {
        Configuring,
        Running
    };

    void handleWiFiConfig();
    void enterRunningState();

    ServoControl servo_;
    TofSensor tofSensor_;
    BleManager ble_;
    DoorController door_;
    WifiManager wifi_;
    WebServerManager web_;
    State state_ = State::Configuring;
};
