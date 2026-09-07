# eHSM BL 与 Host API 自检位图冲突

## Identity

- Conflict ID: CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人 / Vendor
- Decision required by: 已于 2026-07-22 裁决

## Conflict classification

- Type: document_code_mismatch / interface_mismatch
- Affected scope: eHSM Bootloader 自检结果位图、Host API 结果解释、相关测试预期。
- Safe-to-continue scope: 与自检位图无关的 eHSM 代码阅读、架构分析和其他接口 review。
- Must-stop scope: 把 `0x40000`/`0x80000` 确定写成项目要求、实现 Host 解码、建立按算法位判断的验收测试或据此判定 TRNG/SHA256 结果。

## Source A：Bootloader 定义

- Source ID and version: SRC-0012，OSR eHSM Bootloader TRM 4019 v1.1；SRC-0018 中 BL `2.3.5-4019-72f8fdc`。
- Applicable version: eHSM 4019 Bootloader。
- Document evidence: SRC-0012 第 50 页，`bl_read_self_test_result` 位图列出 `0x40000: TRNG`，未列出独立 SHA256 位。
- Code evidence: `source-vault/vendor-code/osr_eshm/ehsm_bl-2.3.5-4019-72f8fdc/src/component/selftest.h:51` 定义 `EHSM_SELF_TEST_TRNG (0x1U << 18)`，即 `0x40000`。
- Expected behavior: bit18 表示 TRNG。

## Source B：Host API 定义

- Source ID and version: SRC-0014，OSR eHSM Host API TRM 4019 v1.0；SRC-0018 中 Host `2.3.1-4019-2ee044d`。
- Applicable version: eHSM 4019 Host API。
- Document evidence: SRC-0014 第 245 页定义 `EHSM_SELF_TEST_SHA256 0x40000`、`EHSM_SELF_TEST_TRNG 0x80000`。
- Code evidence: `source-vault/vendor-code/osr_eshm/ehsm_host-2.3.1-4019-2ee044d/include/ehsmdrv/basic/bl_api.h:34`～`:35` 使用相同定义。
- Expected behavior: bit18 表示 SHA256，bit19 表示 TRNG。

## Exact conflict

BL 文档与 BL 代码一致，Host 文档与 Host 代码一致，但 BL 与 Host 对跨组件接口位图的 bit18/bit19 语义不一致：

| Bit/value | Bootloader | Host API |
|---|---|---|
| bit18 / `0x40000` | TRNG | SHA256 |
| bit19 / `0x80000` | 未定义 | TRNG |

这意味着冲突不是单个文件笔误，而是两个组件及其各自文档之间的协议漂移或版本不匹配。

## Authority comparison

- SRC-0012/SRC-0014 都是 Vendor 正式文档，但分别描述 BL 和 Host API，无法仅按文档类型确定哪一侧正确。
- SRC-0018 的两侧代码与各自文档一致，证明当前交付快照中确实存在接口不一致。
- SRC-0016/SRC-0017 当前没有被核实为规定该位图，因此不能用内部方案直接裁定。
- 项目负责人于2026-07-22确认Vendor回复：Bootloader定义正确，Host定义错误；项目协议基准由ADR-0003裁决。

## Impact

- Security: Host 可能错误解释 TRNG/SHA256 自检结果，进而错误报告算法健康状态；实际影响取决于上层是否按单独 bit 解析或只做整体比较。
- Software: Host 解码、日志、诊断和错误处理可能与 BL 输出不一致。
- Verification: 使用 Host 宏建立的 TRNG/SHA256 测试预期可能误判。
- Manufacturing/operations: 若量产或启动门禁依赖 Host 解析该位图，可能错误放行或错误阻断；当前尚未确认存在该使用方式。
- Schedule: 需要一次 Vendor 协议确认或版本配套说明。

## Resolution

- 当前4019范围以Bootloader定义为准：bit16=SHA2、bit17=SHA3、bit18/`0x40000`=TRNG。
- bit19/`0x80000`在当前Bootloader协议中未定义，保留raw值，不解释为TRNG。
- SRC-0014/Host的独立SHA256=`0x40000`、TRNG=`0x80000`定义对当前配套版本错误，NGU800P Host/adapter不得沿用。
- 原始Vendor回复材料尚待入库；当前裁决通过ADR-0003生效。

## Options

### Option A：以 Bootloader 定义为协议基准

- Action: Host 删除/调整独立 SHA256 位，将 TRNG 改为 `0x40000`。
- Benefits/costs/risks: 与当前 BL 输出及 BL TRM 一致；需要确认 SHA256 是否本来就属于 SHA2 聚合位，且需要修改 Host 及测试。

### Option B：以 Host API 定义为协议基准

- Action: BL 增加独立 SHA256 位并将 TRNG 移到 `0x80000`。
- Benefits/costs/risks: 与 Host API 及 Host TRM 一致；会改变 BL 对外协议和已有结果位图，需要确认兼容性及 ROM/固件更新能力。

### Option C：明确版本映射或兼容转换

- Action: Vendor 给出配套版本和协议版本；Host 根据 BL 版本选择映射或建立兼容层。
- Benefits/costs/risks: 可兼容已发布版本，但增加版本探测和测试矩阵复杂度。

## Recommendation history

先向 Vendor 确认 4019 自检结果位图的正式协议基准，并要求说明：

1. SHA256 是否应作为 SHA2 的一部分，还是确有独立自检位。
2. BL `2.3.5-4019-72f8fdc` 与 Host `2.3.1-4019-2ee044d` 是否为正式配套版本。
3. 量产/启动流程是否按单个 bit 解释结果。

项目负责人取得Vendor回复后采用Option A；结果由ADR-0003记录，并应进入芯片安全软件方案后续有效版本或受控amendment。

## Required owner decision

- 已完成，无剩余协议选择。
- 待办仅为归档Vendor原始回复以及后续Host实现修正。

## Resolution and review history

- 2026-07-21：根据 SRC-0019 的 FINDING-TRNG-05 进行独立核对；代码、Bootloader TRM 第 50 页及 Host API TRM 第 245 页共同确认冲突存在。等待负责人/Vendor 裁决。
- 2026-07-22：项目负责人确认Vendor回复，Bootloader定义正确、Host定义错误；采用Option A，冲突关闭，详见ADR-0003。
