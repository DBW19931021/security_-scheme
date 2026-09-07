---
title: NGU800P SoC 安全测试用例收敛结果
date: 2026-08-13
status: completed
source_ids:
  - SRC-0025
  - SRC-0026
  - SRC-0027
---

# NGU800P SoC 安全测试用例收敛结果

## 本次目标

仅完成安全测试用例设计、文档和表格收敛，为后续 Codex 在 `baremetal` 中移植用例提供直接指导；本次不修改 `baremetal`、eHSM 固件或硬件设计，也不执行目标板测试。

## 收敛结果

- Codex 可移植/组织执行的用例共 73 条：Mailbox BASIC 52 条、eHSM 负向功能 14 条、SoC 软件或联合测试 7 条。
- SoC 顶层用例固定为 7 条：SEC_CFG 全量读取比较、从 Die Debug 控制、SEC_CFG Firewall 默认权限、SPIFC Firewall 默认权限、SRAM Firewall 默认权限、SRAM Firewall 重新配置、真实 eHSM Mailbox 响应中断。
- SRAM 两条用例都在单个用例内部遍历 Region0～Region4，不按 Region 拆分顶层用例。
- 另列 3 条不进入 Codex 移植数量的硬件验证需求：2 条必须由 EDA 覆盖，1 条在软件无真实 eHSM 访问路径时转 EDA 覆盖。
- Firewall 专项细化表中的 49 条历史细化项保留为设计参考，明确标记为不计入 v0.3.8 顶层 SoC Case、未执行。

## EDA／硬件覆盖边界

1. `EDA-ERR-CRITICAL-IRQ-001`：必须由 EDA 注入并覆盖关键错误源、`o_hsm_err_hw` 位映射、组合中断、位间串扰、保持/清除/重触发及多源并发。
2. `EDA-ERR-ECC1B-IRQ-001`：必须由 EDA 注入并覆盖 ECC 1bit 脉冲、独立中断线、SoC 可查询/可清除状态、外部计数、重触发及密集脉冲边界。
3. `EDA-FW-EHSM-MASTER-001`：优先使用现有公开 eHSM 操作自然产生真实 eHSM Master 访问；若无公开路径，必须由 EDA 以真实 eHSM Master ID 验证 SEC_CFG、SPIFC 和 SRAM 默认放行。不得要求 C908 伪造 eHSM Master ID，也不得为测试新增 eHSM 固件命令。

## 主要交付物

- `tests/cases/NGU800P-security-test-cases-v0.3.8-soc-converged.xlsx`
- `docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md`
- `docs/06-verification/NGU800P软件不便覆盖的硬件安全验证项.md`
- `docs/06-verification/NGU800P-security-test-case-workflow-record.md`
- SEC_CFG、Firewall 需求及追踪矩阵、项目状态、变更记录和来源登记同步更新。

## 校验记录

- 工作簿包含 12 个工作表；关键数量检查为 73 条 Codex 用例、7 条 SoC 顶层用例、3 条 EDA／硬件验证需求。
- 工作簿生成后重新导入，原始工作簿和重新导入工作簿的公式错误扫描均为 0 条命中。
- 12 个工作表均已渲染预览；重点复核“总览”“SoC集成用例”“硬件验证需求”“Codex移植指导”“Firewall用例”。
- 工程终检通过：70 个设计文档 frontmatter、27 个 Source ID、25 个 YAML、追踪路径、本地链接及 6 个项目级 Skill 均检查通过；保留 16 个已显式标注、待资料收敛的假设项告警。
- 本次未执行 Git 操作。

## 尚待后续确认

- `dbg_en_cfg`、`soc_dbg_en_out` 的最终地址、位定义、复位值和实际 Debug 联调接口。
- 关键错误组合中断与 ECC 1bit 独立中断的 SoC 侧 IRQ 号、屏蔽/清除寄存器和清除时序。
- ECC 1bit 外部计数器的寄存器、位宽、饱和/回绕、清零及复位规则。
- 是否存在可自然触发 SEC_CFG、SPIFC、SRAM 访问的公开 eHSM 操作；若不存在，按条件 EDA 项执行。

上述未决项不改变 7 条 SoC 顶层用例与 3 条 EDA／硬件需求的分类，只影响后续实现参数和证据采集方式。
