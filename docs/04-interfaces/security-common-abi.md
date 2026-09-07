---
title: "NGU800P安全公共ABI与Registry"
status: approved
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0012
  - SRC-0014
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0033
  - SRC-0036
owners:
  - GSP
last_reviewed: 2026-09-01
supersedes:
  - NGU Manifest v1 registry
superseded_by: []
---

# Purpose

冻结BootROM、FMC、GSP、发布工具、Header Overlay parser、typed-stage policy、loader、Measurement和测试共用的首版数值空间。进入编码时必须由一份机器可读registry生成C头、工具常量和测试fixture，不允许人工复制多份枚举。

> 主详设反向链接：[《NGU800P安全软件详细设计》附录A“公共逻辑ABI、枚举与Registry”](../05-software-design/NGU800P安全软件详细设计.md#附录a-公共逻辑abi枚举与registry)

# 1. ABI基本规则

1. Native Header Overlay和其他wire/storage整数使用little-endian。
2. 只使用固定宽度整数和octet array；wire中禁止C `enum`、`bool`、指针、`size_t`、bit-field和隐式padding。
3. parser逐字段解码，不直接cast不可信字节。
4. 生成的C view必须对`sizeof`和关键`offsetof`作编译期断言。
5. 所有reserved发布为0，接收时非0拒绝。
6. Vendor raw状态和项目状态分别保存。
7. NGU800P不再定义Manifest wire/storage ABI；旧Manifest magic、flags、offset和golden vector不得进入production生成物。

# 2. Vendor原生Image_Type

| 值 | 符号 | 语义/准入 |
|---:|---|---|
| 0 | `NGU_VENDOR_IMAGE_EHSM_FW` | eHSM FW；专用流程，不解释NGU Overlay |
| 1 | `NGU_VENDOR_IMAGE_SOC_SOC_KEY` | FMC/GSP/PMP/RMP/MMP/Die1唯一产品准入值 |
| 2 | `NGU_VENDOR_IMAGE_SOC_EHSM_KEY` | Vendor事实枚举；产品SoC stage拒绝 |
| 3 | `NGU_VENDOR_IMAGE_EHSM_PATCH` | Vendor事实枚举；普通stage流程拒绝 |
| 4～255 | reserved | 拒绝 |

该表镜像Vendor wire事实，不授权修改Vendor Header或公共代码。

# 3. 受信项目镜像类型

以下数值只存在于typed C API、registry、Measurement和审计，不写入固件包：

| 值 | 符号 | 语义 |
|---:|---|---|
| 0 | `NGU_IMAGE_INVALID` | 非法/未初始化 |
| 1 | `NGU_IMAGE_FMC` | FMC |
| 2 | `NGU_IMAGE_GSP` | GSP |
| 3 | `NGU_IMAGE_PMP` | PMP |
| 4 | `NGU_IMAGE_RMP` | RMP |
| 5 | `NGU_IMAGE_MMP` | MMP |
| `0x00000100`～`0x000001ff` | reserved | 多Die/实例扩展，未分配前不得使用 |
| 其他 | reserved | 拒绝 |

eHSM FW不占用该命名空间。Die1的最终值由实际topology registry分配，不能从Host输入或Header空闲位读取。

# 4. 内部Caller/Consumer Stage ID

| 值 | 符号 |
|---:|---|
| 0 | `NGU_STAGE_INVALID` |
| 1 | `NGU_STAGE_BOOTROM` |
| 2 | `NGU_STAGE_FMC` |
| 3 | `NGU_STAGE_GSP` |
| 4 | `NGU_STAGE_PMP` |
| 5 | `NGU_STAGE_RMP` |
| 6 | `NGU_STAGE_MMP` |

Stage ID只用于typed request授权、adapter policy、错误定位和审计归属，由当前镜像的编译期配置或受信调用链产生。Host和任何非可信wire输入不得提供。

# 5. Native Header Overlay ABI

| Header offset | Size | 字段 | 规则 |
|---:|---:|---|---|
| 1008 | 8 | `ngu_load_addr` | little-endian 64位System Address |
| 1016 | 4 | `ngu_header_crc32` | CRC-32/ISO-HDLC，覆盖Header offset 256～1015，LE32 |
| 1020 | 4 | `ngu_reserved` | 必须全0 |

固定常量：

```text
NGU_NATIVE_HEADER_SIZE            = 1024
NGU_HEADER_LOAD_ADDR_OFFSET       = 1008
NGU_HEADER_CRC32_OFFSET           = 1016
NGU_HEADER_CRC32_INPUT_OFFSET     = 256
NGU_HEADER_CRC32_INPUT_SIZE       = 760
NGU_HEADER_RESERVED_OFFSET        = 1020
NGU_HEADER_OVERLAY_SIZE           = 16
NGU_CODE_REGION_OFFSET            = 1024
```

CRC参数固定为poly=`0x04C11DB7`、init=`0xFFFFFFFF`、refin/refout=true、xorout=`0xFFFFFFFF`，检查值`CRC32("123456789")=0xCBF43926`。CRC只用于Header格式完整性筛查；preflight和Vendor PASS后都必须重算，且不得把CRC通过解释为密码认证或授权。该算法不是Measurement使用的CRC-32C，生成符号和测试向量不得共用。

`entry_addr`不编码，固定等于`load_addr`。`Code_Size`是唯一镜像长度。旧Manifest的magic、header size、payload offset/size、policy flags、version和TLV均已删除。

任何Vendor Header版本/布局变化、新算法消费`Public_Key_Ext`超过384B或认证覆盖范围变化，必须使兼容门禁失败并重新评审。

# 6. System Address

C908、Header Overlay、loader、linker和eHSM共享descriptor只使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`定义的64位System Address。公共ABI不定义`LOCAL_REMAP`，不执行Local/System换算。UCIe remote address和Flash offset属于专用接口。

包内`load_addr`必须与typed-stage registry固定目标精确相等。registry同时给出Region ID、最大`Code_Size`、image/instance/die和entry规则；不使用宽allowlist。

# 7. 产品算法Profile

| 值 | 符号 | Code digest | Vendor签名绑定 | Code加密 |
|---:|---|---|---|---|
| 0 | `NGU_ALG_PROFILE_INVALID` | — | — | 拒绝 |
| 1 | `NGU_ALG_PROFILE_SHA256_RSA2048_AES128_CBC` | SHA-256/32 | RSA-2048 RSASSA-PSS | AES-128-CBC、16B IV、无padding |
| 2 | `NGU_ALG_PROFILE_SHA256_ECDSA_P256_AES128_CBC` | SHA-256/32 | ECDSA P-256 raw `r||s` | AES-128-CBC、16B IV、无padding |
| 3 | `NGU_ALG_PROFILE_SM3_SM2_SM4_CBC` | SM3/32 | SM2+SM3 raw `r||s` | SM4-CBC、16B IV、无padding |
| 4～`0xffffffff` | reserved | — | — | 拒绝 |

| 值 | Digest符号 | 长度 |
|---:|---|---:|
| 0 | `NGU_DIGEST_INVALID` | 0；拒绝 |
| 1 | `NGU_DIGEST_SHA256` | 32 |
| 2 | `NGU_DIGEST_SM3` | 32 |
| 3～`0xffffffff` | reserved | 拒绝 |

Profile 1/2使用SHA-256，Profile 3使用SM3。Profile由受信provisioning/release matrix给出，不是包内字段；三个primitive必须整体选择。

# 8. 状态枚举

## 8.1 Completion

| 值 | 符号 |
|---:|---|
| 0 | `NGU_COMPLETION_NOT_SUBMITTED` |
| 1 | `NGU_COMPLETION_COMPLETED` |
| 2 | `NGU_COMPLETION_ACCEPTANCE_UNKNOWN` |

## 8.2 Verify

| 值 | 符号 |
|---:|---|
| 0 | `NGU_VERIFY_EMPTY` |
| 1 | `NGU_VERIFY_PREFLIGHT_PASS` |
| 2 | `NGU_VERIFY_VENDOR_PASS` |
| 3 | `NGU_VERIFY_HEADER_PASS` |
| 4 | `NGU_VERIFY_OVERLAY_PASS` |
| 5 | `NGU_VERIFY_POLICY_PASS` |
| 6 | `NGU_VERIFY_FAILED` |
| 7 | `NGU_VERIFY_QUARANTINED` |

## 8.3 Runtime image

| 值 | 符号 |
|---:|---|
| 0 | `NGU_IMAGE_STATE_EMPTY` |
| 1 | `NGU_IMAGE_STATE_STAGED` |
| 2 | `NGU_IMAGE_STATE_VERIFYING` |
| 3 | `NGU_IMAGE_STATE_VERIFIED` |
| 4 | `NGU_IMAGE_STATE_LOADED` |
| 5 | `NGU_IMAGE_STATE_MEASUREMENT_COMMITTED` |
| 6 | `NGU_IMAGE_STATE_RELEASED` |
| 7 | `NGU_IMAGE_STATE_ISOLATED` |
| 8 | `NGU_IMAGE_STATE_QUARANTINED` |

# 9. Typed-stage policy

每个stage profile至少生成：

- consumer/caller stage；
- project image type、instance、die；
- Vendor type 1；
- fixed System Address `load_addr`；
- `entry_addr=load_addr`规则；
- target Region ID和最大`Code_Size`；
- algorithm/profile matrix引用；
- rollback关系；
- Measurement producer/identity；
- dependency和release profile。

sign/decrypt/rollback/measure/release对所有SoC stage均为必需策略，不再编码成包内bit。board/SKU/key/LCS同样由matrix处理。

# 10. 项目状态码

公共返回值为`uint32_t`：

```text
0x00000000                         success
(domain << 24) | reason[23:0]      project failure
```

| 值 | 符号 |
|---:|---|
| 1 | `NGU_ERR_ARGUMENT` |
| 2 | `NGU_ERR_FORMAT` |
| 3 | `NGU_ERR_TRANSPORT` |
| 4 | `NGU_ERR_EHSM` |
| 5 | `NGU_ERR_POLICY` |
| 6 | `NGU_ERR_COUNTER` |
| 7 | `NGU_ERR_LOADER` |
| 8 | `NGU_ERR_MEASUREMENT` |
| 9 | `NGU_ERR_RELEASE` |
| 10 | `NGU_ERR_PLATFORM` |
| 11 | `NGU_ERR_INTERNAL` |

`reason=0`保留。Vendor raw status保存在独立字段。

# 11. Registry权威源和生成物

权威源分为：

- `security_abi_registry.yaml`：本文件数值、Overlay offset、错误和Measurement ABI；
- `security_product_profiles.yaml`：三套算法Profile；
- `security_provisioning_release_matrix.yaml`：设备/SKU/镜像的Profile、key、board和LCS；
- `security_ram_layout.yaml`：固定目标Region/System Address/最大长度/Firewall/PMA；
- topology/stage registry：image/instance/die/consumer/dependency/release。

同一生成器输出：

1. `gsp-pmp-rmp-omp`公共C头；
2. 发布工具和独立检查器常量；
3. `baremetal` fixture只读常量；
4. `security_abi_registry.json`快照；
5. `sizeof/offsetof`和Header offset静态断言；
6. Header-tail大小端、CRC、reserved和typed-stage golden vectors。

CI必须比较生成物hash、offset1008/1016/1020、Header总长1024、CRC标准向量/覆盖范围、未知值拒绝、little-endian解析和旧Manifest符号缺失。手改生成物或production source graph出现Manifest parser必须失败。

# 12. Measurement关联

- `rollback_counter`固定`uint8_t[16]`，来自Native Header；
- Measurement identity来自typed-stage registry；
- Measurement load/entry来自loader actual result，二者相等；
- digest来自目标回读的全部`Code_Size`字节；
- Measurement物理Region固定16 KiB，理论最多126项；实际`max_fw_entries`来自topology。

# References

- [《安全固件包与Native Header Overlay合同》](secure-firmware-package.md)
- [《镜像Verify、Loader与Release合同》](image-verify-loader.md)
- [ADR-0017《三套产品安全固件算法Profile》](../../decisions/ADR-0017-three-product-secure-package-algorithm-profiles.md)
- [ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](../../decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)
- [ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)
- [CE-SEC-016《Native Header尾部占用与签名覆盖调查》](../../evidence/code-investigations/CE-SEC-016-native-header-tail-and-signature-coverage.md)
