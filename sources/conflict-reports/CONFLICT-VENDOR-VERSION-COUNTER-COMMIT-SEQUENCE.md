# Vendor版本计数器提交机制与FMC/GSP release顺序冲突

## Identity

- Conflict ID: CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE
- Open Question: OPEN-CONFLICT-009
- Status: resolved_with_implementation_requirement
- Evidence state: CONFIRMED_DECISION / IMPLEMENTATION_PENDING
- Owner: 项目负责人；Vendor接口Owner；FMC/GSP启动链Owner
- Resolution: ADR-0019；eHSM BL新增FMC专用16字节staged-candidate commit API

## Conflict classification

- Type: approved_design_vs_vendor_implementation / irreversible_commit_order / stage_ownership
- Affected scope: eHSM FW加载Owner、FMC唯一`rollback_counter`更新Owner、GSP release门禁、Measurement `stored_counter_after`、掉电恢复与测试Expected
- Safe-to-continue scope: 16字节rollback-counter ABI、BootROM candidate暂存和FMC初始化提交状态机、Manifest/Measurement及不依赖command packing的实现准备
- Must-stop scope: eHSM BL/FMC代码实现前未冻结command ID/packing/LCS/交付版本；不得把Host通用counter、raw OTP、GSP值、仅有的BL candidate暂存或FW启动副作用当作commit proof

## 2026-07-28最终裁决

FMC固定为Vendor image type 1。BootROM固定以`check_version=0`验证FMC，BL记录认证candidate到RAM；FMC初始化从有效FMC Measurement Entry取得`expected_candidate`并调用新增eHSM BL专用API：

```c
int32_t ehsm_bl_commit_staged_soc_rollback_counter(
    ehsm_ctx_t *ctx,
    const uint8_t expected_candidate[16],
    ehsm_bl_rollback_counter_result_t *result);
```

BL必须先逐字节确认`expected_candidate`等于RAM candidate；无candidate、invalid或不一致均在OTP改动前拒绝。匹配后执行低值拒绝、相等不写、高值按Vendor单向编码更新和权威readback。commit proof成立后FMC才接收GSP；GSP固定`check_version=0`，认证counter必须等于已提交值，随后才完成后置校验、loader、Measurement commit和release。现有`ehsm_read_counter(uint64_t)`与此资源无关，不再排查映射。

OPEN-CONFLICT-009关闭。准确command ID、wire packing、LCS权限、readback/status和eHSM BL交付版本转为实现DoR，不再是方案方向开放项。

## 2026-07-24负责人延期决定

负责人决定本轮先不继续收敛Counter接口细节：

1. 产品详细设计统一采用16字节opaque `rollback_counter`；
2. 默认存在可供FMC使用的16字节读、比较、提交及状态确认接口；
3. 继续按`counter commit proven -> GSP Measurement commit -> release GSP`完成逻辑详设；
4. 准确API、命令、匹配FW版本、权限、回读证明和与`ehsm_read_counter`的关系后续再收敛；
5. 当前不因该问题重开“GSP加载eHSM Vendor FW”的职责。

这是受控设计假设，不覆盖CE-SEC-011的代码事实，也不代表当前Vendor快照已经证明接口可执行。若Counter实现或EMU前仍无法绑定该接口，本冲突必须重新激活。

## 已批准产品设计（由ADR-0019更新）

1. 单一16字节global `rollback_counter`。
2. BootROM以`check_version=0`验证type 1 FMC并由BL暂存认证candidate；BootROM不读取、比较或写stored counter。
3. FMC是唯一global counter提交发起Owner，并在初始化时把FMC Measurement的expected candidate传给BL专用API。
4. BL先与RAM candidate exact-match，再比较/更新/权威readback；proof成立后FMC才接收以`check_version=0`验证且counter等于已提交值的GSP。
5. eHSM Vendor FW仍由GSP初始化阶段验证、加载和启动，不承担FMC counter提交。
6. eHSM BL交付边界必须增加上述typed API；其他Vendor公共业务代码仍保持Vendor基线。

## 当前Vendor实现

CE-SEC-011确认：

1. eHSM BL成功验证非naked SoC镜像后，只把16字节候选值暂存到eHSM DRAM；`check_version=OFF`也会暂存。
2. 真正OTP提交发生在eHSM FW `secboot_entry()`启动时；写函数内部执行回读比较。
3. 因eHSM FW当前由GSP启动，提交点发生在FMC已经release GSP之后。
4. BL/FW通用OTP read/write只在测试/开发/制造LCS开放，不能作为产品LCS接口。
5. eHSM FW运行期`SOC_VERIFY check_version=ON`会在Host完成NGU Manifest/policy/digest后置校验前直接更新OTP。
6. Host侧确实存在`ehsm_read_counter(ctx, counter_id, uint64_t *counter_value)`及create/increase/delete配套接口；但匹配FW快照没有相应命令定义、dispatch或handler正文，交付build map中counter service/driver的`.text`也为0。该64位接口与16字节OTP Version Counter之间没有已确认映射。

## Exact conflict

以下三项不能在当前Vendor快照下同时成立：

1. FMC在release GSP前完成counter commit并证明结果；
2. GSP而非FMC加载eHSM FW；
3. 不新增Vendor支持的产品LCS version-counter commit/readback接口。

这不是普通函数命名或编码细节，而是不可逆OTP提交点和受信任stage顺序冲突。不得在文档中继续把“BL verify PASS/候选暂存”写成“FMC已经更新counter”。

## Options

### Option A：Vendor提供typed staged-candidate提交/回读接口（已采用）

Vendor正式提供产品LCS可用、仅作用于SoC 16字节Version Counter的BL staged-candidate `compare/commit/readback/status`能力。FMC在自身初始化时回传BootROM提交的expected candidate；接口完成RAM candidate exact-match后才允许比较/更新OTP。

约束：

- 新能力不是开放任意OTP地址的raw接口，也不复用通用64位`ehsm_read_counter`；
- API必须拒绝RAM candidate缺失、invalid或与expected candidate不一致；
- 必须执行Vendor单向编码、旧值比较、写后回读和错误可观察；
- timeout/acceptance unknown时可通过只读状态判定，不能自动重试不可逆写；
- NGU不私改Vendor公共业务代码，接口由Vendor正式交付或书面确认。

优点：最大程度保持SRC-0016和已批准ADR-0012/Measurement顺序。代价：依赖Vendor补充接口或新交付。

### Option B：把eHSM FW验证/启动前移到FMC，并以FW启动提交作为门禁

FMC验证GSP和全部NGU策略后，由FMC使用Vendor type 0 boot流程加载eHSM FW；FW启动消费BL DRAM候选并内部写后回读。FMC必须同时检查ready、raw error和`FW_ERROR_OTP_WRITE_FAILED`等映射，确认提交成功后才记录`stored_counter_after`、提交GSP Measurement并release GSP。

约束：

- 需要修改已批准“GSP初始化阶段加载eHSM FW”的职责；
- 需要Vendor确认内部回读成功及错误位足以作为权威commit证明，或另有产品LCS readback；
- 需要定义FMC退出时eHSM service/context状态和GSP接管方式；当前不采用Handoff，不能隐式传递未完成事务；
- 不得让FMC在NGU Manifest/policy/digest通过前启动eHSM FW。

优点：最接近当前Vendor BL→FW机制。代价：重开stage owner与GSP全生命周期service设计，且仍需错误/证明合同。

### Option C：保持GSP加载FW，允许GSP release后再提交（不推荐）

FMC把候选留在eHSM DRAM并release GSP；GSP加载FW后完成OTP提交，再开放Runtime和普通安全服务。

风险：

- 直接违反已批准“counter提交后才release GSP”和“FMC唯一更新Owner”；
- 形成新GSP在counter未持久化时已经执行的窗口；
- reset/掉电可能出现新GSP已运行但旧counter仍有效；
- Measurement `stored_counter_after`在FMC阶段无法真实成立。

只有负责人明确修改原安全原则并接受风险时才可继续评估。

### Option D：使用raw OTP或Host 64位counter（拒绝）

当前raw OTP接口不支持产品LCS，且缺少typed authorization；Host通用counter为64位、匹配FW未实现，也不是同一物理/逻辑资源。该选项不得进入产品设计。

## Implementation gate

- 主详设按已批准的staged-candidate API完成Counter、Measurement和release逻辑；
- 具体command/packing/LCS/交付版本在eHSM BL/FMC实现DoR前冻结；
- Vendor交付前不得把设计API写成当前硬件已验证Evidence；
- EMU必须验证candidate缺失/失效/不一致、低/同/高、timeout unknown和readback。

## 后续绑定输入

1. Vendor提供BL staged-candidate commit API的command ID、request/response packing、产品LCS权限和Host/BL兼容版本。
2. Vendor冻结DRAM candidate在reset/掉电、多次成功verify和FMC调用完成后的保留/清除规则。
3. Vendor提供timeout/acceptance unknown后的typed状态/readback合同，以及write failure到Host status/error的准确映射。
4. Vendor补充单向编码、寿命、耗尽和跳级限制；这些输入不得改变已冻结stage Owner和调用顺序。

## Review history

- 2026-07-28：负责人冻结FMC type 1、BootROM `check_version=0`暂存认证candidate、FMC初始化exact-match commit/readback及proof后接收同值GSP的最终顺序。
- 2026-07-27：项目负责人选择Option A并要求eHSM BL新增FMC专用16字节rollback-counter update API；关闭OPEN-CONFLICT-009，通用64位Counter映射不再讨论。
- 2026-07-24：INV/CE-SEC-011发现当前BL只暂存、FW启动才提交，而eHSM FW又由GSP加载；同时确认产品LCS raw OTP接口受限、Host 64位counter与FW不配套。建立OPEN-CONFLICT-009，等待负责人裁决。
- 2026-07-24：负责人决定Counter细节延期收敛；当前按16字节值并默认存在相应接口继续逻辑详设，不重开FW加载Owner。具体Vendor绑定在实现/EMU前补齐，假设不成立时重新激活本冲突。
