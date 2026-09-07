---
title: "BootROM、FMC、GSP函数级状态机与代码落点"
status: review
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0012
  - SRC-0014
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
  - SRC-0023
  - SRC-0033
  - SRC-0034
  - SRC-0036
owners:
  - GSP
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

把已批准的包、eHSM、counter、Measurement和release原则展开成可以直接指导编码评审的函数边界与显式状态机。本文不授权修改代码仓，不冻结OPEN-CONFLICT-006涉及的绝对地址，也不把现有test/stub/demo流程当作目标实现。

> 主详设反向链接：[《NGU800P安全软件详细设计》第6～8章](NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)

# 公共实现约束

1. BootROM、FMC和GSP复用同一套package/Header Overlay/typed-stage policy/loader/Measurement组件，不各自复制parser。
2. 公共组件无heap、无递归；context和buffer由stage静态分配并显式传入。
3. wire数据逐字段解析；所有长度、offset、地址和转换先做防溢出检查。
4. 首版每stage最多一条eHSM在途事务，固定poll、零自动retry；timeout为`ACCEPTANCE_UNKNOWN`并quarantine service/context。
5. generic verify/loader不得执行jump、reset、counter update或release；这些副作用只由stage orchestrator调用。
6. eHSM Vendor FW走独立counter域的Vendor type 0专用boot流程且不解释NGU Overlay；FMC/GSP/PMP/RMP/MMP及Die1 SoC stage包全部固定type 1、`check_version=0`与Header Overlay，产品入口拒绝type 2/3。
7. 产品和EMU source graph禁止stub、simulated success、test key/cert/provider、零签名制包和silent fallback。

# 公共逻辑接口

以下名称是目标逻辑接口，进入编码前可按仓库命名规范机械调整，但职责、输入输出和调用顺序不得合并或省略。

```c
typedef uint32_t ngu_sec_status_t;

ngu_sec_status_t ngu_sec_package_preflight(
    const ngu_sec_package_view_t *package,
    const ngu_sec_stage_profile_t *profile,
    ngu_sec_preflight_result_t *result,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_vendor_verify_soc_image(
    ngu_ehsm_service_t *service,
    const ngu_sec_verify_request_t *request,
    ngu_sec_vendor_result_t *result,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_authenticated_header_check(
    const ngu_sec_verify_request_t *request,
    const ngu_sec_vendor_result_t *vendor,
    ngu_sec_authenticated_header_t *header,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_header_overlay_parse(
    const ngu_sec_authenticated_header_t *header,
    ngu_sec_header_overlay_t *overlay,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_typed_stage_policy_check(
    const ngu_sec_authenticated_header_t *header,
    const ngu_sec_header_overlay_t *overlay,
    const ngu_sec_stage_profile_t *profile,
    const ngu_sec_rollback_context_t *rollback,
    ngu_sec_policy_result_t *result,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_image_load(
    const ngu_sec_load_request_t *request,
    ngu_sec_load_result_t *result,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_measurement_prepare(
    const ngu_sec_verified_image_t *image,
    const ngu_sec_load_result_t *load,
    ngu_measurement_pending_t *pending,
    ngu_sec_error_t *error);

ngu_sec_status_t ngu_sec_measurement_commit(
    ngu_measurement_table_t *table,
    const ngu_measurement_pending_t *pending,
    ngu_measurement_commit_result_t *result,
    ngu_sec_error_t *error);
```

## 接口固定语义

- `preflight`必须执行`Code_Size == package_size - 1024`、Header CRC-32/ISO-HDLC和末尾reserved检查，失败时证明eHSM命令未提交。
- `vendor_verify`只封装Vendor Host API和raw result，不决定typed-stage identity或policy。
- `authenticated_header_check`再次执行精确长度，并输出认证Header view。
- `header_overlay_parse`只做offset1008 LE64、offset1016 LE32 Header CRC、offset1020零reserved和有界解析；CRC覆盖offset256～1015，只做格式筛查，不替代Vendor认证。`typed_stage_policy_check`做内部stage/image/instance/die、精确固定System Address、entry=load、rollback counter、算法/LCS和release matrix策略。
- 包内不携带expected digest；`image_load`按受信Profile计算`target+1024`处`Code_Size`字节摘要，搬移后从目标回读重算并常量时间比较。
- `image_load`返回实际System Address load/entry及源/目标摘要，且entry等于load；Measurement记录loader确认后的目标摘要，不照抄未认证Header地址。
- `measurement_commit`成功不自动release；stage必须单独检查commit结果后调用release。
- 所有接口失败均填充公共`ngu_sec_error_t`，同时保留`raw_vendor_status`；错误对象不含payload、key或明文secret。

# 公共镜像处理子状态机

```text
EMPTY
  -> SEALED
  -> PREFLIGHT_PASS
  -> VENDOR_PASS
  -> HEADER_PASS
  -> OVERLAY_PASS
  -> POLICY_PASS
  -> LOADED_AND_READBACK_VERIFIED
  -> MEASUREMENT_COMMITTED
  -> RELEASED
```

任一确定完成失败进入`FAILED/ISOLATED`；无法证明Vendor是否接受命令时进入`QUARANTINED`。状态只能单向前进，清理后接收Host重新发送必须创建新的`request_id`和新的状态实例，不能把旧失败对象回退成`EMPTY`。

# BootROM状态机

## State

| 值 | 状态 | Entry动作 | 成功出口 |
|---:|---|---|---|
| 0 | `BOOTROM_RESET` | 从不可变reset vector进入；回读证明FMC reset/clock/NX和Measurement写Owner；先失效Header、再清零并回读整个Measurement Region | `MIN_INIT` |
| 1 | `BOOTROM_MIN_INIT` | 最小平台、timer、cache基础并保持默认deny权限 | `READ_BOOT_POLICY_INPUTS` |
| 2 | `BOOTROM_READ_BOOT_POLICY_INPUTS` | 先经命名只读平台接口读取`non_sec_boot`及valid/ECC/镜像状态；值0时再读取SoC LCS，并仅对已识别非USER读取raw Strap | `SELECT_BOOT_MODE`；policy-fuse输入异常→`TERMINAL` |
| 3 | `BOOTROM_SELECT_BOOT_MODE` | 纯策略函数按`non_sec_boot`→Lifecycle→Strap优先级选择`mode/subprofile/reason` | fuse=1→`NONSECURE_LOCATE(RESTRICTED)`；fuse=0且secure→`WAIT_EHSM`；fuse=0且DEV/MANU+0→`NONSECURE_LOCATE(MANUFACTURING)`；其他非安全原因→`NONSECURE_LOCATE(RESTRICTED)` |
| 4 | `BOOTROM_WAIT_EHSM` | error优先poll eHSM BL ready/error；eHSM按eFuse自主执行或跳过自检 | `READ_EHSM_BOOT_RESULT` |
| 5 | `BOOTROM_READ_EHSM_BOOT_RESULT` | 只读取raw自检状态/位图，不发起自检 | PASS或NOT_REQUIRED→`LOCATE_FMC` |
| 6 | `BOOTROM_LOCATE_FMC` | 定位、边界检查并seal FMC package | `PREFLIGHT_FMC` |
| 7 | `BOOTROM_PREFLIGHT_FMC` | 未认证Header安全预检、Header CRC及精确长度检查 | `VERIFY_FMC` |
| 8 | `BOOTROM_VERIFY_FMC` | 真实eHSM verify/decrypt | `CHECK_AUTH_HEADER` |
| 9 | `BOOTROM_CHECK_AUTH_HEADER` | PASS后复验认证Header、CRC和稳定性 | `CHECK_HEADER_OVERLAY_POLICY` |
| 10 | `BOOTROM_CHECK_HEADER_OVERLAY_POLICY` | offset1008/1016/1020、typed FMC固定目标、policy和Header/candidate一致性；无Manifest | `CONFIRM_STAGED_COUNTER` |
| 11 | `BOOTROM_CONFIRM_STAGED_COUNTER` | 确认BL已暂存本次FMC candidate；不读/比/写stored counter | `LOAD_FMC` |
| 12 | `BOOTROM_LOAD_FMC` | loader复制、write/release、目标readback摘要、指令侧同步、W^X/Firewall并返回actual地址；不执行data clean/invalidate | `COMMIT_FMC` |
| 13 | `BOOTROM_COMMIT_FMC` | 以真实digest/load/entry提交唯一FMC Measurement条目 | `FINAL_RELEASE_CHECK` |
| 14 | `BOOTROM_FINAL_RELEASE_CHECK` | 复验全部门禁和一次性标志 | `RELEASE_FMC` |
| 15 | `BOOTROM_RELEASE_FMC` | 最终检查并一次性跳转FMC entry | 无返回 |
| 16 | `BOOTROM_NONSECURE_LOCATE` | 按锁定子Profile定位独立Provisioning FW或受限FMC，并检查source/长度/owner/Profile一致性 | `NONSECURE_LOAD`；禁止切换Profile重试 |
| 17 | `BOOTROM_NONSECURE_LOAD` | 执行range/W^X/目标readback/指令同步/Firewall受控加载；制造Profile只开放eHSM BL typed Mailbox最小通路 | `NONSECURE_RELEASE` |
| 18 | `BOOTROM_NONSECURE_RELEASE` | 不创建安全Measurement Entry/SoC State或启动审计；一次性跳转对应entry | 无返回 |
| 19 | `BOOTROM_NONSECURE_TERMINAL` | 撤销临时权限并停在受限终态 | 无安全出口 |
| 127 | `BOOTROM_TERMINAL` | 错误记录/RAS上报/fail-stop | 无安全出口 |

`BOOTROM_SELECT_BOOT_MODE`按SRC-0034/ADR-0031/0034先处理`non_sec_boot`：快照有效、ECC正常且值1时，包括USER在内均输出`NON_SECURE_BOOT/RESTRICTED_NONSECURE`；值0时USER进入secure链，DEV/MANU且`boot_pin.secure_boot[3]=0`输出`MANUFACTURING_PROVISIONING`，其他非安全原因输出`RESTRICTED_NONSECURE`，Strap=1进入secure链。policy-fuse异常进入`TERMINAL`。BootROM本身不写OTP；制造Provisioning FW release后自行等待eHSM BL并只调用typed制造接口。两个非安全子Profile不创建安全Measurement Entry、SoC State或启动审计，且不得互相fallback。

SRC-0023截图中的`boot_pin.secure_boot[3]`、default 0、0非安全/1安全是产品权威。当前SRC-0022 RTL生成头中的`SEC_BOOT` bit0和`DIE_ID` bit3与之冲突；状态2的真实寄存器绑定在RTL/生成头或批准映射同步前保持`BLOCKED_BY_RTL_SYNC`。代码和真实测试只可引用同步后的生成命名宏，禁止使用当前bit0、当前bit3或裸bit。

`non_sec_boot`的物理word/bit/编码、ECC/valid、镜像、复位锁存和只读接口尚未绑定。状态2只允许消费专用Boot Policy Fuse逻辑快照，禁止raw eFuse；目标资料到齐前产品port保持`BLOCKED_BY_NON_SEC_BOOT_BINDING`。

## Ready条件

BootROM仅在以下条件同时成立时离开`WAIT_EHSM`：

```text
hw_boot_done == 1
bootloader_done == 1
hw_boot_err == 0
bootloader_err == 0
```

错误位优先于done位解释。任何已置错误不得因done同时置位而继续；轮询超过批准deadline按acceptance unknown/fail-stop处理。

eHSM自主自检：eHSM依据自身eFuse决定是否执行，BootROM不得发送启动自检命令。eFuse要求自检时只有Vendor报告PASS才可继续；eFuse明确不要求时NOT_REQUIRED是合法结果。BootROM始终保存raw bitmap，bit18按TRNG解释、bit19保持unknown/reserved。

## BootROM函数边界

```c
_Noreturn void ngu_bootrom_entry(void);

ngu_sec_status_t ngu_bootrom_flow_step(
    ngu_bootrom_context_t *context,
    const ngu_bootrom_platform_ops_t *ops);
```

- 产品入口只创建/取得静态context并反复调用`flow_step`；不包含test分支。
- `flow_step`便于host unit test逐状态注入失败，但产品构建必须绑定真实`ops`。
- 模式判定必须通过纯策略函数消费不可变policy-fuse/Strap/LCS snapshot；寄存器读取、policy和副作用不得合并。
- FMC rollback counter必须来自eHSM PASS后认证Header，并在Measurement commit后供FMC读取。
- BootROM不更新counter、不加载eHSM Vendor FW、不自行reset。
- 只有`COMMIT_FMC`成功且entry/Firewall最终复验通过才允许`RELEASE_FMC`。

# FMC状态机

## State

| 值 | 状态 | Entry动作 | 成功出口 |
|---:|---|---|---|
| 0 | `FMC_ENTRY` | 清理静态context，建立stage identity | `VALIDATE_SELF_MEAS` |
| 1 | `FMC_VALIDATE_SELF_MEAS` | 消费BootROM提交的唯一FMC条目 | `INIT_EHSM` |
| 2 | `FMC_INIT_EHSM` | poll模式初始化FMC eHSM adapter | `COMMIT_STAGED_COUNTER` |
| 3 | `FMC_COMMIT_STAGED_COUNTER` | 回传FMC expected candidate；BL exact-match后compare/update/readback | `WAIT_GSP_PACKAGE` |
| 4 | `FMC_WAIT_GSP_PACKAGE` | 等待Host提供新的GSP package | `SEAL_GSP_PACKAGE` |
| 5 | `FMC_SEAL_GSP_PACKAGE` | range/owner检查并撤销Host写权限 | `VERIFY_GSP` |
| 6 | `FMC_VERIFY_GSP` | 以type 1、`check_version=0`处理至Header Overlay/typed-stage policy PASS | `CHECK_GSP_COUNTER_EQUAL` |
| 7 | `FMC_CHECK_GSP_COUNTER_EQUAL` | 要求GSP counter等于FMC初始化已提交值 | `LOAD_GSP` |
| 8 | `FMC_LOAD_GSP` | loader完成源摘要、复制、目标回读摘要比较和执行保护 | `COMMIT_GSP` |
| 9 | `FMC_COMMIT_GSP` | 提交GSP Measurement条目 | `RELEASE_GSP` |
| 10 | `FMC_RELEASE_GSP` | release GSP | 无返回 |
| 11 | `FMC_REARM_INGRESS` | 清理确定失败对象并等待Host新包 | `WAIT_GSP_PACKAGE` |
| 127 | `FMC_TERMINAL` | 共享服务/不可判定失败终态 | 无安全出口 |

## FMC固定规则

1. FMC自身rollback counter只从当前启动Table中唯一、BootROM生产、verify PASS、release authorized、已commit且完整性通过的FMC Measurement条目取得；不重新验证自身package。
2. FMC初始化先把FMC Entry expected candidate回传BL；必须与RAM candidate逐字节一致，之后低于物理counter拒绝、等于不写、高于单向写，并readback证明。
3. counter proof完成后才接收GSP；GSP固定`check_version=0`并必须等于已提交值。
4. GSP等值检查完成前不commit GSP Measurement、不release GSP。
5. 签名/格式/policy等**确定完成**失败可进入`REARM_INGRESS`，等待Host发新包；device不自动请求、不重试旧命令。
6. timeout、异常BUSY或counter update结果不明进入`FMC_TERMINAL/QUARANTINED`，不得复用context或继续收包。
7. FMC不加载eHSM Vendor FW；该工作在GSP阶段执行。
8. staged commit状态必须绑定eHSM BL新增专用16字节exact-match API；具体command/packing/LCS/交付版本在实现前冻结，不能把candidate暂存当作commit。

# GSP状态机

## 启动Owner

ADR-0015已冻结：第一个、最高优先级的`security_service_task`是GSP唯一eHSM service owner。该task从bootstrap开始持有唯一context，完成eHSM FW和Runtime门禁后直接转为长期串行服务循环，不做“bootstrap owner交给runtime owner”的隐式所有权切换。

该选择的安全收益是：

- 单在途和active timeout/cache scope从启动延续到运行期；
- 避免任务切换时遗留channel、context或late response；
- 其他任务只能提交typed request，不能取得Vendor raw handle。

其他task只能通过typed queue提交caller identity、operation、descriptor、deadline和request ID，不得取得Vendor raw handle、channel或context。queue在bootstrap门禁完成前不对普通runtime caller开放；quarantine后不通过Owner移交恢复，只允许RAS批准的新boot instance重新初始化。

## State

| 值 | 状态 | Entry动作 | 成功出口 |
|---:|---|---|---|
| 0 | `GSP_ENTRY` | 建立唯一service owner和静态队列 | `VALIDATE_SELF_MEAS` |
| 1 | `GSP_VALIDATE_SELF_MEAS` | 消费FMC提交的唯一GSP条目 | `INIT_SERVICE` |
| 2 | `GSP_INIT_SERVICE` | 初始化poll adapter，禁止外部请求 | `WAIT_EHSM_FW` |
| 3 | `GSP_WAIT_EHSM_FW` | eHSM保持只运行BL；等待Host下发并seal完整Vendor type 0包 | `BOOT_EHSM_FW` |
| 4 | `GSP_BOOT_EHSM_FW` | GSP以`boot=true, image_out=NULL`请求BL执行Vendor type 0验证/启动；使用独立eHSM FW counter域，不推进SoC counter | `WAIT_FW_READY` |
| 5 | `GSP_WAIT_FW_READY` | 等待`firmware_done=1 && firmware_err=0` | `LOAD_RUNTIME_SET` |
| 6 | `GSP_LOAD_RUNTIME_SET` | 按profile依赖图处理PMP/RMP/MMP | `START_SERVICES` |
| 7 | `GSP_START_SERVICES` | release已满足依赖的Runtime和安全服务 | `RUN_SERVICE` |
| 8 | `GSP_RUN_SERVICE` | 串行typed request队列 | 自循环 |
| 9 | `GSP_DEGRADED` | 保留最小错误/管理控制面 | 仅批准恢复路径 |
| 127 | `GSP_QUARANTINED` | 共享eHSM service不可判定失败 | 无普通服务出口 |

## Runtime per-image状态

eHSM FW、PMP、RMP、MMP各有独立记录；不得用单一global成功位覆盖：

```text
EMPTY -> STAGED -> VERIFYING -> VERIFIED -> LOADED
      -> MEASUREMENT_COMMITTED -> RELEASED
```

- PMP/RMP/MMP的确定验证失败只将对应镜像置为`ISOLATED`；依赖该镜像的consumer保持`NOT_RELEASED`，但不伪造为自身验证失败。
- eHSM FW失败使所有依赖Vendor FW的服务不可用；最小错误/RAS管理控制面可按批准策略保留。
- 共享transport completion unknown使整个service进入`QUARANTINED`，不是单镜像隔离。
- Runtime重发由Host决定；每次新包使用新`request_id`并重新seal。

## Runtime依赖图

首版不按PMP/RMP/MMP名称猜测固定加载先后。stage profile必须显式给出：

- image type/instance；
- 依赖image集合；
- package source和最大值；
- load/entry System Address与Region；
- 目标`die_id`和Measurement实例元组生成规则；
- release primitive；
- 失败后的可保留最小服务。

依赖图必须无环；只有依赖项均`RELEASED`时才可release consumer。实际PMP/RMP/MMP依赖关系和release primitive仍需项目输入后冻结。

# Stage profile

```c
typedef struct {
    uint32_t consumer_stage;
    uint32_t image_type;
    uint32_t instance_id;
    uint8_t allowed_vendor_image_type_mask;
    uint8_t reserved0[3];
    uint64_t max_package_size;
    uint64_t max_code_size;
    uint32_t target_region_id;
    uint32_t die_id;
    uint32_t expected_algorithm_profile;
    uint32_t expected_policy_flags;
    uint32_t dependency_mask;
    uint32_t release_profile_id;
} ngu_sec_stage_profile_t;
```

包内不携带`measurement_slot`或身份。实际Entry由受信stage profile的`image_type/instance_id/die_id`和append顺序定位。

最终C wire/packing不能直接复制该逻辑结构；其中profile ID、容量、Region、die和release值必须在对应开放项关闭后由registry生成。

# 错误与清理顺序

确定失败的固定顺序：

1. 阻断当前及下游release；
2. 保存stage/state/request/project domain+reason/raw Vendor status；
3. 撤销临时Host/loader权限；
4. 清零明文输出、目标未授权内容和临时digest；
5. 执行write/release、acquire/read和full-system barrier；两个批准PMA候选均不执行data clean/invalidate；
6. 提交错误记录并向RAS单向上报；
7. 当前对象进入`ISOLATED`或stage fail-stop。

acceptance unknown时不得按普通失败清理/复用可能仍被eHSM访问的package、output、context和channel；它们与service一起quarantine，等待RAS批准的reset/recovery新boot instance。

# 代码落点

进入实现阶段后，目标落点建议为：

| 责任 | `gsp-pmp-rmp-omp`目标 |
|---|---|
| 公共ABI/registry生成物 | `components/security/include/security/ngu_sec_abi.h` |
| Vendor Header view | `components/security/include/security/ngu_vendor_image.h`及对应`.c` |
| Header Overlay/parser | `ngu_secure_header_overlay.h/.c`；production不链接Manifest parser |
| Package/verify orchestration | `ngu_secure_package.h/.c` |
| Policy/stage profile | `ngu_secure_profile.h/.c` |
| Loader | `ngu_secure_loader.h/.c` |
| Measurement commit | 现有measurement组件按最终ABI重构 |
| eHSM adapter | 真实Vendor Host adapter；公共Vendor源码不修改 |
| BootROM orchestrator | `solutions/bootrom`安全入口/flow文件 |
| FMC orchestrator | `solutions/fmc`安全入口/flow文件 |
| GSP owner/task | `solutions/gsp`首个security service task及队列 |
| Release工具 | 受控真实签名/加密packager和独立validator |

具体文件名可随仓库现状调整，但不得把公共parser重新散落到三个solution。`baremetal`只复用ABI生成物或fixture来执行测试，不承载产品policy和开发规划。

# 编码准入门禁

以下条件满足前仍不授权产品编码：

1. BootROM `non_sec_boot`逻辑和Strap/LCS模式及两个非安全子Profile已由ADR-0031/0018/0034关闭；真实编码仍需OPEN-DESIGN-024提供只读Boot Policy Fuse绑定，并使用RTL生成宏验证Strap/LCS read path；
2. OPEN-DESIGN-010补齐Runtime依赖/release输入；GSP全生命周期唯一service owner已由ADR-0015冻结；
3. OPEN-CONFLICT-006冻结每个镜像Region、最大尺寸、load/entry和C908 linker/PC视图；
4. OPEN-CONFLICT-005/009已由ADR-0019关闭；实现前冻结eHSM BL新增API的具体command/packing/LCS/交付版本，不重开eHSM FW加载stage；
5. ADR-0022已冻结Measurement逻辑C layout/commit，ADR-0028固定物理16 KiB和126项硬上限；编码前仍需OPEN-DESIGN-021产品实际实例数和OPEN-CONFLICT-006 PMA/Firewall输入；
6. 启动链算法集合已由ADR-0017冻结为三套；ADR-0020批准具体设备、镜像、key域、boot/upgrade key、LCS和board绑定来自单一matrix，制包器/OTP/EMU前补齐实际行且缺失默认拒绝；
7. 产品构建source graph有可验证的no-stub门禁。

# References

- [公共ABI](../04-interfaces/security-common-abi.md)
- [安全固件包](../04-interfaces/secure-firmware-package.md)
- [Verify/Loader](../04-interfaces/image-verify-loader.md)
- [Measurement Table](../04-interfaces/measurement-table.md)
- [ADR-0014](../../decisions/ADR-0014-secure-package-manifest-and-length-canonicalization.md)
- [ADR-0015](../../decisions/ADR-0015-gsp-lifetime-ehsm-service-owner.md)
- [CE-SEC-011](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)
- [OPEN-CONFLICT-009](../../sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md)
- [SRC-0023 Source Card](../../sources/source-cards/SRC-0023.md)
- [SRC-0034 Source Card](../../sources/source-cards/SRC-0034.md)
- [OPEN-CONFLICT-010](../../sources/conflict-reports/CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY.md)
- [ADR-0031](../../decisions/ADR-0031-non-sec-boot-efuse-override.md)
- [ADR-0034](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)
- [ADR-0033](../../decisions/ADR-0033-native-header-crc32-format-check.md)
- [主详设第6章](NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)
