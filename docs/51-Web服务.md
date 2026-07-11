# Web 服务

> 对应代码：`src/web/WebServerManager.h`、`src/web/WebServerManager.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

模块启动异步 HTTP 服务，提供控制页面、状态查询、舵机控制、AUTO/MANUAL 切换和重新标定接口。它只做协议适配，实际门控由 DoorController 完成。

---

<!-- ============== 第二部分：给 AI 和开发者阅读 ============== -->

## 分：代码重建规格（给 AI 或修改代码的开发者阅读）

### 类结构和初始化

头文件包含 ESPAsyncWebServer、DoorController、ServoControl、WifiManager。公开构造和 `begin`。私有保存 server 与三个依赖指针，声明路由处理器、`buildStatusJson()`、`doorStateToString()`。构造全部指针 null。begin 保存依赖，以 `Config::Network::webPort` new AsyncWebServer，注册路由并 begin。

### 路由契约

| 方法 | 路径 | 行为 |
|---|---|---|
| GET | `/` | `send_P(200,"text/html",INDEX_HTML)` |
| GET | `/api/status` | 返回 buildStatusJson |
| GET | `/api/servo?angle=N` | 缺参 400；toInt 后钳制 0..180，设置目标，返回 ok |
| POST | `/api/mode` | body 含 `manual` 即 manual，否则 auto，返回 ok |
| POST | `/api/calibrate` | 同步标定，返回 ok 和两位小数 baseline |

mode body 回调按本次 `data,len` 构造 String，当前不处理分片 index/total。

### 状态 JSON

字段顺序和来源：distance、baseline、diff、detect、present、door、servo、mode、wifi、ip、rssi。前三个浮点为两位小数；door 映射 Closed→CLOSED、Open→OPEN、WaitingToClose→WAIT_CLOSE、默认 UNKNOWN；mode 为 MANUAL/AUTO。

### 约束与验收

- Web 不自行执行门状态机。
- 当前没有认证和 CSRF 保护。
- 重新标定是同步阻塞请求。
- 重建后路由、方法、字段、状态码和字符串必须一致。
