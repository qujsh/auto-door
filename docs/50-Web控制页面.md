# Web 控制页面

> 对应代码：`src/web/WebPage.h`
> 重建等级：L3（行为重建；视觉样式允许等价实现）

<!-- ==================== 第一部分：给人阅读 ==================== -->

## 总：模块概要（给人阅读）

这是嵌入固件的单页控制台，显示网络、距离、检测、门和舵机状态，并提供模式选择、角度滑块、开门、关门和标定按钮。页面每 500ms 查询一次设备状态。

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
