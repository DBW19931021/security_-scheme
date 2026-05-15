# NGU800 安全方案 Code Rules（强化版 V1.0）

状态：当前阶段代码约束文件（已纳入 `SRC-005 管理子系统方案` 增量输入）
适用范围：BootROM / SEC1 / SEC2 / eHSM 适配层 / Mailbox Driver / Host 代理层 / Provisioning Tool
目的：将 `01_constraints.md`、`02_baseline.md`、`10_full_design.md` 的设计结论转成工程开发阶段必须遵守的规则；`04_impl_design` 仅作为已同步编辑分片引用

---

# 1. 使用说明

本文件不是方案说明书，而是**开发铁律**。

适用对象：
- BootROM / SEC FW 开发
- eHSM 适配层开发
- Mailbox Driver 开发
- Host 代理层开发
- 制造灌装工具开发
- 测试与验证团队

规则等级：
- `MUST`：必须遵守
- `MUST NOT`：严禁
- `SHOULD`：强烈建议
- `MAY`：可选

违反本文件的实现，即使“能跑通”，也不能视为符合 NGU800 安全方案。

---

# 2. Root of Trust / Key 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-ROOT-001 | MUST | eHSM / Key Service | Root Secret / Root Key 只能由 eHSM 使用，不得被 Host、普通核、管理核直接读取 | C-ROOT-01 | Baseline 3.1 / eFuse-Key-FW Header | Root of Trust 失效 |
| R-ROOT-002 | MUST NOT | BootROM | BootROM 不得持有、缓存或导出 Root Private Key 材料 | C-ROOT-01 | Baseline 3.1 | 扩大攻击面 |
| R-KEY-001 | MUST NOT | SEC / Host / Driver | 私钥不得导出到 Host、普通核或日志 | C-KEY-01 | Baseline 7 / Key Hierarchy | 密钥泄露 |
| R-KEY-002 | MUST | eHSM 适配层 | 所有正式安全路径密钥使用必须经 eHSM 内部机制完成 | C-KEY-01 / C-IF-01 | Baseline 7 / Mailbox / Key Hierarchy | 绕过安全边界 |
| R-KEY-003 | MUST | Key Service / Provisioning | Key 权限必须受 lifecycle gating 控制 | C-KEY-02 | Baseline 8 / Manufacturing | 生命周期策略失效 |
| R-KEY-004 | MUST | FW Header / Verify Path | secure boot / upgrade 算法 authority 必须来自 eHSM control field；NGU manifest / mailbox 只能记录 expected algorithm profile，不得覆盖 eHSM `SocBootAlg / SocUpgradeAlg` | C-IF-01 / C-BOOT-06 | Baseline 8.1 / eHSM Conformance Matrix | 双算法或 eHSM 策略失效 |

---

# 3. BootROM / 启动链规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-BOOT-001 | MUST NOT | BootROM | BootROM 不得实现复杂签名验证、密钥管理、完整证书链处理 | C-BOOT-03 | Baseline 3.2 | 破坏职责边界 |
| R-BOOT-002 | MUST | BootROM | BootROM 只承担最小加载、编排和受控跳转职责 | C-BOOT-03 | Baseline 3 / 5 | 启动链混乱 |
| R-BOOT-003 | MUST | SEC / C908 | SEC/C908 必须是唯一 boot control plane | C-BOOT-02 | Baseline 3.3 | 可能出现未授权 release |
| R-BOOT-004 | MUST | Verify Path | 所有可执行固件必须在执行前完成验签 | C-BOOT-01 | Baseline 5 / FW Header | 可执行恶意镜像 |
| R-BOOT-005 | MUST NOT | Host / 管理核 | Host、管理核不得直接拉起 MCU 或绕过 SEC release 执行 | C-BOOT-02 / C-HOST-01 | Baseline 6 | 执行放行权失控 |
| R-BOOT-006 | MUST | Verify Path | 反回滚检查必须在执行放行前完成 | C-UPDATE-01 | Baseline 5 / eFuse-Key-FW Header | 回滚攻击 |
| R-BOOT-007 | MUST | Verify Path | SEC2 在正式安全启动路径中必须 sign + encrypt，decrypt failure 必须阻断安全控制面启动 | C-BOOT-05 | Baseline 5 / mailbox_if | 运行期安全控制面泄露或带病启动 |
| R-BOOT-008 | MUST | Product Policy / Verify Path | PM/RAS/Codec 等 runtime image 在 USER/PROD 默认 sign + encrypt；signature-only 必须来自显式 image_type 白名单 | C-BOOT-05 | fw_header / spdm_report | signature-only 变成默认弱路径 |

---

# 4. Mailbox / 接口规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-IF-001 | MUST | SEC / Mailbox Driver | 所有正式安全服务调用必须通过受控 mailbox 或定义好的安全接口 | C-IF-01 / C-ACCESS-01 | Baseline 7 / mailbox_if | 绕过安全服务 |
| R-IF-002 | MUST NOT | Host | Host 不得直接调用 eHSM Mailbox 命令面 | C-HOST-01 / C-ACCESS-01 | Baseline 6 / mailbox_if | Host 直接进入信任面 |
| R-IF-003 | MUST | Mailbox Driver | `INFO[0/1]` 只传包地址/控制信息，完整包体必须走共享内存 | C-IF-01 | mailbox_if 4 / 6 | 协议不一致 |
| R-IF-004 | MUST | SEC | Mailbox 请求必须携带 token，响应必须按 token 匹配，不得按“最近一次请求”猜测配对 | C-IF-01 | mailbox_if 6 | 并发错配 |
| R-IF-005 | MUST | SEC / eHSM | 请求包和响应包的长度字段必须做边界检查 | C-ACCESS-01 | mailbox_if 6 | 越界访问 |
| R-IF-006 | MUST | SEC | 所有地址参数（pkt_addr / dst_addr / scope_bitmap_addr 等）必须先做白名单检查 | C-ACCESS-01 / C-ACCESS-02 | mailbox_if 10 | 越权访问 |
| R-IF-007 | MUST | eHSM | eHSM 侧必须再次做地址范围检查，不得只信任 SEC 传入参数 | C-ACCESS-01 | mailbox_if 10 | 双保险缺失 |
| R-IF-008 | MUST | Mailbox Driver | 首版 CH0 必须可用；若未实现多通道，不得在软件中伪装支持 | C-IF-01 | mailbox_if 5 / 13 | 运行时语义失真 |
| R-IF-009 | SHOULD | Mailbox Driver | 对 `BUSY` 支持有限重试，但对 `VERIFY_FAIL / AUTH_FAIL / INVALID_LCS` 不得盲重试 | C-DEBUG-02 / C-UPDATE-01 | mailbox_if 13 | 安全事件被掩盖 |

---

# 5. Host / PCIe / 外部输入规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-HOST-001 | MUST | Host 代理层 | Host 只能投递镜像、请求服务、读取结果 | C-HOST-01 | Baseline 6 | 角色越界 |
| R-HOST-002 | MUST NOT | Host | Host 不得参与信任链裁决、签名判断、密钥决策 | C-HOST-01 | Baseline 6 | 信任链污染 |
| R-HOST-003 | MUST NOT | Host / Driver | Host 不得直接访问 OTP / Secure SRAM / eHSM 私有资源 | C-ACCESS-01 | Baseline 6 / 7 | 敏感区暴露 |
| R-HOST-004 | MUST | SEC / PCIe path | Host 投递的镜像在进入 verify path 前必须被视为不可信数据 | C-HOST-01 / C-BOOT-01 | Baseline 5 / mailbox_if | 恶意输入直达执行面 |
| R-HOST-005 | MUST | Host / SEC | Host 相关接口必须显式区分“投递成功”和“验签通过”，不得混用状态 | C-BOOT-01 | mailbox_if / fw_header | 状态误判 |

---

# 6. Lifecycle / Debug 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-LCS-001 | MUST | 所有安全服务 | 生命周期状态必须由 eHSM / OTP 状态裁决，软件缓存值仅作提示，不得作最终授权依据 | C-KEY-02 / C-DEBUG-02 | Baseline 8 / mailbox_if | 授权错误 |
| R-LCS-002 | MUST NOT | Debug Path | USER 生命周期下不得开放未授权 JTAG / 内部调试 / debug boot | C-DEBUG-01 | Baseline 8 | 量产调试漏洞 |
| R-LCS-003 | MUST | Debug Path | DEBUG/RMA 调试开启必须经过 challenge-response 或等价鉴权 | C-DEBUG-02 | Baseline 8 / mailbox_if | 调试口被滥用 |
| R-LCS-004 | MUST NOT | Lifecycle Tool | 不得支持 USER 直接回退到开发态 | C-KEY-02 / C-DEBUG-02 | Baseline 8 / mailbox_if | 生命周期失控 |
| R-LCS-005 | MUST | Attestation / Report | lifecycle / debug 状态必须可被证明和导出到证明路径中 | C-ATT-01 / C-DEBUG-02 | spdm_report | 证明信息不完整 |

---

# 7. Board / OOB / Management 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-BOARD-001 | MUST | BMC / OOB / Board MCU Proxy | BMC、OOB、板级 MCU、管理子系统只能作为受控链路或代理，不得进入 Root of Trust | C-BOARD-01 / C-BOARD-02 | Baseline 7 / Board Security | OOB 链路变成隐式安全根 |
| R-BOARD-002 | MUST NOT | BMC / OOB / Board MCU Proxy | OOB 链路不得直接修改 lifecycle、secure boot、debug enable、rollback counter、Root/anchor | C-BOARD-02 | Board Security / Interface | 绕过 SEC/eHSM 控制面 |
| R-BOARD-003 | MUST | JTAG / CPLD / MUX Control | JTAG 打开必须经过 lifecycle 检查、debug auth、scope bitmap、session timeout 和审计 | C-BOARD-03 / C-DEBUG-02 | Board Security / Lifecycle Debug | 量产调试口失控 |
| R-BOARD-004 | MUST NOT | JTAG / CPLD / MUX Control | USER/PROD 生命周期不得存在板级 JTAG 直通或常开路径 | C-BOARD-03 / C-DEBUG-01 | Board Security | 直接访问寄存器/DRAM/Flash/安全子系统 |
| R-BOARD-005 | MUST | DMA / Firewall Driver | 管理子系统 DMA / Host DMA / OOB DMA 对安全资源默认拒绝，只能访问 firewall 白名单 staging/data buffer | C-BOARD-04 / C-ACCESS-02 | Board Security / Interface | DMA 绕过安全隔离 |
| R-BOARD-006 | MUST NOT | DMA / Firewall Driver | 管理子系统 DMA 不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、recovery 区、证书/策略区、measurement_table 安全写区、debug/lifecycle/rollback 控制寄存器 | C-BOARD-04 / C-ACCESS-01 | Board Security / Interface | 敏感资产暴露 |
| R-BOARD-007 | MUST | Power / Reset Control | 影响安全启动、恢复、debug 或证明状态的电源/复位/PowerBrake 事件必须进入安全状态机或审计 | C-BOARD-04 | Board Security / Manufacturing | 安全状态不可解释 |
| R-BOARD-008 | SHOULD | Attestation / Report | board binding / die binding / 关键板级安全状态应进入证明报告或本地审计；V2.4 不默认阻断 SEC1 | C-BOARD-01 / C-ATT-01 / C-ATT-02 | Board Security / SPDM Report | verifier 无法判断板级状态 |
| R-BOARD-009 | MUST NOT | OOB / BMC Proxy | OOB/BMC provisioning proxy 不得成为 trust anchor，不得接触 root secret、device private key 或 FW_KEK 明文 | C-BOARD-02 / C-MFG-01 | Interface / Manufacturing | OOB 链路变成制造信任根 |

---

# 8. Firmware Header / Verify / Anti-Rollback 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-FW-001 | MUST | FW Header / Verify | SEC1/SEC2 的 physical verification header 必须采用 eHSM native secure boot image header，不得使用 NGU 自定义 physical header | C-BOOT-06 | efuse_key_fw_header_design / ehsm_source_conformance_matrix | 两套 physical ABI 冲突 |
| R-FW-002 | MUST NOT | FW Header / Verify | 不得把 `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t` 作为 wire/storage verification format | C-BOOT-06 | efuse_key_fw_header_design | eHSM 工具链和验签语义失配 |
| R-FW-003 | MUST | Verify Path | rollback floor/domain 必须映射到 eHSM Version Counter / monotonic counter / owner-confirmed 等价机制，不得只信任镜像自带版本 | C-UPDATE-01 / C-EHSM-01 | eFuse-Key-FW Header | 软件可伪造版本 |
| R-FW-004 | MUST | Upgrade Path | 升级成功前不得先提升 counter | C-UPDATE-01 | eFuse-Key-FW Header | 设备锁死或升级异常 |
| R-FW-005 | MUST | FW Path | SEC1/SEC2 sign+encrypt 必须走 eHSM verify+decrypt output path，不得使用 NVM only verify | C-BOOT-07 | mailbox_if / eFuse-Key-FW Header | 机密性保护失效 |
| R-FW-006 | MUST | FW Header / Verify Path | eHSM `Image_Type` 必须保持 eHSM TRM 定义；NGU `SEC1/SEC2/runtime` image type 必须放入 manifest / policy table | C-BOOT-06 | eFuse-Key-FW Header | image type 语义冲突 |
| R-FW-007 | MUST | OTP / Key / Counter | `OTP-0..OTP-7`、`*_MIN_VER`、NGU key names 只能作为 logical view / alias，所有 physical mapping 必须进入 source-conformance matrix | C-EHSM-01 | ehsm_source_conformance_matrix | RTL/制造/代码使用错误字段 |
| R-FW-008 | MUST NOT | FW Header / Tooling | per-image CEK / wrapped CEK 不得作为已冻结 physical ABI，除非后续 eHSM customization CR 接受 | C-EHSM-01 | eFuse-Key-FW Header | 实现依赖不存在的 eHSM 能力 |
| R-FW-009 | MUST | Image Packager / Tooling | 平台侧固件制作工具必须生成 eHSM native package，并将 NGU manifest + payload 放入受 eHSM verify/decrypt 保护的 Code region | C-BOOT-08 | 10_full_design 3.10 / 10.3 | 工具产物与设备侧验证路径不一致 |
| R-FW-010 | MUST NOT | Image Packager / Tooling | 不得把 `header + Signed Region + signature + wrapped_cek + enc_payload` 作为 NGU800 最终 wire/storage physical format | C-BOOT-06 / C-BOOT-08 | efuse_key_fw_header_design 4.4 | 恢复旧自定义 header，绕开 eHSM-native 裁决 |
| R-FW-011 | MUST | BootROM / SEC Verify Flow | eHSM PASS 前不得信任或解析 NGU manifest 中的 load/entry/policy 字段；必须先完成 eHSM native verify/decrypt output | C-BOOT-08 | 10_full_design 3.10.7 / 10.3 | 恶意 manifest 影响执行放行 |
| R-FW-012 | SHOULD | Image Packager / CI | image packager 应输出 package manifest dump、source-conformance report、policy check report 和 golden vector，供 BootROM/SEC/eHSM adapter 联调验证 | C-BOOT-08 | efuse_key_fw_header_design 4.4.5 | 工具链与设备侧实现难以审查和复现 |
| R-FW-013 | MUST | Image Packager / eHSM Adapter | SEC1 的最低认证覆盖范围必须包含完整 Code region，即 `NGU protected manifest + SEC1 payload + padding/alignment counted by Code_Size` | C-BOOT-08 | 10_full_design 3.10.5 | manifest 或 payload 可被篡改后仍被 release |
| R-FW-014 | MUST | Image Packager / eHSM Adapter | SEC1 正式安全启动路径必须对完整 Code region 使用 eHSM sign+encrypt profile；header 只能作为 eHSM native plaintext metadata，不得承载未保护的 NGU release 决策 | C-BOOT-04 / C-BOOT-08 | 10_full_design 3.10.5 / 3.10.6 | SEC1 机密性或 release policy 被降级 |
| R-FW-015 | MUST NOT | BootROM / SEC Verify Flow | 若 eHSM native header 中某些字段未被 eHSM 认证/AAD 覆盖，BootROM/SEC 不得将这些字段作为 NGU 项目级 image type、load/entry、lifecycle、measurement 或 release 决策依据 | C-BOOT-08 | 10_full_design 3.10.5 / 10.5.21 | 明文 header 被篡改导致策略绕过 |
| R-FW-016 | MUST | Test / CI | package golden/tamper vector 必须覆盖篡改 manifest `ngu_image_type`、`entry_addr`、`version_counter`、payload 字节、`Code_Size` 或截断 Code region 后不得 release | C-BOOT-08 | 10_full_design 3.10.6 / 10.5.21 | 缺少失败路径验证，工具链和启动实现可能不一致 |

---

# 9. Attestation / SPDM 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-ATT-001 | MUST | Attestation Path | 设备身份私钥不得离开 eHSM | C-ATT-01 | Baseline 10 / spdm_report | 身份不可托管 |
| R-ATT-002 | MUST | Report Builder | report header、identity、measurement、lifecycle/debug、nonce/session 绑定信息必须被签名覆盖 | C-ATT-01 / C-DEBUG-02 | spdm_report | 报告可被拼接/重放 |
| R-ATT-003 | MUST | Verifier Path | measurement 至少覆盖安全启动关键阶段和关键固件版本 | C-ATT-01 / C-BOOT-01 | spdm_report | 证明价值不足 |
| R-ATT-004 | MUST | Dual Algorithm Support | report 结构不得假设只有单一算法栈 | C-ATT-01 / C-IF-01 | spdm_report | 双算法方案失效 |
| R-ATT-005 | MUST | Report Builder | lifecycle、debug_state、secure_boot_state、rollback_state 必须进入 report 并被签名覆盖 | C-ATT-02 | spdm_report | RMA/debug 状态可被伪装 |
| R-ATT-006 | SHOULD | Report Builder | image protection policy、decrypt_applied、board_bind_result 应进入 report 或 measurement flags | C-ATT-02 | spdm_report | verifier 无法识别策略降级或 board 状态 |

---

# 10. 制造 / 灌装 / Provisioning 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-MFG-001 | MUST | Provisioning Tool | Root Key / UDS 注入必须通过制造安全通道完成 | C-MFG-01 | Baseline 8 / manufacturing_provisioning | 制造阶段泄密 |
| R-MFG-002 | MUST | Provisioning Tool | Root Key 写入后必须锁定，不得允许重复覆盖 | C-MFG-01 | manufacturing_provisioning | 根密钥被替换 |
| R-MFG-003 | MUST | MANU→USER 流程 | 进入 USER 前必须清理测试 key / 测试 trust / 测试 debug 路径 | C-MFG-01 / C-DEBUG-01 | manufacturing_provisioning | 测试后门残留 |
| R-MFG-004 | MUST | Provisioning / Audit | 制造阶段必须留存审计记录 | C-MFG-01 | Baseline 8 | 无法追责 / 回溯 |
| R-MFG-005 | SHOULD | Provisioning Tool | 对写入后的 OTP 状态做读回校验或等价校验 | C-MFG-01 | manufacturing_provisioning | 灌装不可验证 |
| R-MFG-006 | MUST | MANU→USER 流程 | USER freeze 必须锁定 SEC1/SEC2 decrypt key / FW_KEK、debug、anti-rollback，并完成 test trust cleanup | C-MFG-01 / C-BOOT-05 | manufacturing_provisioning | 量产冻结不完整 |
| R-MFG-007 | MUST NOT | RMA Tool | RMA 不得 long-open debug，不得绕过 challenge/auth，不得长期保留 SEC1/SEC2 decrypt bypass | C-MFG-01 / C-DEBUG-02 | manufacturing_provisioning | 返修后门残留 |

---

# 11. Driver / FW 编码规范补充

## 11.1 Mailbox Driver
- MUST 提供同步等待接口和超时接口
- MUST 将硬件错误码映射为统一软件错误模型
- MUST 在请求提交前校验长度和地址
- MUST 处理中断清除与重复 doorbell 保护

## 11.2 SEC FW
- MUST 维护 token 管理
- MUST 负责 cache flush / invalidate / barrier
- MUST 区分“Host 投递成功”和“eHSM 验证通过”
- MUST 将 lifecycle / permission 检查前置

## 11.3 Logging
- MUST NOT 打印密钥、私钥、完整 challenge 响应材料
- SHOULD 对 verify fail / auth fail / rollback fail 记录最小必要审计信息
- MUST 区分安全事件日志和普通调试日志

---

# 12. 文档事实源 / 代码落地规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-DOC-001 | MUST | FW / Driver / Tool / Test / Reviewer | 代码实现、评审和测试计划必须以 `security_workflow/03_detailed_design/10_full_design.md` 作为完整详设主入口 | CR-0005 | 10_full_design 第 10 章 | 只读分片导致遗漏字段或采用过期 ABI |
| R-DOC-002 | MUST NOT | FW / Driver / Tool / Test / Reviewer | 不得把 `security_workflow/04_impl_design/*.md` 作为独立事实源覆盖 `10_full_design.md` | CR-0005 | 04_impl_design README / 10_full_design 第 10 章 | 多文档事实源冲突 |
| R-DOC-003 | MUST | Design Maintainer / Codex | 修改 `04_impl_design` 中字段、结构、状态机、命令、错误码、manufacturing/SPDM 细节时，必须同步到 `10_full_design.md` 第 10 章 | CR-0005 | 10_full_design 第 10 章 | 主详设不能指导代码落地 |
| R-DOC-004 | MUST | Design Maintainer / Codex | 若 `10_full_design.md` 与 `04_impl_design` 分片冲突，必须按 accepted CR、decision_log、official TRM、`10_full_design.md` 的优先级修正分片 | CR-0005 | decision_log DEC-0015 | 分片反向污染主设计 |

---

# 13. 当前阶段必须优先落地的规则集

首批实现必须优先满足：

1. `R-DOC-001 ~ R-DOC-004`
2. `R-BOOT-001 ~ R-BOOT-006`
3. `R-IF-001 ~ R-IF-009`
4. `R-HOST-001 ~ R-HOST-005`
5. `R-LCS-001 ~ R-LCS-004`
6. `R-BOARD-001 ~ R-BOARD-008`
7. `R-FW-001 ~ R-FW-016`
8. `R-MFG-001 ~ R-MFG-007`

理由：
这些规则直接决定：
- 是否能形成正确 trust boundary
- 是否能阻止 Host 越权
- 是否会发生未验签执行
- 是否会留下量产 debug 后门
- 是否会通过 OOB / JTAG / DMA 留下板级绕过路径
- 是否能支撑制造灌装闭环
- 是否能保证代码落地只跟随一份完整详设

---

# 14. 结论

本文件已经把当前阶段方案结论转成工程执行规则，后续任何实现应按以下顺序落地：

```text
constraints
→ baseline
→ chapter design
→ impl design shards
→ 10_full_design code landing spec
→ code rules
→ traceability
→ code
```

若后续 `mailbox_if.md / spdm_report.md / fw_header.md / manufacturing_provisioning.md` 更新，必须同步更新 `10_full_design.md` 第 10 章和本文件。
