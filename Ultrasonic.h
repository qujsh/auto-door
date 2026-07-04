#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

class Ultrasonic
{
public:
    Ultrasonic();

    /**
     * 初始化超声波模块
     */
    void begin(uint8_t trigPin,
               uint8_t echoPin,
               float maxDistance);

    /**
     * 环境标定
     * 启动时调用一次
     */
    void calibrate();

    /**
     * 获取滤波后的距离(cm)
     * 返回：
     *   >0 ：有效距离
     *   -1 ：无效数据
     */
    float readDistance();

    /**
     * 预热中值滤波器（标定后调用）
     */
    void primeFilter();

    /**
     * 获取环境基线(cm)
     */
    float getBaseline() const;

private:

    /**
     * 原始测距
     */
    float readRaw();

    /**
     * 三点中值滤波
     */
    float median3(float a,
                  float b,
                  float c);

private:

    uint8_t trigPin;
    uint8_t echoPin;

    float maxDistance;
    float baseline;

    float history[3];

    uint8_t historyIndex;
    bool historyReady;
};

#endif