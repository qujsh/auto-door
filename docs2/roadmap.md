# 路线图

路线图只表达顺序、价值、依赖和状态；详细实现以 feature spec 为准。

| 阶段 | 功能 | 状态 | 依赖 | 规格 |
|---:|---|---|---|---|
| 4 | WebSocket 实时状态 | Proposed | REST 状态模型 | [规格](features/websocket-status.md) |
| 5 | 距离传感器抽象和 TOF 支持 | Proposed | 无 | [规格](features/sensor-abstraction.md) |
| 6 | Web 运行时参数配置 | Proposed | 参数模型 | [规格](features/runtime-configuration.md) |
| 7 | 环形日志和距离曲线 | Backlog | Phase 4 | 待建立 |
| 8 | OTA 固件升级 | Backlog | 认证与安全决策 | 待建立 |
| 9 | Web 访问认证 | Backlog | 配置持久化 | 待建立 |
| 10 | MQTT 和 Home Assistant | Backlog | 认证、运行时配置 | 待建立 |

建议先完成 Phase 5，再完成 Phase 4，以降低业务与硬件耦合并改善可测试性。OTA 不应早于基本认证和升级安全策略，实施前需重新评估 Phase 8/9 顺序。

- `Proposed`：已有可执行规格，尚未实施。
- `In Progress`：已选定实施且存在开发改动。
- `Done`：验收完成，当前事实文档已同步。
- `Backlog`：只有方向，必须先创建规格。
