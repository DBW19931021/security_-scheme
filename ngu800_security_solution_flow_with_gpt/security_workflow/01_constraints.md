# NGU800 安全约束定义（01_constraints.md）

版本：v1.1  
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

# 9. Firmware 更新约束

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

# 10. Attestation 约束

## 【C-ATT-01】必须支持设备认证

- 支持设备身份
- 支持测量值输出
- 支持远程证明（SPDM）
- 私钥不得离开 eHSM

---

# 11. Manufacturing / Provisioning 约束

## 【C-MFG-01】必须定义 Root Key 灌装与锁定流程

- 必须定义制造 / 灌装 / 锁定 / 审计流程
- 必须定义 MANU → USER 的冻结动作
- 不得只写“后续补充”

---

# 12. 待补充（后续自动收敛）

- PCIe 安全模型细化
- SPDM report 字段级定义
- mailbox command ID 最终分配
- board binding 是否量产默认开启
