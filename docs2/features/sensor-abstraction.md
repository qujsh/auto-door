# 距离传感器抽象层

- 状态：Proposed
- 关联路线图：Phase 5

## 目标

让 `DoorController` 依赖距离传感器接口而不是 HC-SR04，为 TOF 和测试替身提供稳定边界。

## 非目标

- 不要求完成具体 TOF 驱动。
- 不改变检测阈值、门状态机或 REST 字段。

## 目标行为

- 引入 `DistanceSensor` 接口。
- 现有 `Ultrasonic` 迁移或适配该接口。
- `DoorController` 只依赖接口。
- 当前硬件行为和配置保持兼容。

接口至少提供 `readDistance()`、`calibrate()`、`getBaseline()`。只有存在消费者时才加入 `getType()`。

## 验收条件

- [ ] `DoorController` 不再引用 HC-SR04 具体类型。
- [ ] 现有有效值、过滤和标定行为不变。
- [ ] 测试替身可提供距离序列验证状态机。
- [ ] REST 与 BLE 无破坏性变化。
- [ ] 固件通过确认过的 FQBN 编译。
