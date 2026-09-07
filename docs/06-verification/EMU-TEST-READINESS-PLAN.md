---
title: "NGU800P EMU 安全测试就绪计划"
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
last_reviewed: 2026-07-21
supersedes: []
superseded_by: []
---

# 目的

在EMU（C908 + eHSM）可用前，在`security_-scheme`完成测试策略、版本化工作簿、case规划、输入/oracle清单、环境清单和Evidence结构；由独立baremetal软件栈在`../baremetal/components/ngu_security/`完成运行于安全核的eHSM全能力可执行测试、自动化骨架、EMU runner和工具。该路径不经过GSP固件task/service。本计划不把尚未执行的case标为PASS。

# 当前基线与量化缺口

当前治理快照为 `security_-scheme/tests/cases/NGU800P-security-test-cases-v0.2.xlsx`，共 100 条已编号用例，全部标为 P0。后续工作簿和 case 规划继续进入 `security_-scheme`，只有可执行 case、EMU runner 和工具进入 `baremetal`；版本/追踪/Evidence 索引也统一由 `security_-scheme` 管理。只读统计如下：

| 字段 | 已填写 | 缺失/未落实 | 影响 |
|---|---:|---:|---|
| 用例数/优先级 | 100 | 0 | 全部 P0，无法表达真正的 release critical path |
| 测试载体 | 26 | 74 | 不能确定 Host/QEMU/Vendor sim/EMU/样片分工 |
| RTL 版本 | 1 | 99 | 执行后难以复现和比较 |
| 软件版本 | 0 | 100 | 无法把结果绑定 BootROM/FMC/GSP/eHSM/Host |
| 软件环境 | 26 | 74 | runner、工具链、OS/依赖未冻结 |
| 当前状态 | 26 | 74 | 进度不可统计 |
| 自动化状态 | 25 | 75 | 无法评估 EMU 执行工时 |

当前待确认包括：

- 用例 113：DES/TDES 与弱算法使用边界待裁决。

用例093的OPEN-CONFLICT-001已关闭：采用Bootloader定义，Host定义错误；v0.2保持不变，v0.3中以黄色高亮修改为bit18/`0x40000`=`TRNG`、bit19/`0x80000`=unknown/reserved，并要求保留raw bitmap。

用例107的OPEN-CONFLICT-002已关闭：内部Measurement Table独立记录`NGU_FW_TYPE_DIE1_FW`/`die_id=1`，相同映像可以有相同digest但不能丢失Die1实例/release语义；v0.2保持不变，下一版高亮修改Expected。地址合同最终固定为C908、Header Overlay、loader、linker和eHSM共享descriptor全部使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE` System Address，不存在Local/System映射；必须拒绝旧080x和local/remap输入。精确Region已由ADR-0028冻结，PMA/Firewall/MMP DDR仍待输入。

负责人已确认SRC-0018 `ehsm_demo_test()`提供的全部BL/FW/可选功能和case都进入eHSM能力验证范围。一级入口已登记在`tests/cases/EHSM-DEMO-CASE-CATALOG.md`；二级command/case展开完成后映射到新工作簿和baremetal可执行入口，不覆盖v0.2。Vendor组包代码可在无特殊差异时复用，但顶层执行follow baremetal，Vendor Demo最终打印不作为PASS依据。

现有代码仓test/stub/demo/synthetic flow及历史Expected只作为`CODE_FACT`和差距，不是测试目标或最终oracle。可执行测试从最新有效工作簿和批准方案/Requirement/ADR/OpenSpec派生；EMU/产品验收不得用stub、simulated success、test key/cert/provider、未批准hardcode或silent fallback形成PASS。
- 用例 001/003/025：MD5、SHA-512/256、AES-192-XTS 的产品定位待确认。
- 用例 049/091/100/109/110：counter 原子性、WDT/日志参数、TRNG 阈值、fault point、清零范围待由方案/实现冻结。

# 验证层级

每条 case 必须明确一个主执行层级，可增加辅助层级，但不能用低层结果替代高层验收。

| 层级 | 目标 | 典型内容 | EMU 前能否完成 |
|---|---|---|---|
| L0 规格/静态 | 验证方案、ABI、权限和配置自洽 | schema、边界值、source trace、编译期 layout、静态扫描 | 可以 |
| L1 Host 单元/契约 | 验证纯软件逻辑和硬件 adapter 合同 | Header Overlay/typed-stage policy/measurement、状态机、错误映射、mock provider | 可以 |
| L2 QEMU/Vendor sim | 验证固件编排、标准协议和 Vendor 快照行为 | BootROM demo、MCTP/SPDM requester/responder、Vendor Python 回归 | 尽量完成 |
| L3 EMU 集成 | 验证 C908 + eHSM + RTL 的真实接口行为 | ready/self-test、mailbox、中断、OTP/eFuse、boot chain、Flash、Debug、Firewall | EMU 到位后 |
| L4 故障/安全/发布 | 验证 fail-close、掉电、攻击面、长期/统计和发布门禁 | power-cut、reset race、fault injection、TRNG 连续监控、性能/稳定性 | EMU 后，部分需 FPGA/样片 |

# 用例优先级重构建议

当前 100 条全部 P0 的含义必须调整。新的工作簿版本经负责人批准后采用：

- `P0-Blocker`：失败直接阻止流片前安全方案放行；必须有 EMU Evidence 或书面风险接受。
- `P1-Feature`：所有批准 Feature 的必测功能、异常和边界；失败阻止对应 Feature 关闭。
- `P2-Compat`：兼容/扩展/算法全集/Vendor 回归；失败不自动阻止核心安全链，但必须评估产品承诺。
- `EXP-CONFLICT`：oracle 尚有冲突或参数未冻结；只能探索性执行，裁决前不能形成正式 PASS/FAIL 门禁。

若统一模板的“优先级”列不允许新枚举，则使用 P0/P1/P2，`EXP-CONFLICT` 写入“当前状态”和“备注”。

# EMU 测试波次

## Wave 0：Day-0 非破坏性冒烟

目标是在 2 小时内判断平台是否具备继续测试的基本条件。建议从现有用例中抽取：

- 038：eHSM 上电 ready/自检（冲突位只记录原始值，不做位级判定）。
- 040：Mailbox 基础通信和中断。
- 042：CRG/状态/复位。
- 051：BootROM 等待 eHSM 并验证 FMC 正向。
- 053：native header 正向/边界代表项。
- 055：FMC Header Overlay与typed-stage policy正向；旧Manifest fixture必须转负向拒绝。
- 070：measurement table 基本填充。
- 073：SPDM 版本/能力/算法协商。
- 094：ready/self-test/timeout fail-close 代表项。
- 095：FMC->GSP->Runtime release 链。
- 100：TRNG 启动健康状态和基础输出门禁。
- 102：错误、状态和中断传播/清除代表项。

Wave 0 不执行不可逆 OTP/LCS/Key Rotation/ROM Patch 操作。

## Wave 1：核心信任链和可恢复性

- 安全启动、验签/解密、Header Overlay/typed-stage policy、anti-rollback、measurement。
- 固件更新/OOB 恢复、掉电/复位原子性。
- eHSM 错误、mailbox timeout/busy/非法输入和 fail-close。
- 每个失败路径必须确认未 release 下一级、无未授权 fallback、日志可定位。

## Wave 2：身份、密钥和权限

- OTP/eFuse、LCS、key install/use isolation/rotation。
- Device key/cert slot、SPDM challenge/measurement、nonce/replay。
- Debug/RMA 的全局开关、expiry、reset、revocation 和审计；token携带scope字段必须拒绝。
- 按“测试专用可恢复配置 -> 一次性不可逆配置”的顺序执行；不可逆 case 必须单独审批。

## Wave 3：多 Die、Firewall、接口和故障攻击

- Die1 验签解密/release、UCIe 明文传输、SRAM Firewall、Die1 Debug。
- PCIe/UART/JTAG/DFT/Host DMA 访问矩阵。
- fault injection、敏感材料清零、错误竞态、长稳和性能。
- Die1 measurement按ADR-0004形成确定性PASS/FAIL；SPDM逐条或聚合呈现的具体profile仍待DD-05冻结，但不得丢失Die1实例/release语义。

## Wave 4：算法/兼容/Vendor 回归

- 算法和曲线全集、边界长度、中断/DMA/分段一致性。
- Vendor 4019 BL/FW Python 关键回归，并登记 NGU800P 集成差异。
- 弱算法和非标准组合必须按产品 profile 判定，不因“Vendor 支持”自动纳入安全放行。

# EMU 前必须准备的测试资产

## 输入和 oracle

- 合法镜像：FMC、GSP、PMP、RMP、MMP及eHSM Vendor FW；type 1 SoC package覆盖当前批准算法、`rollback_counter[16]`、offset1008 LE64固定load、offset1016 LE32 Header CRC32、offset1020零reserved和typed-stage Profile，eHSM FW使用Vendor type 0专用流程且不解释Overlay。
- 非法镜像corpus：Header截短/尾随、`Code_Size`小于或大于实际命令长度、Vendor type混用、Plain/Naked/Reserved异常、offset1008大小端/32位截断/错stage目标、offset1016 CRC值/算法/覆盖范围/字节序错误、offset1020非零、CRC正确但签名错误、旧Manifest/旧8字节零reserved/dual parser、offset+size溢出/重叠、Header/FMC/GSP/Runtime rollback counter不一致、签名/密文/Code/LCS/board/profile篡改、PASS后二次Header变化、output容量不足、buffer alias、`target+1152`遗留行为和设备端错误去CBC补零。
- OPEN-CONFLICT-008已由ADR-0014关闭；`Code_Size`不一致和Vendor/NGU `Image_Type`分层在v0.3新版本中转换为确定oracle并高亮，不再作为探索性case。
- 标准密码向量：产品P0覆盖ADR-0017三套Profile所需SHA-256/RSA-2048-PSS/ECDSA-P256/AES-128-CBC/SM3/SM2/SM4-CBC及TRNG；其他Vendor算法保留为baremetal能力/回归分层覆盖，不作为产品release允许。
- OTP/eFuse/LCS 配置包：只用 dummy/test key；标注是否可逆、可复用和消耗资源。
- Key Rotation 场景：Verify/Encrypt/Debug三类Key、重复轮换拒绝、48字节封装长度/属性/CRC/`key_type`副本错误、旧/新slot、HSM Bitmap、USER鉴权失败、写窗口关闭、写Key/证明/Bitmap/destroy/响应/reset各power-cut point、unknown quarantine和首次新Key验证失败。当前Vendor定制Host/BL/FW、专用command和release note到齐前标`BLOCKED_BY_VENDOR_DELIVERY`，不得用通用安装接口伪造PASS。
- 证书链和 SPDM：root/intermediate/device、过期/错误 EKU/错误 slot/损坏 chain、nonce/replay/乱序/畸形包。
- Debug/RMA：正确/错误/过期/错误UID/含scope字段/吊销token，以及Die0/Die1全局开关同步关闭。
- Multi-Die/Firewall：地址区间、master/target、LCS/debug 组合和期望访问矩阵。

## 自动化和工具

- image pack/dump/validate runner；所有输入输出记录 SHA-256。
- Host/QEMU runner；稳定输出 case ID、阶段、结果、错误码、耗时和版本。
- EMU transport adapter；串口/JTAG/mailbox/共享内存细节与 case 逻辑分离。
- 配置加载器；把 RTL/eHSM/BootROM/FMC/GSP/Host 版本和 OTP/LCS profile 写入环境 manifest。
- 日志采集/归一化/哈希；保存 raw log，parser 只生成派生 summary。
- fault injection 控制器；每个注入点有唯一 ID、触发条件、预期终态和恢复步骤。
- rerun 支持；同一 case 能绑定相同输入和配置复跑，失败不能靠手工修改环境“修好”。

## 仓库与目录职责

- `security_-scheme/tests/cases/`：保存 SRC-0020 的受控审视版本和后续版本化测试工作簿，是测试 case 规划的唯一主维护位置。
- `security_-scheme/tests/matrices/`：登记当前测试版本、Feature/Requirement/code/case/环境/波次、可执行入口和 Evidence 追踪。
- `security_-scheme/docs/06-verification/`：测试策略、readiness、runbook 和发布门禁。
- `security_-scheme/tasks/`：测试准备、执行、缺陷闭环和发布相关项目任务；不在 `baremetal` 建立第二套任务计划。
- `gsp-pmp-rmp-omp/components/security/`：GSP 负责的正式安全软件实现；实现侧保留必要单元/契约测试和代码就地说明，但正式详设存放在 `security_-scheme`。
- `baremetal/components/ngu_security/tests/`：独立baremetal软件栈的Host/QEMU/EMU可执行case、runner和必要测试数据，目标代码运行于安全核；不保存测试工作簿或项目测试规划。
- `baremetal/components/ngu_security/tools/`：baremetal能力验证配套的制包、解析、向量/config生成和测试辅助工具；不属于GSP固件运行时组成。
- `security_-scheme/evidence/`：实际执行批次、原始日志、配置、哈希、签核和发布矩阵。

测试私钥、真实量产 key、未脱敏 token 不进入上述普通目录；只登记安全存储引用 ID 和必要哈希。

# 单条用例的 EMU Ready 清单

每条计划在 EMU 执行的 case 必须满足：

- [ ] 唯一 case ID、Feature ID、Requirement ID 和方案 Source/章节。
- [ ] Owner、执行人、复核人和失败 triage Owner。
- [ ] 主执行层级、波次、是否不可逆、预计时长和可并行性。
- [ ] RTL/eHSM/BootROM/FMC/GSP/Host 版本填写规则。
- [ ] 前置 LCS/OTP/Flash/Debug/证书/网络/电源状态。
- [ ] 输入文件、配置、命令和 SHA-256。
- [ ] 可机器判定的通过准则、错误码、终态和禁止行为。
- [ ] timeout、清理、reset 和失败后的恢复步骤。
- [ ] 自动化状态：manual / skeleton / automated / blocked。
- [ ] Evidence 目录和 raw log 名称。
- [ ] 若 CONFLICTING，只执行探索性采集，不宣告确定性 PASS/FAIL。

# Evidence 批次结构

建议每次实际执行使用以下结构；创建时绑定真实日期和环境 ID：

```text
evidence/emu/<run-id>/
  environment.yaml
  software-manifest.yaml
  test-selection.yaml
  cases/<case-id>/
    command.txt
    input-manifest.yaml
    raw.log
    result.yaml
    artifacts.sha256
  summary.md
  approvals/
```

`environment.yaml` 至少记录 EMU/RTL/eHSM、C908、BootROM/FMC/GSP/Runtime/Host、工具链、板卡/实例、LCS/OTP profile、时钟/reset 和已知限制。

# 工作簿版本计划

- v0.2：当前 100 条审视版，保持不变。
- v0.3：在`security_-scheme/tests/cases/`生成，补齐Feature/Requirement映射、优先级/波次、Owner、载体、自动化和阻塞状态；所有修改继续黄色/绿色/橙色高亮。
- v0.4：在同一`security_-scheme/tests/cases/`目录生成EMU前冻结版；绑定首轮软件/RTL/eHSM baseline、独立`baremetal`软件栈的可执行脚本版本、Evidence路径和runbook。
- v1.0：流片前执行和评审完成后的批准版；工作簿汇总状态，但原始 PASS/FAIL 仍以 `evidence/` 为准。

每次更新必须生成新文件，不能覆盖旧版本。由于本轮只修订仓库分工和计划，未修改 `baremetal`，未生成 v0.3，也未修改 v0.2。

# T-4 到 T0 的测试准备排程

| 时间 | 工作 | 出口 |
|---|---|---|
| T-4 | 用例重分层、分配 Owner/层级/波次；确认 EMU 能力和不可逆限制 | 100 条均有处理路径；冲突项隔离 |
| T-3 | 基于 `security_-scheme` 正式详设冻结 Feature oracle；在 `security_-scheme/tests/cases/` 生成 v0.3；建立 corpus/vector/config 清单 | P0-Blocker 的输入/预期完整 |
| T-2 | 在 `baremetal` 完成Host/QEMU/Vendor sim前置回归并实现EMU runner/adapter/parser骨架 | runner和输入/日志链可dry-run；不以模拟transport证明真实eHSM功能通过 |
| T-1 | 在 `security_-scheme` 生成 v0.4；冻结软件包/配置/`baremetal` 脚本；做全流程 dry-run | readiness review 通过 |
| T0 | 执行 Wave 0；确认平台健康后才进入 Wave 1 | 2 小时内 Go/No-Go |

# 发布门禁

- 所有 P0-Blocker 必须 PASS，或有项目负责人批准的风险接受。
- 所有批准 Feature 至少有一条正向和一条代表性安全失败路径的 Evidence。
- 安全启动、rollback、LCS、Debug、key/OTP、更新/OOB、attestation、Firewall、多 Die、fail-close、zeroization 不允许只凭 Vendor 测试关闭。
- 自动化重跑结果可复现；人工 case 需双人复核关键观察。
- 所有失败、跳过、INCONCLUSIVE 和环境限制都进入发布矩阵；不允许将“无法执行”记为 PASS。
- 冲突项必须裁决或风险接受；橙色单元格不能静默转绿。

# Change history

- 2026-08-21：按ADR-0030把Manifest corpus改为Header offset1008/1016、typed-stage固定目标、`target+1024`/`Code_Size`和旧Manifest拒绝用例；尚未生成或覆盖工作簿。
- 2026-07-23：依据CE-SEC-009扩充安全包/Manifest/typed verify/loader corpus计划；增加Code_Size、Vendor/NGU类型分层、16字节epoch、64位地址域、digest/output/alias和release门禁，未生成或覆盖工作簿。
- 2026-07-22：加入`ehsm_demo_test()`全部能力的双层case盘点和baremetal执行规划；BL/FW及不可逆case分profile，Vendor组包可复用但Demo顶层/打印不作为发布oracle。
- 2026-07-22：关闭OPEN-CONFLICT-002/003；规划v0.3高亮更新用例107和GSP canonical地址Expected。当前不采用Handoff，不新增相应用例类别。
- 2026-07-22：OPEN-CONFLICT-001按Bootloader位图关闭并规划v0.3高亮更新用例093；登记现有test/stub非最终oracle和EMU/产品无打桩验收规则。
- 2026-07-21：基于 v0.2 的 100 条用例和公司/Vendor 测试资产，建立 EMU 前测试就绪、波次、资产、Evidence 和版本计划。
- 2026-07-21：按负责人补充信息将后续测试case/EMU runner/tools的主维护仓库调整为`baremetal`；2026-07-28进一步明确能力验证由运行在安全核上的独立baremetal软件栈执行，不属于GSP固件，`security_-scheme`仅保留治理快照和追踪。
- 2026-07-21：按负责人进一步确认，将测试策略、计划、版本化工作簿和 case 规划统一保存在 `security_-scheme`；`baremetal` 只保存可执行测试、EMU runner、工具和必要测试数据。
