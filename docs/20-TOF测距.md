# TOF 测距

> 对应代码：`src/devices/TofSensor.h`、`src/devices/TofSensor.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

### 一句话定位

本模块封装 TOF200C（VL53L0X）测距硬件，为自动门控制器提供经过校验和滤波的距离及环境基线。

### 负责什么

- 通过 I²C 初始化并读取 TOF 传感器。
- 把传感器毫米读数统一转换为厘米。
- 过滤无效值和超量程数据。
- 使用三点中值滤波降低偶发跳变。
- 在启动或人工触发时标定无人环境基线。

### 输入与输出

| 类型 | 内容 | 说明 |
|---|---|---|
| 输入 | I²C 引脚、地址、最大有效距离 | 初始化时由系统入口传入 |
| 输出 | 当前距离（厘米） | 有效测距经过滤波后返回 |
| 输出 | 环境基线（厘米） | 无人环境下多次采样得到 |
| 异常输出 | `-1` | 未初始化、无效或超量程，本轮应由上层跳过 |

### 工作流程

1. 初始化 I²C 和 VL53L0X。
2. 启动时采集无人环境样本并计算基线。
3. 运行时读取原始毫米距离并检查有效性。
4. 将有效值转换为厘米并执行三点中值滤波。
5. 向自动门控制器提供当前距离和基线。

### 职责边界

本模块不判断是否有人，也不决定何时开门或关门；人员检测、时间判断和门状态机由自动门控制模块负责。

### 接线

| TOF200C | ESP32 |
|---|---|
| VCC | 按模块标称电压供电 |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

默认 I²C 地址为 `0x29`。

### 注意事项

- 启动标定时应确保门前无人且没有临时障碍物。
- TOF 初始化失败后系统不会继续驱动自动门，这是硬件安全策略。

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
