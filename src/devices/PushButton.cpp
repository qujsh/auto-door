#include "PushButton.h"

void PushButton::begin(uint8_t pin, unsigned long debounceMs)
{
    pin_ = pin;
    debounceMs_ = debounceMs;
    pinMode(pin_, INPUT_PULLUP);
    rawPressed_ = digitalRead(pin_) == LOW;
    stablePressed_ = rawPressed_;
    lastChangeTime_ = millis();
}

bool PushButton::update()
{
    const unsigned long now = millis();
    const bool pressed = digitalRead(pin_) == LOW;

    if (pressed != rawPressed_)
    {
        rawPressed_ = pressed;
        lastChangeTime_ = now;
    }

    if (rawPressed_ != stablePressed_ && now - lastChangeTime_ >= debounceMs_)
    {
        stablePressed_ = rawPressed_;
        return stablePressed_;
    }

    return false;
}
