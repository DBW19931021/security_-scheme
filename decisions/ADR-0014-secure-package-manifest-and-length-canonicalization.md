# ADR-0014：安全固件包、NGU Manifest与长度规范化

> 2026-08-21后续裁决：[ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](ADR-0030-remove-manifest-and-use-native-header-tail.md)替代本ADR的NGU Manifest拓扑和字段合同。继续有效的只有Vendor 1024字节Header、原生Image_Type、真实签名/加密、`Code_Size == package_size-1024`、typed stage policy及验证失败不得release等原则。

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0012、SRC-0014、SRC-0016、SRC-0017、SRC-0018
- 相关 Requirement/Open Question：OPEN-CONFLICT-008、OPEN-DESIGN-008
- 补充关系：补充ADR-0005、ADR-0012、ADR-0013

> 2026-07-28更新（历史）：本ADR关于Manifest字段、显式地址domain和TLV扩展的内容当时被ADR-0024替代；该128字节Manifest合同又于2026-08-21被ADR-0030整体删除。现行完整合同见[《NGU800P安全软件详细设计》第3章](../docs/05-software-design/NGU800P安全软件详细设计.md#第3章-固件packagengu-manifest制作与发布)。

## 背景

Vendor eHSM Bootloader、Host代码和SRC-0016对原生包`Image_Type`的说明不一致；Vendor verify路径还不能单独证明Header `Code_Size`与实际输入包长度完全相等。与此同时，NGU800P需要在不修改Vendor公共代码的前提下，稳定表达FMC、GSP、PMP、RMP、MMP的项目类型、64位加载/入口地址、16字节`rollback_counter`和Measurement策略。

## 候选方案

1. 修改Vendor wire或公共代码，使其直接理解NGU800P各固件类型。
2. 保持Vendor原生合同，在其认证的Code Region内增加独立NGU Manifest，并在项目边界完成长度规范化。

## 决策

### 1. Vendor原生层保持不变

Vendor Header的`Image_Type`固定解释为：

| Wire值 | 含义 |
|---:|---|
| 0 | eHSM Firmware |
| 1 | 使用SoC密钥的SoC镜像 |
| 2 | 使用eHSM密钥的SoC镜像 |
| 3 | eHSM Patch |

不得把FMC、GSP、PMP、RMP、MMP直接编码为Vendor `Image_Type`的新值，也不得为了项目语义修改Vendor公共源码。SRC-0016第8页相关简化说明按“与Vendor实现不一致”处理，应在下一受控版本或amendment中修正。

### 2. Code_Size执行双阶段精确校验

对所有提交给eHSM的原生包，项目release工具和运行期adapter都必须执行：

```text
Code_Size == package_size - 1024
```

执行顺序和失败语义固定如下：

1. **提交前preflight**：Header尚不可信，只用于长度规范化、容量和溢出检查；不相等时命令不得提交，completion为`NOT_SUBMITTED`。
2. **eHSM PASS后post-check**：从认证输出Header重新读取并再次校验；不相等时为`AUTHENTICATED_FORMAT_ERROR`，不得解析Manifest、load、写Measurement或release。
3. 包尾不得有未计入`Code_Size`的字节；截短、尾随和32位长度截断均拒绝。

Vendor verify成功只证明Vendor层处理通过，不替代项目层精确长度、Manifest、policy、digest、loader、Measurement和release门禁。

### 3. NGU Manifest是唯一项目镜像命名空间

FMC、GSP、PMP、RMP、MMP只使用受保护Code Region内的NGU Manifest区分。eHSM Vendor FW继续使用Vendor type 0原生包，不要求加入NGU Manifest。

首版SoC包物理结构固定为：

```text
Vendor Header[1024]
  + encrypted/authenticated Code Region {
      NGU Manifest v1[128]
      + plaintext payload after decrypt
      + CBC_zero_pad[0..15]
    }
```

不允许在Vendor Header外增加未登记的包装Header，也不允许两个代码仓分别维护Manifest ABI。

### 4. NGU Manifest v1基础合同获批

1. base header固定128字节，整数使用little-endian。
2. 该字段由ADR-0019最终命名为`rollback_counter[16]`，必须与eHSM PASS后认证Vendor Header的`Version_Counter[16]`逐octet相等；它不是第二个counter。
3. `load_addr`和`entry_addr`均为64位、不携带domain，固定按baremetal System Address解释。
4. Manifest不携带expected digest或`digest_algorithm/digest_size/digest_offset`。Loader按已认证Profile分别计算源payload摘要和目标回读摘要并常量时间比较，成功的目标摘要写入Measurement。
5. 所有NGU image type、内部Stage ID、状态、错误域和flag数值由单一公共ABI registry管理；Stage ID不是Manifest字段。
6. Manifest parser只可在Vendor PASS、认证Header复验通过后运行；v1的offset 84～123必须全0，未知required字段/flag、TLV、扩展或尾随对象必须fail-close。
7. 发布包必须由真实、受控的签名/加密工具链生成；零签名、固定IV、伪密文、Naked和独立代码仓私有ABI不得进入产品或EMU路径。

128字节精确布局以`docs/04-interfaces/secure-firmware-package.md`为准；公共数值以`docs/04-interfaces/security-common-abi.md`为准。

### 5. 未由本ADR猜测的内容

本ADR不冻结：

- OPEN-CONFLICT-006中的精确Region offset/容量、PMA和Firewall窗口；地址视图已冻结为baremetal System Address；
- ADR-0019所列eHSM BL专用rollback-counter API的command/packing/LCS/status/readback/交付版本，以及Vendor资源寿命；
- provisioning/release matrix中的设备、key、board和LCS具体行；三套算法Profile本身均为必实现、必测试能力。

这些实现输入不能改变已批准的包分层、`rollback_counter[16]`、Host可读`uint32_t version`、64位地址、双阶段长度校验和真实制包要求。

## ADR-0019后续补充

Manifest v1在offset64保存`rollback_counter[16]`，offset124保存little-endian `uint32_t version`供Host工具读取。Header固定128字节且不携带ABI major/minor；当前无产品发布兼容负担，旧草案包必须重新生成。

## 选择理由

- 保持Vendor交付边界，降低移植偏离和后续Vendor更新冲突。
- 把不可信输入规范化与认证结果校验分开，避免长度歧义、尾随数据和Header/调用参数不一致。
- 让SoC项目语义处于Vendor签名/加密保护范围内，同时不污染Vendor wire命名空间。
- 使发布工具、parser、loader、Measurement和测试共用一份ABI事实源。

## 安全影响

- 消除同一包存在多个长度解释、尾随数据被不同组件解释以及项目类型被未认证外层元数据替换的风险。
- Vendor PASS后仍保持项目policy、digest、loader和release的fail-close门禁。
- Manifest `rollback_counter`与认证Vendor `Version_Counter`逐octet相等，避免两个防回滚视图漂移；Manifest `version`只作发布版本。
- 真实制包门禁阻止当前零签名/未加密stub流程进入EMU和产品。

## 软件影响

- release工具需生成并独立复验Vendor Header、Manifest和payload。
- BootROM/FMC/GSP共用同一ABI registry、package preflight、post-check、Manifest parser和loader合同。
- Vendor公共代码保持只读；NGU差异只进入port、adapter、profile、policy和stage orchestration。
- `gsp-pmp-rmp-omp`现有32位counter、私有Manifest和synthetic package实现仅作为差距输入，不能直接作为产品ABI。

## 测试影响

必须增加正常包与截短、尾随、`Code_Size`大小不等、Header `Version_Counter`/Manifest `rollback_counter`不等、`version`误用于防回滚、未知required扩展、地址截断、digest错误、stub/零签名制包拒绝等negative corpus。preflight失败必须证明Vendor命令未提交；post-check失败必须证明Manifest、loader、Measurement和release均未发生。

## 风险

最终绝对地址、算法profile和counter物理命令仍开放；在这些内容冻结前可以实现并验证纯parser/状态机，但不能发布最终目标包或完成真实counter写入路径。

## 参考资料

- `sources/conflict-reports/CONFLICT-VENDOR-NATIVE-PACKAGE-SEMANTICS.md`
- `evidence/code-investigations/CE-SEC-009-secure-package-verify-loader-contract.md`
- `docs/04-interfaces/secure-firmware-package.md`
- `docs/04-interfaces/image-verify-loader.md`
- `docs/04-interfaces/security-common-abi.md`
