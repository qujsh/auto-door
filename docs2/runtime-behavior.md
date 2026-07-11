# 运行时行为

## 启动流程

1. 初始化串口和舵机，目标为关闭角度。
2. 初始化超声波并执行环境标定。
3. 初始化 Wi-Fi、BLE、门控制器和 Web 服务。
4. 已连接 Wi-Fi 时进入 `Running`，否则保持 `Configuring`。

## 门状态机

| 当前状态 | 条件 | 下一状态 | 动作 |
|---|---|---|---|
| `Closed` | 确认有人 | `Open` | 目标设为开启角度 |
| `Open` | 确认无人 | `WaitingToClose` | 记录等待开始时间 |
| `WaitingToClose` | 再次有人 | `Open` | 取消关门等待 |
| `WaitingToClose` | 无人且延时到期 | `Closed` | 目标设为关闭角度 |

距离差为 `baseline - currentDistance`。达到阈值并持续满足防抖时间后更新最后出现时间；驻留超时内视为有人。无效距离不推进本轮状态机。

## BLE 配网

1. `WifiManager` 周期扫描并缓存 SSID。
2. 客户端读取 WiFiScan，向 WiFiConfig 写入网络索引和密码。
3. `BleManager` 暂存请求，`AutoDoorApp` 在主循环消费。
4. Configuring 状态保存凭据并连接；Running 状态收到新配置时保存并重启。

## 网络异常

- 连接超时后报告失败。
- 已连接网络断开后报告断线并定时重连。
- BLE 更新始终运行，以便运行期间更换 Wi-Fi。
- 自动门控制不以 Web 客户端在线为前提。

## MANUAL

`DoorController::update()` 在 MANUAL 直接返回。Web 仍可修改舵机目标，`ServoControl::update()` 继续执行。
