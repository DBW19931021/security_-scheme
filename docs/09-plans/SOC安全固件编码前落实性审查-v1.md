---
title: "SOC安全固件编码前落实性审查 v1"
status: historical_superseded_in_part
evidence_state: DOCUMENTED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0023
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-08-25
supersedes: []
superseded_by: []
---

# 1. 审查结论

> **历史审查说明（2026-09-01）**：本文关于旧Manifest骨架需要重构的差距仍有效，但目标不再是128B Manifest parser，而是删除production Manifest路径并实现Header offset1008/1016/1020 Overlay、Header CRC双检、typed-stage policy、`target+1024`/`Code_Size` loader。现行依据见ADR-0030/0033和主详设。

BootROM、FMC、GSP的目标职责、正常/失败状态机、Secure Package/Header Overlay/typed-stage policy、Measurement Table、eHSM单Owner、rollback counter和release先后关系已经足以拆分编码工作包；但当前三个产品入口均不是目标安全启动链，公共`security`组件也仍是旧草案/演示骨架，不能在现状上直接补几个调用后作为产品实现。

本轮结论是：

- 三个产品固件整体状态均为`NOT_READY_FOR_PRODUCT_INTEGRATION`，当前没有代码修改授权。
- 可以先做的只有与地址、寄存器、Vendor新增命令无关的纯逻辑实现；正式开始仍需负责人单独授权修改代码仓。
- BootROM真实垂直链受平台地址、PMA/Firewall、包来源、release/RAS接口阻断。
- FMC除上述平台输入外，还被eHSM BL新增16字节staged-candidate commit/readback API的正式交付阻断。
- GSP除公共能力外，还被PMP/RMP/MMP实例拓扑、包/地址/依赖/release profile以及真实Vendor Host/FW集成阻断。
- SPDM Measurement对外block映射已按负责人要求延期，不阻断内部Measurement Table和核心安全启动链；它只阻断SPDM responder的最终wire兼容实现。

本文是落实性审查和后续任务拆分依据，不是编码授权，也不替代[正式详细设计](../05-software-design/NGU800P安全软件详细设计.md)。

# 2. 审查边界与基线

- 设计/追溯仓库：`security_-scheme`，本轮只更新本仓库文档。
- 产品代码仓：`../gsp-pmp-rmp-omp`，仅作只读代码事实检查，未修改。
- Vendor快照、`baremetal`和测试工作簿均未修改。
- 未执行任何Git命令、构建或运行测试。
- 仓库清单此前记录的产品代码基线为`master@08b29...`；当前工作区已有未提交修改，本轮不使用Git，因此只把看到的文件内容记录为“当前live worktree事实”，不判断这些修改的作者、差异或归属。

详细代码证据见[CE-SEC-014](../../evidence/code-investigations/CE-SEC-014-soc-fw-implementation-readiness.md)。

# 3. 状态定义

| 状态 | 含义 |
|---|---|
| `READY_LOGIC_ON_AUTHORIZATION` | 设计合同已经足够明确，负责人另行授权代码修改后，可先实现与平台/Vendor缺口解耦的纯逻辑和host单元测试 |
| `BLOCKED_PLATFORM` | 缺RTL生成地址/寄存器、PMA/cache/Firewall、包来源、release/RAS或实例拓扑，不能写入产品常量和真实副作用 |
| `BLOCKED_VENDOR` | 缺匹配Vendor Host/BL/FW代码、API或release note，不能自行猜测command、packing或状态语义 |
| `BLOCKED_BUILD_RELEASE` | 逻辑可以设计，但生产代码生成、配置唯一Owner、签名/KMS或no-stub发布证明尚未冻结 |
| `DEFERRED_FEATURE` | 已明确延期，不阻断当前核心启动链，但对应功能不得伪实现或宣称完成 |

# 4. 当前代码事实与目标差距

| ID | 当前代码事实 | 对目标的影响 | 分类 |
|---|---|---|---|
| IR-001 | BootROM默认打印Hello World，可选`SECURE_DEMO`使用synthetic package；FMC是Hello World；GSP为QEMU/SPDM测试入口或循环打印 | 三个stage都没有生产安全状态机，不能把现有`main`视为待补全产品链 | GAP |
| IR-002 | 当前Manifest/Measurement/verify flow仍是旧结构：32位counter、固定BSS slot、eHSM状态/sequence、stub verify后直接record | 与ADR-0019/0022及批准调用顺序冲突，必须按新ABI和职责重构，不能增量兼容旧骨架 | GAP |
| IR-003 | 产品公共source graph无条件纳入eHSM/attestation stub和null crypto/platform provider，构建又整体链接公共库 | 不满足产品/EMU no-stub门禁；在真实功能接入前必须先隔离production/test source graph | GAP / BLOCKED_BUILD_RELEASE |
| IR-004 | BootROM/FMC/GSP linker仍使用旧`0x1000_080x_0000`/`0x1010_0808_0000`布局 | 不能作为新2 MiB安全RAM目标的最终linker、Manifest Expected或release PC依据 | BLOCKED_PLATFORM |
| IR-005 | 产品代码仓未定位到真实Vendor Host集成、`ehsm_init/verify/read_counter`、direct aperture/status或ready信号绑定 | 真实eHSM MMIO/Host链路尚不存在；不得用stub或OSR样例地址替代 | BLOCKED_PLATFORM / BLOCKED_VENDOR |
| IR-006 | GSP只创建普通`application_task`，没有最高优先级、从bootstrap持续到runtime的唯一`security_service_task` | eHSM lifetime Owner与typed queue合同尚未实现 | GAP |
| IR-007 | 未定位到批准的loader读回digest、Measurement CRC/commit/snapshot、Firewall权限转换、release和统一RAS终态实现 | 还不能证明“验证成功才加载、commit成功才release、失败不旁路” | BLOCKED_PLATFORM / GAP |

# 5. 公共组件落实性

| 合同 | 设计状态 | 可开始范围 | 仍缺输入 |
|---|---|---|---|
| NGU Manifest v1 parser/validator | 已批准 | `READY_LOGIC_ON_AUTHORIZATION`：精确offset/length、overflow、enum、TLV、算法Profile和negative corpus | 单一registry/profile生成方式；实际SKU provisioning/release matrix |
| Vendor Header preflight/post-check | 已批准 | `READY_LOGIC_ON_AUTHORIZATION`：固定Header和长度交叉校验、raw/typed result分离 | 真实Vendor Host API绑定和匹配交付版本 |
| Measurement Table逻辑ABI | ADR-0022已批准；ADR-0028固定16 KiB/126项硬上限 | `READY_LOGIC_ON_AUTHORIZATION`：128B结构、CRC-32C、32位commit、append/finalize/snapshot/reset的纯逻辑 | 产品实际`max_fw_entries`、PMA/cache/Firewall、跨master可见性Evidence |
| 公共错误/状态机纯逻辑 | 原则已批准 | `READY_LOGIC_ON_AUTHORIZATION`：无副作用的状态转换、错误分类、timeout→acceptance unknown/quarantine测试 | RAS event/action ID、精确deadline、平台timer |
| image loader | 职责已批准 | 仅可先定义接口和host内存模型 | 真实load/entry、PMA/barrier/指令侧同步、Firewall和release接口；System Address视图已关闭 |
| eHSM adapter | 结构/Owner/轮询原则已批准 | 可准备不含寄存器常量的接口层和fake-free编译边界 | direct aperture/status/error宏、地址转换、service channel、Vendor Host源码/API |
| production/test构建隔离 | no-stub原则已批准 | `READY_LOGIC_ON_AUTHORIZATION`：拆分source list、产品负向符号扫描、host-unit mock独立target | 正式toolchain/map/symbol检查方法和发布Owner |

现有旧Manifest、Measurement和`verify_flow`不建立产品兼容义务。编码时应以批准ABI为唯一目标进行替换，不应保留第二套旧字段、旧slot语义或stub fallback。

# 6. BootROM落实性

## 6.1 已冻结的软件合同

- 先读取一次`non_sec_boot`只读快照：可靠值1强制现有受限非安全，输入异常终止，值0才直接读取SoC LCS；值0时USER忽略Strap并强制安全，非USER按SRC-0023的`boot_pin.secure_boot[3]`判定，0为受限非安全、1为安全。policy-fuse和Strap真实绑定在相应RTL资料同步前保持阻断。
- 轮询等待`bootloader_done=1 && bootloader_err=0`，超时/错误/self-test失败均不得绕过eHSM。
- 固定以`check_version=0`验证Vendor type 1 FMC，使BL暂存认证candidate；BootROM不读取、比较或写stored counter。
- eHSM PASS后复验Header Overlay/typed-stage policy/digest，loader成功后提交唯一FMC Measurement，最终复验Firewall/entry后才release/jump。
- 任一失败不release；RAS不可用时保存静态错误、关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出。

## 6.2 编码前缺口

| 缺口 | 分类 | 是否阻断真实BootROM |
|---|---|---|
| `non_sec_boot`准确eFuse位/编码、ECC/valid/镜像、复位锁存、只读接口与烧写锁定Owner | BLOCKED_PLATFORM | 是；`BLOCKED_BY_NON_SEC_BOOT_BINDING` |
| SoC LCS/Strap/ready/error准确寄存器宏与访问函数 | BLOCKED_PLATFORM | 是 |
| FMC package来源、最大长度、ingress/plaintext区域、最终load/entry | BLOCKED_PLATFORM | 是 |
| BootROM新2 MiB linker/stack/尾部回收和release PC视图 | BLOCKED_PLATFORM | 是 |
| 最终PMA、barrier可见性、Firewall编程/lock/readback | BLOCKED_PLATFORM | 是 |
| timer deadline、RAS event/report和WFI终态绑定 | BLOCKED_PLATFORM | 是 |
| 真实Vendor Host BL verify/decrypt调用和状态映射 | BLOCKED_VENDOR / PLATFORM | 是 |

结论：可以先实现模式判定、Header Overlay/typed-stage policy检查、纯状态机和Measurement序列的host单测；不能形成可在D0运行的BootROM产品链。

# 7. FMC落实性

## 7.1 已冻结的软件合同

- FMC不加载eHSM Vendor FW；counter proof成立后才接收并验证GSP package。
- FMC从BootROM已commit的唯一FMC Measurement取得`expected_candidate[16]`。
- FMC初始化调用eHSM BL专用接口；BL先与RAM candidate逐字节exact-match，再执行低值拒绝、相等不写、高值单向写和权威readback。
- proof成立后GSP固定`check_version=0`且认证counter必须等于已提交值；随后才提交GSP Measurement并release GSP。
- 可清理、无不可逆副作用的确定性接收失败允许重新arm等待Host再次发送；timeout/unknown completion进入quarantine。

## 7.2 编码前缺口

| 缺口 | 分类 | 是否阻断真实FMC |
|---|---|---|
| eHSM BL新增staged-candidate commit/readback API的command ID、wire packing、LCS、状态码、candidate生命周期、readback、交付版本和掉电语义 | BLOCKED_VENDOR | 是，硬阻断 |
| GSP package接收来源/通知、最大长度、清理与再次arm合同 | BLOCKED_PLATFORM | 是 |
| GSP最终load/entry、linker/PC视图、Firewall和release接口 | BLOCKED_PLATFORM | 是 |
| counter操作和verify操作deadline/RAS/quarantine数值绑定 | BLOCKED_PLATFORM / VENDOR | 是 |

结论：可以先实现FMC rollback policy和无副作用状态机测试；在Vendor专用API到齐前不得实现或模拟成功的counter提交路径，也不得release GSP。

# 8. GSP落实性

## 8.1 已冻结的软件合同

- GSP替代旧OMP/Q&P产品身份；OMP不再是独立镜像、Measurement或release对象。
- 第一个、最高优先级`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner，其他task只通过typed queue请求。
- GSP加载eHSM Vendor FW，并验证/加载/release PMP、RMP、MMP；单Runtime确定失败只隔离对应Runtime及依赖者。
- 启动阶段使用poll；不向Host开放GSP安全服务；内部通用算法服务优先级较低。

## 8.2 编码前缺口

| 缺口 | 分类 | 是否阻断真实GSP |
|---|---|---|
| eHSM Vendor FW真实包来源、Vendor Host/FW boot API、firmware_done/error绑定 | BLOCKED_VENDOR / PLATFORM | 是 |
| PMP/RMP/MMP最大实例、包尺寸、地址、依赖图、release顺序/接口和降级面 | BLOCKED_PLATFORM（OPEN-DESIGN-010/021） | 是 |
| `security_service_task`优先级、stack、queue、deadline和启动调度集成 | BLOCKED_PLATFORM | 是 |
| GSP/runtime最终RAM分区、linker、Firewall/访问矩阵 | BLOCKED_PLATFORM | 是 |
| SPDM Measurement block/profile | DEFERRED_FEATURE（OPEN-DESIGN-015） | 不阻断核心启动；阻断SPDM最终wire实现 |

结论：可以先实现typed request模型、唯一Owner状态机和公共纯逻辑；不能冻结runtime装载表、最终地址或完整GSP产品启动路径。

# 9. 建议的后续编码顺序

以下顺序只用于负责人后续选择，不代表当前已授权：

1. `DEV-SEC-001`：先拆生产/测试source graph，建立产品no-stub/no-null-provider负向门禁。
2. `DEV-SEC-002/004-logic`：实现公共错误、Manifest、Header复验、Measurement ABI和纯状态机host单元测试。
3. `DEV-SEC-003`：平台/Vendor输入到齐后接入真实NGU800P eHSM adapter。
4. `DEV-SEC-005`：以BootROM→FMC形成第一条真实垂直链。
5. `DEV-SEC-004A/006`：Vendor counter API交付后完成FMC→GSP。
6. `DEV-SEC-007`：Runtime profile/地址/release资料到齐后完成GSP→PMP/RMP/MMP。

任何阶段都不允许用stub、synthetic success、旧demo调用顺序或未批准hardcode填补缺口。

# 10. 需要外部补充但本轮不要求负责人裁决的资料

本轮没有发现需要改变已批准架构原则的新冲突；缺口均为实现DoR或已登记开放项：

1. RTL/平台：`non_sec_boot` eFuse位/编码/只读锁存、direct eHSM aperture/status/error、LCS/Strap/ready寄存器、2 MiB最终访问视图、PMA/cache/Firewall、release/reset/RAS、包来源和地址/容量。
2. Vendor：真实Host/BL/FW集成版本；FMC专用16字节counter API的完整合同。
3. Runtime团队：PMP/RMP/MMP镜像与实例上限、依赖、地址、release/隔离接口。
4. Build/Release：单一registry/profile生成源、正式toolchain/map/symbol检查、KMS/签名和发布Owner。
5. SPDM团队：OPEN-DESIGN-015已延期，待资料齐备后再冻结对外block和session细节。

# 11. 下一步

在不修改代码的前提下，下一项工作应是把本审查中的每个`READY/BLOCKED`合同映射到测试规划v0.3：明确case、oracle、载体、所需平台能力和Evidence；保持测试工作簿不变，先在`security_-scheme`形成规划与差距清单。
