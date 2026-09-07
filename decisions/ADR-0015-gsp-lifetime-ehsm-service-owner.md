# ADR-0015：GSP全生命周期eHSM Service唯一Owner

- 状态：accepted
- 日期：2026-07-24
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关 Requirement/Open Question：OPEN-DESIGN-009
- 补充关系：补充ADR-0011、ADR-0013、ADR-0014

## 背景

ADR-0011已经冻结GSP首版单context、单在途和统一service串行化，但仍需明确bootstrap阶段完成eHSM Vendor FW及Runtime门禁后，是否把Vendor context、channel和service所有权移交给另一个运行期task。隐式移交会扩大context、active cache/timeout scope、迟到响应和异常恢复的状态空间。

## 候选方案

1. bootstrap使用临时Owner，进入运行期后把eHSM service/context移交给另一个task。
2. 由同一个最高优先级`security_service_task`从bootstrap到runtime持续作为唯一Owner，其他task只通过typed queue请求服务。
3. 允许多个task使用锁直接调用Vendor Host API。

## 决策

采用方案2。

### 1. 唯一Owner及生命周期

1. GSP创建的第一个、最高优先级安全服务任务固定为`security_service_task`。
2. 该task从GSP bootstrap开始直至本boot instance结束，持续拥有唯一：
   - Vendor `EHSM_CONTEXT`/session；
   - service channel和静态context slot；
   - active cache/timeout scope；
   - Vendor Host raw handle；
   - 当前事务及迟到响应/quarantine状态。
3. 完成eHSM Vendor FW、PMP/RMP/MMP启动门禁后，该task直接进入长期服务循环，不发生bootstrap Owner向runtime Owner的移交。
4. `QUARANTINED`状态下不在同一boot instance内热重建或转交Owner；只有RAS批准并完成reset/recovery后，新的boot instance才能重新创建service。

### 2. 其他任务的访问方式

1. 其他GSP task和受信任内部模块只能提交版本化typed request，不得直接调用Vendor Host API、Mailbox port或操作context。
2. queue请求必须携带caller identity、operation ID、输入/输出descriptor、64位单调微秒绝对`request_deadline_us`和request ID；不能携带Vendor raw command、channel或context指针。该deadline只允许缩短集中policy的`timeout_us`，不能由caller放大operation执行预算。
3. service task负责caller/operation/policy/range/cache检查、唯一Vendor调用、raw状态保存、结果提交和审计。
4. ADR-0019确认Host永久不能直接进入该queue，也不允许raw passthrough。

### 3. Bootstrap门禁

在以下条件满足前，外部runtime service queue保持关闭：

1. GSP自身Measurement输入验证通过；
2. eHSM service初始化完成；
3. eHSM Vendor FW完成Vendor boot且`firmware_done=1 && firmware_err=0`；
4. 需要在启动阶段release的Runtime按照最终stage profile完成验证、加载、Measurement和依赖门禁；
5. service状态仍为`READY`，不存在未闭环事务、late response或quarantine。

bootstrap内部操作使用与运行期相同的typed operation和service状态机，不建立第二套“初始化直调”路径。

### 4. 优先级和调度约束

“最高优先级”指高于普通GSP业务/协议task的安全服务优先级，不表示无限忙等或屏蔽平台关键中断：

- 每个Vendor operation必须有批准deadline；
- 等待queue时task阻塞，不做空转；
- 首版Vendor transaction仍按批准的poll合同执行；
- RAS、timer和平台关键中断必须保持可服务；
- 普通task不得通过锁竞争、优先级继承或回调取得Vendor Owner身份。

### 5. 未由本ADR冻结的内容

OPEN-DESIGN-010继续开放，等待补充PMP/RMP/MMP的package来源、最大尺寸、依赖图、release primitive及失败后保留服务。本ADR不按镜像名称猜测加载顺序，也不冻结最终queue深度、task stack、operation deadline或RTOS priority数值。

## 选择理由

- 与ADR-0011的单在途、active scope和timeout quarantine直接一致。
- 消除Owner移交过程中遗留context、channel、cache descriptor或迟到响应的风险。
- 把所有调用统一到typed service边界，便于执行caller/policy检查、错误审计和no-raw-passthrough门禁。
- bootstrap与runtime复用同一状态机，减少两套实现发生安全语义漂移。

## 安全影响

- 任何非Owner直接调用、第二条在途事务或raw handle访问均视为本地Owner违规，必须在进入Vendor前拒绝。
- service timeout/异常BUSY导致全局quarantine，不能把Owner切换当作恢复手段。
- 单Runtime确定完成失败仍按ADR-0013局部隔离，不改变共享transport失败的全局影响。

## 软件影响

- GSP入口需优先创建`security_service_task`及静态queue/context，然后由该task执行bootstrap状态机。
- service模块需区分“bootstrap queue关闭”和“runtime queue开放”，但两者复用同一typed dispatch。
- 其他task源码不得包含Vendor Host头或raw command调用；构建/静态检查应验证该依赖边界。

## 测试影响

至少覆盖：

- bootstrap至runtime Owner token保持不变；
- queue开放前普通请求被拒绝；
- 多task并发请求仍只有一条Vendor事务；
- 非Owner/raw调用在提交前失败；
- timeout后Owner不转移、queue不再访问Mailbox；
- RAS recovery后只在新boot instance重新初始化；
- 高优先级service不阻断timer/RAS关键中断，空闲时正确阻塞。

## 风险

单service task可能成为吞吐和调度瓶颈；首版安全正确性优先，后续若要多channel/并发/interrupt，必须以独立OpenSpec/ADR重新设计sequence、cancel、context隔离和恢复，不得局部放开。

## 参考资料

- `docs/04-interfaces/ehsm-host-adapter.md`
- `docs/05-software-design/boot-stage-state-machines.md`
- `docs/05-software-design/gsp.md`
- `decisions/ADR-0011-ehsm-single-flight-cache-timeout-and-late-response.md`
