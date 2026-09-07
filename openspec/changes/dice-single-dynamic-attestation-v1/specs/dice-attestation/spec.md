# DICE-style Single Dynamic Attestation Requirements

## Requirement: Minimal early stages

BootROM必须只把最终加载FMC digest作为DICE阶段贡献，FMC必须只把最终加载GSP digest作为DICE阶段贡献；二者不得生成CDI、Key或X.509。

### Scenario: Certificate code linked into BootROM/FMC

- Given BootROM或FMC产品镜像
- When 扫描符号和调用图
- Then 不得出现X.509 writer、DICE KDF、Alias KeyGen或证书签名入口

## Requirement: Non-exporting DICE derivation

GSP必须由稳定FMC/GSP Measurement和安全状态形成固定TCB digest；eHSM必须以UDS执行域分离KDF并保持CDI和Alias private key不可导出。

### Scenario: Raw CDI requested

- Given 任意GSP或Host调用者请求CDI/private bytes
- When eHSM adapter执行策略检查
- Then 必须拒绝
- And 只能返回opaque handle和Alias Public Key

## Requirement: Single dynamic certificate layer

静态Device Attestation Issuer必须签发且只签发一个当前TCB的动态Firmware Alias Leaf；GSP不得为BootROM/FMC分别生成证书。

### Scenario: Dynamic chain verification

- Given Host取得静态前缀、动态Leaf和Report
- When 验证证书路径和Report
- Then 动态Leaf必须链接到本设备Issuer
- And Leaf TCB扩展必须等于Measurement和状态
- And Alias Public Key必须验证nonce/transcript签名

## Requirement: GSP owns X.509 assembly

GSP必须使用固定Profile writer组装TBSCertificate和最终DER；eHSM只执行Hash/KDF/KeyGen/Sign。

### Scenario: eHSM returns signature

- Given GSP已经生成合法TBSCertificate并提交其摘要
- When eHSM以slot14完成签名
- Then GSP必须按选定P-256或SM2 Profile封装AlgorithmIdentifier和签名
- And 最终DER不得超过4096字节

## Requirement: Memory isolation

动态证明工作区必须计入GSP峰值且不超过12 KiB，不得分配Measurement 16 KiB中的任何字节。

### Scenario: Measurement allocator selected

- Given 动态证书或Report申请工作缓冲
- When allocator返回Measurement范围
- Then 必须视为内存策略错误并fail-close
