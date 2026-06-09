# CR-0013 OOB MCU 纳入安全边界与取消 FMC A/B 备份

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0013` |
| Title | OOB MCU 纳入安全边界与取消 FMC A/B 备份 |
| Status | `applied` |
| Owner | security owner / 项目组 |
| Reviewer | GPT / Codex follow-up review |
| Created Date | 2026-05-27 |
| Source / Context Pack | 用户 review：OOB MCU 纳入安全边界并可烧写 NOR Flash；满足该能力后取消 FMC A/B 备份，以 OOB MCU 受控重刷实现防变砖 |
| Related Decision ID | Supersedes CR-0008 中的 FMC A/B 防变砖口径；保留 CR-0008 中“无静态 recovery 后门、启动仍走 eHSM 验证”的安全原则 |

## 2. 背景

此前基线将 FMC 防变砖机制设计为 Flash 中 `FMC_A / FMC_B` 双分区与 `fmc_slot_metadata` 状态机。新方案明确将 OOB MCU / 板级安全 MCU 纳入安全边界，并允许其通过 QSPI 或等价受控通道对 NOR Flash 中的 FMC 区域执行烧写。这样可以在 SoC 侧 FMC 损坏、升级失败或不可启动时，由 OOB MCU 从板级带外路径重新烧写 FMC，而不再要求 SoC Flash 内部保留 A/B 双分区。

该裁决降低 BootROM、Flash layout、metadata ABI 和 boot confirm 状态机复杂度，但提高了 OOB MCU、NOR Flash 访问控制、烧写授权、审计和板级安全边界的重要性。

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| OOB MCU 安全边界 | OOB MCU / 板级安全 MCU 纳入板级安全边界，可作为受控带外执行体通过 QSPI 或等价通道烧写 NOR Flash 中的 FMC 区域 | 支撑 FMC 损坏后的带外恢复和量产/售后刷写 | `[CONFIRMED]` |
| OOB MCU 信任级别 | OOB MCU 不进入 eHSM Root of Trust，不持有 eHSM 根材料，不替代 BootROM/eHSM 验证结论 | 避免把板级 MCU 扩展为新的 SoC RoT 或后门 | `[CONFIRMED]` |
| FMC 分区 | 取消 FMC_A/FMC_B 双分区与 fallback slot；Flash 中只要求一个主 FMC 区域和必要的受保护状态/审计区 | OOB MCU 受控重刷已提供防变砖路径，BootROM 不再需要 slot 选择和 fallback 状态机 | `[CONFIRMED]` |
| 启动验证 | OOB MCU 烧写后的 FMC 仍必须由 BootROM 调 eHSM 执行 verify/decrypt、rollback、revoke 和 manifest policy 检查 | OOB MCU 只能写入存储介质，不能决定执行放行 | `[CONFIRMED]` |
| Key rotation | key slot / key epoch 只表达客户密钥自主控制、轮换、撤销和过渡期兼容，不再绑定 FMC_A/B 或 fallback slot | 取消 FMC A/B 后，key rotation 的安全性依赖 update authorization、rollback/revoke 策略和 OOB 可重刷恢复路径 | `[CONFIRMED]` |
| 防变砖 | 首版防变砖机制改为 OOB MCU 受控重刷 NOR Flash 中 FMC 主区域；不引入长期静态 recovery FMC，不保留 SoC 厂商单方 recovery 后门 | 同时支持恢复能力与客户密钥控制 | `[CONFIRMED]` |

## 4. 受影响文件

| 文件 | 是否必须修改 | 影响说明 |
|---|---|---|
| `security_workflow/01_constraints.md` | Yes | 将静态 recovery image 待冻结项调整为首版不采用；新增 OOB MCU 受控重刷 FMC 主区域约束 |
| `security_workflow/02_baseline.md` | Yes | 基线摘要、Board/OOB 边界和冻结项同步单 FMC + OOB 重刷口径 |
| `security_workflow/03_detailed_design/10_full_design.md` | Yes | 替换 3.14、7.x、8.13、9.15、10.3、开放项中的 FMC A/B 与 fallback 口径 |
| `00_project/decision_log.md` | Yes | 记录 DEC-0018，建立新基线可追溯裁决 |
| `00_project/changelog.md` | Yes | 记录 CHG-0010，说明本次 CR 已应用 |
| `docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md` | Yes | 同步项目组汇报版中的恢复、key rotation、硬件需求和冻结项 |
| `docs/NGU800_安全软件方案_精简版.md` | Yes | 精简版取消 FMC A/B，改为 OOB MCU 受控刷写单 FMC |
| `docs/NGU800_安全软件方案_精简版_含架构图流程图.md` | Yes | 图形和流程图同步取消 A/B 与 fallback |
| `docs/NGU800_安全软件方案_精简版_含OOB_MCU安全边界.md` | Yes | 已有 OOB MCU 版本从 inactive slot 刷写改为单 FMC 区域受控重刷 |

## 5. 每个文件修改要求

| 文件 | 修改要求 | 不允许改变的内容 |
|---|---|---|
| 所有目标文档 | 将 `FMC_A/FMC_B`、`fmc_slot_metadata`、`active/fallback/inactive slot`、`boot confirm` 等 A/B 备份机制替换为单 FMC 区域 + OOB MCU 受控重刷 | 不改变 eHSM 是唯一 Root of Trust；不降低 FMC 必须签名 + 加密 + rollback + revoke 的要求 |
| 所有目标文档 | 明确 OOB MCU 可烧写 NOR Flash，但 OOB MCU 不是启动裁决方 | 不允许写成 OOB MCU 可绕过 BootROM/eHSM 放行 FMC |
| key rotation 章节 | 去除 key rotation 与 FMC A/B fallback 的绑定，改为 update authorization + key epoch + OOB 可恢复路径 | 不取消客户 key slot / key epoch / revoke 能力 |
| 硬件需求章节 | 将 Flash 分区需求从 FMC_A/FMC_B 改为单 FMC boot-critical 区、OOB QSPI ownership/arbiter、写保护、掉电保护和审计 | 不删除 Host/DMA 不可信边界 |

## 6. 需要替换/删除的旧口径

| 旧口径 | 所在文件 | 替换为 | 原因 |
|---|---|---|---|
| `FMC_A/FMC_B` 双分区 | 详设、汇报版、精简版 | 单 FMC 主区域 + OOB MCU 受控重刷 | 新基线取消 A/B |
| `fmc_slot_metadata` | 详设、汇报版、精简版 | FMC update/recovery status、audit record 或受保护写入状态 | 不再需要 active/fallback slot metadata |
| `inactive slot / fallback slot / boot confirm` | 详设、汇报版、精简版 | OOB MCU 烧写确认、BootROM/eHSM 下次启动验证、失败则继续带外重刷 | 恢复路径由板级带外重刷承担 |
| key rotation 必须绑定 FMC A/B | 详设、汇报版、精简版 | key rotation 绑定 update authorization、key epoch、rollback/revoke 与 OOB 可恢复路径 | A/B 已取消 |

## 7. 不允许 Codex 自行改变的内容

- 不改变 BootROM -> eHSM verify/decrypt -> FMC 的安全启动主链路。
- 不把 OOB MCU 提升为 eHSM Root of Trust 或第一密码学验证者。
- 不新增静态 recovery FMC 或 SoC 厂商单方 recovery signer。
- 不取消 FMC/GSP 强制签名 + 加密策略。
- 不取消客户 key slot / key epoch / revoke 能力。

## 8. 验收标准

- [x] 目标文档明确 OOB MCU / 板级安全 MCU 纳入安全边界，并允许受控烧写 NOR Flash。
- [x] 目标文档不再把 FMC A/B 双分区作为首版防变砖机制。
- [x] BootROM/eHSM 仍是 FMC 启动验证和执行放行裁决方。
- [x] key rotation 不再依赖 FMC fallback slot。
- [x] 硬件需求包含 OOB MCU secure boot、QSPI ownership/arbiter、NOR boot-critical 写保护、授权刷写、掉电保护和审计。
- [x] Mermaid 图可渲染。
- [x] `git diff --check` 通过。

## 9. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-05-27 |
| 修改文件 | `security_workflow/03_detailed_design/10_full_design.md`；`docs/NGU800_安全方案详细设计_项目组汇报版_去重复保留细节_含SoC硬件需求.md`；三份安全软件精简版 |
| 未完成项 | OOB MCU 自身 secure boot 细节、QSPI ownership/arbiter 寄存器、NOR 写保护粒度、刷写授权 token ABI、掉电恢复策略需硬件/板级 owner 冻结 |
| 执行说明 | 将 FMC 防变砖从 A/B fallback 改为 OOB MCU 受控重刷 NOR Flash 单 FMC 区域；保留 BootROM/eHSM verify/decrypt/revoke/rollback 裁决；key rotation 脱离 FMC A/B 状态机。 |
