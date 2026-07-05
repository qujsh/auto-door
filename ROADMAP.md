# AutoDoor 后续拓展路线图

> 当前版本：V3.1 — BLE 配网专用（配网后关闭），DoorController 移除 BLE 耦合，WebServer 一次性创建
>
> 已完成：超声波三状态机、Web 控制面板 + REST API、BLE 3-Characteristic 配网协议、WifiManager 纯 STA + 30s 扫描缓存 + mDNS、DoorController 只依赖 Ultrasonic + ServoControl

---

## 已完成

| 版本 | 内容 |
|------|------|
| V1 | 超声波三状态机 + BLE 舵机遥控 |
| V2 | Web 控制面板 + AsyncWebServer + REST API |
| V2.5 | BLE 多 Characteristic 配网架构（WiFiScan / WiFiConfig / Servo）|
| V3 | WifiManager 纯 STA + 30s 自动扫描缓存 + 状态通知 + mDNS |

---

## Phase 4 — WebSocket 实时推送

**状态**：待开发 | **预估**：~200 行 | **价值**：高

### 概述

当前 `/api/status` 靠 500ms AJAX 轮询。换 WebSocket 后 ESP32 主动推送，距离变化和门状态即时到达，同时为实时曲线图和日志流铺路。

`ESPAsyncWebServer` 自带 `AsyncWebSocket.h`，无需额外库。

### 新增

| 文件 | 内容 |
|------|------|
| `WebSocketManager.h/.cpp` | WebSocket 事件封装、定时广播 |

### 改动

| 文件 | 改动 |
|------|------|
| `WebServerManager.cpp` | 注册 `/ws` 端点 |
| `WebPage.h` | JS 从 `fetch` 轮询改为 WebSocket `onmessage` |

---

## Phase 5 — 传感器抽象层 + TOF 支持

**状态**：待开发 | **预估**：~120 行 | **价值**：高

### 概述

抽 `DistanceSensor` 抽象基类，`DoorController` 持抽象指针。换 VL53L0X 只需写新子类，业务代码零改动。

### 新增

| 文件 | 内容 |
|------|------|
| `DistanceSensor.h` | 抽象接口：`readDistance()`、`calibrate()`、`getBaseline()`、`getType()` |
| `UltrasonicSensor.h/.cpp` | 现有 Ultrasonic 逻辑迁入，实现 DistanceSensor |

### 改动

| 文件 | 改动 |
|------|------|
| `DoorController.h` | `Ultrasonic*` → `DistanceSensor*` |
| `AutoDoorBLE.ino` | `Ultrasonic ultrasonic` → `UltrasonicSensor ultrasonic` |
| `Ultrasonic.h/.cpp` | 删除（逻辑已迁入 UltrasonicSensor） |

---

## Phase 6 — 网页可配置参数（在线调参）

**状态**：待开发 | **预估**：~200 行 | **价值**：高

### 概述

告别改 `Config.h` 重编译。`DISTANCE_CHANGE_THRESHOLD`、`CLOSE_DELAY` 等参数通过网页修改，NVS 持久化。

### API

| 方法 | 路径 | 说明 |
|------|------|------|
| `GET` | `/api/config` | 返回全部参数 |
| `POST` | `/api/config` | `{"key":"threshold","value":3.2}` 更新单项 |

### 改动

| 文件 | 内容 |
|------|------|
| `DoorController` | +`loadConfig()` / `saveConfig()` / `applyConfig()` |
| `WebServerManager` | +2 个路由 |
| `WebPage.h` | + 可折叠配置面板 |

---

## Phase 7 — 日志系统 + 距离实时曲线

**状态**：待开发 | **预估**：~250 行 | **价值**：中

### 概述

- **LogManager**：环形缓冲（100 条），取代 `Serial.println`
- **距离曲线**：`<canvas>` 手写折线图（不引入 Chart.js），标注阈值线和基线
- 两者通过 WebSocket 实时推送

### 新增

| 文件 | 内容 |
|------|------|
| `LogManager.h/.cpp` | 环形缓冲 + `add()` + `getNew()` |

### 改动

| 文件 | 内容 |
|------|------|
| `DoorController.cpp` | Serial 改 log.add() |
| `WebPage.h` | + canvas 曲线 + 日志面板 |

---

## Phase 8 — OTA 固件升级

**状态**：待开发 | **预估**：~100 行 | **价值**：中

### 概述

浏览器上传 `.bin` 文件升级。`ArduinoOTA.h`（ESP32 自带）处理固件写入。

### 新增

| 文件 | 内容 |
|------|------|
| `OtaManager.h/.cpp` | `begin()` → `ArduinoOTA.begin()` + `update()` |

### 改动

| 文件 | 内容 |
|------|------|
| `AutoDoorBLE.ino` | loop 加 `ota.update()` |
| `WebPage.h` | + 固件上传区 |

---

## Phase 9 — 密码保护

**状态**：待开发 | **预估**：~100 行 | **价值**：中

### 概述

同一 WiFi 下任何人可访问控制台。加密码：AP 配网时设置 → NVS 存储 → STA 下请求验证 token → 24h 过期。

---

## Phase 10 — MQTT + Home Assistant

**状态**：待开发 | **预估**：~300 行 | **价值**：高（智能家居）

### 概述

接入 MQTT broker 后可语音控制（小爱 / Siri）、HA 仪表板联动、远程监控。

### Topic

| Topic | 方向 | 说明 |
|-------|------|------|
| `autodoor/distance` | → broker | 实时距离 |
| `autodoor/door` | → broker | 门状态 |
| `autodoor/servo` | → broker | 舵机角度 |
| `autodoor/servo/set` | broker → | 设置舵机 |
| `autodoor/mode/set` | broker → | 切换 AUTO/MANUAL |

### 新增 / 改动

| 文件 | 内容 |
|------|------|
| `MqttManager.h/.cpp` | PubSubClient 封装 |
| `Config.h` | + MQTT broker 地址 |
| `DoorController` | 状态变化时回调 MqttManager |

---

## 远期待选功能

| 功能 | 说明 |
|------|------|
| **双传感器冗余** | TOF + 超声波自动 fallback |
| **NTP 时间同步** | 日志带时间戳 |
| **节能模式** | 夜间降频 |
| **HTTPS** | TLS 加密 |
| **SPIFFS/LittleFS 页面** | 大页面存文件系统 |
| **Vue/React 前端** | SPA 控制台 |
| **微信小程序** | MQTT → 小程序远程控制 |
| **iPhone 配网 App** | 封装 BLE 配网流程，零技术门槛 |

---

## 开发节奏

```
Phase 4 (WebSocket)     ← 实时性，为曲线图和日志铺路
    ↓
Phase 5 (传感器抽象)    ← 换 TOF 零改动
    ↓
Phase 6 (网页调参)      ← 彻底告别 Config.h
    ↓
Phase 7 (日志+曲线)     ← 调试体验飞跃
    ↓
Phase 8 (OTA)           ← 告别 USB
    ↓
Phase 9 (密码)          ← 生产部署
    ↓
Phase 10 (MQTT/HA)      ← 智能家居生态
```

---

> 文档最后更新：2025-07-06
