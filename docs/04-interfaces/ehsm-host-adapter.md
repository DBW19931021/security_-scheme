---
title: "NGU800P eHSM Host Adapter详细合同"
status: review_ready_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0022
owners:
  - GSP
last_reviewed: 2026-07-28
supersedes: []
superseded_by: []
---

# Purpose

把ADR-0010/0011转成可直接指导BootROM、FMC和GSP实现的eHSM Host adapter合同。本文冻结首版分层、结构体职责、单在途状态机、Vendor port scope、cache顺序、deadline、retry、迟到响应、错误输出和测试门禁；不猜测仍缺失的MMIO、PMA和timeout数值。

> 主详设反向链接：[《NGU800P安全软件详细设计》第4章“eHSM BL、Host Adapter、Mailbox与平台Port”](../05-software-design/NGU800P安全软件详细设计.md#第4章-ehsm-blhost-adaptermailbox与ngu800p平台port)

# Scope

本文覆盖产品固件中的Host adapter与NGU800P port，不覆盖：

- Vendor Host公共`api.c/mailbox.c`内部实现变更；
- baremetal全功能测试runner和完整Vendor Demo case catalog；
- 最终native package/Header Overlay字段和各image verify/decrypt业务Code Region；
- RAS最终reset/watchdog/隔离策略；
- 后续interrupt优化。

# Authority and evidence

- Vendor direct Mailbox、源码只读和首版poll见ADR-0010。
- 单在途、cache、timeout、no-retry和quarantine见ADR-0011。
- Vendor公共实现证据见CE-SEC-008。
- SoC地址/寄存器数值只接受SRC-0022；当前direct aperture/status宏仍缺失。

# 首版软件分层

```text
BootROM / FMC / GSP安全业务
        |
        | 只调用产品typed wrapper
        v
ngu_ehsm_service
  - 唯一owner/锁/队列
  - operation policy
  - active transaction
  - error/raw evidence
        |
        | 调用Vendor typed API
        v
Vendor Host公共api.c / mailbox.c（只读）
        |
        | Vendor正式port函数
        v
ngu800p_ehsm_port
  - direct MMIO/status/error
  - 64位System Address范围校验与同值返回
  - active cache descriptor
  - active timeout scope
  - critical section/barrier
```

production业务层不得：

- 直接包含Vendor内部头或调用`ehsm_mb_*()`；
- 直接持有/重置Vendor context；
- 自行设置channel、timeout scope或cache descriptor；
- 调用stub、simulated success或Vendor reset；
- 在同一stage绕过service发出第二条事务。

# Target code placement

后续获得编码授权时，产品仓目标职责建议如下；文件名可在OpenSpec实现评审中微调，职责不得合并回stub：

| 目标职责 | 建议位置 |
|---|---|
| 产品公开typed API、结果类型 | `components/security/include/security/ehsm_service.h` |
| 项目内部service/状态机 | `components/security/src/ehsm_service.c` |
| operation policy表 | `components/security/src/ehsm_policy.c` |
| Vendor正式port实现 | `components/security/src/ehsm_port_ngu800p.c` |
| SoC编译期配置 | `components/security/include/security/ehsm_port_cfg.h` |
| raw错误映射/RAS记录 | `components/security/src/ehsm_error.c` |
| stub | 只允许test-only target；不得链接进EMU/production |

Vendor Host源码由外部构建选择纳入，不复制到`components/security`，也不在Vendor交付目录内添加NGU800P文件。

# 固定常量与编译期门禁

| 常量 | 首版合同 |
|---|---|
| `NGU_EHSM_CHANNEL_COUNT` | 16，与Vendor port配置一致 |
| `NGU_EHSM_CHANNEL_STRIDE` | `0x1000` |
| `NGU_EHSM_MMIO_MIN_SIZE` | `0x10000` |
| `NGU_EHSM_CACHE_LINE_SIZE` | 64 |
| `NGU_EHSM_CTX_SLOT_SIZE` | 256 |
| `NGU_EHSM_CTX_SLOT_COUNT_PER_STAGE` | 1 |
| `NGU_EHSM_MAX_INFLIGHT_PER_STAGE` | 1 |
| `NGU_EHSM_AUTO_RETRY_MAX` | 0 |
| driver mode | `EHSM_DRV_MODE_WAIT_AND_POLL` |

实现必须包含等价编译期检查：

```c
_Static_assert(sizeof(ehsm_ctx_st) == 224u, "Vendor context version mismatch");
_Static_assert(NGU_EHSM_CTX_SLOT_SIZE >= sizeof(ehsm_ctx_st), "context slot too small");
_Static_assert((NGU_EHSM_CTX_SLOT_SIZE % NGU_EHSM_CACHE_LINE_SIZE) == 0u,
               "context slot must own complete cache lines");
_Static_assert(sizeof(raddr_t) == sizeof(uint64_t), "remote address must be 64-bit");
```

若未来Vendor版本改变`ehsm_ctx_st`大小或port ABI，构建必须失败并重新评审，不能静默沿用256字节slot。

# Internal types

以下是项目内部ABI，不是跨stage Handoff，也不写入Measurement Table。

## Stage与事务状态

```c
typedef enum {
    NGU_EHSM_PHASE_BOOT = 1,
    NGU_EHSM_PHASE_RUNTIME = 2
} ngu_ehsm_phase_t;

typedef enum {
    NGU_EHSM_SVC_UNINIT = 0,
    NGU_EHSM_SVC_READY,
    NGU_EHSM_SVC_ACTIVE,
    NGU_EHSM_SVC_QUARANTINED
} ngu_ehsm_service_state_t;

typedef enum {
    NGU_EHSM_ACCEPT_NOT_SUBMITTED = 0,
    NGU_EHSM_ACCEPT_COMPLETED,
    NGU_EHSM_ACCEPT_UNKNOWN
} ngu_ehsm_acceptance_t;
```

`caller_stage`必须使用公共内部Stage ID：BootROM=1、FMC=2、GSP=3。GSP bootstrap/runtime只由独立`phase`区分，不分配第二套Stage ID。Stage ID来自当前镜像编译期配置或受信调用链，不得来自Host、Header空闲位或其他wire输入。

`ACCEPT_NOT_SUBMITTED`只有项目本地参数、地址或policy校验在进入Vendor API前失败时才能使用。Vendor返回`EHSM_ERR_TIMEOUT`时固定为`ACCEPT_UNKNOWN`。

## Context slot

目标布局等价于：

```c
typedef union {
    ehsm_ctx_st vendor_ctx;
    uint8_t cache_lines[NGU_EHSM_CTX_SLOT_SIZE];
} ngu_ehsm_ctx_slot_t;
```

对象本身必须使用项目统一的64字节对齐宏，放入当前stage批准的`EHSM_CONTEXT_ARENA` section。不得使用普通函数栈、heap或caller临时buffer承载该slot。

首版不得启用Vendor三段式API，adapter不定义512字节session slot，init/update/finish operation ID也不得注册。未来启用必须通过独立ADR/OpenSpec。

## I/O descriptor

每个传给Vendor的`EHSM_SHM`对象必须登记：

```c
typedef enum {
    NGU_EHSM_IO_HOST_TO_EHSM = 1,
    NGU_EHSM_IO_EHSM_TO_HOST = 2,
    NGU_EHSM_IO_BIDIRECTIONAL = 3
} ngu_ehsm_io_direction_t;

typedef struct {
    uint64_t system_addr;
    uint64_t length;
    ngu_ehsm_io_direction_t direction;
    uint32_t contains_secret;
    uint32_t zeroize_owner;
} ngu_ehsm_io_desc_t;
```

约束：

- `length != 0`，`system_addr + length`不得溢出。
- System Address范围必须完全位于当前stage批准的arena、Host ingress、尚未执行的原地解密目标Region或获批MMP DDR Region；2 MiB SRAM使用ADR-0028固定边界，普通Host DDR直接拒绝。
- `ehsm_port_addr_to_raddr()`和`ehsm_port_raddr_to_addr()`只对active descriptor中的同一System Address做范围校验并同值返回；local/remap输入直接拒绝。
- cacheable对象首尾扩展到64字节时不得覆盖其他owner对象；否则初始化失败。
- descriptor数组使用固定静态容量；容量由最终选定的产品operation最大`EHSM_SHM`参数数目决定，禁止heap增长。

## Operation policy

```c
typedef enum {
    NGU_EHSM_EFFECT_READ_ONLY = 0,
    NGU_EHSM_EFFECT_VERIFY,
    NGU_EHSM_EFFECT_STATE_CHANGE,
    NGU_EHSM_EFFECT_IRREVERSIBLE
} ngu_ehsm_effect_t;

typedef struct {
    uint32_t operation_id;
    uint64_t timeout_us;
    ngu_ehsm_effect_t effect;
    uint32_t auto_retry_max;
    uint32_t ras_action_request;
    uint32_t allowed_stage_mask;
} ngu_ehsm_command_policy_t;
```

首版所有表项必须满足：

- `timeout_us > 0`；
- `auto_retry_max == 0`；
- stage在`allowed_stage_mask`内；
- 不可逆操作必须有独立operation ID和RAS action request；
- 未登记operation拒绝执行，不能回退到“通用默认timeout”。

最终timeout数值未冻结前，production policy表必须保持构建/初始化不通过，而不是填入猜测值。fake-MMIO单元测试可使用显式test policy，但该表不得进入production target。

## Result record

每次调用必须产生：

```c
typedef struct {
    uint32_t operation_id;
    uint32_t caller_stage;
    ngu_ehsm_phase_t phase;
    uint32_t channel;
    ngu_ehsm_acceptance_t acceptance;
    uint64_t start_us;
    uint64_t elapsed_us;
    uint64_t timeout_us;
    uint32_t raw_vendor_ret;
    uint32_t mapped_status;
    uint32_t raw_hsm_status0;
    uint32_t raw_hsm_error[/* 按RTL冻结 */];
    uint32_t zeroize_state;
    uint32_t ras_action_request;
} ngu_ehsm_call_result_t;
```

业务层不得只接收`true/false`或单一`security_status_t`而丢失raw Vendor/status/error。

# Service configuration

```c
typedef struct {
    uint32_t caller_stage;
    ngu_ehsm_phase_t phase;
    uint32_t service_channel;
    uint64_t mailbox_mmio_cpu_base;
    uint64_t mailbox_mmio_size;
    uint64_t context_arena_system_base;
    uint64_t context_arena_size;
    uint32_t context_memory_attr;
    ngu_ehsm_ctx_slot_t *ctx_slot;
    const ngu_ehsm_command_policy_t *policy_table;
    uint32_t policy_count;
} ngu_ehsm_service_cfg_t;
```

初始化必须检查：

1. direct MMIO base来自SRC-0022批准宏，size至少`0x10000`，属性为device/non-cacheable。
2. `service_channel < 16`。Vendor `ehsm_mb_init()`会清除全部16个channel，因此整个direct aperture专用于eHSM transport；每stage只初始化一次，且必须证明前一stage无在途事务。若硬件要求与其他软件共享，等待Vendor单channel init接口。
3. context arena System Address范围在批准的2 MiB内，64字节对齐，至少容纳256字节slot和选定one-shot operation需要的静态descriptor/buffer；首版不分配session，且不存在CPU/remote双地址。
4. `context_memory_attr`最终只允许`NON_CACHEABLE`或`HARDWARE_COHERENT`。该选择待SoC稳定后裁决；裁决前production初始化固定失败并返回`BLOCKED_BY_PMA_INPUT`，不得使用`CACHED_WITH_MAINTENANCE`替代发布配置。
5. policy表非空，所有启用operation均有非零deadline且retry为0。
6. status/error、timer、cache、barrier、critical section和RAS report依赖全部可用。
7. 任何失败都发生在`ehsm_driver_init_library()`和业务调用前，不得部分初始化后继续启动。

# Project adapter APIs

## Public APIs

产品业务只看到按安全软件方案定义的typed wrapper，例如：

```c
security_status_t ngu_ehsm_service_init(
    const ngu_ehsm_service_cfg_t *cfg,
    ngu_ehsm_call_result_t *result);

security_status_t ngu_ehsm_wait_bl_ready(
    ngu_ehsm_call_result_t *result);

security_status_t ngu_ehsm_run_bl_self_test(
    uint32_t requested_mask,
    ehsm_self_test_result_st *raw_result,
    ngu_ehsm_call_result_t *result);

security_status_t ngu_ehsm_bl_verify_image(
    const ngu_ehsm_bl_verify_request_t *request,
    ngu_ehsm_bl_verify_result_t *result);

security_status_t ngu_ehsm_fw_verify_soc_image(
    const ngu_ehsm_fw_verify_request_t *request,
    ngu_ehsm_fw_verify_result_t *result);
```

Vendor BL/FW对同一`0xff06`使用不同结构体。BL wrapper仅在FW ready前使用，支持type 0 boot和type 1 SoC验证语义；FW wrapper仅在ready后使用，只允许type 1、`boot=0/check_version=0`，type 2/3在NGU typed入口拒绝。构建只比较共同字段offset/width，不要求结构总`sizeof`相同。所有typed wrapper必须复用同一service guard，不能各自实现timeout/cache/retry逻辑。

## Internal guard

typed wrapper内部必须严格执行：

```text
validate local args / stage / policy / address / length
    -> acquire unique owner
    -> require service == READY
    -> build static I/O descriptor set
    -> install active cache descriptor
    -> install active timeout scope
    -> register exact active response context System Address range and service channel
    -> init/reset Vendor context only while not quarantined
    -> mark service ACTIVE
    -> invoke exactly one Vendor one-shot typed API
    -> capture raw return/status/time
    -> complete external RX cache maintenance when completion is proven
    -> commit result record
    -> clear active scopes
    -> service READY on proven completion
       or QUARANTINED on timeout/busy/response-address mismatch/unknown completion
    -> release owner
```

清理顺序不得先清active descriptor或复用buffer，再判断Vendor返回结果。

命令方向固定为先写`s2h_info[0..1]`、最后写`s2h_note`发布；响应从`h2s_info/h2s_note`读取。当前不修改Vendor `mailbox.c`；平台PMA/MMIO和EMU必须证明info先于note可见，不能用delay或重复note代替。

`ehsm_port_raddr_to_addr()`只接受当前active response context范围内的System Address并同值返回。地址/长度/channel不匹配时返回固定invalid-magic sentinel并设置sticky `response_addr_mismatch`；Vendor callback必须在invalid magic处停止，wrapper在接受结果前检查sticky fault并把Service置为`QUARANTINED`，禁止解引用响应选择的任意内存。

# Vendor port scope

Vendor正式port ABI保持原样；项目在其外部增加不可由业务层调用的scope管理。

## Timeout scope

```c
typedef struct {
    uint32_t valid;
    uint32_t owner_token;
    uint64_t timeout_us;
} ngu_ehsm_active_timeout_t;
```

- `ehsm_port_create_timer()`返回当前64位单调微秒值。
- `ehsm_port_is_timeout(start)`读取唯一active scope，并使用`(now - start) >= timeout_us`。
- scope缺失、timeout为0、owner不匹配、嵌套或时钟回退均作为port fault；不得返回false继续无限poll。
- scope只在持有service owner期间有效，Vendor API返回后清除。

## Cache scope

```c
typedef struct {
    uint32_t valid;
    uint32_t owner_token;
    const ngu_ehsm_io_desc_t *descs;
    uint32_t desc_count;
} ngu_ehsm_active_cache_t;
```

`ehsm_port_flush_and_invalidate_cache()`：

1. Vendor port ABI返回`void`，因此所有可能失败的active scope、owner、溢出、Region、cache-line独占和cache能力检查，必须在进入Vendor API前由adapter完成。
2. hook内只读取已验证并锁定的active descriptor集合；不得再进行可能失败的动态分配、地址发现或策略选择。
3. `context_memory_attr`只能是最终裁决的`NON_CACHEABLE`或`HARDWARE_COHERENT`；两者均不执行data clean/invalidate。其他属性属于初始化门禁错误，不能进入本hook。
4. 执行平台write fence。
5. 若hook运行时发现scope缺失、owner变化或descriptor被篡改，属于不可返回的内部安全不变量破坏：记录可用的最小sticky port fault，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出；不得静默返回让Vendor继续发布命令，也不得在此路径自行reset。

Vendor调用该函数后至事务完成前：

- Host不得读取或写入context/外部output；
- 不允许日志打印这些buffer；
- 不允许另一个task进行cache维护或复用相同cache line；
- 不允许把context指针交给caller。

Vendor API成功返回后，adapter对`EHSM_TO_HOST/BIDIRECTIONAL`外部output执行acquire/read fence再发布给caller，不执行data invalidate。context内部rsp已被Vendor读取，因此其正确性必须由send前write/release barrier、在途no-touch、MMIO顺序和最终选定的`NON_CACHEABLE`或`HARDWARE_COHERENT`属性保证；若平台无法保证，必须升级兼容性冲突。

critical section在per-hart第一次进入时保存原MIE并维护nesting depth；只有最后退出按原值恢复，原先关闭则保持关闭。下溢、跨hart/Owner退出或状态破坏立即fail-close，禁止退出时无条件开中断。

# State machine

| Current | Event | Next | Required action |
|---|---|---|---|
| `UNINIT` | 全部配置检查和Vendor init成功 | `READY` | 保存driver版本和raw初始状态 |
| `UNINIT` | 任一配置/port/Vendor init失败 | `QUARANTINED` | 上报、阻断，不允许部分可用 |
| `READY` | 合法typed wrapper取得owner | `ACTIVE` | 安装policy/cache scope，记录开始时间 |
| `READY` | owner冲突/未登记operation/地址、descriptor或cache能力错误 | `READY` | 在进入Vendor前返回本地错误；不标记submitted |
| `ACTIVE` | Vendor返回成功且完成状态可证明 | `READY` | RX维护、结果提交、清scope |
| `ACTIVE` | Vendor确定的command错误且响应已完成 | `READY`或stage fail-close | 保存raw eHSM错误；是否继续由业务/RAS策略决定，但slot可在清零后复用 |
| `ACTIVE` | `EHSM_ERR_TIMEOUT` | `QUARANTINED` | acceptance unknown；保存slot和buffer，不重试 |
| `ACTIVE` | `EHSM_ERR_BUSY`或transport状态不一致 | `QUARANTINED` | 单在途模型下视为集成/旧事务错误 |
| `QUARANTINED` | 任意新调用 | `QUARANTINED` | 返回service unavailable，不访问Mailbox |
| `QUARANTINED` | RAS批准的reset/recovery已完成 | `UNINIT` | 后续状态机重新初始化；security自己不执行reset |

首版没有`CANCELLED`或“timeout后恢复READY”路径，因为Vendor公共合同没有sequence/cancel/abort来证明迟到响应已被排除。

# Retry policy

首版统一`auto_retry_max=0`，包括read-only查询。原因不是所有查询都非幂等，而是Vendor公共`EHSM_ERR_TIMEOUT`不能证明是否已提交，迟到响应也没有transaction ID可区分。

以下不属于retry：

- `wait_bl_ready()`在其单一ready deadline内重复读取status；
- Vendor内部在单次API调用中重复poll note；
- RAS尚未ready时重复尝试“上报错误记录”。

以下属于禁止的自动retry：

- Vendor API返回timeout后重新调用同一API；
- 看到BUSY后业务层循环调用；
- key/counter/OTP/lifecycle/upgrade等有副作用命令的盲目重发；
- reset eHSM后假设原命令未执行并继续启动。

# Stage usage

## BootROM

- 一个静态256字节context slot，poll，单在途。
- ready/self-test/验证FMC使用统一service guard。
- LCS由BootROM通过权威SoC接口直接读取，不建立eHSM LCS typed wrapper；FMC type 1验证固定`check_version=0`并确认BL已暂存candidate，BootROM不调用stored counter read/compare/update。
- 任一timeout/busy/unknown completion后quarantine并执行ADR-0010早期fail-stop；不复用slot，不跳转FMC。

## FMC

- 一个静态256字节context slot，poll，单在途。
- 验证/解密/测量GSP的每个typed wrapper都串行；FMC不提供Vendor FW加载wrapper。
- CE-SEC-011确认BL verify成功会暂存SOC candidate而不提交OTP。FMC初始化必须把FMC Entry的expected candidate回传BL；新增专用API先逐字节匹配RAM candidate，再执行compare/update/readback。
- 已确定完成的验证失败允许清理并重新arm Host ingress，由上位机决定是否发送新package；timeout/BUSY/unknown completion仍quarantine，不能伪装成可重试失败。
- stage退出前只有在所有事务完成且无quarantine时才清零/回收FMC arena；timeout slot不能按普通成功退出路径回收。

## GSP

- 首版一个service owner和一个静态256字节context slot。
- GSP初始化阶段负责Vendor FW定位、验证、加载、启动及`firmware_done && !firmware_err`门禁；依赖FW的typed wrapper在ready前不可达。FMC在初始化时已经通过eHSM BL staged-candidate API提交SoC counter；type 0 eHSM FW使用独立counter域，不得推进SoC counter。
- 产品operation覆盖PMP/RMP/MMP/eHSM FW镜像，以及安全软件方案要求的签名、随机数、Hash、Key/Certificate/Rotation等能力；全部使用[eHSM产品Operation Profile](ehsm-product-operation-profile.md)登记的typed wrapper，禁止通用raw command入口。
- 通用算法只面向受信任内部SoC模块开放且优先级低；ADR-0019确认不向Host注册GSP安全服务或raw Vendor路由。
- GSP负责发起受控操作和完成caller/参数/usage检查，eHSM负责基于实际LCS作最终允许/拒绝；GSP不得覆盖拒绝。
- 第一个、最高优先级的`security_service_task`从bootstrap到runtime持续作为唯一Owner；bootstrap门禁完成后同一task转入长期service loop，不发生Owner移交。
- 其他task只能通过typed queue请求服务；queue在bootstrap门禁通过前不向普通runtime caller开放，任何task不得以持锁为由直接调用Vendor Host或Mailbox。
- service task空闲等待queue时阻塞；每个poll operation有批准deadline，timer/RAS等平台关键中断保持可服务。
- 首版即使Vendor FW ready也继续poll，不建立interrupt callback并发路径。
- runtime发生timeout/busy后service全局quarantine，不只拒绝原caller；后续动作由RAS策略决定。
- 某一PMP/RMP/MMP得到确定完成的密码/策略失败时，只隔离该image；这不等于共享transport失败。显式依赖项保持未release。

# Error and evidence

至少区分：

| Domain | 例子 | Acceptance |
|---|---|---|
| LOCAL_VALIDATION | null、System Address range、local/remap输入、overflow、policy缺失 | NOT_SUBMITTED |
| OWNER | 重入、非owner调用、scope嵌套 | NOT_SUBMITTED |
| PORT | MMIO/status/cache/timer配置错误 | NOT_SUBMITTED或UNKNOWN，按发生阶段记录 |
| TRANSPORT | busy、timeout、note异常 | UNKNOWN，首版quarantine |
| EHSM_COMMAND | Vendor已完成响应中的raw错误 | COMPLETED |
| POLICY | 算法/LCS/counter/typed-stage policy不允许 | COMPLETED或NOT_SUBMITTED |

错误记录不得包含key、明文或完整敏感payload；可以保存Region ID、长度、operation、channel、raw return/status和zeroize结果，不保存可泄露内存布局的完整地址。

# Test contract

## Host unit/fake-MMIO

必须覆盖：

1. context slot大小、64字节对齐和cache-line独占检查。
2. service只允许一个owner和一条在途事务。
3. 未登记operation、零deadline、缺失cache/response-address scope在进入Vendor前失败。
4. send前descriptor/PMA校验、write/release fence和`s2h_info`写入发生在`s2h_note`发布前；data clean/invalidate调用次数必须为0。
5. 在途访问、scope嵌套和第二调用被拒绝。
6. timeout映射为acceptance unknown、retry count保持0、service进入quarantine。
7. timeout后任何新operation都不再访问Mailbox。
8. 迟到note/response不能使quarantined slot重新变为READY，也不能被新事务消费。
9. `BUSY`在首版进入quarantine而不是循环重试。
10. success路径在发布外部output前完成acquire/read fence，data invalidate调用次数必须为0。
11. raw Vendor/status/error完整保留。
12. security不调用Vendor reset。
13. 伪造response地址只能命中sentinel、设置sticky fault并quarantine，不能访问任意内存。
14. critical section覆盖原MIE开/关、嵌套、下溢和跨hart/Owner退出。

## EMU

必须覆盖：

- SoC稳定后选定的唯一`NON_CACHEABLE`或`HARDWARE_COHERENT` production属性必须通过双向可见性验证，未选属性和`CACHED_WITH_MAINTENANCE`配置必须被production初始化拒绝；
- C908写request后eHSM读到新值，eHSM写response/output后C908读到新值；
- `s2h_info[0..1]`对eHSM的可见性严格先于`s2h_note`；
- 延迟响应跨过deadline，确认Host不复用slot、不发第二命令；
- 真实direct Mailbox/status地址和16路IRQ数值核对；
- BootROM/FMC/GSP首版poll时序及RAS错误上报。

# Remaining integration inputs

以下仍需补充，但不重开本文已冻结的结构和状态机：

1. SRC-0022中的Vendor direct Mailbox、Host status/error准确宏和MMIO属性。CE-SEC-010确认当前有效生成头仍缺这些绑定，是真实port硬门禁。
2. 2 MiB安全RAM在C908侧的最终PMA/PBMT/MMU属性；context arena的唯一候选为`NON_CACHEABLE`和`HARDWARE_COHERENT`，待SoC稳定后裁决。当前M-mode基线启用I/D cache，`CONFIG_XUANTIE_SVPBMT`并未为该RAM建立可见的页属性配置。
3. 各stage首版service channel ID。
4. 各operation最终`timeout_us`和RAS action request数值。
5. RAS ready/report通道和early report deadline；失败终态固定为无限WFI循环。
6. 最终产品one-shot operation清单，用于计算静态I/O descriptor容量和arena总大小；首版不得计入或分配512字节session slot。

当前16路SoC encoded IRQ已由CE-SEC-010确认，但首版poll不使用。Vendor channel 0～15与`IRQ1～16`的逐项映射仅作为未来interrupt change门禁，不加入首版port初始化必选项。

# References

- [ADR-0010](../../decisions/ADR-0010-vendor-direct-mailbox-stage-mode-and-early-ras-terminal.md)
- [ADR-0011](../../decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md)
- [ADR-0012](../../decisions/ADR-0012-bootrom-lcs-counter-gsp-fw-and-redelivery-boundary.md)
- [ADR-0013](../../decisions/ADR-0013-gsp-operation-image-service-and-isolation-profile.md)
- [ADR-0015](../../decisions/ADR-0015-gsp-lifetime-ehsm-service-owner.md)
- [eHSM产品Operation Profile](ehsm-product-operation-profile.md)
- [Measurement Table接口与FMC Epoch合同](measurement-table.md)
- [CE-SEC-008](../../evidence/code-investigations/CE-SEC-008-vendor-poll-cache-timeout-and-late-response.md)
- [CE-SEC-010](../../evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md)
- [CE-SEC-011](../../evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md)
- [OPEN-CONFLICT-009](../../sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md)
- [eHSM Mailbox接口](ehsm-mailbox.md)
- [错误处理](../05-software-design/error-handling.md)
- [安全RAM布局](../03-architecture/security-ram-layout.md)

# Change history

- 2026-07-28：Counter边界改为BootROM `check_version=0`暂存FMC candidate、FMC初始化调用BL staged-candidate exact-match commit API；Debug改为无scope的唯一SoC全局开关。
- 2026-07-24：接入CE-SEC-011；记录BL candidate暂存和FW启动提交副作用，FMC不加载FW与FMC release前提交的矛盾由OPEN-CONFLICT-009阻断。
- 2026-07-24：负责人决定Counter细节延期；Host Adapter逻辑默认另有16字节Counter接口，具体Vendor函数/command/FW绑定移至实现/EMU前。
- 2026-07-24：接入CE-SEC-010；明确当前M-mode/cache开启但PMA未知，direct aperture/status仍缺；16路encoded IRQ已确认，但逐channel映射只约束未来interrupt change。
- 2026-07-24：接受ADR-0015；冻结GSP同一个最高优先级`security_service_task`从bootstrap到runtime持续持有service/context，其他task只走typed queue，quarantine后不允许Owner移交或同boot热重建。
- 2026-07-23：接受ADR-0013并新增产品operation profile；冻结GSP镜像/能力/开放优先级/LCS授权和局部隔离边界。
- 2026-07-23：接受ADR-0012；其中BootROM compare-only时序已由2026-07-28裁决取代；LCS直读SoC、Vendor FW由GSP加载及确定失败/unknown quarantine边界保留。
- 2026-07-23：依据负责人批准和ADR-0011建立首版详细合同；冻结单在途、256字节context slot、GSP统一service、active cache/timeout scope、零自动retry、timeout quarantine和测试门禁。
- 2026-07-23：依据CE-SEC-008校正cache顺序：Vendor poll只有send前cache钩子，不能设计不存在的响应前port回调；若SoC稳定后选定的`NON_CACHEABLE`或`HARDWARE_COHERENT`属性不能满足Vendor内部rsp可见性，必须升级兼容性问题。
- 2026-07-28：首版固定只调用one-shot typed API，流式init/update/finish不可达且不分配`ehsm_session_st`。
