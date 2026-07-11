#include "Ultrasonic.h"

Ultrasonic::Ultrasonic()
    : trigPin(0),
      echoPin(0),
      maxDistance(80.0f),
      baseline(-1.0f),
      historyIndex(0),
      historyReady(false)
{
    history[0] = -1.0f;
    history[1] = -1.0f;
    history[2] = -1.0f;
}

void Ultrasonic::begin(uint8_t trig,
                       uint8_t echo,
                       float maxDist)
{
    trigPin = trig;
    echoPin = echo;
    maxDistance = maxDist;

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    digitalWrite(trigPin, LOW);
}

float Ultrasonic::median3(float a,
                          float b,
                          float c)
{
    if (a > b)
    {
        float t = a;
        a = b;
        b = t;
    }

    if (b > c)
    {
        float t = b;
        b = c;
        c = t;
    }

    if (a > b)
    {
        float t = a;
        a = b;
        b = t;
    }

    return b;
}

float Ultrasonic::readRaw()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);

    if (duration == 0)
    {
        return -1.0f;
    }

    float distance = duration * 0.0343f / 2.0f;

    if (distance <= 0.0f)
    {
        return -1.0f;
    }

    if (distance > maxDistance)
    {
        return -1.0f;
    }

    return distance;
}

float Ultrasonic::readDistance()
{
    float distance = readRaw();

    if (distance < 0.0f)
    {
        return -1.0f;
    }

    history[historyIndex] = distance;

    historyIndex++;

    if (historyIndex >= 3)
    {
        historyIndex = 0;
        historyReady = true;
    }

    if (!historyReady)
    {
        return distance;
    }

    return median3(
        history[0],
        history[1],
        history[2]);
}

void Ultrasonic::calibrate()
{
    const uint8_t SAMPLE_COUNT = 10;

    float values[SAMPLE_COUNT];

    uint8_t count = 0;

    Serial.println("Calibrate...");

    while (count < SAMPLE_COUNT)
    {
        float d = readRaw();

        if (d > 0)
        {
            values[count] = d;

            Serial.print("Sample ");
            Serial.print(count + 1);
            Serial.print(" : ");
            Serial.println(d, 2);

            count++;
        }

        delay(50);
    }

    float minValue = values[0];
    float maxValue = values[0];
    float sum = 0.0f;

    for (uint8_t i = 0; i < SAMPLE_COUNT; i++)
    {
        if (values[i] < minValue)
        {
            minValue = values[i];
        }

        if (values[i] > maxValue)
        {
            maxValue = values[i];
        }

        sum += values[i];
    }

    Serial.print("Min      : ");
    Serial.println(minValue, 2);
    Serial.print("Max      : ");
    Serial.println(maxValue, 2);

    baseline = (sum - minValue - maxValue) / (SAMPLE_COUNT - 2);

    Serial.print("Baseline = ");
    Serial.println(baseline);

    primeFilter();
}

void Ultrasonic::primeFilter()
{
    for (uint8_t i = 0; i < 3; i++)
    {
        float d = readRaw();

        if (d > 0)
        {
            history[historyIndex] = d;

            historyIndex++;

            if (historyIndex >= 3)
            {
                historyIndex = 0;
            }
        }

        delay(30);
    }

    historyReady = true;
}

float Ultrasonic::getBaseline() const
{
    return baseline;
}