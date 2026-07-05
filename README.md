# AutoDoor V3 — 智能自动门控制器

基于 ESP32 的智能自动门系统。**BLE 负责首次配网**，WiFi 连接后**全部通过 Web 网页控制**。

| 阶段 | 描述 |
|------|------|
| **配网** | BLE 3-Characteristic 协议：WiFiScan 查询周围网络、WiFiConfig 选择网络输密码 |
| **日常** | Web 浏览器 `http://autodoor.local`：超声波自动感应 + 手动控制 + 参数修改 |

---

## 目录

1. [系统概述](#1-系统概述)
2. [硬件清单与接线](#2-硬件清单与接线)
3. [软件依赖](#3-软件依赖)
4. [模块职责总览](#4-模块职责总览)
5. [配置参数详解](#5-配置参数详解)
6. [业务流程详解](#6-业务流程详解)
   - [6.1 上电启动与系统状态机](#61-上电启动与系统状态机)
   - [6.2 BLE WiFi 配网流程](#62-ble-wifi-配网流程)
   - [6.3 WiFi 连接与自动重连](#63-wifi-连接与自动重连)
   - [6.4 超声波环境标定流程](#64-超声波环境标定流程)
   - [6.5 主循环调度流程](#65-主循环调度流程)
   - [6.6 超声波测距与滤波流程](#66-超声波测距与滤波流程)
   - [6.7 舵机非阻塞运动流程](#67-舵机非阻塞运动流程)
   - [6.8 AUTO 模式 —— 三状态门控流程](#68-auto-模式--三状态门控流程)
   - [6.9 BLE 遥控模式](#69-ble-遥控模式)
   - [6.10 Web 浏览器控制流程](#610-web-浏览器控制流程)
   - [6.11 MANUAL 网页手动模式](#611-manual-网页手动模式)
   - [6.12 模式优先级与切换](#612-模式优先级与切换)
7. [REST API 参考](#7-rest-api-参考)
8. [BLE 协议参考](#8-ble-协议参考)
9. [串口日志参考](#9-串口日志参考)
10. [使用步骤](#10-使用步骤)

---

## 1. 系统概述

本项目是一个**智能自动门控制器**，支持两种控制模式（按优先级）：

| 模式 | 描述 | 触发方式 |
|------|------|----------|
| **MANUAL** | 网页手动控制，暂停自动感应 | 浏览器点击 MANUAL 按钮 |
| **AUTO** | 超声波自动感应，默认模式 | 无人干预 |

WiFi 首次配网通过 **BLE 多 Characteristic 协议**完成。配网成功后 BLE 关闭，浏览器输入 `http://autodoor.local` 访问控制台。BLE 仅用于配网，不参与日常控制。

系统有两个运行状态：

```
┌──────────────┐     BLE 配网成功     ┌──────────┐
│ CONFIGURING  │ ──────────────────▶  │ RUNNING  │
│ (BLE 广播等配网)│   连接 WiFi        │ (正常运行)│
└──────────────┘                     └──────────┘
```

---

## 2. 硬件清单与接线

| 元件 | 数量 | 说明 |
|------|------|------|
| ESP32 开发板 | 1 | 主控（WiFi + 蓝牙） |
| HC-SR04 超声波 | 1 | 距离测量 |
| 舵机 SG90/MG996R | 1 | 驱动门体 |
| 5V 独立电源 | 1 | 舵机供电 |

### 接线表

| HC-SR04 | ESP32 |
|---------|-------|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO 2 |
| ECHO | GPIO 1 |

| 舵机 | 连接 |
|------|------|
| 信号线 | GPIO 5 |
| VCC | 外部 5V |
| GND | 共地 |

---

## 3. 软件依赖

| 库 | 安装方式 |
|----|----------|
| **ESPAsyncWebServer** | 库管理搜索 `ESP32Async/ESPAsyncWebServer` |
| **AsyncTCP** | 库管理搜索 `ESP32Async/AsyncTCP` |
| **NimBLE-Arduino** | 库管理搜索安装 |
| **ESP32Servo** | 库管理搜索安装 |
| `WiFi.h` | ESP32 核心自带 |
| `ESPmDNS.h` | ESP32 核心自带 |
| `Preferences.h` | ESP32 核心自带 |

---

## 4. 模块职责总览

### 文件清单

| 文件 | 类 | 职责 |
|------|-----|------|
| `Config.h` | — | 所有可调参数：引脚、角度、时序、BLE UUID、WiFi 配置 |
| `Ultrasonic.h/.cpp` | `Ultrasonic` | HC-SR04 驱动：测距、三点中值滤波、上电标定、滤波器预热 |
| `ServoControl.h/.cpp` | `ServoControl` | 舵机非阻塞渐进驱动：每 15ms 更新一步，缓开快关 |
| `BleManager.h/.cpp` | `BleManager` | **BLE 配网服务**：3 Characteristic（WiFiScan / WiFiConfig / Servo） |
| `DoorController.h/.cpp` | `DoorController` | **核心状态机**：MANUAL/AUTO 双模式调度 + 防抖 + 公开 getter/setter |
| `WifiManager.h/.cpp` | `WifiManager` | 纯 STA 连接 + 30s 定时 WiFi 扫描缓存 + 连接状态通知 |
| `WebServerManager.h/.cpp` | `WebServerManager` | AsyncWebServer 路由：控制面板 + REST API + JSON |
| `WebPage.h` | — | 嵌入式控制面板 HTML/CSS/JS（PROGMEM） |
| `AutoDoorBLE.ino` | — | 主入口：`setup()` 初始化 + `loop()` CONFIGURING/RUNNING 调度 |

### 模块依赖关系

```
                         AutoDoorBLE.ino
                              │
          ┌───────────────────┼───────────────────────┐
          │                   │                       │
    WifiManager         WebServerManager         DoorController
    [STA/扫描/mDNS]     [路由/JSON/HTML]         [AUTO/MANUAL]
          │                   │                 ┌───────┤
          │                   │                 │       │
          └──────┬────────────┘           Ultrasonic ServoControl
                 │                         [测距]    [舵机]
           BleManager                                  ↑
    [WiFiScan/WiFiConfig/Servo] ←─────────────────────┘
                                仅配网阶段读角度/通知
```

**BleManager 持有 WifiManager + ServoControl 指针**：读扫描缓存、推送 WiFi 状态、Servo Read 返回实时角度。配网成功后 BLE 关闭，日常运行不参与控制。

---

## 5. 配置参数详解

### 5.1 引脚

| 参数 | 值 | 说明 |
|------|-----|------|
| `TRIG_PIN` | 2 | 超声波 TRIG |
| `ECHO_PIN` | 1 | 超声波 ECHO |
| `SERVO_PIN` | 5 | 舵机信号线 |

### 5.2 舵机

| 参数 | 值 | 说明 |
|------|-----|------|
| `SERVO_OPEN_ANGLE` | 90° | 开门角度 |
| `SERVO_CLOSE_ANGLE` | 0° | 关门角度 |
| `SERVO_UPDATE_INTERVAL` | 15ms | 每步间隔 |
| `SERVO_OPEN_STEP` | 30° | 开门步长（快：0→90° 约 45ms） |
| `SERVO_CLOSE_STEP` | 10° | 关门步长（慢：90→0° 约 135ms） |

### 5.3 超声波

| 参数 | 值 | 说明 |
|------|-----|------|
| `DISTANCE_CHANGE_THRESHOLD` | 2.5cm | 检测阈值 |
| `MAX_VALID_DISTANCE` | 80cm | 最大有效距离 |

### 5.4 自动门时序

| 参数 | 值 | 说明 |
|------|-----|------|
| `PRESENCE_TIMEOUT` | 1500ms | 驻留超时 |
| `CLOSE_DELAY` | 2000ms | 关门延时 |
| `DETECT_DEBOUNCE` | 200ms | 检测防抖 |

### 5.5 BLE UUID

| 常量 | UUID | 属性 | 说明 |
|------|------|------|------|
| `SERVICE_UUID` | `4fafc201-...-c331914b` | — | 服务 |
| `SERVO_CHAR_UUID` | `...-b26a8` | R/W/N | 舵机角度 |
| `WIFI_SCAN_CHAR_UUID` | `...-b2901` | Read | WiFi 扫描结果 |
| `WIFI_CONFIG_CHAR_UUID` | `...-b2902` | W/N | 配网指令 & 状态通知 |

### 5.6 Web

| 参数 | 值 | 说明 |
|------|-----|------|
| `MDNS_HOSTNAME` | `autodoor` | mDNS 域名 `.local` |
| `WEB_PORT` | 80 | HTTP 端口 |

### 5.7 调试

| 参数 | 值 | 说明 |
|------|-----|------|
| `DEBUG_DISTANCE` | `true` | 串口输出距离调试 |
| `DEBUG_PRINT_INTERVAL` | 300ms | 打印间隔 |

---

## 6. 业务流程详解

### 6.1 上电启动与系统状态机

```
ESP32 上电
    │
    ├─ Serial 115200 → "AutoDoorBLE Starting..."
    ├─ Servo 归位 0°
    ├─ Ultrasonic 标定（10 样本 → 基线）
    ├─ BLE 广播开始（3 Characteristic Service）
    ├─ DoorController 初始化（state=CLOSED, manualMode=false）
    ├─ WifiManager::begin()
    │    ├─ 读 NVS → 有凭证 → tryConnect()
    │    │   成功 → sysState=RUNNING → Web Server → mDNS
    │    └─ 无凭证 → 等待 BLE 配网
    │
    └─ 串口提示当前状态
```

### 6.2 BLE WiFi 配网流程

```
sysState = CONFIGURING
    │
    ├─ BLE 广播 MyESP32Mini
    │
    ├─ WifiManager 每 30s 自动扫描附近 WiFi → 缓存格式：
    │     "0|HomeWiFi|-45|WPA2\n1|Office|-60|WPA2"
    │
    ├─ 用户 nRF Connect 操作：
    │   ① Read WiFiScan Characteristic → 看到 WiFi 列表
    │   ② Write WiFiConfig Characteristic → "CFG|0|password123"
    │
    ├─ BleManager 收到 Write：
    │   解析 "CFG|index|password"
    │   设置 newWiFiConfig=true
    │
    ├─ loop() 中检测到 newWiFiConfig：
    │   按 index 从 WifiManager 缓存查 SSID
    │   wifi.saveCredentials(ssid, password)
    │   wifi.tryConnect(ssid, password)
    │
    ├─ WifiManager 连接过程：
    │   connectStatus → "STATE|CONNECTING"
    │   BleManager::update() 检测到 statusChanged → wifiConfigChar.notify()
    │
    └─ 连接成功：
        connectStatus → "STATE|CONNECTED|192.168.1.88"
        BleManager 推送 notify
        BleManager::stop() 关闭广播
        Web Server 启动
        sysState → RUNNING
```

### 6.3 WiFi 连接与自动重连

```
WifiManager 内部：

begin():
  读 NVS → 有 ssid → WiFi.begin()
  connecting = true
  connectStatus = "STATE|CONNECTING"
  statusChanged = true

update():
  ├─ 每 30s WiFi.scanNetworks() → 缓存到 cachedNetworks
  │
  ├─ 连接中：
  │   WL_CONNECTED → connected=true, statusChanged=true
  │   >15s 超时 → connecting=false, "STATE|FAILED|TIMEOUT"
  │
  ├─ 已连接断线：
  │   WL_DISCONNECTED → connected=false, "STATE|DISCONNECTED"
  │   每 30s WiFi.reconnect()
  │
  └─ statusChanged=true 时
      BleManager::update() 消费标志 → wifiConfigChar.notify()
```

### 6.4 超声波环境标定流程

```
calibrate() 启动:
    ├─ 采集 10 次有效 readRaw() 数据
    ├─ 过滤：<=0 丢弃，>80cm 丢弃
    ├─ 间隔 50ms，打印每步结果
    ├─ 去 Min/Max，取 8 个平均 → baseline
    └─ primeFilter(): 3 次有效读数预热中值滤波器
```

### 6.5 主循环调度流程

```
loop():
    ├─ wifi.update()
    │   定时扫描 + 连接状态监测 + mDNS
    │
    ├─ servo.update()
    │   非阻塞每 15ms 走一步
    │
    └─ sysState 分支：
        ├─ CONFIGURING：
        │   ble.update()  // 推送 WiFi 状态 notify + 等配网数据
        │   收到 CFG → 查 SSID → 存 NVS → tryConnect
        │   连接成功 → Web Server → 关 BLE → RUNNING
        │
        └─ RUNNING：
            ble.update()  // 应急遥控
            door.update() // 状态机
```

### 6.6 超声波测距与滤波流程

```
readRaw():
    发 10μs 脉冲 → pulseIn(30ms 超时) → duration × 0.0343 / 2 = cm
    有效性：<=0→-1, >80cm→-1, 正常→返回

readDistance():
    d = readRaw()
    d<0 → return -1 (不污染滤波窗口)
    history[3] 环形缓冲 → >=3个 → median3()
```

### 6.7 舵机非阻塞运动流程

```
servo.update():
    now - lastUpdateTime < 15ms → 跳过
    currentAngle == targetAngle → 跳过
    currentAngle < targetAngle → += openStep(30°)
    currentAngle > targetAngle → -= closeStep(10°)
    servo.write(currentAngle)
```

### 6.8 AUTO 模式 —— 三状态门控流程

```
updateAutoMode(now):
    d = ultrasonic->readDistance()
    d<0 → 本帧跳过

    diff = baseline - d
    detect = diff >= 2.5cm

    防抖: detect 持续 >=200ms → lastSeenTime=now
    isPresent = (now - lastSeenTime) < 1500ms

    状态机:
    ┌─────────────────────────────────────────┐
    │ CLOSED   isPresent → (开 90°) → OPEN    │
    │ OPEN     !isPresent → WAIT_CLOSE        │
    │ WAIT_CLOSE  isPresent → OPEN            │
    │ WAIT_CLOSE  2s到 → (关 0°) → CLOSED     │
    └─────────────────────────────────────────┘
```

### 6.9 BLE 遥控模式

```
ble->isBleMode() == true:
    ble->hasNewCommand(angle)
      true → servo->setTargetAngle(angle)
      false → 保持当前角度

    断开连接 → 强制关门 → 切回 AUTO
```

### 6.10 Web 浏览器控制流程

```
浏览器 GET / → INDEX_HTML 控制页

页面 JS 每 500ms:
    fetch /api/status → JSON → 更新 DOM

用户操作:
    滑块 / OPEN / CLOSE → GET /api/servo?angle=N
    AUTO / MANUAL → POST /api/mode
    CALIBRATE → POST /api/calibrate
```

### 6.11 MANUAL 网页手动模式

```
door->isManualMode() == true:
    DoorController::update() → 直接 return
    超声波三状态机暂停
    servo.update() 依然运行（Web API 直接设 targetAngle）
```

### 6.12 模式优先级与切换

```
优先级: MANUAL > AUTO

    ┌─────────┐
    │  AUTO   │ ← 默认（超声波自动感应）
    └────┬────┘
         │
    ┌────┴────┐
    ▼         ▼
  AUTO     MANUAL
(超声波)   (网页)

---

## 7. REST API 参考

运行在 STA 模式下（`http://autodoor.local` 或 `http://IP`）：

| 方法 | 路径 | 参数 | 响应 |
|------|------|------|------|
| `GET` | `/` | — | HTML 控制页 |
| `GET` | `/api/status` | — | JSON 状态 |
| `GET` | `/api/servo` | `?angle=90` | `{"ok":true}` |
| `POST` | `/api/mode` | `{"mode":"manual"}` 或 `{"mode":"auto"}` | `{"ok":true}` |
| `POST` | `/api/calibrate` | — | `{"ok":true,"baseline":35.2}` |

### /api/status 字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `distance` | float | 当前测距 (cm) |
| `baseline` | float | 基线距离 (cm) |
| `diff` | float | baseline - distance |
| `detect` | bool | 是否检测到靠近 |
| `present` | bool | 是否在场 |
| `door` | string | CLOSED / OPEN / WAIT_CLOSE |
| `servo` | int | 舵机当前角度 |
| `mode` | string | AUTO / MANUAL |
| `wifi` | bool | 是否连接 |
| `ip` | string | IP 地址 |
| `rssi` | int | 信号强度 (dBm) |

---

## 8. BLE 协议参考

### Service

| 属性 | 值 |
|------|-----|
| UUID | `4fafc201-1fb5-459e-8fcc-c5c9c331914b` |
| 设备名 | `MyESP32Mini` |

### Characteristic 列表

| 名称 | UUID | 属性 | 方向 | 数据格式 |
|------|------|------|------|----------|
| **Servo** | `...b26a8` | Read / Write / Notify | 双向 | 数字 `0` ~ `180` |
| **WiFiScan** | `...b2901` | Read | ESP32→手机 | `SCAN\|索引\|SSID\|RSSI\|加密方式` 多行 |
| **WiFiConfig** | `...b2902` | Write / Notify | 双向 | Write: `CFG\|索引\|密码` ，Notify: `STATE\|状态` |

### WiFiScan Read 示例

```
SCAN|0|HomeWiFi|-45|WPA2
SCAN|1|TP-LINK|-60|WPA2
SCAN|2|OpenGuest|-72|OPEN
```

无数据时返回 `EMPTY`。

### WiFiConfig Write 示例

```
CFG|0|password123
```

表示选择索引 0（HomeWiFi），密码 `password123`。

### WiFiConfig Notify 状态

```
STATE|CONNECTING
STATE|CONNECTED|192.168.1.88
STATE|FAILED|TIMEOUT
STATE|DISCONNECTED
```

### Servo

- **Write** `90`：舵机转到 90°，回显写入值
- **Read**：返回舵机当前实时角度（`servo->getCurrentAngle()`），非最后一次写入值

---

## 9. 串口日志参考

上电后串口监视器（115200）：

| 日志 | 含义 |
|------|------|
| `AutoDoorBLE Starting...` | 系统启动 |
| `Calibrate...` / `Sample 1 : 35.21` ... | 超声波标定 |
| `Baseline = 35.17` | 标定完成 |
| `BLE Advertising Started` | BLE 开始广播 |
| `WiFi: trying HomeWiFi` | 尝试连接已保存的 WiFi |
| `WiFi Connected, IP: 192.168.1.88` | 连接成功 |
| `mDNS: http://autodoor.local` | 域名可用 |
| `Web Server Started` | HTTP 服务就绪 |
| `No WiFi. Use nRF Connect to:` | 进入配网模式 |
| `WiFi Scan done` | 扫描完成 |
| `BLE Read` | BLE 读操作 |
| `BLE RX: CFG\|0\|password` | 收到配网指令 |
| `Connect via BLE: HomeWiFi` | 解析出 SSID |
| `WiFi saved: HomeWiFi` | 凭证已存 NVS |
| `BLE Notify: STATE\|CONNECTING` | 推送连接状态 |
| `BLE Notify: STATE\|CONNECTED\|192.168.1.88` | 连接成功通知 |
| `BLE Stopped` | BLE 广播关闭 |
| `BLE Connected` | 手机 BLE 已连接 |
| `Distance=21.5cm Baseline=35.1cm Diff=13.6cm ...` | 调试输出（300ms） |

---

## 10. 使用步骤

### 首次配网

1. 按接线表连接硬件，**舵机独立 5V 供电**
2. IDE 安装依赖库：`ESPAsyncWebServer`、`AsyncTCP`、`NimBLE-Arduino`、`ESP32Servo`
3. 烧录程序，串口 115200 看到 `No WiFi. Use nRF Connect to:`
4. 手机安装 **nRF Connect**（iOS/Android）
5. 扫描 BLE → 连接 `MyESP32Mini`
6. 找到 **WiFiScan** Characteristic → Read → 查看 WiFi 列表
7. 找到 **WiFiConfig** Characteristic → Write → `CFG|0|your_password`
8. 等待 Notify：`STATE|CONNECTED|192.168.1.xxx`
9. Safari/Chrome 打开 `http://192.168.1.xxx` 进入控制台
10. 配网完成，以后重启自动连 WiFi

### 日常使用

- **AUTO 模式**：走近传感器 → 门自动打开
- **MANUAL 模式**：网页切 MANUAL → 滑块/按钮手动控制
- **标定**：网页 CALIBRATE 按钮

### 重新配网

nRF Connect 连接后，向 WiFiConfig Characteristic 写入 `CFG|2|new_password` 更换路由器。
（后续版本可通过网页操作）

---

> 文档最后更新：2025-07-06
