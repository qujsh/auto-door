#pragma once

#include <Arduino.h>

class PushButton
{
public:
    void begin(uint8_t pin, unsigned long debounceMs);
    bool update();

private:
    uint8_t pin_ = 0;
    unsigned long debounceMs_ = 0;
    unsigned long lastChangeTime_ = 0;
    bool rawPressed_ = false;
    bool stablePressed_ = false;
};
