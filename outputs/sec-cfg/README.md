# NGU800P SEC_CFG交付导航

本目录保存SEC_CFG正式专题报告。规范性内容按工程工作流分别维护，不以报告复制件替代来源、需求、主详设或测试状态。

## 正式报告

- `NGU800P_SEC_CFG_寄存器与软件使用专题报告.docx`：架构角色、地址边界、17个寄存器、status/hardware error位、FW error/LCS/UID/Debug边界、软件接口、启动读取顺序、Firewall关系、测试建议与编码门禁。

## 工作流落点

- 原始截图与转录：`source-vault/internal-specs/sec-cfg/SRC-0026/`
- 来源治理：`sources/source-cards/SRC-0026.md`、`sources/intake-reports/SRC-0026-sec-cfg-intake.md`
- 冲突：`sources/conflict-reports/CONFLICT-SRC-0026-SEC-CFG-ADDRESS-AND-SEMANTICS.md`、`OPEN-CONFLICT-015`
- 机读输入：`requirements/sec-cfg-register-map.yaml`、`requirements/sec-cfg-requirements.yaml`
- 架构：`docs/03-architecture/sec-cfg-status-observation.md`
- 软件接口：`docs/04-interfaces/sec-cfg-register-interface.md`
- 主详设：`docs/05-software-design/NGU800P安全软件详细设计.md`第5.9.2节
- 测试设计：`docs/06-verification/sec-cfg-test-design.md`
- 测试追踪：`tests/matrices/sec-cfg-traceability.yaml`
- 测试条目：`tests/cases/sec-cfg/NGU800P-sec-cfg-detailed-cases-v0.1.csv`

## 当前边界

- 17个寄存器offset、status/hardware-error可见bit和拼接顺序已登记，可指导参数化软件模型和test case。
- 截图base/size与SRC-0022冲突，当前baremetal绑定仍无效；不得生成产品MMIO常量。
- FW error、LCS编码、Debug scope/lock/合成、UID-valid和64-bit一致性仍受`OPEN-CONFLICT-015`阻断。
- 12条细化用例全部为`NOT_EXECUTED`。

