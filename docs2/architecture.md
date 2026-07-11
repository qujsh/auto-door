# 系统架构

## 组件关系

```text
AutoDoorBLE.ino
  └─ AutoDoorApp                 应用装配、生命周期和系统状态
     ├─ WifiManager              STA、扫描、NVS、重连、mDNS
     ├─ BleManager ────────────→ WifiManager
     ├─ WebServerManager ──────→ DoorController / ServoControl / WifiManager
     └─ DoorController ────────→ Ultrasonic / ServoControl
```

## 模块职责

| 模块 | 负责 | 不负责 |
|---|---|---|
| `AutoDoorBLE.ino` | Arduino 入口 | 业务逻辑 |
| `AutoDoorApp` | 创建模块、初始化顺序、Configuring/Running 调度 | 具体传感和网络实现 |
| `DoorController` | 人员检测、AUTO/MANUAL、门状态机 | 网络协议、底层 PWM |
| `Ultrasonic` | 测距、过滤、标定 | 开关门决策 |
| `ServoControl` | 舵机目标角度和非阻塞移动 | 门状态判断 |
| `WifiManager` | Wi-Fi 生命周期和扫描缓存 | BLE 格式、页面路由 |
| `BleManager` | BLE 配网协议和状态通知 | 保存网络业务策略 |
| `WebServerManager` | HTTP 路由、输入解析、JSON | 自动门决策 |
| `WebPage.h` | 嵌入式前端资源 | 后端状态真相 |

## 依赖规则

- 入口层可以依赖业务模块，底层模块不能反向依赖 `AutoDoorApp`。
- Web 和 BLE 只调用公开接口，不直接修改其他模块内部状态。
- 编译期配置集中在 `Config.h`。
- 新距离传感器通过计划中的抽象接口接入，不在 `DoorController` 加硬件分支。

## 系统状态

- `Configuring`：没有可用 Wi-Fi，BLE 等待配网；舵机、Wi-Fi 和 BLE 更新仍运行。
- `Running`：执行 `DoorController::update()`，Web 可经已连接网络访问。

`AutoDoorApp::update()` 每轮依次更新 Wi-Fi、舵机、BLE，处理新配置，再根据状态更新门控制器。
