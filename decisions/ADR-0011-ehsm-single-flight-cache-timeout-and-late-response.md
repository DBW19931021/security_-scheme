# ADR-0011：eHSM首版单在途、Cache、Timeout与迟到响应

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（批准按推荐保守基线继续）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018、SRC-0022
- 相关证据：CE-SEC-005、CE-SEC-007、CE-SEC-008
- 相关问题：OPEN-DESIGN-001、OPEN-DESIGN-005
- 补充关系：补充ADR-0008、ADR-0010

## 背景

Vendor Host公共实现把cache维护和timeout判断放在无参数port钩子中；poll路径只在发送前调用一次cache钩子，timeout公共返回码也不能区分命令是否已提交。Vendor代码按批准边界保持只读，因此NGU800P必须通过adapter串行化、active transaction描述和stage-local静态arena满足这些合同。

## 决策一：首版并发和Owner

1. BootROM、FMC、GSP各自只配置一个64字节对齐的静态`ehsm_ctx_st` slot；公开context为224字节，物理slot固定占256字节，禁止与其他对象共享cache line。
2. 每个stage最多一条eHSM在途事务。BootROM/FMC天然串行；GSP首版由唯一eHSM service owner持有context，通过锁/队列串行化所有task请求。
3. 16个硬件channel是Vendor能力上限，不作为首版并发目标。首版只使用平台配置指定的一个service channel；准确channel ID属于集成配置，不硬编码在业务层。
4. 首版不使用interrupt、async、send-and-peek或任何init/update/finish流式API；只调用Vendor one-shot typed API，不分配`ehsm_session_st`。未来若要启用流式API，必须通过独立ADR/OpenSpec重新设计session内存、Owner、timeout和迟到响应合同。
5. production调用必须经过项目adapter；业务层不得直接调用Vendor API、设置active port scope或操作Vendor context。

## 决策二：Cache与共享内存

1. Vendor direct Mailbox MMIO必须为non-cacheable/strongly ordered。
2. 所有传给Vendor、由`EHSM_SHM`标识的context、packet内嵌cmd/rsp和外部input/output必须位于eHSM可达的批准共享RAM范围，且按64字节cache line隔离；首版不存在session对象。
3. adapter在进入Vendor API前登记唯一active transaction的共享对象描述，并完成owner、范围、溢出、cache-line独占及PMA能力等全部可失败校验；`ehsm_port_flush_and_invalidate_cache()`只读取该已验证集合，并在两个批准候选属性下执行write/release fence而不执行data clean/invalidate。
4. Vendor公共poll代码只在send前调用cache钩子，因此从send到返回期间Host禁止访问或预取active context/output；不得修改Vendor代码增加第二个钩子。
5. Vendor API返回成功后，adapter在caller读取外部output前执行acquire/read fence，不执行data invalidate。Vendor内部rsp读取依赖最终选定的PMA属性、在途no-touch和Mailbox MMIO顺序。
6. `EHSM_CONTEXT_ARENA`最终只允许映射为`NON_CACHEABLE`或`HARDWARE_COHERENT`；唯一属性待SoC稳定后裁决。若所选属性不能保证上述Vendor合同，升级硬件/平台兼容性问题。不得以修改Vendor公共代码作为默认解决方案。
7. 尚未确认最终PMA属性前，production配置必须以`BLOCKED_BY_PMA_INPUT`失败；不得实现或选择`CACHED_WITH_MAINTENANCE`发布配置。
8. Vendor cache port ABI返回`void`，hook内不得进行可能失败的动态分配、地址发现或策略选择。若运行时发现active scope/owner/descriptor不变量破坏，必须记录最小sticky fault，关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出；不得静默返回让Vendor继续发布命令，也不得自行reset。

## 决策三：Deadline实现

1. 所有ready等待和Vendor command必须有非零、有限、64位单调微秒deadline；禁止OSR样例的恒false timeout和无限poll。
2. typed request中的`request_deadline_us`是绝对单调时刻，约束排队和执行总时长；policy中的`timeout_us`是operation最大执行时长。caller只能用更早的request deadline缩短等待，不能放大policy timeout。
3. GSP request在Vendor命令提交前已经到期时返回`NOT_SUBMITTED`，不触碰Mailbox且不quarantine。开始执行时以`min(request_deadline_us, start_us + timeout_us)`作为实际执行deadline，并对加法做64位溢出检查。
4. command policy在调用Vendor API前安装为唯一active timeout scope；`ehsm_port_create_timer()`返回`csi_tick_get_us()`采样值，`ehsm_port_is_timeout()`使用实际执行deadline或等价的无符号差值判断。
5. active timeout scope只允许在持有stage/service owner且没有其他在途事务时设置；返回后清除。未安装、为0、嵌套或owner不匹配必须fail-close。
6. 每个operation的最终`timeout_us`存放在集中policy表，不散落在业务代码。数值须由Vendor适用资料、启动预算和EMU测量共同冻结；在数值未提供的production配置中构建或初始化失败，不使用通用猜测默认值。

## 决策四：Retry与迟到响应

1. 首版所有eHSM operation的`auto_retry_max=0`。
2. 所有Vendor `EHSM_ERR_TIMEOUT`统一标记为`acceptance_unknown`。adapter不得假定命令未执行，也不得自动重发，包括查询类命令。
3. `EHSM_ERR_BUSY`在首版单在途模型下表示channel被非owner占用、旧事务未清或平台状态不一致；不得当成正常队列压力自动重试。
4. timeout、busy异常或无法证明完成状态时，active context/input/output进入`QUARANTINED`；不得清零、覆盖或复用，GSP service停止发出后续eHSM command。首版不存在session对象。
5. security保存stage、operation、channel、elapsed、raw Vendor ret、raw status/error和buffer清零状态，阻断release并上报RAS。security不调用Vendor reset；reset/recovery后是否重新初始化由RAS策略和后续状态机决定。
6. 如果未来需要自动retry，必须按operation证明幂等性、提交状态可判定、迟到响应可区分且有独立测试Evidence，再通过单独ADR/OpenSpec启用。

## 影响

- OPEN-DESIGN-005中的首版context并发数、slot大小、对齐、流式API禁用和timeout/late-response回收规则关闭；共享RAM最终PMA属性和arena总容量仍为集成输入。
- OPEN-DESIGN-001中的stage并发策略和首版retry策略关闭；direct aperture/status、各operation timeout数值、PMA及RAS通道仍开放。
- 详设可以继续冻结adapter API、状态机和测试合同，不因上述数值未定而停止；真实MMIO和production配置仍受缺失输入门禁。
- 本次不授权修改`baremetal`、`gsp-pmp-rmp-omp`或Vendor源码。

## Review history

- 2026-07-23：项目负责人批准按推荐的首版保守基线继续。
- 2026-07-23：只读核对Vendor公共poll/cache/timer实现后，明确响应前无第二个cache钩子，并冻结Vendor兼容的单在途、active scope、no-retry和quarantine合同。
- 2026-07-28：确定首版只使用one-shot typed API，不启用流式API、不分配`ehsm_session_st`；PMA候选收敛为`NON_CACHEABLE`与`HARDWARE_COHERENT`，待SoC稳定后唯一裁决。
