---
title: "eHSM产品Operation Profile"
status: draft
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0032
owners:
  - GSP
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

把安全软件方案、ADR-0012/0013和Vendor能力映射为BootROM、FMC、GSP正式产品operation清单。本文决定“产品是否需要、由谁调用、成功和失败如何影响release”；Vendor API/command字段继续从配套代码逐项映射，不在这里另造协议。

# 通用规则

1. 所有eHSM调用经[Host Adapter](ehsm-host-adapter.md)统一typed service；首版poll、单context、单在途、零自动retry。
2. `PRODUCT_REQUIRED`表示安全软件方案或已批准决策要求；`PRODUCT_LOW_PRIORITY`表示最终需要但不阻断P0启动链；`PENDING_PROFILE`表示能力方向已批准但具体算法/参数未冻结。
3. LCS最终允许/拒绝由eHSM执行；GSP仍做caller、operation、参数、地址、usage和审计检查。
4. ADR-0019已确认所有GSP运行期operation仅允许受信任内部SoC consumer调用，不向Host开放。
5. timeout/BUSY/unknown completion隔离共享eHSM service；确定完成的业务失败按目标image/operation局部处理。

# BootROM Operation

| Operation ID | 类型 | Owner | 产品状态 | 输入/输出 | 关键行为 |
|---|---|---|---|---|---|
| `BR_STATUS_WAIT_READY` | SoC/Vendor status读取 | BootROM | PRODUCT_REQUIRED | raw hw/BL done/error | `done=1 && error=0`；若有hw和BL两组状态均须通过；非eHSM command |
| `BR_SELF_TEST` | eHSM BL self-test | BootROM | PRODUCT_REQUIRED | requested mask/raw bitmap | 使用Bootloader位图；保存unknown bit |
| `BR_LCS_READ` | SoC寄存器读取 | BootROM | PRODUCT_REQUIRED | raw LCS/eFuse状态 | BootROM直接读取，不建立eHSM LCS wrapper |
| `BR_VERIFY_DECRYPT_FMC` | secure package | BootROM | PRODUCT_REQUIRED | FMC package/plaintext/result | PASS前Overlay只可拒绝；确定PASS后稳定复验Header/typed FMC policy |
| `BR_STAGE_FMC_COUNTER` | anti-rollback | BootROM | PRODUCT_REQUIRED | authenticated FMC 16-byte candidate | FMC固定Vendor type 1、`check_version=0`；确认BL已暂存同一candidate，BootROM不读/比/写stored counter |

# FMC Operation

| Operation ID | 类型 | Owner | 产品状态 | 输入/输出 | 关键行为 |
|---|---|---|---|---|---|
| `FMC_VERIFY_DECRYPT_GSP` | secure package | FMC | PRODUCT_REQUIRED | GSP package/plaintext/result | 确定失败可清理并重新arm Host ingress |
| `FMC_COMMIT_STAGED_GLOBAL_COUNTER` | 不可逆counter | FMC | PRODUCT_REQUIRED / IMPLEMENTATION_PENDING | FMC Entry expected candidate/result | FMC初始化回传candidate；BL先与RAM candidate exact-match，再执行低值拒绝、相等不写、更高单向写和readback；准确command/packing/LCS/status/readback/交付版本在实现/EMU前补齐 |
| `FMC_CHECK_GSP_COUNTER_EQUAL` | anti-rollback | FMC | PRODUCT_REQUIRED | committed/GSP 16-byte counter | GSP固定`check_version=0`并必须等于已提交SoC值，不推进OTP |
| `FMC_LOAD_EHSM_VENDOR_FW` | Vendor FW | FMC | NOT_APPLICABLE | 无 | ADR-0012已移交GSP，FMC不得实现 |

# C908 Provisioning FW / eHSM BL Operation

下列operation只在`NON_SECURE_BOOT / MANUFACTURING_PROVISIONING`中可达；`RESTRICTED_NONSECURE`、`non_sec_boot=1`和USER路径均不得注册。C908 caller不可信，BL必须验证硬件制造条件、LCS、signed ticket、device binding、固定object映射和顺序。

| Operation族 | 产品状态 | 必须输出 | 禁止 |
|---|---|---|---|
| capability/identity/object query | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_DELIVERY` | BL版本、UID/LCS、`BLANK/PROGRAMMING_PARTIAL/PROGRAMMED_INVALID/PROGRAMMED_VALID/PROVED/LOCKED/UNKNOWN` | raw OTP/秘密状态 |
| wrapped install/internal generate | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_DELIVERY` | readback、对象状态、operation proof | caller slot/Level/Attribute/`last_key`、明文Key |
| public key/fixed PoP/proof | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_DELIVERY` | 固定Profile结果和摘要 | 任意消息或任意digest签名 |
| finalize/LCS transition | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_DELIVERY` | 完整矩阵和相邻转换readback | 无产品安全启动预演进入USER |

`PROGRAMMING_PARTIAL`只允许同device/recipe/object/材料、OTP bit单向兼容且BL/backend明确支持时恢复；否则隔离。BootROM不调用上述operation，只加载对应Provisioning FW。

# GSP镜像Operation

| Operation ID | 目标 | 产品状态 | 成功门禁 | 确定完成失败 | completion unknown |
|---|---|---|---|---|---|
| `GSP_LOAD_EHSM_VENDOR_FW` | eHSM Vendor FW | PRODUCT_REQUIRED | Vendor BL接受并达到`firmware_done && !firmware_err` | 隔离FW；依赖FW的服务不可用 | eHSM service quarantine |
| `GSP_VERIFY_RELEASE_PMP` | PMP | PRODUCT_REQUIRED | verify/decrypt、Header Overlay/typed-stage policy/counter/Measurement、Firewall和release全部成功 | 仅PMP=`ISOLATED/NOT_RELEASED` | eHSM service quarantine |
| `GSP_VERIFY_RELEASE_RMP` | RMP | PRODUCT_REQUIRED | 同上，使用RMP独立type/slot/allowlist | 仅RMP=`ISOLATED/NOT_RELEASED` | eHSM service quarantine |
| `GSP_VERIFY_RELEASE_MMP` | MMP | PRODUCT_REQUIRED / PROFILE_BLOCKED | 同上，且已加载到批准的受保护DDR carveout并完成cache、Firewall/IOMMU和release门禁 | 仅MMP=`ISOLATED/NOT_RELEASED` | eHSM service quarantine |

每个镜像分别记录`EMPTY/STAGED/VERIFYING/VERIFIED/LOADED/RELEASED/ISOLATED`。显式依赖项因上游隔离保持`NOT_RELEASED`，但不得伪造为自身验证失败。

# GSP安全服务Operation

| Operation族 | 产品状态 | 首要consumer | 是否允许Host | 仍需冻结 |
|---|---|---|---|---|
| Signing | PRODUCT_REQUIRED | slot14签动态Leaf；Alias Key签SPDM/Report | NO | P-256/SM2输入域、签名编码、准确typed API |
| RNG | PRODUCT_REQUIRED | SPDM nonce、Key/方案指定随机数 | PENDING | 请求上限、health failure、rate/quota |
| Hash | PRODUCT_REQUIRED | Measurement、SPDM、方案指定digest | PENDING | 首版固定one-shot；仍需冻结每个算法的最大输入长度 |
| Secure package verify/decrypt | PRODUCT_REQUIRED | eHSM FW、PMP/RMP/MMP | NO | native profile、最大package、output extent |
| Counter staged commit/equality | PRODUCT_REQUIRED / IMPLEMENTATION BLOCKED | Boot chain、Firmware update | NO | `rollback_counter[16]`、FMC candidate exact-match和eHSM BL专用API方向已冻结；command/packing/LCS/status/readback/交付版本、寿命和掉电原语待输入 |
| DICE UDS KDF/Alias KeyGen | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_BINDING` | GSP动态证明 | NO | slot13 Key Attribute、域分离KDF、确定性KeyGen、opaque handle和清零命令 |
| Key/Certificate/Rotation | PRODUCT_REQUIRED / `BLOCKED_BY_VENDOR_DELIVERY` | GSP安全管理、动态Leaf、制造/轮换 | NO | slot14只签Firmware Alias；云天定制轮换及准确usage/LCS/接口 |
| 通用对称/非对称/MAC服务 | PRODUCT_LOW_PRIORITY | 其他受信任SoC模块 | PENDING | 产品算法矩阵、caller ACL、quota、buffer API |

“以安全软件方案需要完成的功能点为准”落实为：SEC-FEAT-004/005/006/007/010/013～016/019中每个需要eHSM的功能，都必须在本表新增独立operation ID或明确映射到现有operation族。Vendor Demo存在但方案未采用的算法不得自动进入产品清单。

DICE一级动态证书固定分工：eHSM只执行`HASH、KDF、KDF-KeyGen、SIGN`并保存不导出的UDS/CDI/Alias private handle；GSP构造96字节TCB Context、ASN.1/X.509 v3 TBSCertificate、最终DER和Report。不得新增“eHSM生成X.509”command，也不得把通用X.509 parser引入BootROM/FMC。

# 产品算法集合

ADR-0017冻结三套产品Secure Package算法Profile：

1. SHA-256 + RSA-2048/RSASSA-PSS + AES-128-CBC；
2. SHA-256 + ECDSA/secp256r1 + AES-128-CBC；
3. SM3 + SM2 + SM4-CBC。

三套Profile所需的SHA-256、RSA-2048-PSS、ECDSA-P256、AES-128-CBC、SM3、SM2和SM4-CBC primitive均为`PRODUCT_REQUIRED`。RSA-PSS、SM2 Z值、raw签名布局、CBC/IV/no-padding按匹配Vendor原生合同。其他Vendor算法只进入运行在安全核上的独立baremetal软件栈做全能力/回归覆盖；GSP固件不承担该能力验证，产品服务默认不注册、不路由、不允许调用。只有后续软件方案/ADR明确产品用途、caller和安全策略后，才可新增GSP产品operation。ADR-0020规定设备/SKU、镜像和key/board/LCS绑定来自单一matrix，缺失实际行默认拒绝。

# LCS和权限

- GSP发起Key、Certificate、Rotation、OTP/LCS/Debug等受控操作。
- eHSM根据实际LCS和内部策略返回最终允许/拒绝。
- GSP不得把eHSM拒绝转换为成功，也不得因为“eHSM会检查”而省略caller identity、operation allowlist、地址/长度、usage和审计。
- 不注册Host到上述operation的路由；固件下发、SPDM、受控Debug/OOB使用各自独立协议，不允许转成通用GSP service passthrough。

# Descriptor与Session影响

- 固定descriptor容量按所有P0 one-shot operation中最大共享对象数量确定。
- 首版Hash、Signing和RNG固定使用Vendor one-shot typed API；不得注册init/update/finish路由，不分配512字节`ehsm_session_st`。
- 若任一产品输入超过Vendor one-shot API经测试确认的最大长度，则该输入必须拒绝并阻断对应功能发布；不得在首版静默切换到流式API。未来流式支持必须走独立ADR/OpenSpec。
- 通用算法P2服务不得为了预留未知能力而扩大首版并发数或引入heap。

# Open questions

1. 三套Profile的算法/mode/强度已冻结；仍需逐项映射准确Vendor one-shot API、最大输入、descriptor数量和设备provisioning绑定。
2. GSP加载eHSM Vendor FW的package source、加载命令、BL/FW配套版本、最大尺寸和恢复行为。
3. PMP/RMP/MMP各自package source、依赖和release接口；PMP/RMP的安全SRAM目标分别固定256 KiB，MMP主要使用DDR但其carveout、System Address load/entry、eHSM/CPU可达性、Firewall/IOMMU和cache合同仍待输入。Vendor type固定为1、`check_version=0`且`rollback_counter`匹配已提交全SoC值，type 2/3拒绝。type 0 eHSM FW使用独立counter域。
4. slot13 UDS是否具备不导出KDF/确定性Alias KeyGen所需Key Attribute，匹配Vendor command、handle生命周期和P-256/SM2映射。
5. Host路由必须不可达并纳入negative test；OPEN-DESIGN-006已关闭。
6. Key/Certificate/Rotation各operation的usage和调用时序；slot14动态Leaf签发方向已冻结，轮换仍等待云天定制Vendor交付。
7. `OPEN-DESIGN-025`等待eHSM BL制造typed command、硬件授权、状态/接受点、partial-write、readback和LCS掉电合同。

# References

- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)
- [ADR-0013](../../decisions/ADR-0013-gsp-operation-image-service-and-isolation-profile.md)
- [安全Feature落实矩阵](../09-plans/SECURITY-FEATURE-REALIZATION-MATRIX.md)
- [GSP软件设计](../05-software-design/gsp.md)
- [Measurement Table接口](measurement-table.md)
- [ADR-0034](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)

# Change history

- 2026-09-02：按ADR-0034增加仅DEV/MANU非安全制造子Profile可达的eHSM BL typed制造operation、partial恢复和USER提交门禁。
- 2026-08-19：按SRC-0032/ADR-0029增加DICE UDS KDF/Alias KeyGen和两类签名operation；明确eHSM只做基础算法和不导出Key，GSP完成一级动态X.509组装。
- 2026-08-19：按SRC-0031/ADR-0028把PMP/RMP目标固定为安全SRAM各256 KiB；MMP改为受保护DDR目标并在Profile冻结前保持阻断，不再复用FMC尾部启动区。
- 2026-07-27：接受ADR-0019；关闭Host API开放项，GSP operation只服务受信任内部SoC consumer。
- 2026-07-24：接受ADR-0017；将三套Secure Package Profile所需primitive设为产品必需能力，其他Vendor算法保留为baremetal能力/回归输入，产品GSP service默认不开放；具体设备/key/board/LCS绑定进入OPEN-DESIGN-011。
- 2026-07-23：根据负责人BootROM/FMC/GSP裁决建立首版产品operation profile；冻结PMP/RMP/MMP/eHSM FW镜像集合、签名/RNG/Hash必需能力、通用算法低优先级开放、GSP发起/eHSM LCS最终授权、Host API待定和单Runtime局部隔离。
