# Vendor原生包Image_Type与Code_Size语义冲突

## Identity

- Conflict ID: CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS
- Open Question: OPEN-CONFLICT-008
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人；Vendor接口Owner和Release工具Owner待指定
- Resolution date: 2026-07-23
- Decision: 批准Option A，详见ADR-0014

## Conflict classification

- Type: baseline_to_vendor_implementation_mismatch / package_canonicalization
- Affected scope: Vendor原生Header `Image_Type`解释、`Code_Size`校验Owner、release package验收和verify wrapper。
- Safe-to-continue scope: Header固定offset/字段宽度、Manifest位于Code Region且PASS后解析、`rollback_counter[16]`透传、Host可读`uint32_t version`分离、loader/result状态机、eHSM Vendor FW保持Vendor包。
- Must-stop scope: 把SRC-0016中简化的`Image_Type`说明写成wire枚举；在没有长度一致性门禁时发布或接受SoC安全包；把current stub packager升级为发布工具。

## Evidence A：Vendor正式Header语义

- SRC-0012第29页定义：
  - `Image_Type=0`：eHSM FW；
  - `Image_Type=1`：SoC镜像，使用SoC密钥；
  - `Image_Type=2`：SoC镜像，使用eHSM密钥；
  - `Image_Type=3`：eHSM Patch。
- 同页定义`Code_Size`为offset 604处的4字节Code Region长度，Code从offset 1024开始。
- SRC-0018 Bootloader `fw_verify.h:17-26,102-108`与上述offset和枚举一致。

## Evidence B：软件方案表格的简化说明

- SRC-0016第8页把`Image_Type`简化描述为`1=SoC FW`、`2=eHSM FW`。
- SRC-0016第7～12页总体又明确物理包遵循eHSM原生格式，NGU Manifest位于Code Region并承载FMC/GSP/Runtime项目类型。

解释：如果按第8页文字把2传给Vendor BL，Vendor会把它解释为“使用eHSM密钥验证的SoC镜像”，而不是eHSM FW；真正eHSM FW是0。

## Evidence C：Vendor direct verify没有检查Code_Size等于实际命令长度

- SRC-0018 Bootloader `fw_verify.c:798-820`只检查`image_size > 1024`。
- `fw_verify.c:289-400`按`cmd->image_size - 1024`复制/解密，并按`cmd->image_size`完成签名验证。
- `fw_verify.c:450-520,524-657`的各算法路径从调用参数推导hash长度。
- 直接verify路径没有读取Header `Code_Size`；upgrade路径另有读取。

解释：Header本身和实际命令字节范围均被签名保护，但两者可以由制包错误产生不一致。当前实现会以命令长度为实际处理范围，不会因Header `Code_Size`不一致而拒绝。

## Impact

- Compatibility：错误使用原生`Image_Type=2`会选择错误业务分支/密钥域。
- Canonicalization：同一Header声明可能对应不同调用长度，制包器、Host和eHSM对包边界的理解不唯一。
- Loader safety：若Host按Header长度分配，而eHSM按命令长度写出，可能产生容量检查和实际输出范围不一致。
- Verification：无法为截短、尾随数据和Header长度不一致构造唯一的产品Expected。
- Governance：若不记录冲突，后续代码可能错误修改Vendor公共实现，违反已批准的Vendor只读边界。

## Options

### Option A：保持Vendor wire/代码不变，由项目边界做双阶段精确校验（推荐）

1. 软件方案amendment明确原生`Image_Type`采用Vendor 0/1/2/3语义；项目FMC/GSP/PMP/RMP/MMP只出现在NGU Manifest命名空间。
2. Release工具必须生成`Code_Size == package_size - 1024`。
3. Host adapter提交前把Header当作不可信输入，只做防溢出、容量和精确长度preflight；不因preflight通过而信任内容。
4. eHSM PASS后从输出中的已认证Header重新检查同一等式，任何不一致都禁止Manifest解析、loader、Measurement和release。
5. Vendor公共代码不修改；若未来Vendor正式交付新增内部校验，项目仍保留边界校验作为defense in depth。

优点：符合已批准的Vendor代码只读边界；消除格式歧义；无需等待Vendor补丁即可让项目侧fail-close。代价：项目wrapper和发布工具都要维护明确校验及negative tests。

### Option B：要求Vendor修改Bootloader并重新交付

- Vendor在直接verify路径内部检查`Code_Size == image_size - 1024`，提供版本说明和回归Evidence。

优点：eHSM内部也强制canonical长度。代价：改变Vendor基线、交付周期不可控；项目边界仍应做容量校验。

### Option C：废弃Code_Size，以命令image_size为唯一长度

- 软件方案和工具不再依赖Header `Code_Size`，允许字段仅作信息。

不推荐：与Vendor TRM字段语义和SRC-0016不一致，且保留了一个已签名但无约束的长度字段。

## Recommendation

采用Option A。它不修改Vendor代码，符合“与eHSM交互首先follow Vendor实现”的批准原则，同时在NGU800P项目边界恢复唯一包长度。将SRC-0016第8页`Image_Type`说明视为需要amendment的简化错误，而不是覆盖Vendor wire定义。

## Owner decision

1. 批准Option A作为当前D0产品合同。
2. 批准把SRC-0016下一受控版本/amendment中的原生`Image_Type`表修正为Vendor 0/1/2/3，并把NGU项目镜像类型放入Manifest独立命名空间。
3. 接受“preflight失败不提交；PASS后复验失败视为authenticated-format-error，禁止Manifest、loader、Measurement和release”的错误分类。
4. Vendor公共代码保持不变；后续Vendor若自行增加内部校验，项目双阶段边界校验仍保留。

## Review history

- 2026-07-23：在INV-SEC-009复核SRC-0012/SRC-0016及Vendor Bootloader函数体时发现；建立OPEN-CONFLICT-008，未修改Vendor或公司代码仓。
- 2026-07-23：项目负责人批准推荐Option A；冲突关闭并形成ADR-0014、Manifest v1和公共ABI合同。
