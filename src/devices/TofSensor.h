#ifndef TOF_SENSOR_H
#define TOF_SENSOR_H

#include <Adafruit_VL53L0X.h>
#include <Arduino.h>
#include <Wire.h>

class TofSensor
{
public:
    TofSensor();

    bool begin(uint8_t sdaPin,
               uint8_t sclPin,
               uint8_t address,
               uint16_t maxDistanceMm);

    void calibrate();
    float readDistance();
    void primeFilter();
    float getBaseline() const;

private:
    float readRaw();
    float median3(float a, float b, float c);

    Adafruit_VL53L0X tof;
    uint8_t address;
    uint16_t maxDistanceMm;
    float baseline;
    float history[3];
    uint8_t historyIndex;
    bool historyReady;
    bool initialized;
};

#endif
