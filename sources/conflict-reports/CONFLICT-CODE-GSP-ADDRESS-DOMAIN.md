# GSP Manifest Load/Entry 地址域冲突

> 2026-08-21最终替代说明：ADR-0030已删除Manifest。C908、Header offset1008的LE64 `load_addr`、typed-stage loader、linker和eHSM共享descriptor全部使用SRC-0022 baremetal头定义的System Address，`entry_addr=load_addr`，不存在Local/System映射。`0x1000_0808_0000`和`0x1010_0808_0000`都是旧080x实现事实，不是新D0产品地址。以下Evidence/Options只保留冲突发现历史，产品实现不得采用其中的Manifest或转换方案。

## Identity

- Conflict ID: CONFLICT-CODE-GSP-ADDRESS-DOMAIN
- Open question: OPEN-CONFLICT-003
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人；SoC地址映射/实现验证Owner待指定
- Decision required by: 冻结 GSP package manifest、loader、range check、handoff/jump 或建立精确地址测试之前

## Conflict classification

- Type: code_contract_mismatch / address_domain_ambiguity
- Affected scope: GSP/SEC2 manifest `load_addr`、`entry_addr`、eHSM/loader 写地址、GSP jump 地址、allowlist 和测试 oracle。
- Safe-to-continue scope: FMC 地址、header/manifest 非地址字段、parser 长度检查、真实 eHSM adapter 的非地址部分、显式传入非 GSP 地址的测试。
- Must-stop scope: 把任一 GSP 默认地址写成最终软件方案、生成发布包、实现/验收 GSP loader/jump、固定相关 range allowlist。

## Evidence A：packager 和安全组件测试使用本地/remap 地址

- Repository: REPO-GSP-FIRMWARE，调查时工作区。
- `components/security/tools/image_packager/pack_image.py:52-64`：GSP/SEC2 默认地址为 `0x1000_0808_0000`。
- `components/security/tools/image_packager/tests/test_packager.py:57-63`：把该值固化为 expected。
- `components/security/tests/test_manifest_gate.c:19-51`、`test_policy.c:15-31`、`test_verify_flow.c:20-45,74-87`：均使用该值作为 GSP load/entry。
- Expected behavior implied by these files: manifest 按 `0x1000_0808_0000` 表示 GSP 目标/入口。

## Evidence B：GSP 构建产物使用 NoC/system 地址

- `components/chip_riscv_gsp/gcc_flash_sram.ld:19-33`：GSP 所有 section 链接到 `0x1010_0808_0000`，长度 `0x40000`。
- 仓库 `README.md:210-222`：明确列出 `gsp.bin` 为 `0x1010_0808_0000 - 0x1010_080B_FFFF`。
- Expected behavior implied by these files: GSP PC/符号/链接运行地址采用 `0x1010_0808_0000`。

## Evidence C：地址映射头表明可能是不同 master view

- `components/chip_riscv_c908_common/include/ngu800p/config_bus_address_mapping.h:607-616`：security SRAM 起始 `0x1000_0808_0000`，remap start `0x1010_0808_0000`。
- `components/chip_riscv_c908_common/include/ngu800p/subsys_address_mapping.h:812-822`：外部可见地址为 `0x1010_0808_0000`，die0 remap 为 `0x1000_0808_0000`。

这些信息说明两个数值可能都指向同一物理 SRAM，但没有定义 manifest 字段属于哪个 address domain，也没有当前 loader 证明转换方式。

## Exact conflict

| 工件/消费者 | GSP 地址 | 地址视图含义 |
|---|---:|---|
| packager / parser tests | `0x1000_0808_0000` | 可能是 security subsystem 本地/remap view |
| GSP linker / README | `0x1010_0808_0000` | NoC/system/外部可见 view |
| manifest contract | 未定义 | `load_addr`/`entry_addr` 是否同一 domain、是否需要转换均未知 |

该问题不是简单的十六进制笔误：地址映射头为两者提供了潜在 alias 关系。因此不能只修改其中一个常量；必须先确定 BootROM、FMC、eHSM DMA、GSP C908 和外部 master 各自使用的地址视图。

## Impact

- Security: 错误 address domain 可能导致明文写入非预期区域、越权覆盖或跳入错误入口。
- Software: linker symbol、manifest、eHSM output、CPU jump 和 cache 操作可能使用不同地址。
- Verification: 当前 packager/Host tests 会认可与 linker 不同的数值，可能形成假通过。
- Release: 在未裁决前不能生成可信的 GSP release package 或冻结 loader allowlist。

## Temporary handling

- CE-SEC-002 和相关详设将 GSP address domain 标为 `CONFLICTING`。
- 不更改 packager、linker 或测试，不把任一地址提升为最终方案。
- FMC `0x1000_080C_0000` 的 packager/linker一致性不受本冲突直接影响。

## Options

### Option A：Manifest 使用全局/NoC canonical 地址

- `load_addr`/`entry_addr` 使用 `0x1010_0808_0000` 视图。
- packager/tests 更新为 linker/README 地址；仅在 NGU800P port 内对不支持该视图的 master 做转换。
- 优点：跨组件、日志和审计使用统一全局地址；代价是确认 eHSM/FMC 是否能访问该 aperture。

### Option B：Manifest 使用执行子系统本地/remap 地址

- 保留 `0x1000_0808_0000`，但正式定义其 address domain。
- loader/eHSM/CPU handoff 必须明确从写地址到 linker/PC 地址的转换及 cache/fence 规则。
- 优点：可能符合本地 master 访问；风险是跨 master 误用和日志歧义。

### Option C：显式区分地址域/双地址

- package/loader contract 增加 address-domain ID，或分别定义 write/load view 与 execution entry view。
- 优点：消除隐式 alias；代价是 ABI、兼容和测试复杂度增加。

## Recommendation

先由 SoC/地址映射负责人确认各 master 的可见视图及自动 remap 行为，再优先采用“manifest 使用单一全局 canonical 地址、转换收敛在 port/loader”的方案；如果硬件只能使用本地 view，则采用 Option B 并把 address-domain ID、转换公式和跳转规则写入软件方案。不要仅凭当前测试常量修改 linker 或 packager。

## Required owner decision

1. `manifest.load_addr` 与 `entry_addr` 的正式 address domain 分别是什么，是否允许不同？
2. eHSM decrypt/DMA、FMC C908、GSP C908、NoC/Host 各使用哪一 aperture？
3. remap 是硬件自动完成还是软件显式转换；cache、firewall 和 measurement 记录哪个地址？
4. 最终采用 Option A/B/C 中哪一项，并进入 SRC-0016 后续有效版本或受控 amendment。

## Resolution and review history

- 2026-07-22：负责人批准推荐Option A。Manifest `load_addr`/`entry_addr`采用NoC/system canonical domain，当前值`0x1010_0808_0000`；`0x1000_0808_0000`仅为local/remap view。不能访问canonical aperture的master只允许在NGU800P port/loader显式转换，业务代码不得隐式alias。各master aperture、remap、cache/fence和Firewall仍需RTL/EMU Evidence验证。见ADR-0004。
- 2026-07-28：最终裁决取代上述转换方案；产品唯一使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE` System Address，Local/System转换从产品合同删除。
- 2026-07-21：INV-SEC-002 对 packager、tests、linker、README 和地址映射头独立核对后建立；等待负责人裁决。
