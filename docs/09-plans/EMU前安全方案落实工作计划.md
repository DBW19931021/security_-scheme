---
title: "EMU 前安全方案落实工作计划"
status: active
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0020
owners: []
last_reviewed: 2026-07-22
supersedes: []
superseded_by: []
---

# 目标

本文件是两个顶层任务共用的EMU前阶段节奏和门禁记录，不是额外顶层任务。任务边界、优先顺序和交付物以[NGU800P两项任务总计划](NGU800P两项任务总计划.md)为准。

以约 4 周后 EMU（C908 + eHSM）可用为假设，在环境到位前完成两件事：

1. 由 GSP 在 `security_-scheme/docs/03-architecture/`、`docs/04-interfaces/` 和 `docs/05-software-design/` 完成基于 SRC-0016/SRC-0017 的正式详细设计，覆盖全部 20 个 Feature，详细到开发者可直接据此编码和联调。
2. 在`security_-scheme/tests/cases/`和`docs/06-verification/`将安全测试从“100条初版用例”推进到“版本、载体、自动化、输入、oracle、Evidence路径均已规划”；独立baremetal软件栈在`../baremetal/components/ngu_security/`准备运行于安全核的eHSM全能力可执行测试、EMU runner和工具，使EMU到位后可按runbook执行。

本计划不承诺在 EMU 到位前证明硬件功能通过。EMU 前的完成标准是：正式详设通过评审、方案细节冻结、生产接口可接、非硬件测试尽量前移、EMU 执行包可直接使用；硬件事实仍需 EMU/RTL/样片 Evidence。`security_-scheme` 承载唯一的正式详设、全部项目任务、测试规划/工作簿、开发追溯和 Evidence 索引，但不复制正式软件代码或可执行测试代码。

具体开发波次、候选任务、代码落点、Definition of Ready/Done 和阻塞关系见 [NGU800P安全软件开发计划](NGU800P安全软件开发计划.md)。本文件继续管理 EMU 前总节奏，新开发计划管理实际实施顺序；两者不在代码仓复制。

# 当前起点

## 已有资产

- SRC-0017：系统/架构上位方案；SRC-0016：软件工程落地下位方案。
- SRC-0001～SRC-0015：Vendor eHSM/Core 文档；SRC-0018：4019 代码/测试/工具快照。
- `../gsp-pmp-rmp-omp/components/security` 已有 image/header/manifest、policy、measurement、BootROM demo、MCTP/SPDM、Host/QEMU 测试框架。
- `../baremetal/components/ngu_security`已有早期image/manifest/policy/measurement、Host测试和制包工具骨架；后续eHSM全能力可执行测试、EMU runner和工具由独立baremetal软件栈在该仓库收敛并运行于安全核，不经过GSP固件；项目测试规划和工作簿不放在该仓库。
- `tests/cases/NGU800P-security-test-cases-v0.2.xlsx` 已有 100 条测试用例。

## 不能当作已完成的部分

- `verify_image()` 当前仍经 eHSM stub；BootROM 路径是 demo。
- Attestation/SPDM 的 crypto、cert、signing 和部分 transport 仍为 stub/模拟。
- 尚未观察到 key rotation、OTP key install、LCS、debug auth、正常更新、OOB、多 Die/Firewall 的完整生产实现映射。
- v0.2 的 100 条用例全部是 P0；74 条缺测试载体，99 条缺 RTL 版本，100 条缺软件版本，75 条缺自动化状态。
- OPEN-CONFLICT-001～005/007～009均已关闭：Native Header offset1008、Measurement、C908、typed-stage loader、linker和eHSM共享descriptor统一使用64位baremetal System Address，不携带domain且不执行Local/System转换；`entry_addr=load_addr`，产品统一使用Header `rollback_counter[16]`。BootROM以`check_version=0`暂存FMC candidate，FMC初始化调用eHSM BL staged-candidate exact-match commit/readback API，proof后才接收同值GSP。准确command/packing/LCS/status/readback/交付版本属于实现DoR；ADR-0022已关闭Measurement逻辑ABI，OPEN-DESIGN-021待最大实例容量。ADR-0016已关闭GSP/OMP产品关系；OPEN-CONFLICT-006只剩`NON_CACHEABLE/HARDWARE_COHERENT`唯一属性、精确容量和Firewall参数阻断最终linker。Mailbox follow Vendor及security只上报/RAS决定reset已冻结，不受冲突影响的设计继续推进。
- 现有 test/stub/demo 和历史 Expected 只说明当前代码事实与差距，不是目标流程或最终 oracle；EMU/产品实现不得以任何 stub、模拟成功或有最终落地风险的编码方式补齐能力。

# EMU 前可以完成的工作

## 1. 正式详细设计闭环

- 在 `security_-scheme/docs/05-software-design/NGU800P安全软件详细设计.md` 建立主入口，并在 `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 建立专题详设；把每个 Feature 的角色、模块/文件、API、数据结构、状态机、权限、错误、并发/内存、reset/timeout、掉电、回退/不回退、审计、清零和测试钩子写清。
- 将 SRC-0016/SRC-0017、Vendor 文档/代码和公司实现逐项对照；有明显冲突时按 ADR-0002 升级，不用“软件方案为准”掩盖差距。
- 把批准结论进入 SRC-0016 后续版本或受控 amendment；再形成 requirements、OpenSpec 和测试 oracle。
- 详设逐项给出 target file/symbol 和 requirement-to-code/test map；开发者不应再依赖聊天记录、Vendor 示例或临时 review 猜测实现。

## 2. 软件和工具前置

- 复用现有安全组件和 OSR Host 通用业务代码，冻结 eHSM、OTP/eFuse、crypto/cert、Flash、measurement/SPDM 的 adapter/provider ABI；OSR 平台地址/timer/reset 由 NGU800P port 重做或核实。
- 硬件依赖收敛在明确的 NGU800P port/provider；Host 侧契约测试只能验证布局、状态机、错误映射和调用顺序，不能以 stub/simulated success 代替真实 eHSM 验收或进入 EMU/产品构建。
- 正式详设和全部任务/追溯进入 `security_-scheme`，正式实现进入 `gsp-pmp-rmp-omp`；测试资产清单、策略和版本化工作簿进入 `security_-scheme`，合法/非法镜像 corpus、标准密码向量、OTP/LCS 测试配置集、debug 测试 token、测试证书链、可执行故障注入、runner 和日志解析器进入 `baremetal` 的安全组件 tests/tools 范围。
- 在 Host/QEMU 能覆盖的范围完成单元、契约、协议和状态机测试；不等待 EMU 才发现纯软件问题。

## 3. 测试与 Evidence 前置

- 对 100 条用例重新分层和排期，明确哪些可在 Host/QEMU/Vendor 仿真执行，哪些必须等 EMU。
- 为 EMU 测试准备镜像、向量、脚本骨架、命令、预期输出、失败码和证据目录。
- 建立 environment manifest、版本记录、日志哈希和 requirement->case->evidence 追踪模板。

# 方案“细节已落实”的统一判定

每个 Feature 必须回答以下问题；任何一项缺失都不能标为“细节闭环”：

1. 谁发起、谁执行、谁授权、谁持久化、谁审计？
2. 输入/输出 ABI、字节序、尺寸、地址、版本和兼容策略是什么？
3. 正常状态机和所有终态是什么？
4. LCS、Debug、Role、Key Usage 和接口权限矩阵是什么？
5. timeout、busy、retry、reset、掉电、重复请求和乱序如何处理？
6. 失败是否 fail-close，是否允许 recovery/重试，明确禁止哪些 fallback？
7. Key/OTP/eFuse/Flash/counter 的原子性、寿命、耗尽和不可逆点是什么？
8. 错误码、中断、日志、审计字段和敏感数据脱敏规则是什么？
9. 明文、key、token、nonce、临时 buffer 在成功/失败/复位后的清零时点是什么？
10. 如何通过 Host/unit/QEMU 预验证，如何通过 EMU 最终验证？
11. oracle 来自哪个 Source/章节/accepted decision，是否存在冲突？
12. 实现、测试和 Evidence 的 Owner、版本、入口和完成门禁是什么？

# 四周排程

以下用 `T0` 表示 EMU 首个可用日；如日期变动，按相对周整体平移。

## 执行方式：按Feature滚动闭环

T-4和T-3不是必须串行完成的两个阶段。当前已具备Feature矩阵、资料基线、代码盘点和冲突清单，因此立即启动正式详设；Owner、算法profile和EMU能力等剩余准备项与详设并行补齐。

每个Feature独立执行“资料/代码映射 → 设计草案 → 冲突/决策 → 评审 → Requirement/测试oracle → 编码准入”小循环。不受局部冲突影响的设计继续推进；受影响字段保持 `CONFLICTING`。满足详设门禁的Feature可以先进入后续编码任务，不必等待20个Feature全部完成。

## T-4 周：启动详设并并行关闭前置条件

目标：用最短时间确认“要做什么、依据是什么、哪些问题会阻塞设计”，并在同一周开始首批P0 Feature详设，不把准备工作做成整周等待门槛。

- 逐项评审 20 个 Feature，分配方案 Owner、软件 Owner、RTL/eHSM Owner、测试 Owner。
- 在 `security_-scheme/docs/05-software-design/NGU800P安全软件详细设计.md` 建立主入口，在 `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 建立专题章节和 requirement-to-code-and-test map；每章指定 GSP Owner 和评审人。
- 形成 feature->方案章节->代码模块->测试用例的首版映射。
- 按ADR-0003/0004把三项已关闭冲突转化为详设和v0.3用例oracle；当前不采用Handoff，不建立相应ABI或测试范围。
- 确认 SRC-0016/SRC-0017 的 Owner、适用 D0 修订和当前批准状态。
- 已由ADR-0017确认Secure Package只支持三套Profile：SHA-256+RSA-2048-PSS+AES-128-CBC、SHA-256+ECDSA-P256+AES-128-CBC、SM3+SM2+SM4-CBC；MD5、SHA-512/256、AES-192/256、XTS、DES/TDES及其他Vendor组合只作baremetal能力/回归输入，产品release拒绝。后续只需冻结设备/镜像/key/board/LCS绑定。
- 收集 EMU 能力清单：加载方式、串口/JTAG、日志、reset/power-cut、OTP/eFuse 模拟、Flash、mailbox、中断、UCIe/Firewall、可观测信号。
- 同步启动 DD-01 系统边界/安全启动链和 DD-02 eHSM adapter 的只读映射与设计草案。

周出口：20 个 Feature 均有 gap 和临时责任路径；详设主入口、接口清单和评审日程建立；首批P0章节已形成可评审草案；所有阻断项有裁决人或升级路径。Owner尚未最终确认的项可继续设计，但不能通过编码准入。

## T-3 周：关闭首批详设并扩展到全部Feature

目标：让实现和测试不再依赖口头解释。

- 冻结启动链、eHSM adapter、Native Header Overlay/typed-stage package、anti-rollback 和更新/恢复状态机。
- 冻结 LCS/Debug/OTP/Key/Cert/Rotation 的对象、权限、slot/bitmap、鉴权和异常处理。
- 冻结 measurement table/SPDM profile、MCTP/mailbox 参数和真实 provider 接口。
- 冻结 Multi-Die/Firewall/UCIe 的配置顺序、地址/访问矩阵和错误传播。
- 按ADR-0028/0030冻结2 MiB安全RAM六段布局、C908 linker/Header Overlay的System Address、Host ingress与目标原地加载隔离、Measurement/GSP/PMP/RMP/MMP生命周期、BootROM/FMC复用、Firewall权限和清零；loader固定使用`target+1024`和`Code_Size`，不存在Manifest地址转换或独立plaintext Region；继续关闭OPEN-CONFLICT-006剩余PMA/Firewall/MMP DDR项。
- 冻结统一错误/中断/日志/审计/清零规则。
- 每个详设章节给出 API/struct/enum、模块/目标文件、时序/状态机、边界/失败表和测试钩子，并完成交叉评审。
- 在`security_-scheme/tests/cases/`对v0.2用例进行发布级重分层并准备新的版本化工作簿；由独立baremetal软件栈在`baremetal/components/ngu_security/`准备对应可执行case，不覆盖v0.2。

周出口：正式详设初稿覆盖全部 Feature，开发可直接拆分代码任务；每个 Feature 有 ABI/状态机/权限表/错误表/验收 oracle；所有新增设计进入 accepted amendment 或待批准 OpenSpec，不直接改写事实状态。

## T-2 周：实现接口、Mock、工具和非硬件测试

目标：把 EMU 依赖收敛到最薄硬件适配层。

- 在批准任务下，由 GSP 按详设在 `gsp-pmp-rmp-omp` 移除 EMU/产品路径对 `ehsm_stub`、模拟 crypto/cert/sign provider 的依赖，接入真实 transport/provider；未完成能力显式禁用或 fail-close，不保留可能随产品发布的桩路径。
- 完成可在 Host/QEMU 跑的 header/manifest/policy/measurement/SPDM/MCTP/attestation/状态机回归。
- 建立 Flash/OTP/eFuse/LCS/Debug/Key Rotation mock 与 fault injection hooks。
- 生成 golden images、negative corpus、标准算法向量、证书链和 token/config 包。
- 由独立baremetal软件栈在`baremetal`建立运行于安全核的EMU runner骨架和日志/结果解析，所有case具有稳定PASS/FAIL/INCONCLUSIVE输出；不经过GSP固件task/service。

周出口：关键 Feature 至少达到 C2；计划进入 EMU 的生产路径不得偷偷调用 test stub；非硬件问题已在 Host/QEMU 暴露。

## T-1 周：Dry-run、冻结和 EMU 接入包

目标：EMU 到位后第一天即可冒烟，而不是先补资料和脚本。

- 在 Host/QEMU/Vendor 可用环境做一次全流程 dry-run，记录失败和环境差异。
- 冻结首轮 EMU 软件包、镜像、OTP/LCS 配置、测试脚本、case 清单和版本 manifest。
- 准备 Day-0/Day-1 冒烟集、Week-1 全量集和 fault/security 深入集。
- 建立 Evidence 批次目录、日志哈希、缺陷模板、rerun 规则和 release dashboard。
- 对不能按期闭环的项形成风险接受或明确阻断，不把“用例已写”当作“风险已释放”。

周出口：EMU readiness review 通过；首轮测试不再依赖临场生成 key、镜像、证书、命令或预期结果。

# 工作包和优先级

| 工作包 | 覆盖 Feature | 优先级 | EMU 前交付物 | 前置依赖 |
|---|---|---|---|---|
| WP-00 正式详细设计 | 全部 | P0-Blocker | `security_-scheme/docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 主详设、专题详设和 code/test map | GSP、WP-01～09 输入 |
| WP-01 权威基线与冲突闭环 | 001/002/009/016/017 | P0-Blocker | 裁决、amendment、Owner/版本 | 项目负责人/Vendor/架构 Owner |
| WP-02 启动链和 eHSM adapter | 002～006 | P0-Critical | DD-02A `ehsm_demo_test()`完整case catalog/baremetal规则；DD-02B `bl_demo`底层/格式复用表、产品语义接口、NGU800P production adapter和状态机 | 二级case展开、eHSM command/version和NGU800P port参数；自检位图已裁决 |
| WP-03 更新与 OOB | 006～008 | P0-Critical | 更新/掉电状态机、Flash/OOB 接口、恢复 runbook | Flash/OOB Owner |
| WP-04 Measurement/Attestation/SPDM | 009/010/014 | P0-Critical | BootROM/FMC/GSP producer/consumer及16 KiB物理Region已冻结；交付产品实际`max_fw_entries`、静态X.509 cert/sign provider、完整SPDM Profile和Host requester | OPEN-DESIGN-015、OPEN-DESIGN-021、证书Profile |
| WP-05 LCS/Debug/RMA | 011/012 | P0-Critical | 权限矩阵、token、gating/expiry/reset、审计 | LCS/端口 RTL 定义 |
| WP-06 Key/OTP/Cert/Rotation | 013～015/020 | P0-Critical | ADR-0021/0025/0026及SRC-0024已冻结一机一密、Table 34、16槽、Chip Root→Level1→Level2、slot14单Profile、slot13 UDS权限、Cert0/1、制造接口、三类轮换和KEK流程 | OPEN-CONFLICT-012、Vendor定制、Key Attribute/backend、Flash/KMS/CA/MES和Provisioning wire实施绑定 |
| WP-07 Crypto/TRNG | 002/016 | P0-Critical | 产品算法矩阵、标准向量、TRNG/self-test oracle | 算法发布裁决、Vendor mapping |
| WP-08 Multi-Die/Firewall/接口 | 017/018 | P0-Critical | 时序、地址/访问矩阵、配置锁定、验证点 | RTL/UCIe/Firewall Owner |
| WP-09 Error/Interrupt/Audit/Clear | 全部，重点 019 | P0-Critical | 统一错误矩阵、日志、fault hooks、zeroization rules | 各 Feature 状态机 |
| WP-10 测试用例和自动化 | 全部 | P0-Critical | `security_-scheme`新版工作簿/case metadata和Vendor Demo完整能力清单；`baremetal`可执行case、runner/工具和dry-run | WP-00～09的oracle；Demo组包可复用但顶层/Expected不得照搬 |
| WP-11 EMU 接入与 Evidence | 全部 | P0-Critical | environment manifest、bring-up runbook、Evidence 模板 | EMU 团队能力清单 |

# 每周管理节奏

- 周一：Feature/阻塞项评审，确认本周必须裁决的接口和参数。
- 周二至周四：方案、实现、测试三方针对同一 Feature 联合闭环，不拆成互不对齐的文档任务。
- 周五：检查 feature->requirement->code->case->evidence 链；更新红/黄/绿状态和下周 critical path。
- 每个重要裁决当日进入 ADR/amendment/open question；不等月底集中补文档。

# EMU 到位时的准入门禁

- 所有 P0-Blocker 已裁决，或存在项目负责人批准的书面风险接受。
- 20 个 Feature 都已进入正式详设，并有 Owner、Source、模块/文件/API、测试 case 和完成标准。
- 首轮用例有确切执行载体、软件/RTL/eHSM 版本占位规则、输入、预期、命令和 Evidence 路径。
- 生产包不使用未声明的 demo/test-only key、cert、crypto、eHSM 或 transport stub。
- Day-0 冒烟集可以在 2 小时内判断 ready/boot/mailbox/log/基本 crypto/基本启动链是否可继续。
- 失败能够归类为环境、RTL、eHSM、BootROM/FMC/GSP、工具、用例或方案冲突，且有明确停止/继续规则。

# 当前立即需要负责人推动的事项

1. 指定 GSP 详设负责人和各专题章节 Owner；在 `security_-scheme/docs/05-software-design/NGU800P安全软件详细设计.md` 及 `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 启动正式详设。
2. OPEN-CONFLICT-001～005/007～010已裁决；ADR-0019要求eHSM BL新增FMC专用16字节staged-candidate exact-match commit API。实现/EMU前冻结command/packing/LCS/交付版本和candidate生命周期，并继续指定OPEN-CONFLICT-006、OPEN-DESIGN-015/021、Measurement/SPDM和安全RAM/Firewall Evidence Owner。
3. 三套产品算法Profile已确认；继续确定各设备/镜像provisioning绑定和测试优先级，不继续维持 100 条全部 P0 的不可排程状态。
4. 从 EMU 团队拿到能力/限制、首个可用版本、加载和日志方式；没有这些信息无法完成真实执行脚本。
5. 登记 `baremetal` 的只读 branch/commit/既有修改；确认后续批准任务可分别修改 `gsp-pmp-rmp-omp/components/security` 和 `baremetal/components/ngu_security` 的明确文件范围。

# Change history

- 2026-07-23：回填Measurement→FMC epoch批准结论；跨stage门禁可继续设计，最终table ABI/integrity/commit/reset进入OPEN-DESIGN-007。
- 2026-07-22：B0-R2按ADR-0007改用P1生命周期布局；OPEN-CONFLICT-006剩余地址视图/OMP/容量/Firewall问题保留在EMU前P0阻断清单。
- 2026-07-22：DD-02拆为baremetal完整Vendor Demo能力验证与gsp产品安全链移植；一级case catalog已建立，二级case/复用映射进入W0近期任务。
- 2026-07-22：接受ADR-0004；关闭OPEN-CONFLICT-002/003，采用Die1独立Measurement实例和GSP NoC/system canonical地址；R1-05当前不采用。
- 2026-07-22：登记W0批准原则；自检位图按Bootloader定义关闭冲突；明确现有test/stub不作为目标依据、EMU/产品不允许打桩，并优先复用OSR Host通用业务代码但隔离NGU800P平台适配。
- 2026-07-21：基于当前方案、Vendor 资料、公司安全组件和 100 条测试用例建立首版 4 周 EMU 前落实计划。
- 2026-07-21：按负责人补充信息调整主线：GSP 在 `gsp-pmp-rmp-omp` 交付可完全指导开发的正式详设/实现，在 `baremetal` 交付测试 case/EMU runner/tools。
- 2026-07-21：按负责人进一步确认，将正式详设、全部任务、测试规划/工作簿和开发追溯统一放在 `security_-scheme`；`gsp-pmp-rmp-omp` 仅承载软件实现，`baremetal` 仅承载可执行测试、EMU runner 和工具。
- 2026-07-22：新增NGU800P安全软件开发主计划，明确W0～W4、DEV/TST候选任务、代码落点和DoR/DoD；本计划继续作为EMU前总节奏入口。
- 2026-07-21：将T-4/T-3调整为按Feature滚动闭环；正式详设立即启动，剩余Owner/参数/EMU输入并行补齐，满足门禁的Feature逐项开放编码。
