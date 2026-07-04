#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoControl
{
public:
    ServoControl();

    /**
     * 初始化舵机
     */
    void begin(uint8_t pin,
               int initAngle,
               unsigned long updateInterval,
               int openStep,
               int closeStep);

    /**
     * 非阻塞更新（loop中持续调用）
     */
    void update();

    /**
     * 设置目标角度
     */
    void setTargetAngle(int angle);

    /**
     * 获取目标角度
     */
    int getTargetAngle() const;

    /**
     * 获取当前角度
     */
    int getCurrentAngle() const;

    /**
     * 是否已经到达目标角度
     */
    bool isIdle() const;

private:

    Servo servo;

    uint8_t servoPin;

    int currentAngle;
    int targetAngle;

    int openStep;
    int closeStep;

    unsigned long updateInterval;
    unsigned long lastUpdateTime;
};

#endif