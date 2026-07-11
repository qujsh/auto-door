# AutoDoor V3 — 智能自动门控制器

AutoDoorBLE 是一套运行在 ESP32 上的智能自动门程序。它使用超声波传感器判断门前是否有人，通过舵机完成开门和关门，并提供 BLE 配网与局域网网页控制。

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

- ESP32 开发板
- HC-SR04 超声波传感器
- SG90 或 MG996R 舵机
- 舵机独立 5V 电源

## 整体工作方式

设备启动后先初始化舵机和传感器，并测量无人时的环境基线。没有 Wi-Fi 配置时，可用手机 BLE 工具读取附近网络并提交配置。连接 Wi-Fi 后，设备在本地持续执行自动门逻辑，同时提供 Web 控制页面。

AUTO 模式根据距离变化自动控制门；MANUAL 模式暂停自动判断，由网页直接调整舵机。BLE 在运行期间仍保持服务，可以用于更换 Wi-Fi。

## 项目模块

详细文档采用中文编写。每份文档上半部分给人快速阅读，下半部分记录可供 AI 或开发者重建代码的完整规格。

| 模块文档 | 对应代码 |
|---|---|
| [系统入口与调度](docs/00-系统入口与调度.md) | `AutoDoorBLE.ino`、`src/app/AutoDoorApp.h/.cpp` |
| [配置参数](docs/10-配置参数.md) | `src/config/Config.h` |
| [超声波测距](docs/20-超声波测距.md) | `src/devices/Ultrasonic.h/.cpp` |
| [舵机控制](docs/20-舵机控制.md) | `src/devices/ServoControl.h/.cpp` |
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
- ESP32 Arduino Core 自带的 WiFi、ESPmDNS 和 Preferences

## 后续计划

后续计划会随着实际开发调整，目前主要方向是：

1. 抽象距离传感器接口，为 TOF 传感器和测试替身提供支持。
2. 使用 WebSocket 推送实时状态，减少网页轮询。
3. 支持通过网页调整安全范围内的运行参数。
4. 增加实时曲线和环形日志。
5. 在安全和认证机制完善后增加 OTA 升级。
6. 接入 MQTT 和 Home Assistant。

## 开发说明

项目开发纪律记录在 [AGENTS.md](AGENTS.md)。修改任何模块前，应先阅读该模块的中文文档，并在修改代码后同步更新文档中的重建规格。
