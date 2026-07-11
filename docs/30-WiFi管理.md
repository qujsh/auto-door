# WiFi 管理

> 对应代码：`src/network/WifiManager.h`、`src/network/WifiManager.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

### 一句话定位

本模块管理 ESP32 以 STA 模式加入局域网的完整生命周期，包括扫描、连接、凭据保存、断线检测和重连。

### 负责什么

- 扫描附近 Wi-Fi，并按信号强度整理网络列表。
- 使用指定 SSID 和密码发起连接。
- 把凭据保存到 ESP32 NVS，供下次启动复用。
- 监测连接、超时和断线状态，并尝试恢复连接。
- 连接成功后启动 `autodoor.local` mDNS 名称。
- 向 BLE 和 Web 模块提供网络状态、IP 和信号强度。

### 连接流程

1. 启动时从 NVS 读取已保存凭据。
2. 有凭据时直接尝试连接；没有凭据时扫描附近网络。
3. BLE 配网提交新凭据后保存并发起连接。
4. 连接状态变化时生成统一状态文本供 BLE 通知。
5. 已连接网络丢失后，按照重连间隔尝试恢复。

### 对外提供的信息

| 信息 | 使用者 |
|---|---|
| 排序后的网络列表和索引 | BLE 配网 |
| 扫描、连接、成功、失败、断线状态 | BLE 配网、系统入口 |
| 是否连接、IP、RSSI | 系统入口、Web 服务 |
| 保存凭据和立即连接能力 | 系统入口 |

### 职责边界

本模块不解析 BLE 配网数据，不实现网页接口，也不决定门的动作；它只提供网络连接能力和可观察状态。

### 注意事项

- Wi-Fi 密码只应写入 NVS，不应出现在日志、API 或 BLE 状态中。
- 当前连接和扫描恢复流程包含少量阻塞延时，属于现有实现约束。

---

<!-- ============== 第二部分：给 AI 和开发者阅读 ============== -->

## 分：代码重建规格（给 AI 或修改代码的开发者阅读）

### 接口和状态

头文件包含 WiFi、ESPmDNS、Preferences、vector。公开：`begin/update`；连接、IP、RSSI、连接中查询；`tryConnect`、`saveCredentials`；缓存网络、索引 SSID、连接状态和一次性状态变化读取。私有 `startScan()`、`processScanResult(int)`。

成员依次保存 connected/connecting/scanning，连接和重试时间，缓存字符串、扫描时间、`vector<String> scannedSSIDs`、状态字符串和 statusChanged。`begin()` 显式把这些值初始化为 false、0 或空。

### 启动与 NVS

设置 `WIFI_STA`，注册事件回调打印事件和断线原因。从 Preferences 命名空间 `wifi` 只读 `ssid`、`pass`。有 SSID 则打印并 `tryConnect`，否则启动扫描。保存时 SSID 空返回 false；写入同名键并只打印 SSID。

### update 状态优先级

1. connecting：连接成功时更新标志、`STATE|CONNECTED|IP`、状态变化并启动 mDNS；超时则断开，依次切换 WIFI_OFF/STA（包含 100/500/500ms delay），设置 `STATE|FAILED|TIMEOUT`；随后 return。
2. scanning：`scanComplete()` 为 -1 表示未完成；-2 打印失败；其他交给结果处理；return。
3. 达到 scanInterval 时启动扫描；return。
4. 未标记 connected 则 return。
5. 实际连接丢失时设置 disconnected 和 `STATE|DISCONNECTED`；达到重连间隔才 `WiFi.reconnect()`。

### 扫描结果

启动扫描前 disconnect 并 delay 200ms，记录时间、设置 scanning 和 `STATE|SCANNING`，调用 `scanNetworks(true)`。结果按 RSSI 从强到弱排序；缓存每行 `显示索引|SSID|中文信号标签`，行间 CRLF，并以同序填充 scannedSSIDs。标签阈值依次为 -30/-40/-50/-60/-70/-80。

结果不大于 0 时保留旧缓存；旧缓存为空则把下次扫描安排到约 5 秒后。成功后 `scanDelete()`。

### 连接和一次性状态

`tryConnect` 强制断开、delay 1000，STA、delay 500，关闭省电，调用 begin，设置 connecting 和开始时间，总返回 true。`hasStatusChanged()` 返回当前标志并立即清零。

### 已知约束与验收

- 当前代码包含多处阻塞 delay，文档按真实实现记录。
- 网络缓存索引必须与排序后的 SSID vector 一致。
- 密码只存 NVS，不应在此模块输出。
