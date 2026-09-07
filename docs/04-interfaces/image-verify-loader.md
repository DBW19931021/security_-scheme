---
title: "镜像验签解密、Loader与Release接口合同"
status: approved
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0014
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0033
  - SRC-0036
owners:
  - GSP
last_reviewed: 2026-09-01
supersedes:
  - Manifest-based verify-loader contract
superseded_by: []
---

# Purpose

把Vendor镜像验签解密、Native Header Overlay、Header CRC、typed-stage policy、loader、Measurement和stage release拆成可审计接口，冻结“Vendor PASS不等于允许执行”的状态机。本合同依据[ADR-0030](../../decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)删除Manifest parser和第二套payload/entry长度来源，并依据[ADR-0033](../../decisions/ADR-0033-native-header-crc32-format-check.md)加入Header格式校验。

> 主详设反向链接：[《NGU800P安全软件详细设计》第9章“Verification、Loader、Counter与Measurement”](../05-software-design/NGU800P安全软件详细设计.md#第9章-verificationloadercounter与measurement)

# 1. 分层原则

```text
Host ingress/seal
  -> package preflight（Header不可信，只能拒绝）
  -> Header CRC32格式预检
  -> Vendor eHSM verify/decrypt
  -> authenticated Header稳定复验
  -> Header Overlay + typed-stage policy
  -> source Code digest
  -> memmove(target, target + 1024, Code_Size)
  -> target readback digest compare
  -> counter/Measurement commit
  -> stage-owned release
```

1. 任一失败都不得跳过后续门禁。
2. generic verify/loader不得jump、写release寄存器、更新物理counter或把Measurement标为已运行。
3. eHSM type 0 Vendor FW使用独立typed wrapper，不解释NGU Overlay。
4. 产品/EMU路径不得链接stub、模拟成功、旧Manifest parser或dual-format fallback。

# 2. Buffer与地址类型

```c
typedef struct {
    uint64_t system_addr;
    uint64_t size;
    uint32_t owner;
    uint32_t lifecycle;
} ngu_system_span_t;
```

- C908、Header Overlay、loader、linker和eHSM descriptor全部使用baremetal System Address，不存在Local/System转换。
- `system_addr + size`必须防溢出；`size=0`、local/remap地址和跨Region span拒绝。
- ingress、eHSM output、Code target、Measurement和eHSM context arena不得发生未批准alias。
- eHSM可访问整个2 MiB不代表adapter可接受任意指针；仍需检查Owner、固定Region、生命周期和容量。

# 3. Typed-stage descriptor

删除Manifest后，业务身份和策略必须由受信调用上下文给出：

```c
typedef struct {
    uint32_t caller_stage;
    uint32_t image_type;
    uint32_t instance_id;
    uint32_t die_id;
    uint32_t expected_vendor_image_type; /* type 1 */
    uint32_t policy_profile_id;
    uint32_t algorithm_profile_id;
    uint32_t target_region_id;
    uint64_t fixed_load_addr;
    uint64_t max_code_size;
} ngu_typed_stage_profile_t;
```

该对象由中心registry生成并编译进consumer；Host不得填写或覆盖。BootROM只持有FMC profile，FMC只持有GSP profile，GSP按独立typed入口选择PMP/RMP/MMP/Die1 profile。Profile/key/board/SKU/LCS由该对象关联的provisioning/release matrix决定。

# 4. Verify request

```c
typedef struct {
    uint64_t request_id;
    const ngu_typed_stage_profile_t *stage_profile;
    ngu_system_span_t package;
    ngu_system_span_t output;
    uint32_t check_version;             /* SoC stage固定0 */
    uint32_t vendor_boot_after_verify;  /* SoC stage固定0 */
} ngu_verify_request_t;
```

## 4.1 Request不变量

1. `request_id`在当前boot generation内不重复。
2. `package`必须是已seal且Host写权限已撤销的buffer。
3. `output.system_addr == stage_profile->fixed_load_addr`，且目标尚未执行、为`RW/NX`。
4. `output.size >= package.size`；SoC type 1 Vendor输出包含完整Header和解密Code。
5. `package.size <= UINT32_MAX`且同时不超过Host ingress、stage package和目标Region上限。
6. input/output/context不得重叠；output仅允许精确落入当前typed target。
7. 所有SoC stage固定Vendor type 1、`check_version=0`、`vendor_boot_after_verify=0`；type 2/3提交前拒绝。
8. eHSM Vendor FW type 0不用本request，走专用`boot=1` wrapper。

# 5. Verify状态与结果

```c
typedef enum {
    NGU_COMPLETION_NOT_SUBMITTED = 0,
    NGU_COMPLETION_COMPLETED = 1,
    NGU_COMPLETION_ACCEPTANCE_UNKNOWN = 2
} ngu_completion_state_t;

typedef enum {
    NGU_VERIFY_EMPTY = 0,
    NGU_VERIFY_PREFLIGHT_PASS = 1,
    NGU_VERIFY_VENDOR_PASS = 2,
    NGU_VERIFY_HEADER_PASS = 3,
    NGU_VERIFY_OVERLAY_PASS = 4,
    NGU_VERIFY_POLICY_PASS = 5,
    NGU_VERIFY_FAILED = 6,
    NGU_VERIFY_QUARANTINED = 7
} ngu_verify_state_t;

typedef struct {
    uint64_t request_id;
    ngu_completion_state_t completion;
    ngu_verify_state_t state;
    uint32_t raw_vendor_status;
    uint32_t project_error;
    uint8_t authenticated_vendor_image_type;
    uint8_t authenticated_plain_flag;
    uint8_t authenticated_naked_flag;
    uint8_t reserved0;
    uint32_t authenticated_code_size;
    uint32_t authenticated_header_crc32;
    uint8_t authenticated_rollback_counter[16];
    uint64_t authenticated_load_addr;
    ngu_system_span_t authenticated_code_region;
    uint32_t trusted_image_type;
    uint32_t trusted_instance_id;
    uint32_t trusted_die_id;
    uint32_t validated_algorithm_profile;
} ngu_verify_result_t;
```

结果必须无损保存Vendor raw status。`validated_algorithm_profile`表示受信stage/provisioning matrix已经选择并通过兼容检查，不表示Vendor response返回算法值。counter、地址和身份必须复制到stage私有结果，不能悬挂指向Vendor临时响应。

timeout等无法证明命令是否被接受的路径进入`ACCEPTANCE_UNKNOWN`，package/output/context和service均quarantine，不自动retry或清零。

# 6. 两阶段Header校验

## 6.1 Preflight：Header不可信

允许且必须检查：

- buffer/owner/lifecycle/System Address/容量；
- `package_size > 1024`、`package_size <= UINT32_MAX`及防溢出；
- `Code_Size == package_size - 1024`；
- type 1、Plain/Naked/Reserved的提交前约束；
- offset1008 LE64 `load_addr == stage_profile->fixed_load_addr`；
- offset1016 LE32 `ngu_header_crc32`等于对Header offset 256～1015重算的CRC-32/ISO-HDLC；
- offset1020～1023全0；
- `1024 + Code_Size <= target_region_size`。

这些结果只能用于拒绝并限制eHSM副作用；CRC通过不是密码认证，不得驱动counter、Measurement、权限扩大或release。eHSM output地址始终来自`stage_profile`，不得来自Header。

## 6.2 Post-verify：Header已认证

Vendor PASS后从稳定output重新检查：

1. Valid/Reserved/Plain/Naked和Vendor type；
2. `1024 + Code_Size == package_size == required output span`；
3. `Version_Counter[16]`完整复制；
4. offset1008 LE64地址精确等于typed target；offset1016 CRC重算一致；offset1020～1023全0；
5. Code Region恰好是`output[1024 .. 1024 + Code_Size)`；
6. 再次读取安全相关Header字段并确认未变化，包括CRC输入范围、CRC值和reserved。

任一失败分类为`AUTHENTICATED_FORMAT_ERROR`。不得保留“Vendor PASS后解析Manifest”状态。

# 7. Typed-stage policy门禁

1. consumer、image、instance、die必须等于调用的typed入口。
2. algorithm Profile、key域、board/SKU/LCS必须匹配受信provisioning/release matrix。
3. Header `load_addr`必须精确等于stage固定目标，不接受覆盖多个Region或多个stage的宽allowlist。
4. `entry_addr`不编码，固定等于`load_addr`。
5. rollback counter只来自Header；FMC candidate、GSP/global、Runtime/global关系由对应stage policy检查。
6. `Code_Size`是唯一长度；CBC补零属于Code Region，不去padding。
7. 包内不存在expected digest；copy verification由源/目标双摘要完成。

缺少任何受信绑定时fail-close，不能从Host metadata、Header空闲位或默认Profile补齐。

# 8. Loader request/result

```c
typedef struct {
    uint64_t request_id;
    const ngu_typed_stage_profile_t *stage_profile;
    ngu_system_span_t authenticated_package;
    uint32_t code_size;
    uint64_t authenticated_load_addr;
} ngu_loader_request_t;

typedef struct {
    uint64_t request_id;
    uint32_t status;
    uint32_t target_region_id;
    uint64_t bytes_loaded;
    uint64_t actual_load_addr;
    uint64_t actual_entry_addr;
    uint32_t digest_algorithm;
    uint32_t digest_size;
    uint8_t source_digest[64];
    uint8_t target_readback_digest[64];
} ngu_loader_result_t;
```

## 8.1 Loader固定顺序

1. 复核stage profile、固定目标、`Code_Size`、Region、BSS/stack/heap边界和W^X模板。
2. 保持目标`RW/NX`及下游reset；确认eHSM已完成且不会继续访问output。
3. 源Code固定为`target_base + 1024`，长度固定为`Code_Size`。
4. 按受信Profile计算源摘要；完成前不得改写源Code。
5. 执行`memmove(target_base, target_base + 1024, Code_Size)`；禁止`memcpy`。
6. 清理旧Header尾部和目标BSS；CBC补零不剥离。
7. 执行write/release barrier，不执行data clean/invalidate；从`target_base`回读`Code_Size`并重算摘要，常量时间比较。
8. 摘要一致后执行`fence.i`/批准的指令侧同步，提交Measurement，再切换最终W^X/Firewall。
9. 返回`actual_load_addr == actual_entry_addr == fixed_load_addr`和`bytes_loaded == Code_Size`。

确定失败保持NX并清零可安全清理范围；acceptance unknown时quarantine而不清零。FMC 128 KiB、GSP 880 KiB静态目标、PMP/RMP各256 KiB和Host ingress 512 KiB按ADR-0028；MMP DDR Profile未到齐前不得发布。

# 9. Measurement与Release

只有以下条件全部满足才可准备Measurement：

- completion=`COMPLETED`；
- Vendor/Header/Overlay/typed-stage policy全部PASS；
- source digest与target readback digest比较通过；
- actual load/entry/length与stage profile一致；
- rollback门禁满足；
- service未quarantine。

Measurement的image/instance/die来自typed-stage registry；load/entry来自loader实际结果；digest覆盖目标回读的全部`Code_Size`字节。不得从包内自声明值抄写。

| Stage owner | 被release对象 | 最后门禁 |
|---|---|---|
| BootROM | FMC | BL candidate等于认证Header counter；FMC Measurement commit；入口权限锁定 |
| FMC | GSP | staged-candidate commit/readback proven；GSP counter等于全局值；GSP Measurement commit |
| GSP | eHSM Vendor FW | type 0 boot完成；`firmware_done=1 && firmware_err=0` |
| GSP | PMP/RMP/MMP | 对应Measurement commit；counter/依赖/Firewall门禁满足 |

generic verify/loader只返回候选结果，不能执行release。release返回一律FATAL。

# 10. 错误分类

| Error domain | 示例 | Completion | 后续 |
|---|---|---|---|
| Preflight | 长度/容量/地址/offset/CRC/reserved/overflow | NOT_SUBMITTED | 不调用eHSM；可清理后等新包 |
| Vendor verify | 签名、密文、key失败 | COMPLETED | 保存raw status；不load |
| Transport unknown | timeout且不知是否接受 | ACCEPTANCE_UNKNOWN | quarantine；不自动retry |
| Authenticated format | Header长度/flags/Overlay/稳定性错误 | COMPLETED | 不load |
| Policy | stage/Profile/LCS/counter不允许 | COMPLETED | 隔离对象；不load |
| Loader | overlap/range/move/digest/Firewall失败 | COMPLETED | 保持NX；清理可清范围 |
| Measurement/counter | commit/update/readback失败 | COMPLETED或UNKNOWN | 不release |

# 11. 代码落点（实施阶段）

`gsp-pmp-rmp-omp`目标处理：

- `ehsm_image.*`：Vendor Header有界view、offset1008/1016/1020 parser和CRC-32/ISO-HDLC校验；
- `manifest.*`：从production source graph删除，不保留兼容parser；
- `verify_flow.*`：实现本状态机，不直接release；
- 新增typed-stage policy/profile/registry和原地loader；
- `measurement.*`：只接受loader实际结果和受信identity；
- `tools/image_packager/`：真实Header Overlay、签名/加密和独立复检；
- production/EMU source graph静态拒绝stub、old manifest和dual parser符号。

本文只更新设计，不授权本轮修改产品代码。

# 12. 必须验证的行为

- Preflight失败确保Vendor命令未提交；
- offset1008 LE64、offset1016 CRC算法/范围/值/字节序、offset1020 reserved、32位截断、错stage目标和宽allowlist误接受；
- CRC正确但Vendor签名错误仍拒绝，证明CRC不构成认证旁路；
- PASS前字段不产生授权；PASS后Header二次读取稳定性；
- 三套Profile正常/负向包和provisioning不匹配；
- 旧Manifest包、dual-format开关和第二包头拒绝；
- raw Vendor status不丢失；acceptance unknown不retry/复用；
- `target+1024`、`Code_Size`、重叠搬移、源/目标摘要和BSS清理；
- CBC补零不被设备端错误去除；
- `actual_entry_addr == actual_load_addr`；
- Measurement commit失败不release；
- Vendor Header/算法演进导致尾16B占用条件失效时兼容门禁失败。

# 13. Open items

1. PMA/Firewall、MMP DDR Profile和最终link map峰值仍阻断production linker/release。
2. eHSM BL staged-candidate commit接口的command/packing/LCS/交付版本待Vendor绑定。
3. 具体设备/镜像的Profile、key、board/LCS provisioning matrix行缺失时默认拒绝。
4. operation deadline按OPEN-DESIGN-001管理。

# References

- [《安全固件包与Native Header Overlay合同》](secure-firmware-package.md)
- [ADR-0033《Native Header增加CRC32格式校验》](../../decisions/ADR-0033-native-header-crc32-format-check.md)
- [《安全公共ABI与Registry》](security-common-abi.md)
- [《Boot/FMC/GSP函数级状态机》](../05-software-design/boot-stage-state-machines.md)
- [ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](../../decisions/ADR-0030-remove-manifest-and-use-native-header-tail.md)
- [CE-SEC-016《Native Header尾部占用与签名覆盖调查》](../../evidence/code-investigations/CE-SEC-016-native-header-tail-and-signature-coverage.md)
- [CE-SEC-011《Vendor Version Counter合同调查》](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)
