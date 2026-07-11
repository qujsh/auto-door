# BLE 配网

> 对应代码：`src/network/BleManager.h`、`src/network/BleManager.cpp`
> 重建等级：L4（结构与行为重建）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

本模块提供设备首次联网和后续更换 Wi-Fi 所需的蓝牙入口。用户可以使用支持 BLE Characteristic 读写的手机工具连接 `AutoDoor`，读取附近 Wi-Fi 列表，再提交选择的网络和密码，不需要先让设备接入局域网。

BLE 服务包含两个主要通道：一个只读通道提供 Wi-Fi 扫描结果，另一个可写并支持通知，用于接收“网络索引+密码”并把连接进度返回给客户端。真正的网络扫描、凭据保存和连接动作由 Wi-Fi 管理及应用调度模块完成，本模块只负责蓝牙协议与数据传递。

手机连接后，模块记录当前 BLE 状态；断开时会重新开始广播，方便再次连接。即使设备已经进入正常运行阶段，BLE 仍保持可用，因此用户无需清除固件或连接串口就可以切换到新的 Wi-Fi。

该模块处理的是敏感配网数据。当前实现仍存在调试日志输出密码的已知风险，后续应移除；在完成修复前，不应在不可信环境中公开串口日志。

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
