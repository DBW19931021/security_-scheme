# NGU800 约束提取表

> 目的：在任何详细设计之前，先把输入材料中真正影响方案落地的硬约束抽出来。  
> 本文件是 `02_baseline.md` 和 `03_detailed_design.md` 的前置输入。  
> 模板来源：`.codex/skills/ngu800-security/templates/constraint_table_template.md`

## 1. 输入状态摘要

| 输入类别 | 当前状态 | 完整度 | 主要影响章节 | 备注 |
|---|---|---|---|---|
| 总体安全方案基线（`SRC-001`） | 已登记，已抽取关键章节 | 中 | Root of Trust、secure boot、职责划分、升级/调试/生命周期 | 作为当前总体方案基线；`host` 相关细节仅作参考，不能单独拿来冻结接口 |
| 启动流程方案（`SRC-003`） | 已登记，已抽取关键章节，并由用户追加冻结口径 | 中偏高 | 启动模式、生命周期、主从 Die 启动、微核数量与职责、板级启动参与者 | 安全/非安全启动流程以及各系统中的微核数量和职责当前以本文件为准 |
| 安全子系统硬件方案（`SRC-004`） | 已登记，已抽取关键章节 | 中偏低 | 安全子系统边界、firewall/userid、复位释放、镜像分发路径 | 提供硬件集成边界，但存在待讨论项，不能把所有实现细节视为已冻结 |
| eHSM 目录级资料（`SRC-002`） | 已按目录级策略抽取产品介绍、TRM、Host API、Bootloader/FW 文档，并由用户追加 efuse/算法口径 | 高 | Mailbox、OTP/eFuse、生命周期、安全启动、升级、调试鉴权、计数器、算法栈 | efuse 字段当前以 eHSM 为准并按单控制器设计；生产阶段相关操作优先对齐 eHSM 推荐；方案必须同时支持国密和国际标准两套算法 |
| Host / BMC / OOB 专项资料 | 未单独提供 | 低 | host boundary、SPDM/PLDM、板级安全、板级升级 | 目前仅能从 `SRC-001`、`SRC-003` 间接推断，无法冻结板级安全口径 |
| 制造 / 灌装 / RMA 冻结资料 | 未单独提供 | 低 | provisioning、证书下发、量产密钥烧录、RMA 清除与恢复 | 生产阶段相关操作先与 eHSM 推荐对齐；其余缺失流程允许按常规工程做法补充，但未知部分不强行写死 |

## 2. 约束表

| Constraint ID | Category | Statement | Source | Strength | Impact | Status | Conflict With | Resolution |
|---|---|---|---|---|---|---|---|---|
| C-BOOT-000 | boot_chain | 安全/非安全启动流程，以及各系统中的微核数量和职责，当前以 `SRC-003`《启动方案》为主基线；其它来源若与其不一致，优先按该文档和用户冻结口径收敛。 | `SRC-003`，`CHG-002` | HARD | ch4/ch5/ch6/ch7 | [CONFIRMED] |  | 后续基线和详设不得再自行改写启动阶段划分。 |
| C-BOOT-001 | boot_chain | SoC BootROM 负责上电最小初始化、启动分流、拉起 eHSM 并编排首阶段加载，但不直接承担 SEC 首阶段镜像的密码学验签或解密。 | `SRC-001` §1.3、§3.4、§4.1 | HARD | ch4/ch6 | [CONFIRMED] | `CF-003` | 保持“最早执行入口”和“首个密码学验证根”两层口径分离。 |
| C-BOOT-002 | boot_chain | 在安全启动路径中，eHSM 是系统首个可用的密码学验证根；BootROM 只有在收到 eHSM 验证通过结果后，才能装载并启动首阶段 SoC 可变固件。 | `SRC-001` §3.2-§3.4；`SRC-002` 产品介绍 §2、§5 | HARD | ch4/ch6/ch7 | [CONFIRMED] | `CF-003` | 后续基线文档统一写成“BootROM = orchestration，eHSM = first cryptographic root”。 |
| C-BOOT-003 | boot_chain | 系统必须同时支持安全启动和非安全启动；非安全启动路径应允许系统在不依赖 eHSM 密码学校验的情况下完成基本 bring-up。 | `SRC-001` §1.3、§5.1；`SRC-003` §4.1、§4.2.3-§4.2.5 | HARD | ch4/ch5/ch6 | [CONFIRMED] |  | 作为启动模式硬约束保留，不在本阶段删除非安全路径。 |
| C-BOOT-004 | boot_chain | 安全核两级固件命名统一为 `sec1` 和 `sec2`；其中 `sec1` 固件来自 NOR Flash，由 BootROM/eHSM 路径完成验证与装载，并负责最小平台初始化、PCIe 通道建立以及后续固件下载准备，`sec2` 作为完整安全控制固件承接运行期安全控制面职责。 | `SRC-003` §5.1.1，`SRC-001` §4.2-§4.3，`CHG-002`，`CHG-003` | HARD | ch4/ch5/ch6/ch7 | [CONFIRMED] |  | 原先关于 `SEC1 / GSP FMC / C908` 命名不一致的问题按 `sec1` / `sec2` 口径收敛，并进一步明确 `sec1` 来源为 NOR Flash。 |
| C-BOOT-005 | boot_chain | Host/UCIe 提供的后续微核固件在通过安全子系统或 eHSM 验签/解密并被显式放行前不得执行。 | `SRC-001` §4.3、§4.6；`SRC-003` §5.1.1；`SRC-004` §2.1.1 | HARD | ch5/ch6/ch9 | [CONFIRMED] |  | 后续设计中统一采用“verify before release”原则。 |
| C-INTF-001 | interface | SEC 侧涉及签名验签、HASH、加解密、随机数、OTP、生命周期和调试鉴权等安全服务时，应通过 SEC/C908 到 eHSM 的 Mailbox 服务边界调用；eHSM 不应对任意 Core/Master 直接开放服务接口。 | `SRC-001` §1.3、§2.2；`SRC-002` 产品介绍 §2、TRM §12；`SRC-004` §2.1.1 | HARD | ch6/ch8/ch10 | [CONFIRMED] |  | Mailbox 作为主服务边界冻结，后续只细化命令集和共享内存协议。 |
| C-TRUST-001 | trust_boundary | eHSM 专用存储区构成封闭安全边界，SoC CPU 不可直接访问；SoC 与 eHSM 的交互模式应为 Mailbox 命令加共享内存/受控缓冲区数据交换。 | `SRC-002` 产品介绍 §2.1、§2.2；`SRC-004` §2.1.1 | HARD | ch3/ch6/ch8 | [CONFIRMED] |  | 后续详细设计中不得把 eHSM 私有内存暴露为通用 RAM。 |
| C-TRUST-002 | trust_boundary | 安全子系统必须通过 firewall、userid 和主从 Die 区分来控制管理子系统 RAM/Slave 访问权限；所有镜像都应先经安全子系统验证后再分发。 | `SRC-004` §2.1.1-§2.1.3 | HARD | ch3/ch5/ch8/ch10 | [CONFIRMED] |  | 先冻结隔离原则，具体 region/bit 映射等待硬件定版。 |
| C-HOST-001 | host_boundary | Host 仅负责下发 `sec2` 及其他后续固件到管理子系统 IRAM；`sec1` 不由 Host 下发，而是来自 NOR Flash。无论物理落点如何，最终执行放行权都不属于 Host。 | `SRC-003` §5.1.1，`SRC-004` §2.1.1，`CHG-003` | HARD | ch5/ch6/ch8/ch9/ch13 | [CONFIRMED] |  | Host 的角色冻结为“投递者”，但不参与 `sec1` 供应路径，更不是信任裁决者。 |
| C-KEY-001 | key_cert | OTP/eFuse 至少需要承载生命周期、UID、控制字段、版本计数器以及安全启动/升级所需的 Verify/Encrypt Key 域；当前字段规划以 eHSM 定义为基线，按单控制器方案设计，确有项目需要时再在此基础上扩充。 | `SRC-002` eHSM TRM §3-§5；Bootloader TRM §3、A.2.2、A.2.3；Firmware TRM §3、§6.4；`CHG-002`，`CHG-004` | HARD | ch7/ch8/ch9/ch11 | [CONFIRMED] |  | 当前冻结字段族和来源归属；扩充只能作为增量扩展，不能推翻 eHSM 基线。 |
| C-UPDATE-001 | update_rollback | 防回滚不能依赖 Host 自证，必须由 eHSM/OTP 的版本计数器或单调计数器参与裁决；镜像版本必须与 OTP/eHSM 计数状态比较后才能放行。 | `SRC-001` §1.1、§4.5；Bootloader TRM §6、§7；Firmware TRM §9、§11、A.10 | HARD | ch9/ch11 | [CONFIRMED] |  | 后续基线中将 anti-rollback 定义为 Root of Trust 裁决项。 |
| C-LC-001 | lifecycle_debug | 方案必须区分至少 TEST/DEVE、MANU、USER、DEBUG/RMA、DEST 类生命周期，并在 USER/量产态强制安全启动、关闭或严格授权调试路径。 | `SRC-003` §3.2；`SRC-002` 产品介绍 §8、§9；Host API §2.2.7.5-§2.2.7.7 | HARD | ch10/ch11/ch12 | [CONFIRMED] |  | 生命周期名称可后续统一，但能力开关矩阵不能缺失。 |
| C-DEBUG-001 | lifecycle_debug | 调试访问必须采用 challenge-response 鉴权并受生命周期约束；TEST/DEVELOP 可保留 OTP/调试便利性，但量产态不能保留无限制 JTAG/内部总线访问。 | `SRC-003` §3.2；Bootloader TRM §5、A.2.10、A.2.11；Firmware TRM A.7.3-A.7.5 | HARD | ch10/ch12 | [CONFIRMED] |  | 后续详设中需要把“谁签 challenge、谁审批授权、授权有效期”补齐。 |
| C-UPDATE-002 | update_rollback | 在不与既定架构冲突的前提下，后续镜像头、升级封装、Verify/Encrypt Key 域划分应尽量复用 eHSM Bootloader/FW 已定义的安全启动和升级格式。 | `SRC-002` 目录级策略；Bootloader TRM §6-§8；Firmware TRM §8-§9；`CHG-004` | SOFT | ch7/ch9/ch11 | [CONFIRMED] |  | 作为优先复用约束保留；若与项目冻结架构冲突，再在冲突记录中升级处理。 |
| C-EHSM-001 | integration_rule | eHSM 资料中凡已明确给出的方案和技术细节，优先按 eHSM 原定义集成，尤其包括固件镜像字段、OTP/eFuse 排布、key slot 语义、版本计数器和生产阶段操作；项目侧只在 eHSM 未覆盖处做兼容性增量扩展。 | `SRC-002` 目录级策略；Bootloader TRM；Firmware TRM；`CHG-004` | HARD | ch5/ch7/ch9/ch11/ch12 | [CONFIRMED] |  | 该约束用于约束详设写法，避免在 eHSM 已定义处再起一套平行格式。 |
| C-ALG-001 | key_cert | 方案必须同时支持国密和国际标准两套算法栈进行设计；后续章节需要先给出算法无关框架，再分别给出国密映射和国际算法映射，不能只实现单套算法口径。 | `SRC-002` 产品介绍 §2；Host API §2.2；Firmware TRM §5、附录 C；`CHG-002` | HARD | ch7/ch9/ch12/ch13 | [CONFIRMED] |  | 双算法栈从参考要求提升为冻结约束。 |
| C-TRUST-003 | trust_boundary | 文档中的 FSP 明确指 eHSM 内部核，不再与 SoC 安全管理固件混用；后续写 Root of Trust、Boot 责任和服务边界时必须区分“eHSM 内核/FSP”和“sec1/sec2”。 | `SRC-002`，`SRC-003`，`CHG-002` | HARD | ch3/ch4/ch6 | [CONFIRMED] | `CF-003` | 用统一术语消除 FSP 与 sec1/sec2 的角色混淆。 |

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
| CF-001 | 首个 SoC 可变固件的命名、位置与 PCIe 初始化 owner 不一致 | `SRC-001`：SEC1 位于 Flash，负责基础初始化与 PCIe 初始化 | `SRC-003`/`SRC-004`：GSP FMC 或安全子系统 C908 先执行并完成 PCIe 初始化/复位释放 | 按用户冻结口径统一为 `sec1` / `sec2`；`sec1` 承担最小 bring-up 和 Host 下发准备职责，后续文档沿此命名与职责展开 | ch4/ch5/ch6/ch7 | `resolved` |
| CF-002 | Host 下发镜像的物理 ingress 路径未冻结 | `SRC-004` 方案 A：Host 直接写安全子系统 share memory | `SRC-004` 方案 B：Host 写管理子系统 IRAM，再由 C908 处理；`SRC-001` 只给出了 Host 下发模型，未定具体落点 | 按最新收敛口径：`sec1` 来自 NOR Flash，由 BootROM/eHSM 路径完成验证与装载；Host 仅下发 `sec2` 和其他后续固件到管理子系统 IRAM | ch5/ch6/ch9/ch13 | `resolved` |
| CF-003 | Root of Trust 口径与模块命名存在两套表述 | `SRC-003`：FSP 被描述为整个系统安全信任根，GSP BootROM/FMC 负责主链路 | `SRC-001`/`SRC-002`：SoC BootROM 是最早执行代码，eHSM 是首个密码学验证根 | FSP 统一解释为 eHSM 内部核；系统级口径仍保留“BootROM 是最早执行入口，eHSM 是首个密码学验证根”的分层表述 | ch3/ch4/ch6 | `resolved` |

## 4. 缺失输入对冻结的影响

| Missing Input | Blocking Area | Freeze Impact | Temporary Handling |
|---|---|---|---|
| Host / SPDM / PLDM / DOE / PCIe 安全接口需求 | host boundary、attestation、board update | 高：无法冻结 Host 包格式、证书链字段、板级测量与报告边界 | 当前把 Host 仅定义为投递者/辅助者，不冻结协议字段 |
| eFuse 细化位分配和扩展字段表 | key hierarchy、lifecycle、anti-rollback、debug control | 中：当前能冻结字段族和单控制器方向，但还不能冻结所有 bit offset 和扩展位用途 | 先按 eHSM 字段体系设计，新增需求以增量扩展方式补充 |
| BMC / OOB MCU / SMBus / Board Flash 的专项安全材料 | board_security、board update path | 中：无法做到完全贴板级真实实现的边界定义 | 板级边界先按业内通用方式补充；没有证据的内容不硬写 |
| 制造、灌装、证书下发、RMA 恢复流程 | manufacturing、provisioning、device identity | 中：无法冻结根密钥烧录 owner、证书 owner、返修审批链 | 生产阶段操作先对齐 eHSM 推荐，其余流程按通用工程实践补充，未知处保留空缺 |
| 首阶段与后续镜像头格式 / 存储映射的项目正式规范 | secure boot、upgrade、镜像布局 | 中：镜像实际落盘格式仍需项目最终定版，但 eHSM 已定义字段和安全启动/升级格式应优先沿用 | 暂按 eHSM 安全启动/升级格式做主基线；仅对 eHSM 未覆盖项保留扩展 |
| 双 Die 安全启动 / UCIe 信任链和度量链资料 | dual-die boot、measurement、attestation | 中：无法冻结从 Die 的认证、回滚和异常恢复策略 | 当前仅冻结“从 Die 不能绕过主安全控制面拿到执行放行权” |
| 调试鉴权审批流程与生命周期切换责任人 | lifecycle_debug、RMA | 中：无法冻结 challenge 签发者、授权时效和审计要求 | 保留 challenge-response 和生命周期门控，审批链路标 `[TBD]` |

## 5. 当前阶段结论

- 本轮有效硬约束数量：`14` 条 `HARD`，`1` 条 `SOFT`
- 仍需确认的关键约束：Host/SPDM/PLDM 外部接口字段、eFuse 细化位分配、板级真实实现边界、制造/RMA 审批与 owner；但 `sec1` 固件来源已进一步明确为 NOR Flash
- 可以进入设计基线阶段的前提是否满足：`满足，可进入 02_baseline.md；但外部接口和板级章节仍非完全 freeze-ready`
- 进入 `02_baseline.md` 时 `CF-001`、`CF-002`、`CF-003` 应视为已决议口径，不再按开放冲突处理；缺失输入对应章节继续保留必要的 `[TBD]`
