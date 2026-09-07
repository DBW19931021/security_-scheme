# Device Personalization规范增量

## ADDED Requirements

### Requirement: OTP Table 34与16-slot单一方案基线

NGU800P软件、Provisioning配置和测试派生物MUST以SRC-0024及ADR-0025/0026为唯一OTP方案基线。eHSM内部OTP基址、Table 34字段、物理Key ID、Level、截图Key类型和已给权限SHALL保持一致；baremetal不得建立独立槽序。

#### Scenario: 生成方案与测试registry

- **WHEN** 生成Provisioning registry、baremetal manifest或fixture字段表
- **THEN** 生成器MUST逐行使用SRC-0024的slot 0～15和Table 34公式
- **AND** slot 3～5 MUST为Level 1，slot 8/13/14 MUST保留七项权限
- **AND** 任何差异MUST阻断生成，不得选择“方案版”或“baremetal版”之一继续

#### Scenario: 使用未冻结字段

- **WHEN** slot 2类型、具体算法、Key Attribute、CRC、ECC、锁位、backend或材料引用尚未获批
- **THEN** 对应字段MUST保持待绑定或`INCONCLUSIVE/BLOCKED`
- **AND** 实现MUST NOT从Vendor demo、当前eHSM旧map或测试读回值反向生成方案值

### Requirement: Device-unique RTL secrets

量产RTL Root和Install KEK MUST按die唯一且只供硬件使用；共享Vendor默认宏MUST NOT作为量产值。KMS记录设备与Key句柄绑定，日志MUST NOT包含秘密。

#### Scenario: 建立设备个性化记录

- **WHEN** Secure ATE完成一台设备的RTL个性化
- **THEN** Evidence MUST包含设备绑定、KMS句柄、lock和operation proof
- **AND** CPU、Debug、Scan或普通软件MUST NOT读回明文RTL Key

### Requirement: 分层灌装与不可逆动作

设备MUST在DEV LCS依次完成Chip Root、slot 1～5 Level1、目标Profile的Level2对象和证书；slot6/7/12轮换位置MUST保持blank。Device Root MUST在任何Level2对象之前可用。每个不可逆动作MUST先query，提交后不得自动retry，结果未知时MUST隔离。

#### Scenario: 灌装顺序正确

- **WHEN** Provisioning recipe请求安装Level2对象
- **THEN** 设备MUST证明Chip Root、Device Root和全部依赖对象有效
- **AND** 依赖证明失败或未知时MUST停止且不得写目标OTP record

#### Scenario: 写入后响应丢失

- **WHEN** OTP写入可能已被eHSM接收但Host在确定响应前timeout
- **THEN** 对象MUST进入unknown/quarantine
- **AND** Controller MUST NOT自动重发同一写操作

### Requirement: 非安全制造子Profile

BootROM MUST只保留`SECURE_BOOT`和`NON_SECURE_BOOT`两个顶层模式。只有`non_sec_boot=0`、LCS为DEV/MANU、latched `secure_boot=0`且制造Profile有效时，`NON_SECURE_BOOT` MUST选择`MANUFACTURING_PROVISIONING`；`non_sec_boot=1`、LCS异常或其他非制造组合 MUST选择`RESTRICTED_NONSECURE`。两个子Profile MUST使用独立镜像、权限和release配置，且不得相互fallback。

#### Scenario: DEV/MANU由pin进入灌装

- **WHEN** `non_sec_boot=0`、LCS为DEV或MANU且latched `secure_boot=0`
- **THEN** BootROM MUST加载独立C908 Provisioning FW
- **AND** BootROM本身 MUST NOT写OTP或安装Key
- **AND** Provisioning FW MUST只能调用eHSM BL typed制造接口

#### Scenario: 强制或异常非安全启动

- **WHEN** `non_sec_boot=1`或LCS无效/未知
- **THEN** BootROM MUST选择`RESTRICTED_NONSECURE`
- **AND** OTP/eFuse/KMU写与eHSM BL制造接口 MUST不可达

### Requirement: eHSM BL typed制造接口与部分写恢复

Vendor eHSM BL目标交付 MUST把C908 Provisioning FW视为不可信caller，并在不可逆接受点前同时验证LCS、硬件锁存制造条件、signed recipe/ticket、设备绑定、固定object映射、依赖和顺序。BL MUST提供typed query/install/generate/public-key/fixed-PoP/proof/finalize/LCS能力，MUST NOT暴露raw OTP、caller自选slot/Level/Attribute/`last_key`、任意消息签名或任意内存访问。

#### Scenario: 查询半写对象后恢复

- **WHEN** BL返回`PROGRAMMING_PARTIAL`
- **THEN** 只有device、recipe、object、材料摘要完全一致、OTP剩余bit单向兼容且CRC/ECC backend与BL明确支持续写时才能继续
- **AND** 其他情况 MUST进入`PROGRAMMED_INVALID`或`UNKNOWN`并隔离/报废
- **AND** Controller MUST NOT盲重试、覆盖或更换材料

### Requirement: USER最终提交

设备 MUST保持DEV完成全部目标Key、证书和proof，并使用最终Key/证书/策略完成reset/reload后的真实产品安全启动预演。DEV→MANU前全部Level1和目标Profile对象 MUST为`PROVED/LOCKED`；MANU只允许收口白名单。`MANU→USER` MUST是最后一个不可逆制造提交，并要求相邻转换readback。

#### Scenario: USER提交门禁

- **WHEN** 制造流程请求`MANU→USER`
- **THEN** 必须已有FMC/GSP/Measurement、动态证书、SPDM/签名和审计的最终配置证明
- **AND** 任一证明缺失、失败或状态未知时 MUST拒绝转换
- **AND** USER后pin和`non_sec_boot=1` MUST NOT重开`MANUFACTURING_PROVISIONING`

#### Scenario: USER安全链失效后的逃生位

- **WHEN** 产品声明可在USER设备无法启动安全FMC/GSP时再断言`non_sec_boot`
- **THEN** 必须存在独立于C908产品启动和Provisioning FW的强授权Secure ATE/维修端口或不可变eHSM BL窄操作
- **AND** 该路径未绑定时 MUST NOT作出上述逃生能力声明
- **AND** 位断言后下一次启动仍 MUST进入`RESTRICTED_NONSECURE`

### Requirement: Device Attestation Key与证书A/B

每台设备MUST只选择P-256或SM2中的一个Provisioning Profile，并在slot 14保存一把Device Private Key。产品Provisioning路径MUST使用eHSM内部生成、导出公钥和固定PoP；Cert0/Cert1 MUST只保存同一身份的A/B证书链和可验证metadata，不得保存私钥。

#### Scenario: 生成设备证明Key

- **WHEN** slot 14为空且recipe选择了批准的Attestation Profile
- **THEN** eHSM MUST内部生成该Profile的私钥并只返回公钥和固定PoP结果
- **AND** Controller MUST NOT提交任意私钥或任意待签摘要

#### Scenario: 安装证书链

- **WHEN** Host/CA提供signed install ticket和SPDM CertificateChain Blob
- **THEN** Device MUST校验ticket、设备/Profile、slot14公钥、slot8 Root身份证明、长度、摘要和Flash readback后commit
- **AND** Device MUST NOT构造PKCS#10、解析X.509或执行完整链/时间/吊销策略

### Requirement: DICE Root与UDS属性边界

slot 8 DICE root CA、slot 13 UDS和slot 14 Device private MUST按SRC-0024记录为Level 2 asymm对象并保留七项OTP/KMU权限。产品软件MUST通过typed policy、Lifecycle和Provisioning Profile限制实际调用，不得把属性能力扩大为Host raw Key服务。

#### Scenario: Root身份证明编码尚未冻结

- **WHEN** slot 8的材料形态或Root Anchor编码尚未由Key Attribute/证书Profile冻结
- **THEN** 证书安装MUST保持`DESIGN_BLOCKED_BY_KEY_ATTRIBUTE_INPUT`
- **AND** 实现MUST NOT沿用旧slot6公钥Hash假设

#### Scenario: Host请求通用Key操作

- **WHEN** 外部Host尝试使用slot 8、13或14执行未列入产品typed service的签名、加解密、派生或明文导入
- **THEN** GSP MUST拒绝请求
- **AND** 拒绝MUST NOT改变OTP/KMU对象状态

### Requirement: SoC Key Rotation

SoC Verify、Encrypt和Debug轮换MUST遵循SRC-0015和ADR-0021。外部KMS形成双层密文，eHSM管理物理slot、Bitmap和destroy；GSP不得向Host开放通用轮换或raw OTP服务。

#### Scenario: 执行一次批准轮换

- **WHEN** GSP收到设备绑定且一次性的批准轮换请求
- **THEN** eHSM MUST按写新Key、operation proof、Bitmap提交、销毁旧Key和reset生效的顺序执行
- **AND** 任何unknown状态MUST quarantine且不得回退旧Key
