# TOF 测距

> 对应代码：`src/devices/TofSensor.h`、`src/devices/TofSensor.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

本模块负责连接和读取基于 VL53L0X 的 TOF200C 激光测距模块，是自动门判断“是否有人靠近”的主要数据来源。ESP32 通过 I²C 与传感器通信，传感器原始结果为毫米，模块对外统一转换为厘米，使上层门控逻辑不需要理解具体芯片的数据格式。

设备启动后会先测量门前无人时的环境距离，并把结果作为基线。日常运行时，自动门控制器比较当前距离与基线的差值：当门前出现物体或人员时，距离通常会明显缩短，从而触发人员检测。

真实传感器读数可能出现轻微波动或偶发无效值，因此模块使用三点中值滤波降低跳变影响，并明确过滤超出设定量程的数据。有效距离以厘米返回；传感器未初始化、测距超范围或本次测量无效时返回 `-1`，由上层决定暂时跳过本轮判断。

本模块只负责提供稳定距离和环境基线，不负责决定何时开门或关门。门控状态和时间判断由“自动门控制”模块完成。

### 接线

| TOF200C | ESP32 |
|---|---|
| VCC | 按模块标称电压供电 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

默认 I²C 地址为 `0x29`。

---

<!-- ============== 第二部分：给 AI 和开发者阅读 ============== -->

## 分：代码重建规格（给 AI 或修改代码的开发者阅读）

### 1. 文件映射

- 本模块拥有：`src/devices/TofSensor.h`、`src/devices/TofSensor.cpp`。
- 依赖：Arduino Core 的 `Wire.h`、第三方库 `Adafruit_VL53L0X.h`。
- 被使用：`src/app/AutoDoorApp.h/.cpp`、`src/control/DoorController.h/.cpp`。

### 2. 类和公开接口

头文件 guard 为 `TOF_SENSOR_H`。类名 `TofSensor`。

```cpp
TofSensor();
bool begin(uint8_t sdaPin, uint8_t sclPin,
           uint8_t address, uint16_t maxDistanceMm);
void calibrate();
float readDistance();
void primeFilter();
float getBaseline() const;
```

私有方法为 `readRaw()` 和 `median3(float,float,float)`。

### 3. 成员和初值

- `Adafruit_VL53L0X tof`
- `uint8_t address = 0x29`
- `uint16_t maxDistanceMm = 800`
- `float baseline = -1.0F`
- `float history[3]`：三项均为 `-1.0F`
- `uint8_t historyIndex = 0`
- `bool historyReady = false`
- `bool initialized = false`

### 4. 初始化

`begin()` 保存地址和最大距离，执行 `Wire.begin(sdaPin, sclPin)`，再调用 `tof.begin(address, false, &Wire)`。失败时打印初始化失败和接线提示并返回 false；成功时打印成功日志并返回 true。`AutoDoorApp` 收到 false 后进入每秒 delay 的永久停止循环，禁止在没有测距能力时继续驱动门。

### 5. 原始测距

未初始化时返回 -1。创建 `VL53L0X_RangingMeasurementData_t`，调用 `tof.rangingTest(&measurement, false)`。RangeStatus 等于 4、毫米值为 0 或超过 maxDistanceMm 时返回 -1；否则返回 `RangeMilliMeter / 10.0F` 厘米。RangeStatus 判断遵循用户已验证示例。

### 6. 过滤和标定

有效读数写入三项环形 history；不足三项直接返回当前值，填满后返回三点中值。无效读数不污染窗口。

标定同步收集 10 个有效原始样本，每轮间隔 50ms；去掉一个最小和最大值，取剩余 8 点平均作为 baseline。随后 `primeFilter()` 持续取得 3 个有效读数，每轮间隔 30ms。

### 7. 异常和约束

- 初始化失败停止应用，这是硬件安全策略。
- 标定或预热一直无有效值时会持续等待。
- 单次 `rangingTest` 为同步测量；主循环中不增加示例的 200ms delay。
- 门控阈值和公开距离继续使用厘米。

### 8. 重建验收

- GPIO 21/22、地址 0x29 可初始化 TOF200C。
- 毫米值正确转换为厘米。
- 状态 4、零值和大于 800mm 返回 -1。
- 三点滤波和 10 点去极值标定与本文一致。
- 上层不再包含 `Ultrasonic` 类型或 HC-SR04 引脚。
