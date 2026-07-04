#include "ServoControl.h"

ServoControl::ServoControl()
    : servoPin(0),
      currentAngle(0),
      targetAngle(0),
      openStep(30),
      closeStep(10),
      updateInterval(15),
      lastUpdateTime(0)
{
}

void ServoControl::begin(uint8_t pin,
                         int initAngle,
                         unsigned long interval,
                         int openStepValue,
                         int closeStepValue)
{
    servoPin = pin;

    updateInterval = interval;
    openStep = openStepValue;
    closeStep = closeStepValue;

    servo.attach(servoPin);

    currentAngle = constrain(initAngle, 0, 180);
    targetAngle = currentAngle;

    servo.write(currentAngle);
}

void ServoControl::update()
{
    unsigned long now = millis();

    if (now - lastUpdateTime < updateInterval)
    {
        return;
    }

    lastUpdateTime = now;

    if (currentAngle == targetAngle)
    {
        return;
    }

    if (currentAngle < targetAngle)
    {
        currentAngle += openStep;

        if (currentAngle > targetAngle)
        {
            currentAngle = targetAngle;
        }
    }
    else
    {
        currentAngle -= closeStep;

        if (currentAngle < targetAngle)
        {
            currentAngle = targetAngle;
        }
    }

    servo.write(currentAngle);
}

void ServoControl::setTargetAngle(int angle)
{
    targetAngle = constrain(angle, 0, 180);
}

int ServoControl::getTargetAngle() const
{
    return targetAngle;
}

int ServoControl::getCurrentAngle() const
{
    return currentAngle;
}

bool ServoControl::isIdle() const
{
    return currentAngle == targetAngle;
}

