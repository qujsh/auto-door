#pragma once

#include <Arduino.h>

enum class DoorState
{
    Closed,
    Open,
    WaitingToClose
};

namespace Config
{
namespace Pins
{
constexpr uint8_t i2cSda = 21;
constexpr uint8_t i2cScl = 22;
constexpr uint8_t servo = 5;
}

namespace Servo
{
constexpr int openAngle = 90;
constexpr int closedAngle = 0;
constexpr unsigned long updateIntervalMs = 15;
constexpr int openStep = 30;
constexpr int closeStep = 10;
}

namespace Tof
{
constexpr uint8_t address = 0x29;
constexpr float changeThresholdCm = 2.5F;
constexpr uint16_t maxValidDistanceMm = 2000;
}

namespace Door
{
constexpr unsigned long presenceTimeoutMs = 1500;
constexpr unsigned long closeDelayMs = 2000;
constexpr unsigned long detectionDebounceMs = 200;
}

namespace Ble
{
constexpr char deviceName[] = "AutoDoor";
constexpr char serviceUuid[] = "a550d001-0000-a550-d001-a550d0010001";
constexpr char wifiScanCharacteristicUuid[] = "a550d001-0000-a550-d001-a550d001a001";
constexpr char wifiConfigCharacteristicUuid[] = "a550d001-0000-a550-d001-a550d001a002";
}

namespace Network
{
constexpr char mdnsHostname[] = "autodoor";
constexpr uint16_t webPort = 80;
constexpr unsigned long connectTimeoutMs = 15000;
constexpr unsigned long reconnectIntervalMs = 30000;
}

namespace Debug
{
constexpr bool logDistance = true;
constexpr unsigned long printIntervalMs = 300;
}

namespace Serial
{
constexpr unsigned long baudRate = 115200;
constexpr unsigned long startupDelayMs = 500;
}

namespace System
{
constexpr unsigned long restartDelayMs = 500;
}
}
