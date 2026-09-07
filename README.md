# NGU800 SoC 安全工程工作空间

本仓库是 NGU800 SoC 安全需求、架构、决策、正式详细设计、全部项目任务、测试规划/工作簿、开发追溯、验证证据索引和长期问题知识的唯一事实源。`../gsp-pmp-rmp-omp` 只承载正式安全软件实现，`../baremetal` 只承载可执行测试代码、EMU runner 和测试工具；两个公司仓库保持独立边界。

## 角色分工

- Work：资料登记、事实分级、方案推演、OpenSpec 提案、任务定义、结果评审和知识维护。
- GSP：负责产出并评审可完全指导开发的正式详设，详设存放在本仓库 `docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/`；在 `gsp-pmp-rmp-omp` 完成正式安全软件实现，在 `baremetal/components/ngu_security/` 完成可执行测试、EMU runner 和测试工具。
- 远端 Codex：在批准的任务边界内直接修改指定公司仓库工作区、实现、测试并回写 RESULT/Evidence；不执行任何 Git 提交。
- OpenSpec：管理设计或行为变更的提案、审批、实现、验收和归档。
- Git：保存正式事实、历史决策、问题闭环和可追踪证据索引。

## 快速开始

1. 阅读 [PROJECT_STATUS.md](PROJECT_STATUS.md)。
2. 阅读 [manifests/repositories.yaml](manifests/repositories.yaml) 和 [manifests/workspace-map.md](manifests/workspace-map.md)。
3. 查看 [sources/source-index.yaml](sources/source-index.yaml)、相关设计和 ADR。
4. 方案变更先创建 OpenSpec change，经人工批准后再创建实现任务。
5. 运行 `python tools/scripts/project_check.py` 检查仓库一致性。

正式详设需要核验当前代码时，使用 [代码调查与证据回填工作流](docs/09-plans/CODE-INVESTIGATION-WORKFLOW.md)：调查任务进入`tasks/active/INV-SEC-*`，只读证据进入`evidence/code-investigations/CE-SEC-*`，不会在调查阶段修改两个代码仓。

涉及eHSM内部硬件细节时，先查[SRC-0035《OSR eHSM 4019 Vendor RTL硬件实现快照》Source Card](sources/source-cards/SRC-0035.md)，并执行[《eHSM Vendor RTL调查与证据规则》](docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md)。当前快照只读，结论为`VENDOR_IMPLEMENTATION`，必须证明实际elaboration，不能直接外推SoC集成或硅片行为。

正式开发按 [NGU800P 安全软件开发计划](docs/09-plans/NGU800P安全软件开发计划.md) 滚动推进：先按 Feature 完成详细设计和冲突闭环，再通过 OpenSpec/人工批准实例化近期实施任务。候选任务本身不授权修改代码。

## 事实分级

允许状态：`CONFIRMED`、`DOCUMENTED`、`VENDOR_IMPLEMENTATION`、`ASSUMPTION`、`PROPOSED`、`DEPRECATED`、`OBSOLETE`、`CONFLICTING`。硬件相关结论必须记录 Source ID、适用芯片/版本、事实状态和未解决问题。

## 方案权威层级

```text
SRC-0017 芯片系统安全方案（系统/架构/原则）
        ↓ 约束和支撑
SRC-0016 芯片安全软件方案（软件工程落地与最终采用方案）
        ↓ 形成可完全指导开发的详细设计
security_-scheme 正式详设/Requirement/任务/测试计划与工作簿
        ↓ 指导实现                         ↓ 派生测试 oracle
gsp-pmp-rmp-omp 正式软件代码       baremetal 可执行测试/EMU 自动化
        └──────────────→ Evidence/开发追溯回写 security_-scheme

Vendor eHSM/Core 文档、软件代码和RTL ──作为输入/实现证据──→ SRC-0016
```

Vendor 仅指 eHSM 及其内部 Core，不包含 SoC。采纳的 Vendor 建议必须体现在“芯片安全软件方案”中；密钥轮换策略已批准，SRC-0015机制已通过[ADR-0021《SoC Key轮换采用Vendor定制机制》](decisions/ADR-0021-soc-key-rotation-vendor-mechanism.md)进入受控软件方案增量；[ADR-0025《一机一密RTL Key与16槽OTP Key基线》](decisions/ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md)冻结量产一机一密和16槽对象，[ADR-0026《量产灌装、设备证明、证书A/B与制造接口合同》](decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)进一步冻结灌装顺序、单设备Attestation Profile、UDS、物理Key ID、Cert0/1和制造接口。Vendor/RTL实现仍分别由OPEN-CONFLICT-011/012管理。总体权威规则见[ADR-0001《方案权威层级与Vendor边界》](decisions/ADR-0001-solution-authority-and-vendor-boundary.md)。

PDF 保持为受控原始基线，当前有效版本、ADR/amendment 和冲突统一由 [方案基线控制](docs/09-plans/BASELINE-CONTROL.md) 管理。软件方案是规范目标，但与 Vendor 文档/代码或实测出现明显冲突时必须先记录差距并提交负责人裁决，不能静默覆盖；详见 [ADR-0002](decisions/ADR-0002-baseline-derivation-and-conflict-escalation.md)。

eHSM内部RTL实现以SRC-0035为第一查询源，具体权威和停止边界见[ADR-0032《eHSM Vendor RTL硬件实现基线与查询规则》](decisions/ADR-0032-ehsm-vendor-rtl-authority-and-query-rules.md)。NGU800P SoC地址/寄存器/IRQ仍以SRC-0022为准，产品策略仍以SRC-0017/SRC-0016和accepted裁决为准；不同来源冲突时必须显式升级。

现有代码仓中的 test、stub、demo 和历史 Expected 只用于说明当前代码事实与差距，不作为目标设计、最终测试 oracle、验收或发布依据。EMU/产品路径不允许 stub、模拟成功、测试密钥/证书/provider、未批准 hardcode 或 silent fallback；最终实现和可执行测试必须从有效方案、批准的 Requirement/OpenSpec/ADR 以及最新版本化测试工作簿派生。BootROM/FMC/GSP 可优先复用 OSR Host 通用业务代码，但 NGU800P 平台适配和安全门禁必须独立核实。完整规则见 [ADR-0003](decisions/ADR-0003-w0-approved-principles-and-self-test-bitmap.md)。

当前跨阶段度量信息按Measurement Table管理；C908、Native Header Overlay、typed-stage loader、linker和eHSM共享descriptor统一使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`定义的64位System Address，不建立Local/System映射，旧`0x1010_0808_0000`不得继续使用。NGU type 1包不再含Manifest，唯一包格式为`Native Header[1024] + Code[Code_Size]`，详见[ADR-0030](decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)。项目当前不采用版本化Handoff；只有出现具体必要性时才重新提案。见[ADR-0004](decisions/ADR-0004-die1-measurement-gsp-address-and-handoff-scope.md)。

## 标准闭环

```text
资料与事实源 → Work 分析 → OpenSpec 提案 → 人工批准
  → Codex 工作区实现 → 构建/测试/Evidence → Work 评审
  → 文档、ADR、追踪矩阵和项目状态更新
```

## 仓库边界

- 不把软件仓库作为普通目录纳入本仓库，不删除或重写任何已有 `.git`。
- 正式详细设计、全部任务规划、测试策略/计划/工作簿和开发追溯统一进入本仓库；正式软件代码进入 `../gsp-pmp-rmp-omp`，可执行测试代码、runner 和工具进入 `../baremetal`。
- 两个代码仓库只保留实现所必需的就地说明，不新增本项目的主计划、正式详设或测试规划副本；已有历史文档不在本次迁移或删除。
- 允许直接在多人仓库默认分支的工作区修改；不要求创建功能分支。
- Codex 不执行 `git add`、`git commit`、`git push`、merge、rebase 或 tag；所有任务均不要求 Git 提交。
- Vendor 代码只能作为实现证据，不能替代芯片规范。
- Vendor 代码按交付快照管理，不逐文件登记；后续更新依据 delivery/release note 建立新的 Source 版本。
- Vendor RTL同样按交付快照和树哈希管理，不逐文件登记；`source-vault/vendor_rtl`只读，新投递建立新Source ID。引用RTL必须带实际top/filelist/define/parameter/elaboration条件。
- RTL中的Key/KEK字面量不得复制、外发、写入日志/测试Expected/软件或用作量产Key；未确认授权前按受限资料处理。
- 未确认的 OTP/eFuse、生命周期、密钥、复位和调试行为必须标为 `ASSUMPTION` 或 `UNKNOWN`。
- 完成任务必须具备可审查的未提交 diff、RESULT、验证结果、Evidence 和剩余风险。

详细规则见 [AGENTS.md](AGENTS.md)，协作架构见 [ARCHITECTURE.md](ARCHITECTURE.md)。
