# Version/rollback Counter宽度、比较与更新语义冲突

## Identity

- Conflict ID: CONFLICT-COUNTER-WIDTH-COMPARE-UPDATE
- Open Question: OPEN-CONFLICT-005
- Status: resolved
- Evidence state: CONFIRMED
- Owner: 项目负责人；eFuse/eHSM/Anti-rollback Owner待指定
- Resolution: ADR-0019

## Conflict classification

- Type: cross_baseline_semantic_conflict / data_width_mismatch / irreversible_update_risk
- Affected scope: eHSM native `Version_Counter`、项目Manifest `rollback_counter`、eFuse stored counter、Measurement counter、启动接受规则和counter更新时机。
- Safe-to-continue scope: verify API可以把Vendor counter作为opaque字段传递并保留raw result；低版本不得release的原则。
- Must-stop scope: 继续使用32/64位counter、把Manifest `version`用于防回滚、绕过eHSM BL专用API写OTP。

## Final owner decision（2026-07-28）

1. 不存在“32字节Counter”歧义。Vendor手册、Header和代码均为16字节；旧差异是SRC-0016 Measurement示例的`uint32_t`，即32位/4字节。
2. 产品唯一使用`rollback_counter[16]`；Vendor物理字段继续名为`Version_Counter[16]`。32位和Host通用64位方案均被否决。
3. Manifest独立增加`uint32_t version`供Host工具读取；它与防回滚无关。
4. BootROM固定以`check_version=0`验证Vendor type 1 FMC并由BL暂存认证candidate；FMC初始化从有效FMC Measurement取得`expected_candidate`并调用BL专用staged-candidate commit/readback API。BL必须先逐字节exact-match，再执行低值拒绝、相等不写、高值单向写和权威readback。
5. OPEN-CONFLICT-005关闭；API实施绑定由已关闭OPEN-CONFLICT-009的resolution和实现DoR管理。

## Partial owner decision（2026-07-22）

- Version/rollback Counter逻辑宽度以Vendor native格式为准，统一为16字节。
- Manifest、Measurement、工具和软件接口不得继续把32位示例作为最终ABI，也不得将16字节值截断为32位。
- ABI优先使用`uint8_t counter[16]`或等价16-octet字段，避免依赖编译器`__int128`布局。
- Vendor字节序、比较规则、写入命令、同版本重启、更新时点、掉电和耗尽语义仍需继续核对/裁决，因此OPEN-CONFLICT-005暂不关闭。

## Historical owner decision（2026-07-23，Counter时序已由2026-07-28裁决取代）

- BootROM直接读取LCS和可信stored counter，只比较FMC package counter，不执行counter写入。
- 产品接受语义收敛为：`image < stored`拒绝，`image == stored`允许重复启动且不写，`image > stored`作为候选升级；不采用每次正常启动都要求`stored + 1`。
- 全SoC只使用一个16字节global `rollback_counter`；FMC是唯一更新Owner。
- FMC和GSP必须携带同一rollback counter。FMC完整验证GSP后主动调用eHSM BL专用API；高值更新并读回成功才release，相等允许重复启动且不写，低值拒绝。
- eHSM FW/PMP/RMP/MMP必须匹配已提交值，不得独立推进counter；不匹配/验证失败时只隔离对应镜像并等待Host重发。
- 完整签名bundle manifest、A/B或journal整包激活为可选增强，不是counter推进的必需前提。最低掉电恢复保证是同epoch FMC已持久化，counter推进后可重新启动FMC并等待Host重发同epoch GSP/Runtime。

## Source A：eHSM native header使用128位Version_Counter

- Source ID/version: SRC-0016，芯片安全软件方案v1.2。
- Evidence: 第8页§3.2把`Version_Counter`定义为16字节，并描述为128位单向版本计数。
- Interpretation: native package提供128位版本/counter字段。

## Source B：项目Measurement/global counter示例使用32位

- Source ID/version: SRC-0016 v1.2。
- Evidence: 第14页`measurement_table_header_t.stored_global_counter`和第15页`ngu_meas_fw_entry_t.image_counter`均为`uint32_t`。
- Related evidence: 第4页表示eHSM验证FMC/GSP/Runtime/Die1时比较header `image_counter`和eFuse `stored_global_counter`，但没有定义128→32位映射。

## Source C：SRC-0017使用严格`stored + 1`接受条件

- Source ID/version: SRC-0017，NGU800P芯片系统安全方案，版本UNKNOWN。
- Evidence: 第15页§8.1描述读取镜像counter并要求`镜像counter >= 安全存储counter + 1`，否则判定回滚或重放。
- Risk: 若每次成功启动后把stored更新为当前镜像counter，则相同已批准镜像在下次复位时不再满足`>= stored + 1`；若不更新，则又没有说明何时推进安全counter。

## CE-SEC-011实现收敛（2026-07-24）

- Vendor Header和SOC OTP字段均为16字节；当前测试向量采用从第一个octet开始逐bit推进的thermometer/unary单向编码。
- Vendor BL和FW比较允许`image == stored`；只有逻辑版本严格更高时才写OTP。
- BL成功验证非naked SoC镜像后只把候选值暂存在eHSM DRAM，且`check_version=OFF`也会暂存；eHSM FW启动时才消费候选并写SOC OTP。
- Vendor写函数内部执行写后回读比较，但当前Host没有产品LCS可用的确切16字节counter-after查询合同。
- Host中的64位`create/read/increase/delete counter`与安全启动16字节OTP版本计数器不是同一合同，且匹配FW快照没有handler/dispatch实现。
- “GSP加载eHSM FW”与“FMC release GSP前完成counter提交”的物理绑定差距曾转由OPEN-CONFLICT-009管理；ADR-0019现已关闭该项，目标设计要求eHSM BL新增FMC专用16字节staged-candidate exact-match commit/readback API，不再依赖Vendor FW启动副作用。

## Remaining implementation inputs（不重开宽度/Owner裁决）

1. 完整128级编码、允许跳级/多bit写、最大推进次数、未烧写值、寿命和耗尽规则；
2. eHSM DRAM candidate在eHSM reset、SoC reset和掉电下的保留/清除，以及多次成功verify覆盖的正式合同；
3. eHSM BL新增staged-candidate commit API的command ID、packing、产品LCS权限、交付版本和timeout后权威查询；
4. `FW_ERROR_OTP_WRITE_FAILED`到Host可观察status/error的准确映射；
5. 提交stage逻辑顺序固定为BootROM暂存candidate并commit FMC Entry→FMC初始化exact-match commit/readback proven→接收并验证同值GSP→GSP Measurement commit→release。

## Impact

- Security: 错误比较可能允许回滚，或导致合法镜像被永久拒绝。
- Irreversibility: eFuse写入不可逆，错误更新时机会造成设备报废或counter耗尽。
- Software: package工具、eHSM command、Manifest、Measurement和更新状态机无法共享同一语义。
- Verification: 无法构造相同版本重启、升级、降级、跳级、掉电和耗尽的确定Expected。

## Options

### Option A：32位项目counter（已拒绝）

- 产品有效范围为`uint32_t`，native 16字节字段按Vendor规定字节序编码，要求高96位为0。
- 启动允许`image_counter == stored_counter`；拒绝`image_counter < stored_counter`。
- 只有经授权的新版本完整验证并满足更新提交条件时，才把stored推进到更大的image counter；更新必须定义掉电/重试语义。

优点：与当前Measurement示例一致、实现较简单；前提是eFuse真实counter宽度和Vendor比较接口支持该映射。

### Option B：16字节端到端counter（已采用并命名rollback_counter）

- Manifest、eFuse抽象、Measurement和工具全部使用16字节counter。
- 所有比较按无符号128位整数和明确字节序执行。

优点：与native字段一致；代价是eFuse存储、更新原子性、ABI和工具复杂，需要硬件支持证据。

### Option C：native counter与项目global counter分域（已拒绝）

- native 128位字段由eHSM处理；项目另有32位发布序号/rollback domain。
- 明确二者各自owner、比较顺序和一致性约束。

优点：可适配Vendor实现；风险是双counter不同步和策略复杂。

## 原推荐与当前状态

负责人已明确采用Vendor 16字节格式和`rollback_counter`命名，并由ADR-0019要求eHSM BL新增FMC专用staged-candidate exact-match commit API。宽度、Owner、调用stage和Host通用Counter排除均已关闭；剩余项只属于实现参数和EMU验证。

## Required owner decision/input

1. Vendor完整128级编码、允许跳级/多bit写、最大推进次数、寿命和耗尽预警是什么？
2. DRAM candidate在各reset/掉电域下如何保留或清除，多次verify是否正式定义为最后一次成功值覆盖？
3. Vendor update timeout或状态未知后，产品LCS使用哪个typed只读接口判定实际值；若无法判定，RAS恢复流程是什么？
4. `FW_ERROR_OTP_WRITE_FAILED`是否一定进入Host检查的error状态，ready与error并存时的权威行为是什么？
5. eHSM BL新增staged-candidate commit API的command/packing/LCS/交付版本必须在实现前冻结；不重开eHSM FW加载Owner。
6. 已批准产品语义必须进入SRC-0016后续版本或受控amendment，并同步工具、Measurement和测试。

## Review history

- 2026-07-28：后续裁决冻结BootROM `check_version=0`暂存FMC candidate、FMC初始化回传expected candidate并由BL exact-match后提交/readback；proof成立后才接收同值GSP。
- 2026-07-27：接受ADR-0019；澄清旧差异是32位而非32字节，统一`rollback_counter[16]`，FMC主动调用eHSM BL新增专用API；关闭OPEN-CONFLICT-005。
- 2026-07-22：在任务二首轮详细设计复核SRC-0016第4/8/14/15页及SRC-0017第15页时发现；建立OPEN-CONFLICT-005，阻断counter最终ABI和eFuse更新代码。
- 2026-07-22：负责人部分裁决counter宽度以Vendor为准统一为16字节；32位方案不再采用，字节序、比较和更新语义继续开放。
- 2026-07-23：负责人最终批准单一global security epoch、GSP package作为epoch锚点、FMC唯一更新；其他固件匹配已提交epoch，失败镜像隔离并等待Host重发。整包manifest/原子激活降为可选增强。
- 2026-07-23：负责人批准BootROM把已验证FMC epoch写入committed FMC Measurement条目，FMC从该唯一条目读取自身epoch；冻结FMC/GSP epoch一致性和counter update→GSP Measurement commit→release顺序。
- 2026-07-24：完成CE-SEC-011；冻结当前Vendor单向测试编码、BL DRAM暂存、FW启动提交和内部写后回读事实，排除Host 64位counter/raw OTP作为产品替代。提交阶段冲突升级为OPEN-CONFLICT-009。
- 2026-07-24：负责人决定Counter细节延期；按16字节值和默认FMC接口继续逻辑详设，准确Vendor绑定移至实现/EMU前。
