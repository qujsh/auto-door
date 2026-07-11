# Web 控制页面

> 对应代码：`src/web/WebPage.h`
> 重建等级：L3（行为重建；视觉样式允许等价实现）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

本模块是用户日常查看和控制自动门的浏览器界面。页面直接嵌入 ESP32 固件，不依赖互联网服务器或额外前端部署；手机或电脑与设备处于同一局域网时，就可以通过设备 IP 或 `autodoor.local` 打开。

页面集中显示 Wi-Fi 连接、IP、信号强度、TOF 距离、环境基线、距离差、人员检测、门状态和舵机角度。不同颜色帮助用户快速区分正常、检测到人、等待关门或网络离线等状态。

用户可以在 AUTO 与 MANUAL 模式之间切换。在 MANUAL 模式下，可以通过角度滑块或开门、关门按钮直接设置舵机目标；标定按钮会让设备重新测量当前环境基线，使用前应确保门前没有人员或障碍物。

页面目前每 500ms 调用一次状态接口获取最新数据，属于简单可靠的轮询方案。它只负责展示和发送操作请求，所有状态判断、输入限制和硬件控制仍由 ESP32 后端模块负责。

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
