# NGU800 约束提取表

> 目的：在任何详细设计之前，先把输入材料中真正影响方案落地的硬约束抽出来。  
> 本文件是 `02_baseline.md` 和 `03_detailed_design.md` 的前置输入。

## 1. 输入状态摘要

| 输入类别 | 当前状态 | 完整度 | 主要影响章节 | 备注 |
|---|---|---|---|---|

## 2. 约束表

| Constraint ID | Category | Statement | Source | Strength | Impact | Status | Conflict With | Resolution |
|---|---|---|---|---|---|---|---|---|
| C-BOOT-001 | boot_chain | 示例：BootROM 无能力、无权限直接对 SEC1 做密码学校验 | SRC-001 | HARD | ch4/ch6 | [CONFIRMED] |  | 由 eHSM 承担首个密码学校验 |

### 约束字段说明
- `Category`：boot_chain / trust_boundary / key_cert / lifecycle_debug / host_boundary / board_security / update_rollback / manufacturing / interface
- `Strength`：
  - `HARD`：必须遵守，不能被低优先级来源覆盖
  - `SOFT`：优先遵守，但在更高优先级资料下可调整
  - `REFERENCE`：仅作参考，不直接视为强约束
- `Status`：
  - `[CONFIRMED]`
  - `[ASSUMED]`
  - `[TBD]`

## 3. 冲突汇总

| Conflict ID | Topic | Source A | Source B | Current Baseline | Impacted Chapters | Status |
|---|---|---|---|---|---|---|

## 4. 缺失输入对冻结的影响

| Missing Input | Blocking Area | Freeze Impact | Temporary Handling |
|---|---|---|---|

## 5. 当前阶段结论

- 本轮有效硬约束数量：
- 仍需确认的关键约束：
- 可以进入设计基线阶段的前提是否满足：
