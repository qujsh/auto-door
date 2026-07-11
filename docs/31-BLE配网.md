# BLE 配网

> 对应代码：`src/network/BleManager.h`、`src/network/BleManager.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

### 一句话定位

本模块提供首次联网和后续更换 Wi-Fi 的 BLE 协议入口，使手机能读取网络列表、提交凭据并接收连接状态。

### 负责什么

- 创建并广播 `AutoDoor` BLE 服务。
- 通过只读特征提供附近 Wi-Fi 列表。
- 通过可写特征接收“网络索引 + 密码”。
- 把新配置作为一次性消息交给系统入口处理。
- 通过通知特征向已连接手机发送 Wi-Fi 状态变化。
- 手机断开后重新广播，运行阶段也持续保持可用。

### 配网数据流

1. 手机连接 `AutoDoor` BLE 服务。
2. 手机读取 Wi-Fi 扫描特征，获得由 Wi-Fi 管理模块生成的网络列表。
3. 手机向配置特征写入网络索引和密码。
4. BLE 模块解析并暂存配置。
5. 系统入口消费配置，由 Wi-Fi 管理模块保存并连接。
6. 连接状态变化通过 BLE 通知返回手机。

### 通道与职责

| 通道 | 方向 | 用途 |
|---|---|---|
| Wi-Fi 扫描特征 | 设备 → 手机 | 读取网络列表和操作提示 |
| Wi-Fi 配置特征 | 手机 → 设备 | 提交网络索引和密码 |
| Wi-Fi 配置通知 | 设备 → 手机 | 返回扫描、连接、成功或失败状态 |

### 职责边界

本模块只负责 BLE 协议和数据传递，不执行 Wi-Fi 扫描、凭据持久化或连接，也不参与门控决策。

### 安全注意事项

> 当前实现的调试日志会输出包含 Wi-Fi 密码的原始配网数据，这是已知安全缺陷，不代表期望设计。在修复前，不应向不可信人员开放串口日志。

---

<!-- ============== 第二部分：给 AI 和开发者阅读 ============== -->

## 分：代码重建规格（给 AI 或修改代码的开发者阅读）

### 类结构

`BleManager` 同时继承 `NimBLEServerCallbacks` 和 `NimBLECharacteristicCallbacks`。公开：构造、六参数 begin、update、isConnected、isBleMode、stop、`hasWiFiConfig(int&,String&)`。覆盖 onConnect、onDisconnect、onRead、onWrite；私有 parseWiFiConfig。

成员包括 server/service/两个 characteristic/WifiManager 指针；volatile connected、bleMode、newWiFiConfig；configIndex 和 configPassword。构造时指针 null、布尔 false、索引 -1。

### 服务创建

初始化设备名，功率 P9，MTU 256；创建 server/service。WiFiScan 特征仅 READ；WiFiConfig 为 WRITE|NOTIFY；两者回调均为 this。启动服务，广告加入 service UUID、名称和 scan response，开始广播。

### 回调和协议

- 连接：connected/bleMode=true。
- 断开：两者 false，打印 reason，重新广播。
- Read：仅 WiFiScan 生效；缓存为空返回中文“扫描中”提示，否则返回中文输入提示加网络列表。
- Write：空值返回；当前实现逐字符打印完整收到的数据，再对 WiFiConfig 解析。
- 解析：trim，找到第一个 `+`；没有则报错。前段 `toInt()` 为索引，后段 trim 为密码，设置 newWiFiConfig。
- `hasWiFiConfig` 是一次性消费：有值时复制索引和密码并清零标志。

### 状态通知

`update()` 仅在 wifi 存在、状态发生变化且 BLE 已连接时读取状态；characteristic 存在则 setValue、notify 并打印。未连接时调用 `hasStatusChanged()` 的短路会保留 WifiManager 标志。

### 当前安全问题和重建要求

当前 `onWrite` 和解析日志会输出包含 Wi-Fi 密码的原文，这是已识别风险，不应被当作理想设计。若严格重建当前代码需保持行为；正常维护应另行修复并同步本文。其余重建需保持 UUID 由调用者注入、属性、协议分隔方式和一次性消费语义。
