---
name: soc-test-case-generator
description: "Generate reviewable SoC security test cases linked to requirements, threats, environments, expected logs, and Evidence. Use for normal, negative, boundary, power-loss, rollback, authorization, fault-injection, integration, simulation, or hardware-validation scenarios after expected behavior is supported by approved facts."
---

# 测试用例生成

1. 读取 Requirement、威胁场景、批准设计、接口、适用环境和事实状态。
2. 为正常、异常、边界、掉电、回滚、越权和故障注入选择适用场景。
3. 每个用例记录 Requirement ID、前置条件、输入、步骤、预期行为、预期日志、Evidence、自动化状态、环境和结果状态。
4. 不根据 `ASSUMPTION` 自动生成确定性硬件预期；改为阻塞问题或探索性测试。
5. 区分单元、集成、仿真、QEMU 和真实硬件能力。
6. 检查清理、可重复性、密钥/敏感数据保护和失败判定。

使用 `templates/test-case.md`。
