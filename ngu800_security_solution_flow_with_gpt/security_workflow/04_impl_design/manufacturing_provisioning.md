# NGU800 制造 / 灌装 / Provisioning / RMA 实现级设计（V1.0）

状态：实现级详设  
适用范围：NGU800 / NGU800P 制造、灌装、板级 bring-up、量产冻结、返修/RMA  
定位：供安全架构评审、SEC/C908 FW、eHSM 适配层、制造工站、Provisioning Tool、测试团队对齐使用

---

# 1. 设计目标

本文档用于把 NGU800 当前方案中的制造与灌装链路，收敛到**流程级 / 字段级 / 接口级 / 审计级**，重点明确：

1. 制造阶段和生命周期阶段的对应关系
2. Root Secret / Root Key / signer hash / debug anchor 的灌装对象与顺序
3. OTP / eFuse 写入、锁定、校验、审计要求
4. MANU → USER 的冻结动作
5. RMA / DEBUG 的授权与恢复规则
6. 工站、SEC/C908、eHSM、Host/BMC 在制造阶段的职责边界
7. 与 `efuse_key_fw_header_design.md`、`mailbox_if.md`、`spdm_report.md` 的对接关系

本文档不是制造作业指导书本身，而是 NGU800 项目实现层的**制造安全控制基线**。

---

# 2. 设计输入与项目裁决

## 2.1 当前项目已采用的事实

当前项目已经明确或基本收敛的事实包括：

- eHSM 是安全服务根和首个密码学验证主体
- Root Secret / Root Key 不应离开 eHSM 使用域
- 生命周期必须受 OTP / eFuse 与 eHSM 联合控制
- USER 生命周期必须关闭未授权 debug
- Host 不进入信任链，只能作为受控投递方
- 制造阶段必须定义 key 注入、锁定、审计和生命周期推进
- 方案必须同时考虑国密与国际算法双栈

## 2.2 项目裁决

当前项目对制造链路采用以下裁决：

> **制造工站通过受控 Provisioning 路径与 SEC/C908 交互，由 SEC 调 eHSM 完成 OTP / eFuse 写入与状态变更。**

即：
- 工站 / Provisioning Tool 不直接操作 Root of Trust 决策逻辑
- 工站不应直接触达 eHSM 私有执行面
- Root Secret / Root Key / 调试锚点 / signer anchor 的最终写入与锁定由 SEC + eHSM 路径完成
- MANU → USER 的冻结动作必须是原子化、可审计的步骤集合，而不是人工口头流程

---

# 3. 生命周期与制造阶段映射

## 3.1 生命周期总体映射

| 生命周期 | 制造阶段语义 | 目标 | 默认安全策略 |
|---|---|---|---|
| TEST | 晶圆 / 封测 / 初始 bring-up | 验证硬件基本功能 | 可开放测试路径，不得等价于量产 |
| DEVE | 开发板 / EVB 调试 | 软件 bring-up / 调试 | 可有限开放 debug |
| MANU | 正式制造 / 板级生产 / 工站灌装 | 注入根材料、配置控制位、建立量产基础 | 启用基础安全校验 |
| USER | 量产交付 | 面向客户交付 | 关闭未授权 debug、锁定根材料 |
| DEBUG / RMA | 返修 / 厂商授权分析 | 故障定位与返修 | 仅限授权开启 |
| DEST | 销毁 | 退役 / 数据清除 | 不再允许正常启动 |

## 3.2 制造阶段建议分解

为方便工程落地，建议把制造过程进一步分解为：

| 阶段 ID | 阶段名称 | 主要动作 |
|---|---|---|
| MFG-0 | 裸片 / 封测初测 | 测试路径、基础检查、非量产 trust |
| MFG-1 | 板级 bring-up | 板卡电源、时钟、接口连通性验证 |
| MFG-2 | 安全灌装准备 | 建立工站认证、选择算法栈、装载 provisioning agent |
| MFG-3 | Root Material Provisioning | 注入 Root / UDS / anchor / counter 初值 |
| MFG-4 | 安全控制位配置 | 写 secure boot / debug / attestation / anti-rollback 控制位 |
| MFG-5 | 校验与锁定 | 写入结果校验、锁位、审计归档 |
| MFG-6 | MANU 验证启动 | 在 MANU 策略下进行带验证启动 |
| MFG-7 | USER 冻结 | 清除测试 trust、推进 USER、关闭未授权 debug |
| MFG-8 | 出厂验收 | 生成验收报告、归档审计记录 |

---

# 4. 角色与职责边界

## 4.1 角色分工

| 角色 | 职责 | 不允许做的事 |
|---|---|---|
| Provisioning Tool / 工站 | 组织灌装步骤、提交请求、记录审计、接收结果 | 直接持有设备证明私钥、直接控制 eHSM 内部策略 |
| Host / BMC（制造场景） | 作为链路承载方、传输工站请求、获取状态 | 参与 Root of Trust 决策、直接写 Root 密钥到最终安全区 |
| SEC / C908 | 唯一 provisioning 控制面；参数校验；流程编排；调用 eHSM；状态收敛 | 绕过 eHSM 直接完成正式安全路径密钥使用 |
| eHSM | OTP/eFuse 写入控制、锁定、lifecycle、counter、debug auth、key 服务执行 | 接受非 SEC 的非受控制造命令 |
| OTP / eFuse | 持久保存生命周期、控制位、Root 材料、signer hash、counter | 被 Host 或普通核直接改写 |

## 4.2 强边界规则

1. Provisioning Tool 可以发起“灌装动作”，但不能直接触达 eHSM 内部寄存器语义。
2. Host/BMC 在制造阶段仍然不进入信任链，只是链路承载者。
3. SEC/C908 必须是唯一 provisioning caller。
4. eHSM 必须是唯一 Root 材料写入与锁定执行者。
5. OTP / eFuse 的最终控制位不得通过普通非安全路径直接改写。

---

# 5. 制造对象清单

## 5.1 必须灌装对象

| 对象 | 是否必需 | 说明 |
|---|---|---|
| UDS / Root Secret | 必需 | 根种子 / 根材料 |
| Root Key / Root KEK 材料 | 必需 | 可直接写入或由 UDS 派生 |
| FW signer hash / trust anchor | 必需 | 支撑固件验签 |
| Debug auth anchor | 必需 | 支撑 RMA / DEBUG 调试鉴权 |
| Attestation anchor / identity seed | 必需 | 支撑设备证明 |
| 版本计数初值 | 必需 | 支撑 anti-rollback |
| Secure boot / debug / attestation 控制位 | 必需 | 建立量产策略 |
| Board binding 信息 | 可选 | 按产品策略启用 |
| 主 / 从 Die binding 信息 | 双Die 推荐 | 支撑多Die一致性约束 |

## 5.2 不允许在制造阶段永久保留的对象

| 对象 | 原因 |
|---|---|
| 测试 signer key / 测试证书锚点 | 进入 USER 前必须清除 |
| 测试 debug 白名单 | 进入 USER 前必须清除 |
| 非量产 secure boot bypass 配置 | 进入 USER 前必须关闭 |
| 明文导出的 Root 私钥 | 根本不允许存在于最终流程 |

---

# 6. OTP / eFuse 灌装内容与顺序

## 6.1 推荐灌装顺序

建议顺序如下：

```text
(1) 读取当前生命周期和 OTP 状态
    ↓
(2) 验证设备处于允许灌装状态（MANU 或受控 provisioning 态）
    ↓
(3) 写入 UDS / Root Secret / Root Key 材料
    ↓
(4) 写入 FW signer hash / trust anchor
    ↓
(5) 写入 debug auth anchor
    ↓
(6) 写入 attestation seed / anchor
    ↓
(7) 写入 anti-rollback 初始计数 / 最低版本门限
    ↓
(8) 写入 secure boot / debug / attestation / algorithm 控制位
    ↓
(9) 读回校验或等价校验
    ↓
(10) 锁定 key / 控制位 / 生命周期回退路径
    ↓
(11) 执行 MANU 验证启动
    ↓
(12) 执行 USER 冻结
```

## 6.2 为什么不能乱序

- Root 材料必须先于 signer / attestation 生效，否则后续校验没有可信根。
- counter 初值必须在正式启动前建立，否则 anti-rollback 没有约束基线。
- 控制位必须在写入关键 anchor 后再打开，避免系统处于“要求安全启动但尚未具备信任锚”的中间态。
- 锁定位必须在校验通过后再写，避免把错误内容永久锁死。

---

# 7. Provisioning 命令模型

## 7.1 建议与 Mailbox 对接

制造阶段不建议引入完全独立的新链路，而建议复用受控 Mailbox / Shared Memory 模型。

对应 `mailbox_if.md` 中建议命令：

- `PROVISION_ROOT_MATERIAL`
- `CHANGE_LIFECYCLE`
- `READ_COUNTER`
- `INCREASE_COUNTER`（制造初始写入时按策略使用）
- `GET_CHALLENGE`
- `DEBUG_AUTH`（仅 RMA/DEBUG 场景）

## 7.2 Provisioning 请求结构建议

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t provision_type;        /* ROOT / SIGNER_HASH / DEBUG_ANCHOR / ATTEST / CTRL_BITS */
    uint32_t algo_family;           /* GM / INTL */
    uint32_t write_flags;           /* write / verify / lock / advance_lcs */
    uint64_t blob_addr;             /* 写入包地址 */
    uint32_t blob_len;              /* 写入包长度 */
    uint32_t target_slot;           /* 目标 slot / OTP region */
} ngu_mb_provision_req_t;
```

## 7.3 Provisioning 响应结构建议

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t provision_type;
    uint32_t write_result;          /* success / partial / verify_fail */
    uint32_t lock_result;           /* 0/1 */
    uint32_t lcs_after;             /* 若发生生命周期推进 */
} ngu_mb_provision_resp_t;
```

## 7.4 关键约束

- `write_flags` 不得允许任意组合；必须由 SEC 侧先做合法性白名单检查。
- `target_slot` 必须映射到项目冻结的 OTP 区域语义。
- `blob_addr/blob_len` 必须满足共享内存白名单和长度边界检查。
- provisioning 命令必须只允许在受控 lifecycle 下执行。

---

# 8. Root Material Provisioning 细化

## 8.1 支持两种注入模式

### 模式 A：直接写入根材料
- 工站侧准备 Root Secret / Root KEK 材料
- 通过受控通道写入 OTP/eFuse 安全区
- 适合工厂中心化生成密钥模型

### 模式 B：写入种子 / 设备标识后由 eHSM 内部派生
- 工站只写入 UDS / seed / identity seed
- Root 派生由 eHSM 内部完成
- 更利于减少明文根材料在工站流转

## 8.2 当前项目建议

当前优先建议：

> **优先采用“Seed / UDS 注入 + eHSM 内部派生”的模式。**

原因：
1. 更符合“私钥不出 eHSM”的长期方向
2. 减少制造链路中的明文高敏根材料暴露
3. 更利于后续 attestation / key hierarchy 一致化

若项目现实约束要求直接写入 Root 材料，也必须满足：
- 工站 HSM/KMS 保护
- 不落明文磁盘
- 写入后立即锁定
- 全流程审计

---

# 9. 控制位配置策略

## 9.1 必须配置的控制位

| 控制位 | 建议 USER 前状态 | 说明 |
|---|---|---|
| `SECURE_BOOT_EN` | 1 | 量产态强制启用 |
| `DEBUG_AUTH_EN` | 1 | 调试必须鉴权 |
| `JTAG_FORCE_DISABLE` | 1 | USER 默认关闭 JTAG |
| `FW_ENCRYPT_EN` | 视产品策略 | 若启用机密性则置 1 |
| `ATTEST_EN` | 1 | 启用设备证明 |
| `ANTI_ROLLBACK_EN` | 1 | 启用反回滚 |
| `DUAL_ALGO_EN` | 1 | 允许双算法栈共存 |

## 9.2 控制位写入原则

- 控制位写入必须晚于根材料 / signer anchor 写入
- USER 前必须确保控制位与实际信任锚一致
- 不得出现“开启 secure boot，但信任锚还未完成注入”的中间状态

---

# 10. 校验与锁定策略

## 10.1 校验策略

推荐按以下顺序校验：

1. 校验写入命令执行返回状态
2. 读回校验（若策略允许）
3. 若不可读回，则通过 eHSM 内部状态校验 / 试运行校验
4. 进行一轮 MANU 策略下的验证启动
5. 校验 debug / lifecycle / attestation / rollback 状态是否符合预期

## 10.2 锁定对象

| 对象 | 锁定时机 | 说明 |
|---|---|---|
| Root Secret / Root Key 区 | 灌装校验通过后 | 防止重复覆盖 |
| signer hash / anchor 区 | 校验通过后 | 防止验签根被替换 |
| debug anchor 区 | 校验通过后 | 防止调试授权根被替换 |
| 控制位区 | USER 冻结前 | 防止量产策略回退 |
| lifecycle 回退路径 | USER 推进后 | 防止回退到开发态 |

## 10.3 锁定规则

- 锁定必须是 provisioning 流程中的显式步骤，不允许“假设已经锁定”
- 锁定动作本身必须被审计记录
- 若锁定失败，不得继续推进 USER 生命周期

---

# 11. MANU 验证启动

## 11.1 目标

在进入 USER 前，必须先在 MANU 策略下完成一次“接近量产条件”的验证启动，用于确认：

- secure boot 可正常工作
- signer anchor 可正确校验
- anti-rollback 路径可正常读取
- mailbox / verify / counter 基本命令可工作
- attestation 基本能力可用

## 11.2 最小验证项

| 验证项 | 说明 |
|---|---|
| 验签 SEC1 / SEC2 | 核心启动链验证 |
| 读取 counter / version floor | 反回滚链路验证 |
| 读取 lifecycle | 生命周期状态验证 |
| 生成 challenge / 或最小 report | 证明路径基本可用 |
| debug 默认策略检查 | 验证未授权 debug 未被放开 |

---

# 12. MANU → USER 冻结动作

## 12.1 必须冻结的动作集合

进入 USER 前，必须完成以下动作：

1. `SECURE_BOOT_EN = 1`
2. `DEBUG_AUTH_EN = 1`
3. `JTAG_FORCE_DISABLE = 1`
4. `ANTI_ROLLBACK_EN = 1`
5. Root Key / UDS / signer anchor 完成锁定
6. 测试 signer / 测试证书链 / 测试调试白名单全部清除
7. 如启用 attestation，则 `ATTEST_EN = 1`
8. 将生命周期推进到 USER
9. 锁定生命周期回退路径
10. 生成冻结完成审计记录

## 12.2 原子性要求

这些动作在工程实现上不一定要单条命令原子完成，但在**流程语义上必须被当成一个事务性步骤集合**处理：

- 任何一步失败，都不得报告“已完成 USER 冻结”
- 出错后必须进入可恢复的 MANU 故障处理分支
- 不得进入“部分已冻结、部分未冻结”的不明状态

---

# 13. Audit / 审计模型

## 13.1 审计事件建议

| Audit Event | 必须记录内容 |
|---|---|
| PROVISION_START | 设备 ID、工站 ID、时间、操作员 |
| ROOT_WRITE | 写入对象类型、slot、结果 |
| ANCHOR_WRITE | signer/debug/attest anchor 类型、结果 |
| CTRL_BITS_WRITE | 控制位变化前后、结果 |
| LOCK_APPLY | 锁定对象、结果 |
| MANU_BOOT_VERIFY | 验证启动结果 |
| USER_FREEZE | 生命周期变化前后、结果 |
| PROVISION_END | 总结果、失败码、日志索引 |

## 13.2 审计要求

- 审计日志不得包含明文 Root 私钥材料
- 可以记录 hash / ID / slot / result，但不能记录敏感明文
- 审计日志至少需可关联：
  - 设备
  - 工站
  - 时间
  - 操作员 / 工单
  - 结果

---

# 14. RMA / DEBUG 流程

## 14.1 基本原则

RMA / DEBUG 不是普通制造路径，而是**受授权的返修分析路径**。

必须满足：

1. 不直接破坏 USER trust state
2. 必须先经过授权
3. 必须记录审计日志
4. 必须在维修后恢复量产安全状态

## 14.2 建议流程

```text
接收返修设备
    ↓
校验工单 / 设备身份 / 厂商授权
    ↓
执行 challenge / debug auth
    ↓
按策略打开受限 debug 能力
    ↓
读取故障信息 / 维修 / 刷写恢复
    ↓
重新写回量产镜像
    ↓
恢复 USER 安全状态
    ↓
记录 RMA 审计结案
```

## 14.3 RMA 约束

- 不得因为进入 RMA 就默认长期开放 debug
- 不得跳过 challenge / auth
- 不得允许返修后继续带测试 trust 出厂

---

# 15. 与其他实现文件的对接

## 15.1 与 `efuse_key_fw_header_design.md`
- 该文件定义“灌什么”
- 本文件定义“怎么灌、何时锁、何时推进生命周期”

## 15.2 与 `mailbox_if.md`
- 该文件定义 provisioning 命令如何走 SEC → eHSM
- 本文件定义这些命令在哪些阶段被允许执行

## 15.3 与 `spdm_report.md`
- 该文件定义 report 如何表达 lifecycle/debug/board 状态
- 本文件定义这些状态在制造 / USER / RMA 阶段如何变化

## 15.4 与 `05_code_rules.md`
- 本文件的流程冻结点必须转化为 MUST / MUST NOT 开发规则

---

# 16. RTL / FW / Tool 影响

## 16.1 RTL 侧
- 需要支持 OTP/eFuse 写入控制位与锁位语义
- 需要支持生命周期状态持久化与回退限制
- 需要为 provisioning 命令保留合法状态机支撑

## 16.2 SEC FW 侧
- 需要实现 provisioning state machine
- 需要实现写入顺序控制、失败回滚、阶段状态记录
- 需要实现 MANU 验证启动和 USER 冻结动作编排

## 16.3 eHSM 适配层
- 需要支持 Root / anchor / counter / lifecycle / debug auth 的命令封装
- 需要支持写入后校验和锁定状态读取

## 16.4 Provisioning Tool
- 需要实现设备识别、工站认证、blob 管理、日志审计
- 需要显式区分“写入成功”“锁定成功”“USER 冻结完成”

---

# 17. 当前冻结建议

当前建议优先冻结以下内容：

1. 制造阶段推荐顺序（MFG-0 ~ MFG-8）
2. Root / signer / debug / attestation / counter / ctrl_bits 的灌装顺序
3. MANU 验证启动的最小检查项
4. USER 冻结动作集合
5. 审计事件最小集合
6. RMA 需先 challenge / auth，再开受限 debug

---

# 18. 开放问题

1. Root Material 首版是否完全采用 seed/UDS 注入，而非直接 Root Key 注入
2. OTP/eFuse 是否支持对部分区做读回校验，哪些区仅支持状态校验
3. Provisioning Tool 与 SEC 的承载链路最终经由 PCIe、BMC 还是独立工装接口
4. 双Die 产品的主 / 从 Die 灌装是独立还是联动事务
5. USER 冻结失败时，允许停留在 MANU，还是进入显式故障态
6. RMA 完成后恢复 USER 状态时，是否强制重新生成 attestation 相关状态摘要

---

# 19. 结论

本文件已经把 NGU800 的制造 / 灌装 / Provisioning 路径推进到实现级设计，明确了：

- 生命周期与制造阶段的映射
- 灌装对象、顺序、校验和锁定策略
- MANU 验证启动与 USER 冻结动作
- RMA / DEBUG 的授权与恢复规则
- 审计模型和工程职责边界

到此为止，NGU800 当前阶段的“方案 → 实现”主链已经具备：

```text
约束
→ baseline
→ 实现级 eFuse / Key / FW Header
→ 实现级 Mailbox
→ 实现级 SPDM Report
→ 实现级 Manufacturing / Provisioning
→ Code Rules
→ Traceability
```

后续应继续把这些实现级设计反填充进章节级详设，并同步约束后续代码开发与测试验证。
