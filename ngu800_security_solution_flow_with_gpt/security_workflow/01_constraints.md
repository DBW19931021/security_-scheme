# NGU800 安全约束定义（01_constraints.md）

版本：v1.2  
状态：Draft（当前阶段约束收敛版本）  
适用范围：NGU800 / NGU800P 安全子系统 + 启动链路  

---

# 1. 设计目标

本文件定义 NGU800 安全架构的强约束条件，用于：

- 统一架构设计边界
- 约束详细设计输出
- 指导后续代码实现（Codex）
- 支撑安全评审 / 流片冻结

---

# 2. Root of Trust 约束

## 【C-ROOT-01】Root of Trust 必须在 eHSM

- Root Key 必须存储在 eFuse / OTP 安全区中，由 eHSM 使用
- 不允许 BootROM 持有 Root Private Key
- 不允许管理核持有 Root Key

Evidence：
- eHSM 作为芯片信任根，提供安全启动、生命周期、密钥管理、身份认证、固件升级等服务  
- eHSM 通过 OTP/eFuse 接口访问 OTP/eFuse

Decision Rationale：
- Root Secret 一旦扩散到 BootROM 或管理核，会扩大攻击面
- 项目当前基线要求 eHSM 作为安全服务根和首个密码学验证主体

Chapter Binding：
- ch4 / ch5 / ch6 / ch12

Impl Binding：
- efuse_design / key_hierarchy / fw_header / mailbox_if

---

# 3. Secure Boot 约束

## 【C-BOOT-01】所有镜像必须经过安全子系统验签

适用对象：
- SEC 核
- PMP / RMP / OMP / MMP
- 大核相关 FW

要求：
- 所有固件必须在执行前完成验签
- 验签必须由安全子系统执行（C908 + eHSM）
- 未验签固件不得 release 执行

---

## 【C-BOOT-02】Boot 顺序必须由安全核控制

- 所有 MCU reset release 必须由 SEC 核控制
- 不允许 Host 直接拉起 MCU
- 不允许管理核自行启动

---

## 【C-BOOT-03】BootROM 不实现复杂加解密

- BootROM 仅允许：
  - 基础加载
  - 跳转 / 编排
- 禁止：
  - 复杂签名验证
  - 密钥管理

---

# 4. Crypto 约束

## 【C-IF-01】所有密码操作必须走 eHSM

必须：
- HASH
- 签名验证
- 加解密
- Key 派生

禁止：
- 软件实现 SM2/SM3/SM4（或其他正式安全路径算法）
- 普通核直接调用 crypto 引擎

---

# 5. Key Management 约束

## 【C-KEY-01】私钥不可导出

- Private Key 不允许：
  - 被 Host 读取
  - 被普通核访问
- Key 使用必须通过 eHSM 内部机制

---

## 【C-KEY-02】Key 必须绑定生命周期

- Key 使用权限与 lifecycle 强绑定
- 不同生命周期：
  - TEST / DEVE / MANU / USER / DEBUG / DEST
  - Key 权限必须不同

---

# 6. Debug 约束

## 【C-DEBUG-01】USER 态关闭调试能力

USER 生命周期：
- 禁止 JTAG
- 禁止内部总线访问
- 禁止 debug boot

---

## 【C-DEBUG-02】DEBUG / RMA 必须认证

- Debug 开启必须：
  - 鉴权
  - 生命周期允许
  - 可审计

---

# 7. Host 约束

## 【C-HOST-01】Host 不可信

Host 只能：
- 传输 firmware
- 发起请求（mailbox / PCIe）

Host 不允许：
- 参与签名
- 参与 Root of Trust
- 访问密钥
- 直接 release 执行

---

# 8. 访问控制约束

## 【C-ACCESS-01】安全子系统必须隔离

禁止直接访问：
- eHSM
- Secure SRAM
- OTP

访问方式：
- mailbox
- interrupt
- 受控共享内存

---

## 【C-ACCESS-02】必须使用 UserID + Firewall

- 所有 master 必须带 UserID
- 所有访问必须经过 firewall
- 权限必须可配置、可隔离

---

# 9. Board / Management 约束

## 【C-BOARD-01】管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决

来源：
- `SRC-005` 管理子系统方案

要求：
- 管理子系统文档中的总体架构、模块职责、带外管理链路、电源/复位流程、单/双 Die 约束原则上作为系统级流程输入。
- 涉及 Root、debug、JTAG、secure boot、lifecycle、provisioning、firmware update、secure memory、OTP/eFuse、security subsystem 访问的内容，必须以安全基线为准。
- 若管理子系统文档中存在未鉴权调试、越权访问、绕过 SEC/eHSM、绕过 lifecycle gating 或直接访问安全资产的设计，不能直接继承，必须在详设中列为风险并给出替代设计。

Evidence：
- `SRC-005` 描述了 BMC、OAM 模组、板级 MCU/GPU 之间的 SMBus/I2C、I3C、SPI、PCIe、JTAG、UART、电源/复位管理和管理子系统整体逻辑。
- `SRC-005` 对 JTAG 的描述包括可接入 GPU 芯片 JTAGBUS、寄存器空间、DRAM、Flash、安全子系统和 CPU 调试单元。

Decision Rationale：
- 管理子系统属于系统流程和板级集成的重要输入，但其带外通道和调试能力具备高权限，不能天然视为安全可信通道。
- JTAG、DMA、Flash 更新、电源复位和 OOB 链路若不受 lifecycle/debug auth/firewall 约束，会绕过现有安全启动和密钥边界。

Chapter Binding：
- ch4 / ch6 / ch10 / ch11 / ch13 / ch14

Impl Binding：
- mailbox_if / spdm_report / manufacturing_provisioning / firewall_access_rules

---

## 【C-BOARD-02】带外管理通道不得成为安全策略绕过路径

来源：
- `SRC-005` 管理子系统方案

要求：
- SMBus/I2C、I3C、PCIe VDM、SPI、UART、BMC/OOB/板级 MCU 链路只能作为受控管理或转发通道。
- 带外管理通道不得直接修改 lifecycle、secure boot、debug enable、Root/anchor、rollback counter、provisioning 状态。
- 带外管理通道若承载 firmware update、状态查询、power/reset、debug request 或 provisioning proxy，必须经 SEC/C908 收敛，并受地址白名单、命令白名单、lifecycle gating 和审计约束。

Evidence：
- `SRC-005` 明确带外管理通道支持 SMBus/I2C、I3C、JTAG，且存在 BMC、OAM 模组、模组 MCU、GPU 和板级 MCU/GPU 之间的多条链路。

Decision Rationale：
- OOB 链路物理上独立、权限高、部署复杂，若作为安全服务直接入口，会破坏 Host 不可信和 SEC 统一控制面的基线。

Chapter Binding：
- ch10 / ch11 / ch12 / ch14

Impl Binding：
- mailbox_if / firewall_access_rules / audit_log

---

## 【C-BOARD-03】JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制

来源：
- `SRC-005` 管理子系统方案

要求：
- USER/PROD 生命周期默认关闭 JTAG 和等价调试访问。
- 任何 JTAG 接入 GPU、CPU、DRAM、Flash、安全子系统或板级 MCU 的能力，必须先通过 challenge-response / debug auth。
- JTAG MUX / CPLD / 板级控制单元不得提供绕过 eHSM debug authorization 的直通路径。
- 授权结果必须包含 scope、目标、时限和审计记录。

Evidence：
- `SRC-005` 描述 JTAG 可接入 BMC、UBB、OAM、板级 MCU/GPU，并可访问 GPU 芯片 JTAGBUS、所有寄存器空间和 DRAM，也可接入安全子系统、CPU 调试单元、GPU Flash 和板级 MCU。

Decision Rationale：
- JTAG 是最高风险板级入口之一，若在量产态未被强制关断或受控授权，会直接绕过 secure boot、内存隔离、密钥和生命周期保护。

Chapter Binding：
- ch9 / ch10 / ch11 / ch14

Impl Binding：
- lifecycle_control / debug_auth / firewall_access_rules / audit_log

---

## 【C-BOARD-04】管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须被隔离和审计

来源：
- `SRC-005` 管理子系统方案

要求：
- 管理子系统 DMA 只能访问被 firewall 白名单允许的普通 buffer，不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、证书/策略区和 recovery 区。
- 管理子系统 mailbox、中断、互斥寄存器只能用于普通协作或经 SEC 收敛后的安全服务请求，不得作为直接安全服务入口。
- 电源、上下电、复位、PowerBrake 等板级控制信号若影响安全启动或故障恢复，必须进入安全状态机和审计模型。

Evidence：
- `SRC-005` 描述 CPU 子系统通用 DMA、mailbox 中断、互斥访问机制、电源管理接口、上下电和复位管理。

Decision Rationale：
- DMA、复位和中断可改变系统状态或数据路径，若缺少隔离与审计，会破坏安全启动、证明状态和运行态可信边界。

Chapter Binding：
- ch6 / ch10 / ch11 / ch12 / ch13

Impl Binding：
- mailbox_if / firewall_access_rules / spdm_report / audit_log

---

# 10. Firmware 更新约束

## 【C-UPDATE-01】必须支持防回滚

- 固件版本必须受控
- 必须具备 anti-rollback 机制
- 反回滚计数必须落到 OTP / monotonic counter 体系，而不是仅软件字段

---

## 【C-UPDATE-02】必须支持受控升级 / 恢复

- 必须定义升级路径
- 必须定义失败恢复策略
- A/B 是否启用可在后续实现中裁决，但恢复机制不能缺失

---

# 11. Attestation 约束

## 【C-ATT-01】必须支持设备认证

- 支持设备身份
- 支持测量值输出
- 支持远程证明（SPDM）
- 私钥不得离开 eHSM

---

# 12. Manufacturing / Provisioning 约束

## 【C-MFG-01】必须定义 Root Key 灌装与锁定流程

- 必须定义制造 / 灌装 / 锁定 / 审计流程
- 必须定义 MANU → USER 的冻结动作
- 不得只写“后续补充”

---

# 13. 待补充（后续自动收敛）

- PCIe 安全模型细化
- SPDM report 字段级定义
- mailbox command ID 最终分配
- board binding 是否量产默认开启
- JTAG scope bitmap、CPLD/MUX 控制权和板级调试授权闭环
- 管理子系统 DMA / mailbox / 复位控制的 firewall 和审计字段
