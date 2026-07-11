# WiFi 管理

> 对应代码：`src/network/WifiManager.h`、`src/network/WifiManager.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

本模块管理 ESP32 加入局域网的完整生命周期。它负责发现附近网络、使用用户提供的凭据建立连接、保存配置供下次启动复用，并在运行期间监测网络是否仍然可用。

### 网络连接的生命周期

```mermaid
stateDiagram-v2
    [*] --> 读取凭据
    读取凭据 --> 正在连接: 已保存 SSID
    读取凭据 --> 正在扫描: 没有凭据
    正在扫描 --> 等待配置: 网络列表可供 BLE 读取
    等待配置 --> 正在连接: 收到新凭据
    正在连接 --> 已连接: 连接成功
    正在连接 --> 连接失败: 超时
    连接失败 --> 正在扫描: 等待再次配置
    已连接 --> 已断线: 网络丢失
    已断线 --> 已连接: 重连成功
```

设备启动时首先从 NVS 读取以前保存的 SSID 和密码。有凭据时直接尝试连接；没有凭据时扫描附近网络，并把按信号强度排序的列表提供给 BLE 配网模块。新配置由系统入口交给本模块保存和连接。

连接成功后，模块向系统提供 IP、RSSI 和连接状态，并启动 `autodoor.local`，让用户不必记住设备 IP。扫描、连接、超时和断线等变化会转换成统一状态，供 BLE 返回手机、供 Web 页面展示。

本模块不解析手机提交的 BLE 数据，也不实现网页或门控逻辑。它只负责让其他模块获得稳定的局域网连接和可观察的网络状态。

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
