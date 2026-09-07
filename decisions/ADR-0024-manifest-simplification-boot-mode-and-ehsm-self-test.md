# ADR-0024：Manifest精简、LCS异常启动策略与eHSM自检Owner

> 2026-08-21后续裁决：[ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](ADR-0030-remove-manifest-and-use-native-header-tail.md)删除了本ADR的Manifest v1部分；LCS异常启动策略和eHSM自检Owner裁决继续有效。

> 2026-08-25后续裁决：[ADR-0031《non_sec_boot eFuse强制受限非安全启动》](ADR-0031-non-sec-boot-efuse-override.md)在本文模式矩阵前增加最高优先级覆盖。只有`non_sec_boot=0`时执行本文LCS/Strap规则；可靠值1时包括USER在内均进入现有受限非安全路径。

- 状态：accepted
- 日期：2026-07-28
- 决策人：项目负责人
- 相关决策：ADR-0014、ADR-0018、ADR-0019、ADR-0022
- 相关OpenSpec：`secure-package-manifest-v1`、`bootrom-mode-and-ehsm-self-test-v2`

## 背景

负责人要求进一步降低Manifest和BootROM复杂度：

- Manifest不再携带当前产品不需要的ABI版本、地址domain、board/Measurement/component绑定和TLV扩展字段；
- Vendor签名已经覆盖完整Code Region，Manifest不再重复携带expected payload digest及其算法/长度/offset字段；
- LCS读取或来源异常时不再fail-close，而是进入独立受限非安全启动；
- 非安全启动不写启动审计；
- eHSM自检由eHSM依据自身eFuse策略自主决定和执行，BootROM不发起。

这些决定分别改变ADR-0014/0019的Manifest布局和ADR-0018的LCS异常处理，必须以新受控决策替代对应部分。

## 决策一：NGU Manifest v1精简为固定128字节

删除以下字段：

- `abi_major`、`abi_minor`；
- `load_addr_domain`、`entry_addr_domain`；
- `board_binding_policy`；
- `measurement_slot`；
- `component_id`；
- `extension_offset`、`extension_size`；
- 全部扩展TLV规则。

保留128字节固定little-endian布局：

| Offset | Size | 字段 | 规则 |
|---:|---:|---|---|
| 0 | 4 | `magic` | `0x4D55474E` |
| 4 | 4 | `header_size` | 固定128 |
| 8 | 4 | `manifest_flags` | v1固定0 |
| 12 | 4 | `ngu_image_type` | stage allowlist |
| 16 | 4 | `instance_id` | 镜像实例 |
| 20 | 4 | `security_policy_flags` | bit0～4固定安全门禁；其余保留为0 |
| 24 | 4 | `expected_algorithm_profile` | 只允许1/2/3 |
| 28 | 4 | `reserved0` | 必须为0 |
| 32 | 8 | `payload_offset` | v1固定128 |
| 40 | 8 | `payload_size` | 实际明文payload长度 |
| 48 | 8 | `load_addr` | 64位baremetal System Address |
| 56 | 8 | `entry_addr` | 64位baremetal System Address |
| 64 | 16 | `rollback_counter` | 与认证Vendor Header逐octet一致 |
| 80 | 4 | `lifecycle_allow_mask` | 允许LCS集合 |
| 84 | 40 | `reserved1` | 必须全部为0，不得解释为digest元数据或扩展 |
| 124 | 4 | `version` | Host工具读取，不参与防回滚 |

Payload固定从offset 128开始，Code Region为`Manifest[128] + Payload + CBC_zero_pad[0..15]`。Manifest和Code Region中没有独立expected digest对象。Loader按`expected_algorithm_profile`唯一派生算法，对已认证源payload和目标Region回读分别计算摘要并常量时间比较，成功后的目标摘要写入Measurement。无扩展区、无TLV、无可选字段。未来需要不兼容扩展时使用新的Manifest magic/新正式ABI，不在v1中恢复TLV。

被删除字段的语义归属如下：

- 地址不携带domain，固定为baremetal System Address；loader/platform profile只做范围和Region检查，不执行Local/System转换；
- board/SKU/key/LCS等发布绑定由单一provisioning/release matrix管理；
- Measurement身份由`ngu_image_type + die_id/profile + instance_id`和stage producer生成，不从Manifest提供物理/逻辑slot；
- 组件身份由image type、instance和release profile确定；
- ABI兼容由`magic + header_size`控制；不支持的magic/header size直接拒绝。
- payload摘要算法由完整Profile唯一派生，不在Manifest重复编码；摘要用于load/readback一致性和Measurement，不替代Vendor签名。

## 决策二：LCS异常进入受限非安全启动

启动模式矩阵更新为：

| 条件 | 启动模式 |
|---|---|
| `LCS=USER` | 忽略Strap，强制安全启动 |
| 已识别非USER且`boot_pin.secure_boot[3]=0` | 受限非安全启动 |
| 已识别非USER且`boot_pin.secure_boot[3]=1` | 安全启动 |
| LCS读取失败、非法、`UNDEFINED`、来源有效性无法证明或组合未批准 | 受限非安全启动 |

该策略只改变“选择哪条启动路径”，不把异常LCS解释为任何有效生命周期，也不开放OTP、生产Key、counter更新、raw eHSM、安全RAM明文区、受保护Debug或GSP安全服务。受限非安全FMC仍必须来自平台明确的source/Region/权限profile；profile缺失时只能停在非安全路径不可交付状态，不能回到安全启动。

非安全启动不创建Measurement Firmware Entry、不提交SoC State，也不写启动审计记录。安全路径的错误、Measurement和审计规则不受影响。

## 决策三：eHSM自主决定并执行自检

1. eHSM依据自身eFuse配置判断本次上电是否需要执行自检，并在Vendor BL内部自主执行。
2. BootROM不得发送“开始自检”命令，也不得维护第二套自检策略。
3. BootROM只轮询`bootloader_done/bootloader_err`及Vendor定义的ready/error状态，并在ready后读取raw自检状态/bitmap用于诊断和Evidence。
4. eFuse配置为“不需要自检”时，未执行自检不是失败；eFuse要求自检且eHSM报告失败/错误时，不得进入FMC安全启动。
5. 精确eFuse字段、状态位和bitmap继续follow匹配Vendor手册/代码；项目不修改Vendor自检业务逻辑。

## 替代关系

- 本ADR替代ADR-0018中“LCS失败/非法/UNDEFINED/未识别时fail-close”的部分；LCS独立读取继续有效，USER强制安全自ADR-0031起仅在`non_sec_boot=0`时适用。Strap字段和非USER极性已由2026-07-29负责人裁决更新为SRC-0023的`boot_pin.secure_boot[3]`、`0=非安全/1=安全`；当前RTL/生成头绑定仍受OPEN-CONFLICT-010约束。
- 本ADR替代ADR-0014/0019中Manifest `abi_major/abi_minor`、独立地址domain、expected digest和TLV扩展相关部分；Vendor Header、128字节总长、16字节counter、offset124 `version`和双阶段长度门禁仍有效。
- ADR-0022中“Manifest `measurement_slot`作为逻辑class”的描述被本ADR删除；Measurement Entry由stage按实际实例生成。

## 主详设

- [NGU800P安全软件详细设计](../docs/05-software-design/NGU800P安全软件详细设计.md)
