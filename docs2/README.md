# AutoDoorBLE 文档中心

这是项目文档的唯一入口，也是人工开发和 Agent 编程的起点。

## 当前版本

当前实现是 V3：ESP32 自动门控制、BLE Wi-Fi 配网、Web 控制台和 REST API。`AutoDoorBLE.ino` 是薄入口，应用装配与运行调度由 `AutoDoorApp` 负责。

## 首次阅读顺序

1. [产品与范围](product-overview.md)
2. [系统架构](architecture.md)
3. [运行时行为](runtime-behavior.md)
4. [硬件与安全](hardware.md)
5. [接口契约](interfaces.md)
6. [验证手册](verification.md)
7. [路线图](roadmap.md)

## 按任务阅读

| 任务 | 必读文档 |
|---|---|
| 自动开关门逻辑 | 产品、运行时、硬件、目标 feature spec |
| BLE 或 Wi-Fi | 架构、运行时、接口、目标 feature spec |
| Web 或 API | 架构、接口、验证、目标 feature spec |
| 传感器或舵机 | 硬件、运行时、验证、目标 feature spec |
| 路线图功能 | 路线图和对应 feature spec，再按规格补读 |
| 代码审查 | 架构、接口、验证和目标 feature spec |

## 文档权威性

- 本目录的非 `features/` 文档描述当前事实和稳定契约。
- `features/` 描述目标变更；`Proposed` 不代表已经实现。
- 代码与当前事实冲突时，以代码为调查起点，修正代码或同步文档，不静默忽略。
- 路线图只管理优先级与依赖；详细需求只写在 feature spec。
- 根目录 `AGENTS.md` 定义开发纪律，项目 Skill 定义重复执行流程。

## 功能规格

- [规格模板](features/template.md)
- [WebSocket 实时状态](features/websocket-status.md)
- [传感器抽象层](features/sensor-abstraction.md)
- [运行时参数配置](features/runtime-configuration.md)

## 维护规则

1. 每个概念只有一个权威定义，其他文件使用链接。
2. 接口字段、状态名称、单位和时间必须明确。
3. 每项需求必须包含非目标、异常行为和可观察验收条件。
4. 未验证的设计放入 `features/`，不能写成当前能力。
5. 完成功能后同步规格状态、当前事实文档和路线图。
