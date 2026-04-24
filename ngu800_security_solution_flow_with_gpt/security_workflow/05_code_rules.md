# NGU800 安全方案 Code Rules（强化版 V1.0）

状态：当前阶段代码约束文件  
适用范围：BootROM / SEC1 / SEC2 / eHSM 适配层 / Mailbox Driver / Host 代理层 / Provisioning Tool  
目的：将 `01_constraints.md`、`02_baseline.md`、`04_impl_design/*.md` 的设计结论转成工程开发阶段必须遵守的规则

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
| R-KEY-004 | MUST | FW Header / Verify Path | `algo_family / hash_algo / sig_algo / enc_algo` 必须由镜像头明确表达，不得写死在代码里 | C-IF-01 / C-ATT-01 | Baseline 9 / FW Header | 双算法失效 |

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

# 7. Firmware Header / Verify / Anti-Rollback 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-FW-001 | MUST | FW Header / Verify | `load_addr / entry_point / image_version / algo_family` 必须进入 signed region | C-BOOT-01 / C-UPDATE-01 | fw_header | 关键执行语义可被篡改 |
| R-FW-002 | MUST NOT | Verify Path | 不得只验证 payload 而忽略执行属性字段 | C-BOOT-01 | fw_header | 签名覆盖不完整 |
| R-FW-003 | MUST | Verify Path | rollback floor 必须来自 OTP / monotonic counter，不得只信任镜像自带版本 | C-UPDATE-01 | eFuse-Key-FW Header | 软件可伪造版本 |
| R-FW-004 | MUST | Upgrade Path | 升级成功前不得先提升 counter | C-UPDATE-01 | eFuse-Key-FW Header | 设备锁死或升级异常 |
| R-FW-005 | SHOULD | FW Path | 镜像机密性若启用，必须通过 wrapped key / 受控 CEK 路径，不得明文散布 CEK | C-IF-01 | fw_header / key_hierarchy | 密钥暴露 |

---

# 8. Attestation / SPDM 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-ATT-001 | MUST | Attestation Path | 设备身份私钥不得离开 eHSM | C-ATT-01 | Baseline 10 / spdm_report | 身份不可托管 |
| R-ATT-002 | MUST | Report Builder | report header、identity、measurement、lifecycle/debug、nonce/session 绑定信息必须被签名覆盖 | C-ATT-01 / C-DEBUG-02 | spdm_report | 报告可被拼接/重放 |
| R-ATT-003 | MUST | Verifier Path | measurement 至少覆盖安全启动关键阶段和关键固件版本 | C-ATT-01 / C-BOOT-01 | spdm_report | 证明价值不足 |
| R-ATT-004 | MUST | Dual Algorithm Support | report 结构不得假设只有单一算法栈 | C-ATT-01 / C-IF-01 | spdm_report | 双算法方案失效 |

---

# 9. 制造 / 灌装 / Provisioning 规则

| Rule ID | Level | Applies To | Rule Statement | Source Constraint | Baseline / Impl | Violation Impact |
|---|---|---|---|---|---|---|
| R-MFG-001 | MUST | Provisioning Tool | Root Key / UDS 注入必须通过制造安全通道完成 | C-MFG-01 | Baseline 8 / manufacturing_provisioning | 制造阶段泄密 |
| R-MFG-002 | MUST | Provisioning Tool | Root Key 写入后必须锁定，不得允许重复覆盖 | C-MFG-01 | manufacturing_provisioning | 根密钥被替换 |
| R-MFG-003 | MUST | MANU→USER 流程 | 进入 USER 前必须清理测试 key / 测试 trust / 测试 debug 路径 | C-MFG-01 / C-DEBUG-01 | manufacturing_provisioning | 测试后门残留 |
| R-MFG-004 | MUST | Provisioning / Audit | 制造阶段必须留存审计记录 | C-MFG-01 | Baseline 8 | 无法追责 / 回溯 |
| R-MFG-005 | SHOULD | Provisioning Tool | 对写入后的 OTP 状态做读回校验或等价校验 | C-MFG-01 | manufacturing_provisioning | 灌装不可验证 |

---

# 10. Driver / FW 编码规范补充

## 10.1 Mailbox Driver
- MUST 提供同步等待接口和超时接口
- MUST 将硬件错误码映射为统一软件错误模型
- MUST 在请求提交前校验长度和地址
- MUST 处理中断清除与重复 doorbell 保护

## 10.2 SEC FW
- MUST 维护 token 管理
- MUST 负责 cache flush / invalidate / barrier
- MUST 区分“Host 投递成功”和“eHSM 验证通过”
- MUST 将 lifecycle / permission 检查前置

## 10.3 Logging
- MUST NOT 打印密钥、私钥、完整 challenge 响应材料
- SHOULD 对 verify fail / auth fail / rollback fail 记录最小必要审计信息
- MUST 区分安全事件日志和普通调试日志

---

# 11. 当前阶段必须优先落地的规则集

首批实现必须优先满足：

1. `R-BOOT-001 ~ R-BOOT-006`
2. `R-IF-001 ~ R-IF-009`
3. `R-HOST-001 ~ R-HOST-005`
4. `R-LCS-001 ~ R-LCS-004`
5. `R-FW-001 ~ R-FW-004`
6. `R-MFG-001 ~ R-MFG-004`

理由：
这些规则直接决定：
- 是否能形成正确 trust boundary
- 是否能阻止 Host 越权
- 是否会发生未验签执行
- 是否会留下量产 debug 后门
- 是否能支撑制造灌装闭环

---

# 12. 结论

本文件已经把当前阶段方案结论转成工程执行规则，后续任何实现应按以下顺序落地：

```text
constraints
→ baseline
→ chapter design
→ impl design
→ code rules
→ traceability
→ code
```

若后续 `mailbox_if.md / spdm_report.md / fw_header.md / manufacturing_provisioning.md` 更新，必须同步更新本文件。
