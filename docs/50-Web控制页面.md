# Web 控制页面

> 对应代码：`src/web/WebPage.h`
> 重建等级：L3（行为重建；视觉样式允许等价实现）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

### 一句话定位

本模块是嵌入 ESP32 固件的浏览器控制界面，为同一局域网内的手机或电脑提供状态查看和日常操作入口。

### 页面提供什么

| 类别 | 内容 |
|---|---|
| 网络状态 | Wi-Fi 连接、IP、信号强度 |
| 测距状态 | 当前距离、环境基线、距离差 |
| 门控状态 | 检测中、人员存在、门状态、运行模式 |
| 舵机状态 | 当前角度和手动目标角度 |

页面使用不同颜色区分正常、检测到人、等待关门和网络离线等状态。

### 用户可以做什么

- 在 AUTO 和 MANUAL 模式之间切换。
- 在 MANUAL 模式下使用滑块或按钮设置舵机目标角度。
- 触发环境基线重新标定。
- 通过设备 IP 或 `autodoor.local` 访问页面，无需互联网或独立前端部署。

### 数据更新方式

1. 页面加载后立即请求一次设备状态。
2. 此后每 `500ms` 轮询状态接口。
3. 用户操作通过对应 HTTP API 发送给设备。
4. 页面根据响应和最新状态刷新文本、颜色及控件。

### 职责边界

页面只负责展示状态和发送操作请求，不执行人员判断、输入安全限制或硬件控制；这些行为均由 ESP32 后端模块负责。

### 使用注意事项

- 手动控制舵机前应确认门体运动范围安全。
- 重新标定前必须确保门前无人且没有临时障碍物。
- 当前页面和接口没有身份认证，只适合可信局域网。

---

<!-- ============== 第二部分：给 AI 和开发者阅读 ============== -->

## 分：代码重建规格（给 AI 或修改代码的开发者阅读）

### 文件结构

`WebPage.h` guard 为 `WEB_PAGE_H`，声明 `const char INDEX_HTML[] PROGMEM`，内容使用 C++ raw string。HTML 语言 zh、UTF-8、移动端 viewport，标题 `Auto Door V3`。页面最大宽度 440px，深色卡片式布局。

### 必备 DOM

状态元素 id：`wifi`、`ip`、`rssi`、`dist`、`base`、`diff`、`detect`、`present`、`door`、`servo`。模式 radio 的 name 为 `mode`、value 分别 auto/manual。角度控件为 0..180 的 `slider`，显示为 `angleDisp`。存在 CLOSE(0)、OPEN(90)、CALIBRATE 按钮和 `log` 容器。

### JavaScript 行为

- `setServo(a)` 同步控件后 GET `/api/servo?angle=`。
- slider 的 input 和 change 均调用 setServo。
- `setMode(m)` POST JSON 到 `/api/mode`。
- `calibrate()` POST `/api/calibrate`，成功时刷新 baseline。
- `update()` GET `/api/status`，刷新全部字段、颜色、radio 和门状态。
- AUTO 时用后端 servo 更新 slider；MANUAL 时保留用户控制位置。
- 请求失败时 Wi-Fi 显示 Offline。
- 页面加载后立即 update，并 `setInterval(update,500)`。

`addLog()` 会追加带本地时间的行并限制 50 条，但当前没有调用者。

### 验收

重建后必须保留全部 API 调用、DOM 功能和 500ms 轮询；CSS 可等价但需适合手机、区分正常/告警/等待颜色。
