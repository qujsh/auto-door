# AutoDoor V3 — 智能自动门控制器

AutoDoorBLE 是一套运行在 ESP32-C3 上的智能自动门程序。它使用 TOF200C 激光测距模块判断门前是否有人，通过舵机完成开门和关门，并提供 BLE 配网与局域网网页控制。

## 项目能做什么

- 上电后自动测量环境距离并建立基线。
- 检测到有人靠近时自动开门。
- 人离开后等待一段时间再关门；等待期间再次检测到人会取消关门。
- 在网页中查看距离、门状态、网络状态和舵机角度。
- 在 AUTO 自动模式和 MANUAL 手动模式之间切换。
- 通过 BLE 选择附近 Wi-Fi 并提交密码。
- 保存 Wi-Fi 配置，断线后尝试恢复连接。
- 通过设备 IP 或 `http://autodoor.local` 访问控制页面。

## 使用的主要硬件

- ESP32-C3 开发板
- TOF200C（VL53L0X）激光测距模块
- SG90 或 MG996R 舵机
- 舵机独立 5V 电源

当前接线：TOF200C 的 SDA/SCL 分别连接 GPIO4/GPIO5，舵机信号连接 GPIO6。PBS-11 点动开关的两个触点分别连接 GPIO7 和 GND（不是连接两个 GPIO），程序使用内部上拉。舵机 5V 电源必须与 ESP32-C3 共地；SG90/MG996R 启动电流较大，优先使用容量足够的独立 5V 电源，不要默认由开发板 USB 5V 引脚直接承担舵机负载。

## 整体工作方式

设备启动后先初始化舵机和传感器，并测量无人时的环境基线。自动门逻辑随后立即在本地持续运行，不依赖 Wi-Fi。没有 Wi-Fi 配置时，可用手机 BLE 工具读取附近网络并提交配置；连接 Wi-Fi 后额外提供 Web 控制页面。

AUTO 模式根据距离变化自动控制门；MANUAL 模式暂停自动判断，由网页直接调整舵机。BLE 在运行期间仍保持服务，可以用于更换 Wi-Fi。

## 项目模块

详细文档采用中文编写。每份文档上半部分给人快速阅读，下半部分记录可供 AI 或开发者重建代码的完整规格。

| 模块文档 | 对应代码 |
|---|---|
| [系统入口与调度](docs/00-系统入口与调度.md) | `AutoDoorBLE.ino`、`src/app/AutoDoorApp.h/.cpp` |
| [配置参数](docs/10-配置参数.md) | `src/config/Config.h` |
| [TOF 测距](docs/20-TOF测距.md) | `src/devices/TofSensor.h/.cpp` |
| [舵机控制](docs/20-舵机控制.md) | `src/devices/ServoControl.h/.cpp` |
| [点动开关输入](docs/20-点动开关输入.md) | `src/devices/PushButton.h/.cpp` |
| [Wi-Fi 管理](docs/30-WiFi管理.md) | `src/network/WifiManager.h/.cpp` |
| [BLE 配网](docs/31-BLE配网.md) | `src/network/BleManager.h/.cpp` |
| [自动门控制](docs/40-自动门控制.md) | `src/control/DoorController.h/.cpp` |
| [Web 控制页面](docs/50-Web控制页面.md) | `src/web/WebPage.h` |
| [Web 服务](docs/51-Web服务.md) | `src/web/WebServerManager.h/.cpp` |

源码按职责放在 `src/`：`app` 负责应用装配，`config` 保存公共配置，`devices` 驱动硬件，`network` 管理 BLE/Wi-Fi，`control` 实现门控业务，`web` 提供页面和 HTTP 接口。Arduino 主入口 `AutoDoorBLE.ino` 保留在项目根目录。

## 软件依赖

- ESPAsyncWebServer
- AsyncTCP
- NimBLE-Arduino
- ESP32Servo
- Adafruit VL53L0X
- ESP32 Arduino Core 自带的 WiFi、ESPmDNS 和 Preferences

## 后续计划

后续计划会随着实际开发调整，目前主要方向是：

1. 抽象距离传感器接口，为测试替身和其他测距模块提供支持。
2. 使用 WebSocket 推送实时状态，减少网页轮询。
3. 支持通过网页调整安全范围内的运行参数。
4. 增加实时曲线和环形日志。
5. 在安全和认证机制完善后增加 OTA 升级。
6. 接入 MQTT 和 Home Assistant。

## 开发说明

项目开发纪律记录在 [AGENTS.md](AGENTS.md)。修改任何模块前，应先阅读该模块的中文文档，并在修改代码后同步更新文档中的重建规格。
