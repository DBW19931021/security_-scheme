# ADR-0017：三套产品安全固件算法Profile

> 2026-08-21后续裁决：ADR-0030删除NGU Manifest及包内`expected_algorithm_profile/payload_size`。三套算法组合继续有效，但Profile由受信provisioning/release matrix和typed-stage context选择；loader摘要对象改为完整`Code[Code_Size]`，包含CBC零对齐字节。

- 状态：accepted
- 日期：2026-07-24
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关 Requirement/Open Question：OPEN-DESIGN-004、OPEN-DESIGN-011
- 补充关系：补充ADR-0013、ADR-0014

## 背景

ADR-0013已经确认签名、随机数、Hash及安全软件方案所需eHSM能力必须进入产品，ADR-0014冻结Vendor原生Header，但具体产品算法组合尚未冻结。没有明确组合时，制包工具、OTP/provisioning、release matrix、BootROM/FMC/GSP后置检查和测试corpus无法形成一致的产品Profile。

SRC-0018当前4019 Vendor BL实现已经提供以下原生绑定：

- RSA-2048镜像签名使用SHA-256和RSASSA-PSS；
- ECC P-256R1镜像签名使用SHA-256和ECDSA/secp256r1；
- SM2镜像签名使用SM3、SM2 Z值和SM2验签；
- RSA/ECC镜像解密使用AES，SM2镜像解密使用SM4；
- 镜像对称解密模式固定为CBC、16字节IV、`NO_PADDING`。

以上是当前Vendor实现事实，不赋予Vendor定义SoC产品策略的权力；产品允许哪些组合仍由本ADR记录项目负责人裁决。

## 决策

NGU800P产品安全固件只支持以下三套完整算法Profile：

| Profile ID | Registry符号 | Loader/Measurement Code digest | 镜像签名 | Code Region加密 |
|---:|---|---|---|---|
| 1 | `NGU_ALG_PROFILE_SHA256_RSA2048_AES128_CBC` | SHA-256，32字节 | RSA-2048，RSASSA-PSS，message hash和MGF1均为SHA-256；签名256字节 | AES-128-CBC，16字节IV，无padding |
| 2 | `NGU_ALG_PROFILE_SHA256_ECDSA_P256_AES128_CBC` | SHA-256，32字节 | ECDSA/secp256r1（NIST P-256），SHA-256，raw `r || s` 64字节 | AES-128-CBC，16字节IV，无padding |
| 3 | `NGU_ALG_PROFILE_SM3_SM2_SM4_CBC` | SM3，32字节 | SM2 + SM3，采用Vendor原生SM2 Z值/签名格式，raw `r || s` 64字节 | SM4-CBC，16字节IV，无padding |

补充规则：

1. Profile ID 0为INVALID；首版受信registry/release matrix只允许1、2、3，其余值拒绝。包内不编码Profile ID。
2. FMC、GSP、PMP、RMP、MMP的产品包必须选择且完整使用其中一套，不允许跨Profile混搭Hash、签名或加密算法。
3. eHSM Vendor FW使用Vendor type 0及其OTP/provisioning选择；SoC stage使用type 1 Header Overlay。两者均不含NGU Manifest，发布Evidence必须记录实际套件和Vendor配置。
4. Vendor原生RSA签名padding、SM2 Z值、签名字节布局、CBC/IV和无padding行为follow匹配Vendor BL/制包工具，不在项目侧另造变体。
5. CBC无padding要求Code Region长度为16字节倍数。发布工具可在原始bin末尾补0～15字节零并把结果作为完整`Code[Code_Size]`；这些字节进入Vendor签名、加密、loader源/目标摘要和Measurement。设备端不存在第二个`payload_size`或去padding动作；镜像/linker必须允许该完整Code长度。包尾不得有`Code_Size`之外的字节。
6. RSA-3072、AES-192/256、AES/SM4-CMAC、MD5、SHA-1、SHA-224/384/512、SHA-512/256、SHA-3、DES/TDES、XTS及Vendor支持的其他组合不属于产品Secure Package Profile。它们可以作为`baremetal` Vendor能力/回归覆盖输入，但产品parser、policy和release路径必须拒绝。
7. 通用运行期密码服务不得因Vendor存在其他算法而自动开放。上述三套Profile所需primitive为产品必需能力；其他算法只有在后续软件方案/ADR明确批准用途、caller和安全策略后才可进入产品服务。
8. 产品软件、制包器和BootROM/FMC/GSP验证路径必须同时实现三套Profile，每套都必须通过完整启动、更新和负向测试；任何SKU或provisioning配置不得裁掉其中一套实现。每个具体发布包由受控release matrix唯一绑定一套Profile，不得由Host或包内字段自声明。所有SoC stage包固定使用Vendor type 1/SoC key域；boot/upgrade密钥对象、board binding和LCS mask由受控provisioning/release matrix绑定。

## Vendor绑定说明

本ADR采用当前SRC-0018 4019实现中已经固定的算法细节：

- `CODE_VERIFY_ALG_RSA2048`通过SHA-256/RSASSA-PSS验签；
- `CODE_VERIFY_ALG_ECC_P256R1`通过SHA-256/ECDSA-P256验签；
- `CODE_VERIFY_ALG_SM2`通过SM3、Z值和SM2验签；
- RSA/ECC映射AES解密，SM2映射SM4解密；
- `fwverify_ske_decrypt_dma()`固定CBC和`SKE_NO_PADDING`。

如果新Vendor交付改变这些绑定，必须依据release note重新核对并更新Source ID；不允许仅修改项目Profile名称掩盖Vendor不兼容。

## 软件影响

- 公共ABI Registry保留3个稳定Profile ID；provisioning/release matrix、typed-stage context和工具不得各自复制不同数值，wire package不保存Profile ID。
- Loader按受信Profile唯一派生摘要算法：Profile 1/2为SHA-256/32，Profile 3为SM3/32，并对完整源`Code[Code_Size]`与目标回读执行双摘要比较。
- BootROM/FMC/GSP在Vendor PASS后必须确认typed-stage Profile与受控设备/release配置一致；不得从Host或Header reserved推导Profile。
- release工具必须按完整Profile选择Vendor兼容签名、加密、IV和对齐规则，禁止用户分别自由选择三个算法参数。
- 产品代码不需要实现Vendor BL已有的密码算法；项目侧只实现Profile选择、策略校验、制包绑定、错误和Evidence。

## 测试影响

1. FMC、GSP、PMP、RMP、MMP分别覆盖三套Profile的合法包；三套Profile都必须完成完整BootROM→FMC→GSP→Runtime启动链和更新链，不允许用pairwise替代任一Profile的端到端通过。
2. 增加跨Profile混搭、未知/Host自选Profile、源/目标回读摘要不一致、设备provisioning与release matrix不符、CBC长度非16字节倍数、错误去除或漏度量CBC零对齐等负向用例。
3. RSA-PSS、ECDSA-P256、SM2签名和AES/SM4-CBC使用Vendor兼容golden vector；不能以通用库默认值代替Vendor格式。
4. Vendor其他算法继续进入baremetal全能力case清单，但Expected必须明确为“Vendor能力测试”，不能视为产品release允许。

## 尚未冻结但不阻断当前详设

- 三套Profile共存时各设备/SKU、FMC/GSP/Runtime的boot/upgrade密钥对象和证书/发布配置矩阵；SoC stage的Vendor key域已固定为type 1；
- board binding和LCS mask的最终值；
- signing key ID、certificate ID、密钥轮换批次和发布Owner；
- Vendor工具中需要固化到golden evidence的RSA-PSS salt策略及各签名/公钥序列化细节。

这些参数在制包器实现、OTP/provisioning或EMU发布包生成前必须冻结，但不阻断第4章eHSM BL/Host Adapter/Mailbox/平台Port以及与具体Profile无关的解析和状态机设计。

## 参考资料

- `docs/04-interfaces/security-common-abi.md`
- `docs/04-interfaces/secure-firmware-package.md`
- `docs/04-interfaces/ehsm-product-operation-profile.md`
- `source-vault/vendor-code/osr_eshm/ehsm_bl-2.3.5-4019-72f8fdc/src/component/fw_verify.h`
- `source-vault/vendor-code/osr_eshm/ehsm_bl-2.3.5-4019-72f8fdc/src/component/fw_verify.c`
