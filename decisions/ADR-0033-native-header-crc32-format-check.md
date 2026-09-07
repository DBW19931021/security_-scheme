# ADR-0033：Native Header增加CRC32格式校验

- 状态：accepted
- 日期：2026-09-01
- 决策人：项目负责人
- 来源：SRC-0036、CE-SEC-016
- 关联：ADR-0014、ADR-0017、ADR-0030
- 替代关系：仅替代ADR-0030中offset 1016～1023为8字节零reserved的定义；其余无Manifest、load地址、长度、认证、加载和兼容结论继续有效

## Context

NGU800P type 1 SoC包已经复用Native Header末尾16字节：offset 1008～1015保存LE64 `load_addr`，offset 1016～1023原为零reserved。项目负责人要求在后8字节中增加CRC字段，用于BootROM在调用eHSM之前快速发现Header格式损坏，并在Vendor PASS后再次确认认证Header的一致性。

CRC字段位于Vendor签名/CMAC覆盖范围，若把`Signature`也纳入CRC会形成“CRC依赖Signature、Signature又依赖CRC”的生成循环。因此CRC只覆盖除Signature以外、CRC之前的Header内容。

## Decision

### 1. 字段布局

```text
offset 1008..1015  ngu_load_addr_le64      [8]
offset 1016..1019  ngu_header_crc32_le     [4]
offset 1020..1023  ngu_reserved            [4] = 0
offset 1024..      Code Region[Code_Size]
```

Native Header总长仍为1024字节，Overlay总长仍为16字节。

### 2. CRC参数和输入范围

`ngu_header_crc32`固定采用CRC-32/ISO-HDLC：

- width：32；
- polynomial：`0x04C11DB7`，反射实现多项式为`0xEDB88320`；
- init：`0xFFFFFFFF`；
- refin/refout：true/true；
- xorout：`0xFFFFFFFF`；
- check：ASCII `123456789`的结果为`0xCBF43926`；
- 字段编码：little-endian 32位无符号整数。

CRC输入固定为Native Header offset 256～1015，共760字节。该范围包含`Public_Key`、Vendor控制字段、`Code_Size`、`Version_Counter`、`Public_Key_Ext_Used`和`ngu_load_addr`，不包含offset 0～255的`Signature`、CRC字段自身、末尾四字节reserved或Code Region。

### 3. 制包顺序

1. 生成全部Header字段并把offset 1020～1023清零。
2. 对offset 256～1015计算CRC-32/ISO-HDLC，并以LE32写入offset 1016。
3. 按批准Vendor流程对Header认证范围和完整Code Region执行签名/CMAC，并完成Code加密。
4. 独立复读成品，重新计算CRC、验证reserved、精确包长和密码签名。

CRC先于签名生成；由于CRC字段位于Vendor认证范围，最终签名同时认证CRC值，不存在循环依赖。

### 4. 接收和安全边界

1. Preflight在提交eHSM前检查末尾四字节reserved为0，并重新计算Header CRC；不匹配时以`NOT_SUBMITTED`拒绝。
2. CRC通过只能说明Header通过非密码格式完整性筛查，不能认证`load_addr`或其他字段，也不能授权DMA、Measurement、counter或release。
3. Vendor PASS后，从稳定认证输出Header重新计算CRC并比较，同时重复长度、地址、counter、reserved和typed-stage policy检查。
4. Preflight/PASS后CRC值或参与CRC的Header字节不一致，均按Header不稳定或TOCTOU fail-close。
5. Code Region不进入Header CRC；其完整性继续由Vendor签名/CMAC、解密和loader源/目标摘要负责。

### 5. 兼容边界

- Overlay只适用于NGU800P Vendor `Image_Type=1`的SoC stage包。
- Vendor type 0 eHSM FW不解释offset 1008～1023。
- 旧`crc=0 + reserved[4]=0`包不再兼容；产品parser不保留旧8字节零reserved fallback。
- Vendor Header布局、签名覆盖范围或`Public_Key_Ext`消费范围变化时，必须重新证明Overlay仍可用。

## Consequences

- BootROM增加一次760字节CRC计算和4字节比较，RAM只需常数状态，不需要整头副本。
- 制包工具和所有type 1 golden package必须重生成；旧包进入negative corpus。
- CRC不能替代密码学Hash/签名，也不改变`Code_Size`、CBC对齐、原地搬移或Measurement范围。

## Rejected alternatives

- CRC覆盖完整1024字节并包含Signature：会与签名生成形成循环依赖。
- CRC覆盖Code Region：与现有签名、解密和源/目标摘要重复，并增加ROM工作量。
- 继续保留8字节全零：不能满足新增Header格式校验要求。
- 同时兼容旧reserved格式：形成双格式和降级面，拒绝。

