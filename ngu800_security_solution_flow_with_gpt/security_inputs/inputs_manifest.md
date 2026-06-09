# Inputs Manifest

Use this file as the active source inventory for NGU800 security design inputs.

Current status:
- Initial project references have been registered.
- When new files are added under `security_inputs/`, register them here first.
- Directory-level policy may be used when the same guidance applies to a whole folder. Add per-file overrides later if needed.

## 引用显示名约定

设计正文中引用输入资料时，应保留 `SRC-xxx` 编号用于追溯，同时追加文档名称用于阅读识别。推荐写法如下：

| Source ID | 正文推荐写法 | 对应文件 / 目录 |
|---|---|---|
| `SRC-001` | `SRC-001 历史当前安全方案基线` | `security_inputs/current_plan/安全方案.pdf` |
| `SRC-002` | `SRC-002 eHSM资料目录级策略` | `security_inputs/ip_manuals/ehsm/` |
| `SRC-003` | `SRC-003 启动方案` | `security_inputs/soc_arch/启动方案.pdf` |
| `SRC-004` | `SRC-004 安全子系统硬件方案` | `security_inputs/soc_arch/安全子系统硬件方案.pdf` |
| `SRC-005` | `SRC-005 管理子系统方案` | `security_inputs/board/管理子系统.pdf` |
| `SRC-006` | `SRC-006 eHSM Firmware TRM` | `security_inputs/ip_manuals/ehsm/OSR_eHSM_Firmware_TRM_1.0.pdf` |
| `SRC-007` | `SRC-007 eHSM Bootloader TRM` | `security_inputs/ip_manuals/ehsm/OSR_eHSM_Bootloader_TRM_1.0.pdf` |
| `SRC-008` | `SRC-008 当前收敛安全软件方案 2.0` | `security_inputs/current_plan/芯片安全软件方案_2.0.pdf` |

## Source Inventory

| Source ID | Path | Title / Topic | Owner | Version / Date | Confidence | Use Policy | Applies To | Must Follow | Optional Reference | Ignore / Out of Scope | Supersedes | Notes |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| SRC-001 | `security_inputs/current_plan/安全方案.pdf` | 历史当前安全方案基线 |  |  | `draft` | `historical-reference` | 历史方案、旧流程意图、章节组织参考 | 仅在 `SRC-008 当前收敛安全软件方案 2.0`、accepted CR、decision_log 或官方 eHSM/TRM 未覆盖时作为辅助参考 | 固件制作和验证的历史流程表达可作为 CR-0006 之前的意图参考 | 不再作为当前方案基线；与 `SRC-008` 冲突时不得优先采用 | `SRC-008` | 2026-06-03 后降级为历史参考，当前收敛方案以 `SRC-008` 为准 |
| SRC-008 | `security_inputs/current_plan/芯片安全软件方案_2.0.pdf` | 当前收敛安全软件方案 2.0 | 项目组 / security owner | 2.0 / 2026-06-03 | `confirmed` | `current-baseline` | 完整安全软件方案、FMC/GSP 命名、单 FMC 固定分区、OOB/QSPI 恢复、固件包制作与验证、密钥/证书/制造、生命周期/Debug、SPDM/attestation、密钥轮换、板级/OOB 边界 | 除 accepted CR、decision_log、官方 eHSM/TRM、后续用户特殊说明或源内明确例外外，当前方案口径以本文件为准 | 与旧 `SRC-001` 相同的章节组织、图形表达可继续参考，但必须按本文件的 2.0 收敛口径解释 | 不覆盖 eHSM TRM 已冻结的 physical header、OTP/control/key/counter 字段；不关闭仍在 open questions 中登记的 ABI/bit-level TBD | `SRC-001` | 用户于 2026-06-03 明确：`current_plan` 中更新的 `芯片安全软件方案_2.0.pdf` 为最新收敛版本，未特殊说明时均以该版为准 |
| SRC-002 | `security_inputs/ip_manuals/ehsm/` | eHSM资料目录级策略 |  |  | `confirmed` | `preferred` | eHSM集成、secure boot支撑能力、密钥体系、接口复用思路、efuse字段、生产阶段操作建议、固件镜像字段 | 采用该硬件子系统这一前提需要遵循；eHSM中已经明确定义的固件字段、OTP/eFuse排布、key slot语义、计数器和生产阶段操作优先按eHSM定义设计；当前按单控制器方案设计 | 最终方案尽量复用其设计思路和能力边界；算法框架需同时兼容国密和国际标准两套；若项目确有新增需求，应在eHSM基线上做兼容性扩展，而不是重定义已有字段 | 若与用户已明确冻结的系统架构口径冲突，不直接跟随，需在输出方案文档中保留差异记录 |  | 该条为目录级策略，适用于 `ip_manuals/ehsm/` 下资料，后续可增加单文件覆盖项；用户于 2026-04-22 明确要求方案同时支持国密和国际标准两套算法栈，并于 2026-04-23 进一步明确 eHSM 已定义技术细节应尽量优先沿用 |
| SRC-003 | `security_inputs/soc_arch/启动方案.pdf` | 启动方案 |  |  | `draft` | `preferred` | boot flow、secure boot流程、阶段职责划分、微核数量与职责 | 安全/非安全启动流程、各系统中的微核数量和职责当前以本文件为准；`sec1` 固件来源以启动方案描述为准，从 NOR Flash 加载 |  | 若后续发现明显错误或与更高优先级材料冲突，可调整，但在新的冻结决定出现前按本文件执行 |  | 作为当前启动流程主参考；用户于 2026-04-22 明确确认流程口径，并于 2026-04-23 进一步确认 `sec1` 从 NOR Flash 加载 |
| SRC-004 | `security_inputs/soc_arch/安全子系统硬件方案.pdf` | 安全子系统硬件方案 |  |  | `draft` | `partial` | 安全子系统架构、管理子系统架构、集成边界 | 重点参考客户需求和安全子系统架构方案、管理子系统架构方案 |  | 未被上述重点覆盖的部分暂不视为强约束 |  | 用于硬件集成和架构边界判断 |
| SRC-005 | `security_inputs/board/管理子系统.pdf` | 管理子系统方案 |  |  | `draft` | `preferred` | 管理子系统总体架构、模块划分、系统流程、管理面交互路径、与安全子系统的集成边界 | 文档中的总体架构和流程口径原则上需要遵循，特别是管理子系统模块职责、数据/控制流向、集成位置和系统级流程编排 | 当文档中的安全设计考虑不足、未覆盖关键攻击面、与既定安全基线冲突或明显存在安全漏洞时，不直接跟随；这类内容仅作为问题输入，必须在安全方案输出中明确指出差异、风险和替代设计 | 不将该文档中未经安全评审确认的弱化安全假设、越权访问路径、绕过鉴权的便捷流程直接继承到正式安全方案 |  | 该文档用于补充管理子系统视角下的总体架构与流程；对其中安全相关内容采用“遵循总体架构与流程、但不盲从安全细节”的策略，若发现考虑不够或明显不安全之处，应在约束、详设或风险章节中显式指出并给出安全侧裁决 |
| SRC-006 | `security_inputs/ip_manuals/ehsm/OSR_eHSM_Firmware_TRM_1.0.pdf` | eHSM Firmware TRM |  | 1.0 / 2026-04-16 | `confirmed` | `preferred` | eHSM Firmware 阶段 SOC secure boot image header、OTP control field、SOC/eHSM version counter、OTP key layout、SOC FW RAM/NVM deploy、`soc_verify` / `fw_upgrade` 命令 | Firmware TRM 已定义字段为实现级事实源；SOC secure boot image header、`SocBootAlg / SocUpgradeAlg`、SOC FW Version Counter、SOC key slots、RAM deploy/NVM only verify 条件必须沿用 | 可作为 SEC2/runtime verify、升级、制造命令映射的直接参考 | 不直接覆盖 BootROM 早期只能调用 Bootloader/ROM path 的事实；SEC1 early boot 仍需与 Bootloader TRM/BootROM 集成确认 | `SRC-002` 目录级策略的单文件细化 | CR-0004 已接受：NGU 不再并列定义 physical FW header / physical OTP layout / physical key slot；项目字段必须进入 logical alias、manifest extension 或 customization TBD |
| SRC-007 | `security_inputs/ip_manuals/ehsm/OSR_eHSM_Bootloader_TRM_1.0.pdf` | eHSM Bootloader TRM |  | 1.0 / 2026-04-16 | `confirmed` | `preferred` | eHSM Bootloader 阶段 eHSM/SOC secure boot image header、`bl_verify_image`、`bl_fw_upgrade`、早期 verify/decrypt path | Bootloader TRM 中已定义的 image header、Image_Type、verify/decrypt command fields 和 NVM only verify 限制是 SEC1 early boot 对齐依据 | 可作为 BootROM/eHSM Bootloader early SEC1 verify/decrypt path 参考 | 不直接替代 Firmware 阶段 `soc_verify` / runtime command；具体 BootROM 可调用路径仍需实现集成确认 | `SRC-002` 目录级策略的单文件细化 | CR-0004 已接受：SEC1 early boot 要区分 Bootloader `bl_verify_image` 与 Firmware `soc_verify`，不得混用命令语义 |

## Conflict Log

| Conflict ID | Topic | Preferred Source ID | Other Source ID | Reason | Impacted Chapters | Status |
|---|---|---|---|---|---|---|
| CF-001 | 安全核两级固件命名、位置与职责映射 | `SRC-003` | `SRC-001`, `SRC-004` | 用户已明确安全核两级固件统一命名为 `sec1` / `sec2`，并以启动方案作为流程与职责基线 | boot chain、阶段职责、镜像布局 | `resolved` |
| CF-002 | Host下发镜像的存放路径 | `SRC-003`, `SRC-004` | `SRC-001` | 进一步收敛后按启动方案口径：`sec1` 固件来自 NOR Flash，由 BootROM/eHSM 路径完成验证与装载；Host 仅下发 `sec2` 及其后续固件到管理子系统 IRAM | host boundary、镜像加载、内存布局 | `resolved` |
| CF-003 | FSP语义与Root of Trust口径 | `SRC-002`, `SRC-003` | `SRC-001` | 用户已明确 FSP 指 eHSM 内的核；系统最早执行入口与首个密码学验证根仍按分层口径描述 | trust boundary、模块命名、Root of Trust | `resolved` |
| CF-004 | eHSM native image header vs NGU 自定义 physical FW header | `SRC-006`, `SRC-007`, `CR-0004` | `SRC-001`, `efuse_key_fw_header_design.md` 草案 | eHSM TRM 已定义 1KB plaintext image head；NGU 不再定义并列 physical verification header，项目 metadata 改为 manifest extension / logical policy | boot chain、FW header、mailbox、tools | `resolved-by-CR-0004` |
| CF-005 | eHSM OTP/control/key/counter layout vs NGU OTP-0..OTP-7 / `*_MIN_VER` | `SRC-006`, `SRC-007`, `CR-0004` | `efuse_key_fw_header_design.md` 草案 | eHSM 已定义 control field、Version Counter 和 OTP key layout；NGU `OTP-0..OTP-7` 和 `*_MIN_VER` 降级为 logical view / rollback domain，不表达 physical OTP offset | efuse_otp、key_cert、manufacturing、traceability | `resolved-by-CR-0004 / partial-TBD` |
| CF-006 | SEC1/SEC2 sign+encrypt vs eHSM NVM only verify | `SRC-006`, `SRC-007`, `CR-0004` | 旧 `VERIFY_SEC1/VERIFY_IMAGE` 抽象口径 | eHSM TRM 明确 NVM deploy 只能 only verify 且镜像不能加密；SEC1/SEC2 sign+encrypt 必须走 verify+decrypt output path | boot、mailbox、firewall、manufacturing | `resolved-by-CR-0004` |
| CF-007 | eHSM `Image_Type` vs NGU `SEC1/SEC2/...` image type | `SRC-006`, `SRC-007`, `CR-0004` | `efuse_key_fw_header_design.md` 草案 | eHSM `Image_Type` 保持 TRM 定义；NGU 项目级 image type 放入 manifest / policy table，不直接写成 eHSM `Image_Type` 编码 | FW header、boot、attestation、mailbox | `resolved-by-CR-0004 / customization-TBD` |
| CF-008 | 当前方案源优先级：2.0 PDF vs 旧 current plan | `SRC-008` | `SRC-001` | 用户已明确 `security_inputs/current_plan/芯片安全软件方案_2.0.pdf` 为最新收敛版本；无特殊说明时方案以 2.0 为准，旧 `安全方案.pdf` 降级为历史流程参考 | constraints、baseline、boot、FW package、key/cert、attestation、OOB recovery、code rules、traceability、docs | `resolved-by-CR-0014` |

## Change Intake

| Change ID | Date | New / Updated Sources | Summary of Change | Expected Impact | Freeze Risk | Registration Notes |
|---|---|---|---|---|---|---|
| CHG-001 | 2026-04-22 | `SRC-001`, `SRC-002`, `SRC-003`, `SRC-004` | 首批方案基线、eHSM目录级策略、启动方案和安全子系统硬件方案已登记 | 将影响完整详设基线、secure boot、eHSM集成和子系统架构定义 | `unknown` | `SRC-002` 为目录级策略，可后续按单文件细化 |
| CHG-002 | 2026-04-22 | `SRC-002`, `SRC-003`, `SRC-004` | 用户补充冻结口径：启动流程和微核职责以 `SRC-003` 为准；方案必须同时支持国密和国际标准两套算法；安全核两级固件命名统一为 `sec1` / `sec2`；Host下发固件时 `sec1` 放在安全子系统 firewall 划分出的非安全区域，其他固件放在管理子系统 IRAM；FSP 指 eHSM 内部核；efuse 字段与生产阶段操作优先对齐 eHSM，按单控制器设计 | 将直接影响约束表、设计基线、镜像布局、OTP/eFuse规划与算法章节 | `medium` | 该变更已用于回写 `01_constraints.md`，后续文档需沿用同一口径 |
| CHG-003 | 2026-04-23 | `SRC-003` | 用户补充修正：`sec1` 固件来源应以启动方案描述为准，从 NOR Flash 加载；Host 不负责下发 `sec1`。同时要求详设增加术语表、图下文字说明以及 OTP 布局图表 | 将影响约束表、设计基线、启动章节、总体架构图、OTP 章节和最终交付风格 | `medium` | 本轮修正以 `SRC-003` 优先，覆盖之前关于 Host 下发 `sec1` 的表述 |
| CHG-004 | 2026-04-23 | `SRC-002` | 用户补充冻结口径：eHSM 中已明确的方案和技术细节应尽量以 eHSM 为准，尤其是固件字段、OTP/eFuse 排布、key slot 语义和生产阶段操作 | 将直接影响约束表、设计基线、镜像头字段、OTP/eFuse 布局、升级封装和量产流程章节 | `medium` | 本轮修正把 eHSM 从“优先参考”进一步上升为“已定义技术细节优先沿用”的集成规则 |
| CHG-005 | 2026-04-27 | `SRC-005` | 新增管理子系统文档，并明确使用口径：总体架构和流程原则上遵循；涉及安全且考虑不足、明显存在漏洞或与既定安全基线冲突的内容不直接继承，需在输出中显式指出问题并给出安全侧替代方案 | 将影响总体架构、板级安全、接口边界、Host/管理子系统交互、启动/装载流程及风险章节 | `medium` | 本轮变更用于把管理子系统资料纳入正式输入，并建立“系统流程可参考、安全细节需二次裁决”的输入策略 |
| CHG-006 | 2026-05-08 | `SRC-006`, `SRC-007`, `CR-0004` | 用户接受 CR-0004：eHSM native header 作为 SEC1/SEC2 密码学 verify/decrypt container；NGU metadata 进入 Code region manifest；OTP/key/counter 以 eHSM native mapping 为准，NGU 字段降级为 logical alias / customization TBD | 将影响 constraints、baseline、boot、key/cert、interface、manufacturing、implementation design、code rules、traceability、full design | `high` | 本轮进入 accepted-apply；manifest ABI、per-image rollback、wrapped CEK、exact key ID 和 exact OTP/control bit 仍保持 TBD |
| CHG-007 | 2026-05-08 | `CR-0005` | 用户裁决 `10_full_design.md` 作为完整详设与代码落地主入口；`04_impl_design` 内容必须全量合入主详设，不做省略性或总结性删减 | 将影响 full design、impl design 分片定位、README、skill、prompts、templates、code rules、traceability、change impact | `medium` | 本轮为文档事实源与流程变更，不改变安全架构裁决；`04_impl_design` 变为 editing shard / extracted implementation shard |
| CHG-008 | 2026-05-08 | `CR-0006`, `SRC-001` 第 6/7 章 | 用户要求安全启动详设补充固件包格式、平台侧制作流程、设备侧验签解密流程，并用图形和流程增强可落地性 | 将影响 constraints、baseline、boot detailed design、FW header/manifest implementation design、full design、code rules、traceability、change impact、templates | `medium` | 本轮保留 `SRC-001` 第 7 章的流程表达价值，但 physical container 仍 follow CR-0004 eHSM native header；旧 `header + Signed Region + signature + wrapped_cek + enc_payload` 只作为历史流程意图参考，不作为最终 wire/storage ABI |
| CHG-009 | 2026-05-09 | `CR-0006` refinement / user review | 用户指出 `10_full_design.md` 仍未清楚说明最终 SEC1 固件制品组成、签名位置、必须被签名覆盖的区域，以及制作端和设备侧解密验签过程的对应关系 | 将影响 full design、CR-0006 记录、code rules、traceability、decision log、changelog | `medium` | 本轮不新增 eHSM physical ABI；显式增加 `Part S: eHSM signature/authentication material`；冻结 NGU 侧最低保护契约：SEC1 完整 Code region 必须被认证覆盖，manifest 必须位于保护范围内，未认证 header 字段不得作为 NGU release 决策依据 |
| CHG-010 | 2026-06-03 | `SRC-008`, `CR-0014` | 用户新增当前收敛版 `芯片安全软件方案_2.0.pdf`，并明确无特殊说明时当前方案以该版为准 | 将影响 source precedence、constraints、baseline、boot recovery、FW package source references、code rules、traceability、change impact 和方案文档页首口径 | `medium` | 本轮为已收敛方案源升级和一致性补洞；不覆盖 eHSM TRM physical facts，不关闭 manifest ABI、exact key ID、OOB/QSPI ABI 等未冻结项 |

## Open Questions

| Q ID | Topic | Blocking Area | Needed From | Status |
|---|---|---|---|---|
| Q-001 | eHSM资料中若与既定架构明显冲突时的取舍 | eHSM集成、接口复用、boot链路 | 用户确认 | `closed` |
