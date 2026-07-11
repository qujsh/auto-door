# 接口契约

## REST API

服务地址为设备 IP 或 `http://autodoor.local`。

| 方法 | 路径 | 输入 | 成功结果 |
|---|---|---|---|
| GET | `/` | 无 | Web 页面 |
| GET | `/api/status` | 无 | 状态 JSON |
| GET | `/api/servo` | `angle` 查询参数 | `{"ok":true}` |
| POST | `/api/mode` | `{"mode":"manual"}` 或 `{"mode":"auto"}` | `{"ok":true}` |
| POST | `/api/calibrate` | 无 | `ok` 和新 baseline |

`/api/status` 当前字段：`distance`、`baseline`、`diff`、`detect`、`present`、`door`、`servo`、`mode`、`wifi`、`ip`、`rssi`。字段变化必须同步前端并说明兼容性。

## BLE

- 设备名：`AutoDoor`
- Service：`a550d001-0000-a550-d001-a550d0010001`
- WiFiScan：`a550d001-0000-a550-d001-a550d001a001`
- WiFiConfig：`a550d001-0000-a550-d001-a550d001a002`

WiFiScan Read 每行格式为 `index|SSID|signal-label`，无结果返回 `EMPTY`。

WiFiConfig Write 当前格式为 `index+password`。密码包含 `+` 时应只将第一个 `+` 作为分隔符；若实现不满足，先建立修正规格。

Notify 状态：

```text
STATE|SCANNING
STATE|CONNECTING
STATE|CONNECTED|<ip>
STATE|FAILED|TIMEOUT
STATE|DISCONNECTED
```

## 兼容规则

- 字段、UUID、状态字符串或输入格式变化必须在 feature spec 标明兼容性。
- 不得通过串口或 API 回显 Wi-Fi 密码。
- 错误输入必须可诊断，不能阻塞主循环或导致崩溃。
