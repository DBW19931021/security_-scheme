# ADR-0030：删除NGU Manifest并复用Native Header尾部承载load_addr

> 后续更新：offset 1016～1023的8字节零reserved定义已由[ADR-0033：Native Header增加CRC32格式校验](ADR-0033-native-header-crc32-format-check.md)替代；本ADR其余结论继续有效。

- 状态：accepted
- 日期：2026-08-21
- 决策人：项目负责人
- 来源：SRC-0033、CE-SEC-016
- 关联：ADR-0014、ADR-0017、ADR-0019、ADR-0022、ADR-0024、ADR-0028
- 替代关系：替代ADR-0014/0024中NGU Manifest的全部包内合同；保留Vendor Header、精确长度、真实签名/加密、stage policy、copy/readback digest、Measurement和release门禁

## Context

原方案在Vendor 1024字节Native Header后的加密Code Region前放置128字节NGU Manifest，用于携带类型、实例、策略、Profile、payload长度、load/entry地址、counter和版本。项目负责人现决定删除该Manifest，只在Native Header `Public_Key_Ext[400]`未使用的末尾16字节中保存8字节`load_addr`，其余8字节预留。

CE-SEC-016确认当前Vendor BL/FW的RSA-3072路径只消费`Public_Key_Ext`前384字节，OTP公钥Key ID也只对按算法重组出的64字节指数和384字节模数计算，不会把原始400字节扩展区整体纳入公钥哈希。Header偏移1008～1023未被公钥解析或Key ID校验使用；Vendor签名/CMAC覆盖Header偏移592～1023和整个Code Region，因此新增字段处于镜像认证范围。

## Decision

### 1. 唯一物理包格式

NGU800P type 1 SoC stage包固定为：

```text
Native Header[1024]
  0..1007     Vendor原生字段/公钥扩展有效区
  1008..1015  ngu_load_addr_le64
  1016..1023  ngu_reserved[8] = 0
Code Region[Code_Size]
  1024..      firmware_image（含必要CBC零对齐）
```

不再存在NGU Manifest、第二个项目Header、payload offset或TLV。`package_size=1024+Code_Size`且必须精确相等。

### 2. 字段语义

- `ngu_load_addr`是little-endian 64位baremetal System Address，不携带domain。
- `ngu_reserved[8]`发布时全0，接收时任一非0均拒绝；当前不得把它解释为version、entry、flags或长度。
- 独立`entry_addr`删除，所有当前stage固定`entry_addr=load_addr`；需要非零入口偏移的未来镜像必须重新裁决，不能偷用reserved。
- `Code_Size`同时是认证、解密、copy、目标范围检查和Measurement的唯一镜像长度。
- CBC对齐零字节是`firmware_image`和`Code_Size`的一部分，进入签名、解密、copy和Measurement；设备端不存在“真实payload长度”或去padding动作。

### 3. 原Manifest字段的替代来源

| 原字段/语义 | 新权威来源 |
|---|---|
| image type、instance、die、consumer | BootROM/FMC/GSP typed stage入口、固定分区/通道和受控stage registry；包不得自声明 |
| sign/decrypt/measure/release policy | 产品固定规则、LCS和stage状态机；type 1包强制`Plain=0,Naked=0` |
| algorithm profile、key域 | Provisioning/release matrix、OTP/eFuse和eHSM配置 |
| payload offset | 固定1024，即Native Header后第一个字节 |
| payload size | Native Header `Code_Size` |
| load address | Header偏移1008的`ngu_load_addr` |
| entry address | 固定等于`ngu_load_addr` |
| rollback counter/domain | Native Header `Version_Counter[16]`；type 1使用SoC global域，type 0保持eHSM域 |
| lifecycle allow mask | stage产品策略和LCS权限矩阵 |
| Host显示version | 外部受控release metadata或固件内部版本；不得参与安全启动 |
| component/Measurement slot | stage-owned常量和实例registry；Measurement只记录已完成事实 |

### 4. 解析、加载和TOCTOU门禁

1. Host ingress提交后必须seal；preflight检查精确包长、type 1、Plain/Naked、`Code_Size`、reserved和地址，但此时Header仍视为不可信。
2. preflight的`ngu_load_addr`必须等于当前typed stage profile给出的唯一目标地址，不能只做宽泛allowlist；不匹配时不提交eHSM，防止失败验签前写错目标。
3. eHSM PASS前CPU不得解析Code Region、不得release或写Measurement。
4. PASS后从稳定输出Header重新读取`Code_Size、Version_Counter、ngu_load_addr、reserved`并重复精确检查；前后Header、sealed ingress或generation不一致视为TOCTOU并fail-close。
5. 受控原地加载使用`source=target+1024`、`length=Code_Size`，先计算源摘要，再`memmove(target,source,Code_Size)`，清零`target+Code_Size`至Region末尾，回读目标摘要并常量时间比较。
6. `load_addr+Code_Size`必须无溢出并落在当前stage唯一Region；使用完整包输出到目标的路径还必须满足`1024+Code_Size<=target_region_size`。
7. `fence.i`、权限lock/readback、Measurement commit全部成功后，才从`load_addr` release/jump。

### 5. 类型和兼容边界

- Overlay只用于NGU800P Vendor `Image_Type=1`的FMC/GSP/PMP/RMP/MMP及批准Die1 SoC stage包。
- Vendor type 0 eHSM FW保持原生包，不解释偏移1008为load地址。
- 旧Manifest包与新格式不兼容；新parser不保留fallback。旧包通常因`ngu_load_addr=0`或地址不匹配被拒绝，即使人为填入地址也会因Code Region以Manifest字节开头、启动镜像不合法而失败。
- Header版本、Vendor算法、公钥扩展消费范围或公钥Key ID输入变化时，必须重新证明1008～1023仍未占用且仍在镜像认证范围；否则停止使用overlay。

## Consequences

- 包开销减少128字节；原地加载源偏移由1152降为1024，相同目标Region的最大Code Region增加128字节。
- BootROM/FMC/GSP不再需要Manifest parser、Manifest ABI、字段重复比较和Manifest scratch；ROM路径进一步简化。
- 项目类型和策略不再由包自描述，所有调用必须是typed、stage-owned且默认拒绝，Host不能选择任意image kind、实例或目标。
- `Code_Size`包含CBC零对齐，Measurement和Expected必须按完整Code Region计算；工具不得再报告另一个设备端payload长度。
- `version`不再存在于设备包合同；发布系统若需要展示版本，只能使用外部受控release metadata。

## Rejected alternatives

- 保留最小Manifest：与负责人“Manifest头直接去掉”的要求不符。
- 把entry或payload_size塞进剩余8字节：当前明确预留，且会重新引入包内可变语义。
- 验签前仅按Header自由地址选择输出目标：未认证地址可造成验证失败前的越权覆盖。
- 继续兼容旧Manifest parser：形成两套包语义和降级面，拒绝。
