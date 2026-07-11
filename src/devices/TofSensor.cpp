#include "TofSensor.h"

TofSensor::TofSensor()
    : address(0x29),
      maxDistanceMm(800),
      baseline(-1.0F),
      historyIndex(0),
      historyReady(false),
      initialized(false)
{
    history[0] = -1.0F;
    history[1] = -1.0F;
    history[2] = -1.0F;
}

bool TofSensor::begin(uint8_t sdaPin,
                      uint8_t sclPin,
                      uint8_t sensorAddress,
                      uint16_t maximumDistanceMm)
{
    address = sensorAddress;
    maxDistanceMm = maximumDistanceMm;

    Wire.begin(sdaPin, sclPin);
    initialized = tof.begin(address, false, &Wire);

    if (!initialized)
    {
        Serial.println("TOF200C initialization failed!");
        Serial.println("Please check wiring and I2C address.");
        return false;
    }

    Serial.println("TOF200C initialized successfully");
    return true;
}

float TofSensor::median3(float a, float b, float c)
{
    if (a > b)
    {
        float temp = a;
        a = b;
        b = temp;
    }
    if (b > c)
    {
        float temp = b;
        b = c;
        c = temp;
    }
    if (a > b)
    {
        float temp = a;
        a = b;
        b = temp;
    }
    return b;
}

float TofSensor::readRaw()
{
    if (!initialized)
    {
        return -1.0F;
    }

    VL53L0X_RangingMeasurementData_t measurement;
    tof.rangingTest(&measurement, false);

    if (measurement.RangeStatus == 4 || measurement.RangeMilliMeter == 0)
    {
        return -1.0F;
    }

    if (measurement.RangeMilliMeter > maxDistanceMm)
    {
        return -1.0F;
    }

    return measurement.RangeMilliMeter / 10.0F;
}

float TofSensor::readDistance()
{
    const float distance = readRaw();
    if (distance < 0.0F)
    {
        return -1.0F;
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

    return median3(history[0], history[1], history[2]);
}

void TofSensor::calibrate()
{
    constexpr uint8_t sampleCount = 10;
    float values[sampleCount];
    uint8_t count = 0;

    Serial.println("Calibrate TOF200C...");
    while (count < sampleCount)
    {
        const float distance = readRaw();
        if (distance > 0.0F)
        {
            values[count] = distance;
            Serial.print("Sample ");
            Serial.print(count + 1);
            Serial.print(" : ");
            Serial.println(distance, 2);
            count++;
        }
        delay(50);
    }

    float minValue = values[0];
    float maxValue = values[0];
    float sum = 0.0F;
    for (uint8_t index = 0; index < sampleCount; index++)
    {
        minValue = min(minValue, values[index]);
        maxValue = max(maxValue, values[index]);
        sum += values[index];
    }

    baseline = (sum - minValue - maxValue) / (sampleCount - 2);
    Serial.print("Baseline = ");
    Serial.println(baseline, 2);
    primeFilter();
}

void TofSensor::primeFilter()
{
    uint8_t validCount = 0;
    while (validCount < 3)
    {
        const float distance = readRaw();
        if (distance > 0.0F)
        {
            history[historyIndex] = distance;
            historyIndex = (historyIndex + 1) % 3;
            validCount++;
        }
        delay(30);
    }
    historyReady = true;
}

float TofSensor::getBaseline() const
{
    return baseline;
}
