# ADR-0019：Rollback Counter、Manifest Version与GSP Host边界

> 2026-08-21后续裁决：[ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](ADR-0030-remove-manifest-and-use-native-header-tail.md)删除Manifest及其Host可读version/重复counter字段；Vendor Header `Version_Counter[16]`、FMC staged-candidate exact-match提交和禁止Host访问GSP安全服务继续有效。展示版本如有需要只存在于外部受控release metadata。

- Status: accepted
- Date: 2026-07-27
- Owner: 项目负责人
- Related: OPEN-CONFLICT-005、OPEN-CONFLICT-009、OPEN-DESIGN-006、ADR-0005、ADR-0012、ADR-0014
- Last reviewed: 2026-07-28

> 2026-07-28更新（历史）：本ADR的Manifest布局当时被ADR-0024替代；Manifest及其中的`rollback_counter/version`副本又于2026-08-21被ADR-0030删除。Counter提交Owner和不向Host开放GSP服务的结论不变；现行包合同见[《NGU800P安全软件详细设计》第3章](../docs/05-software-design/NGU800P安全软件详细设计.md#第3章-固件packagengu-manifest制作与发布)。

## Context

Vendor手册、Vendor Header和匹配代码中的SoC `Version_Counter`均为16字节。此前冲突中的“32”来自SRC-0016旧Measurement示例的`uint32_t stored_global_counter/image_counter`，表示32位而不是32字节；Host侧`uint64_t`通用Counter又属于另一套未证明映射的资源。两者都不应继续造成产品Counter宽度歧义。

项目还需要把“供上位机读取的软件版本”与“防回滚单向Counter”分离，并明确GSP不向Host提供安全服务。

## Decision

1. 产品SoC防回滚值唯一命名为`rollback_counter`，固定为16字节opaque octets；其Vendor物理源是16字节`Version_Counter`。禁止32位/64位截断、第二套项目Counter或`__int128`布局依赖。
2. NGU Manifest v1在offset 64保存`rollback_counter[16]`，并与已认证Vendor Header `Version_Counter[16]`逐octet相等。
3. NGU Manifest v1把原offset 124的`reserved0`改为`version`，类型为little-endian `uint32_t`。它是供Host/上位机制包、发布和本地包解析工具读取和显示的固件发布版本，不参与安全启动接受、防回滚、Counter比较或OTP更新，也不因此新增运行期GSP查询服务。若工具只持有无法解密的成品密文包，则从同一受控制包输入生成的受认证release metadata读取，不得从密文猜测或绕过包保护。
4. 当前没有已发布的NGU Manifest v1产品ABI，因此上述调整直接进入v1基线，`header_size`固定为128；Manifest不携带`abi_major/abi_minor`。所有旧草案包必须重新生成，不提供旧`security_epoch/reserved0`布局兼容。
5. FMC固定为Vendor image type 1。BootROM以`check_version=0`验证FMC，Vendor BL把认证`Version_Counter[16]`及valid状态暂存在RAM；BootROM不独立读取、比较或写stored SoC counter。
6. BootROM把同一candidate写入并commit唯一FMC Measurement Entry。FMC初始化从该有效Entry取得`expected_candidate`，主动调用eHSM BL新增的专用staged-candidate commit API；该调用发生在接收GSP之前。
7. eHSM BL新增的是typed产品API，不是raw OTP或通用Counter透传。逻辑合同为：

```c
int32_t ehsm_bl_commit_staged_soc_rollback_counter(
    ehsm_ctx_t *ctx,
    const uint8_t expected_candidate[16],
    ehsm_bl_rollback_counter_result_t *result);
```

BL必须先把`expected_candidate`与RAM candidate逐字节比较；无candidate、invalid或不一致均在OTP改动前拒绝。匹配后，`result`至少返回before/after 16字节值、比较关系、是否实际写入、权威读回状态和Vendor raw status。候选低于stored时拒绝；相等时成功但不写；高于stored时按Vendor单向编码更新并读回。timeout/acceptance unknown不得自动重试。
8. commit proof成立后FMC才接收GSP。GSP及后续SoC镜像固定Vendor type 1、`check_version=0`且认证counter必须等于已提交值；Vendor type 2/3直接拒绝，Vendor type 0 eHSM FW属于独立counter域。
9. 该API必须由eHSM BL交付边界正式增加；FMC只调用批准的Host/BL API，不直接访问raw OTP offset，也不把现有`ehsm_read_counter(uint64_t)`映射为SoC rollback counter。
10. GSP不向Host提供通用算法、Key、Certificate、Rotation、raw eHSM或其他安全服务API。Host只能使用方案明确规定的独立产品协议入口，例如固件下发、SPDM、受控Debug/OOB流程；这些入口不属于“GSP通用安全服务开放”。

## Consequences

- OPEN-CONFLICT-005的宽度/命名问题关闭。
- OPEN-CONFLICT-009从“是否存在/映射接口”变为已批准的eHSM BL staged-candidate commit API实施项；准确command ID、结构packing、LCS权限和代码交付仍需在实现前冻结，但不再重开Owner或选择通用Counter。
- OPEN-DESIGN-006关闭，默认和最终结论均为不向Host开放GSP安全服务。
- Manifest parser、制包工具、Measurement、测试工作簿和上位机工具必须分别处理`version`和`rollback_counter`，不得互相替代。
- 本ADR只更新`security_-scheme`设计和任务约束；不在本次直接修改Vendor、`gsp-pmp-rmp-omp`或`baremetal`代码。
