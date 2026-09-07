---
title: "安全固件包与Native Header Overlay合同"
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
  - NGU Manifest v1 package contract
superseded_by: []
---

# Purpose

定义FMC、GSP、PMP、RMP、MMP及Die1 SoC固件的唯一物理包格式、Native Header末尾16字节项目Overlay、Header CRC、发布工具输入输出和验收门禁。本合同依据[ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](../../decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)和[ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)。

> 主详设反向链接：[《NGU800P安全软件详细设计》第3章“固件Package、Native Header Overlay、制作与发布”](../05-software-design/NGU800P安全软件详细设计.md#第3章-固件packagenative-header-overlay制作与发布)

本合同只复用当前Vendor type 1 SoC镜像中`Public_Key_Ext`未被RSA-3072最大公钥及OTP公钥Key ID计算消费的末16字节，不改变Vendor Header总长、签名/CMAC覆盖、eHSM密码算法或Bootloader处理流程。eHSM Vendor FW type 0不适用该Overlay。

# 1. 设计结论

1. NGU800P type 1 SoC package固定为：

   ```text
   Native Header[1024] || Code Region[Code_Size]
   ```

2. 不再存在NGU Manifest、第二包头、TLV、`payload_offset`、`payload_size`或包内`entry_addr`。
3. Native Header偏移1008～1015存放little-endian 64位`load_addr`；偏移1016～1019存放LE32 `ngu_header_crc32`；偏移1020～1023为4字节reserved，发布时必须为0、接收时非0拒绝。
4. `Code_Size`是签名、加密、解密、搬移、目标回读和Measurement的唯一镜像长度。
5. `entry_addr`固定等于`load_addr`。需要非零入口偏移时必须另立受控变更，不能复用reserved。
6. 镜像身份、实例、die、consumer、算法Profile、key、board/SKU/LCS和release policy全部来自受信typed-stage registry及provisioning/release matrix，不由包自声明。
7. `ngu_header_crc32`采用CRC-32/ISO-HDLC，覆盖Header offset 256～1015；只用于格式损坏筛查，不替代Vendor密码认证。
8. 旧Manifest包和旧8字节零reserved包不兼容，产品不得保留fallback、dual parser或Host选择格式开关。

# 2. Native Header布局

## 2.1 完整布局

| Offset | Size | 字段 | NGU800P规则 |
|---:|---:|---|---|
| 0 | 256 | `Signature` | 由批准的Vendor兼容签名工具生成；禁止零签名 |
| 256 | 320 | `Public_Key` | 按批准密钥/Profile生成 |
| 576 | 16 | `Encrypt_IV` | 必须由批准工具生成；禁止固定IV |
| 592 | 4 | `Valid_Flag` | little-endian `0x8E97645D` |
| 596 | 1 | `Image_Type` | SoC stage包固定Vendor type 1 |
| 597 | 1 | `Plain_Flag` | 产品安全启动必须为ciphertext profile |
| 598 | 1 | `Naked_Flag` | 产品必须禁用 |
| 599 | 5 | `Reserved` | 必须全0 |
| 604 | 4 | `Code_Size` | little-endian；精确等于实际Code Region长度 |
| 608 | 16 | `Version_Counter` | 16 octets；项目唯一`rollback_counter`来源 |
| 624 | 384 | `Public_Key_Ext`有效区 | 当前最大RSA-3072扩展公钥占用范围 |
| 1008 | 8 | `ngu_load_addr` | NGU Overlay；little-endian 64位System Address |
| 1016 | 4 | `ngu_header_crc32` | NGU Overlay；CRC-32/ISO-HDLC结果，LE32 |
| 1020 | 4 | `ngu_reserved` | NGU Overlay；必须全0 |
| 1024 | `Code_Size` | `Code Region` | 固件镜像本体，包含CBC零对齐字节；无Manifest |

CE-SEC-016确认当前Vendor实现对RSA-3072只拷贝384字节扩展公钥，未消费1008～1023；Header签名/CMAC覆盖从偏移592到1023，因此Overlay处于密码认证范围内。该结论必须与Vendor版本兼容矩阵绑定，不能推导为永久Vendor ABI。

## 2.2 Overlay逻辑类型

```c
typedef struct {
    uint64_t load_addr_le;
    uint32_t header_crc32_le;
    uint8_t reserved[4];
} ngu_header_overlay_v2_t;
```

该C结构仅表示逻辑布局。parser必须逐字节有界读取，不得把Host输入或共享RAM中的Header直接cast为结构体。实现必须有以下静态/生成期断言：

- Header总长为1024；
- Overlay起点为1008；
- CRC起点为1016；
- reserved起点为1020；
- Overlay总长为16；
- 1008～1023位于当前Vendor认证覆盖范围内。

## 2.3 Header CRC合同

`ngu_header_crc32`固定采用CRC-32/ISO-HDLC：poly=`0x04C11DB7`（反射实现`0xEDB88320`）、init=`0xFFFFFFFF`、refin/refout=true、xorout=`0xFFFFFFFF`；ASCII `123456789`的检查值为`0xCBF43926`。

CRC输入是Header offset 256～1015共760字节。该范围不包含`Signature[0..255]`、CRC字段自身、末尾reserved或Code Region，从而避免CRC和Signature生成循环。发布工具先写CRC，再执行Vendor签名；CRC字段自身位于Vendor签名/CMAC覆盖范围。

Header使用的CRC-32/ISO-HDLC与Measurement结构的CRC-32C不同，公共实现、registry和测试向量必须使用不同符号，禁止混用多项式。

# 3. Vendor类型与适用范围

| Vendor `Image_Type` | 对象 | Overlay规则 |
|---:|---|---|
| 0 | eHSM Vendor FW | 不适用NGU Overlay；保持Vendor原格式和专用启动流 |
| 1 | FMC/GSP/PMP/RMP/MMP/Die1 SoC固件 | 必须使用本合同Overlay |
| 2 | Vendor SoC/eHSM key域变体 | NGU800P产品SoC stage拒绝 |
| 3 | eHSM Patch | 不属于普通stage包，首版拒绝 |

FMC/GSP/PMP/RMP/MMP不是`Image_Type`的新wire值。它们由调用者持有的typed-stage descriptor区分，不能从Host metadata或包内空闲位恢复。

# 4. 物理包和长度不变量

1. `package_size = 1024 + Code_Size`，不得多一个或少一个字节。
2. `package_size > 1024`，并同时不超过Host ingress及目标Region容量。
3. 所有`offset + size`使用防溢出运算；向Vendor `uint32_t image_size`转换前证明不大于`UINT32_MAX`。
4. `Code_Size == package_size - 1024`在提交eHSM前检查，并在Vendor PASS后的稳定认证Header上再次检查。
5. 对type 1 SoC包，eHSM output容量至少为完整`package_size`，因为输出包含1024B Header和解密后的Code Region。
6. `Plain_Flag=0`、`Naked_Flag=0`、Vendor Reserved及offset 1020的`ngu_reserved[4]`全0。
7. Preflight和Vendor PASS后的稳定Header都必须重新计算offset 256～1015的CRC并与offset 1016的LE32值比较；CRC通过不能形成认证或授权结论。
8. 当前三套CBC Profile要求`Code_Size`为16字节倍数。发布工具对原始bin补入的0～15字节零一旦计入`Code_Size`，就是受签名、加密、搬移和Measurement保护的Code字节。
9. 设备端没有原始bin长度或`payload_size`，不得去padding；执行语义若不能容忍末尾零，必须由镜像/linker合同在制包前解决。

# 5. 删除Manifest后的字段来源

| 原Manifest语义 | 新权威来源 | 设备端要求 |
|---|---|---|
| `ngu_image_type` | typed-stage API/registry | consumer固定请求对象，Host不能选择 |
| `instance_id`、`die_id` | stage topology registry | 进入Measurement前由producer产生 |
| consumer/stage | 编译期调用路径 | BootROM只接FMC，FMC只接GSP，GSP按typed runtime API接收 |
| `load_addr` | Header offset 1008 + typed-stage固定目标 | 两者必须精确相等，不能只落入宽allowlist |
| `entry_addr` | 固定规则 | 始终等于`load_addr` |
| `payload_offset` | 固定规则 | 始终为1024 |
| `payload_size` | Native Header `Code_Size` | 唯一长度，不去padding |
| rollback counter | Native Header `Version_Counter[16]` | 不维护第二份包内counter |
| algorithm Profile/key/board/LCS | provisioning/release matrix | 不从Vendor response或包内空闲位推导 |
| security policy | typed-stage policy + 当前LCS | verify/measure/release等门禁由调用上下文固定 |
| `version` | 外部release metadata | 不参与设备安全接受或OTP比较 |
| Measurement身份 | producer常量 + topology registry | 不由镜像自声明slot或实例 |

删除字段不等于删除检查。凡是原来用于授权的字段，必须由受信上下文替代；没有权威来源时fail-close。

# 6. Stage package profile

| Consumer | Package | Vendor type | `boot` | 固定规则 |
|---|---|---:|---:|---|
| BootROM | FMC | 1 | 0 | `check_version=0`；`load_addr`精确等于FMC Region；BL暂存Header candidate |
| FMC | GSP | 1 | 0 | `check_version=0`；`load_addr`精确等于GSP Region；counter等于已提交SoC值 |
| GSP | eHSM Vendor FW | 0 | 1 | Vendor专用包；不解释NGU Overlay；等待`firmware_done=1 && firmware_err=0` |
| GSP | PMP/RMP/MMP | 1 | 0 | 每个typed stage固定独立目标；counter等于已提交SoC值 |

PMP/RMP目标Region为固定256 KiB安全SRAM；MMP目标位于受保护DDR，DDR Profile未冻结前不得发布其包。

# 7. 接收、验证和TOCTOU合同

## 7.1 PASS前

Header仍不可信，但接收端必须先进行安全预检：

1. 固定长度、flag、`Code_Size`、overflow和output容量检查；
2. 读取offset1008/1016/1020，要求`load_addr`已精确等于本次typed stage固定目标、Header CRC重算一致且reserved为0；
3. 只以受信stage descriptor配置eHSM output地址，绝不让Header决定DMA/output目标；
4. 预检结果只能用于拒绝，不能驱动Measurement、counter提交、权限扩大或release。

精确目标预检用于减少认证失败前eHSM对目标Region的副作用；不能用覆盖多个stage的宽地址allowlist替代。

## 7.2 PASS后

Vendor明确PASS后：

1. 从eHSM稳定输出重新解析整个安全相关Header；
2. 重做长度、type、flag、counter、offset1008/1016/1020、Header CRC及精确目标检查；
3. 再次读取Header并确认安全相关字段未变化；
4. 绑定受信Profile、key、board/SKU/LCS和typed-stage policy；
5. 只有全部通过后才计算源Code摘要、搬移、回读和Measurement。

任何CRC不匹配、二次读取不稳定、reserved非0、目标不匹配或旧格式形态都fail-close。

# 8. 原地加载合同

当eHSM把完整明文包输出到目标Region时：

```text
target + 0       : Header[1024]
target + 1024    : Code[Code_Size]
```

loader顺序固定为：

1. 目标保持`RW/NX`，下游CPU/reset未release；
2. 对`target + 1024`开始的`Code_Size`字节计算受信Profile摘要；
3. 执行`memmove(target, target + 1024, Code_Size)`；
4. 清理旧Header尾部和目标BSS；CBC零对齐不单独剥离；
5. 从`target`回读`Code_Size`字节重算摘要并常量时间比较；
6. 比较成功后才提交Measurement、切换最终W^X/Firewall并release；
7. Measurement记录的`load_addr`和`entry_addr`相等，digest覆盖目标回读的全部`Code_Size`字节。

目标Region必须满足`1024 + Code_Size <= region_size`。不得继续使用旧Manifest方案的`target+1152`或`payload_size`。

# 9. 产品算法Profile

ADR-0017批准以下唯一产品组合：

| Profile | Digest | Signature | Encryption |
|---:|---|---|---|
| 1 | SHA-256/32 | RSA-2048、RSASSA-PSS、SHA-256/MGF1-SHA-256 | AES-128-CBC、16B IV、无padding |
| 2 | SHA-256/32 | ECDSA/secp256r1、SHA-256、raw `r || s` | AES-128-CBC、16B IV、无padding |
| 3 | SM3/32 | SM2+SM3、Vendor Z值、raw `r || s` | SM4-CBC、16B IV、无padding |

Profile整体选择，禁止混搭。设备具体Profile来自受信provisioning/release matrix；包不再携带Profile ID。三套Profile都必须实现和测试。

# 10. 发布制包合同

## 10.1 输入

- 受控原始bin及其hash；
- typed image/stage、固定目标Region和64位System Address；
- 16字节`rollback_counter`；
- Profile、key域、LCS、board/SKU和release policy ID；
- 外部发布版本、构建标识、工具版本和source ID。

## 10.2 顺序

1. 从typed-stage registry取得唯一`load_addr`，不得接收自由地址。
2. 按Profile把原始bin补零到CBC block边界，得到最终`Code_Size`和最终Code hash。
3. 生成Vendor Header，写入`Version_Counter`、offset1008 LE64 load，并把offset1020～1023写0。
4. 对Header offset 256～1015计算CRC-32/ISO-HDLC，以LE32写入offset1016。
5. 调用批准的Vendor兼容工具完成真实加密和签名。
6. 独立解析成品，重算Header CRC并检查reserved、Overlay、精确长度、flags、counter、Profile和签名/密文形态。
7. 输出package SHA-256、最终Code hash、原始bin hash/length、padding length、Header摘要、工具/KMS引用和release metadata。

## 10.3 禁止事项

- 零签名、固定IV、Naked、Plain或“名为密文实际明文”；
- Host自由选择type/instance/die/Profile/target；
- 生成或兼容旧Manifest；
- 覆盖历史发布包；
- 把私钥、明文production key或未脱敏token写入仓库/Evidence。

# 11. 验证要求

至少覆盖：

- FMC/GSP/PMP/RMP/MMP/Die1正常type 1包及eHSM FW正常type 0包；
- 截短Header/Code、尾随字节、`Code_Size`小于/大于实际长度和算术溢出；
- offset1008大小端错误、32位截断、目标错stage/错instance/宽allowlist误接受；
- offset256～1015任一bit损坏、offset1016 CRC错误、CRC算法/覆盖范围/字节序错误，以及offset1020～1023任一非0；
- CRC正确但Vendor签名错误，必须继续拒绝；
- 旧128B Manifest包、dual-format标志和伪第二包头；
- Plain/Naked/Vendor Reserved异常、type 2/3误接受；
- counter、签名、密文、Code和Header Overlay篡改；
- PASS前Header与PASS后Header不同、PASS后二次读取变化；
- output容量不足、input/output非法alias、目标Region容不下`1024+Code_Size`；
- 源Code与目标回读摘要不一致；
- `entry_addr != load_addr`的任何企图；
- Vendor版本/Header布局变化或公钥扩展超过384B时构建/兼容门禁失败。

# 12. 演进与重新评审门禁

下列任一条件出现时，禁止继续使用尾16B，必须重新做Vendor代码调查、签名覆盖证明和ADR评审：

1. Vendor Header版本或布局变化；
2. 新算法/密钥格式会消费`Public_Key_Ext`超过384B，或Vendor改为对原始400B字段整体计算公钥Key ID；
3. Vendor签名/CMAC覆盖范围改变；
4. eHSM parser开始读取1008～1023；
5. 需要独立`entry_addr`、更多策略字段或多目标地址；
6. type 0/2/3需要采用相同Overlay。

# 13. Open items

1. Profile 1/2/3已冻结；具体设备/SKU/镜像的Profile、key、board/LCS绑定行仍由Product/Provisioning/KMS/Release Owner补齐。
2. FMC/GSP/PMP/RMP SRAM Region已冻结；PMA/Firewall、MMP DDR Profile和最终link map峰值Evidence仍是发布前输入。
3. rollback counter固定16B；eHSM BL专用staged-candidate commit接口的command/packing/LCS/交付版本仍需Vendor绑定。
4. 产品代码、真实制包器和E2E测试尚未获得本次文档修改所代表的编码授权。

# References

- [《NGU800P安全软件详细设计》第3章](../05-software-design/NGU800P安全软件详细设计.md#第3章-固件packagenative-header-overlay制作与发布)
- [《安全公共ABI与Registry》](security-common-abi.md)
- [《镜像Verify、Loader与Release合同》](image-verify-loader.md)
- [CE-SEC-016《Native Header尾部占用与签名覆盖调查》](../../evidence/code-investigations/CE-SEC-016-native-header-tail-and-signature-coverage.md)
- [ADR-0014《安全固件包、NGU Manifest与长度规范化》（Manifest部分已替代）](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)
- [ADR-0017《三套产品安全固件算法Profile》](../../decisions/ADR-0017-three-product-secure-package-algorithm-profiles.md)
- [ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](../../decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)
- [ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)
