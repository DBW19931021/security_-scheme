# ADR-0012：BootROM LCS/Counter、GSP Vendor FW与重新下发边界

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018、SRC-0022
- 相关问题：OPEN-CONFLICT-005、OPEN-CONFLICT-009、OPEN-DESIGN-001
- 补充关系：补充ADR-0005、ADR-0010、ADR-0011
- 最后复核：2026-07-28

## 已接受决策

### 1. BootROM ready与LCS

1. BootROM必须同时检查`bootloader_done`和`bootloader_err`；成功语义固定为`bootloader_done == 1 && bootloader_err == 0`，错误位优先，保存raw状态。这里的“同时满足”不表示两个bit都为1。
2. 本次强调不取消此前Vendor基线中的硬件启动门禁；若状态接口同时提供`hw_boot_done/hw_boot_err`，仍按ADR-0010要求`hw_boot_done == 1 && hw_boot_err == 0`后才放行。只有硬件Owner提供证据并另行裁决，才能移除硬件状态条件。
3. LCS由BootROM直接从NGU800P权威SoC接口读取，不通过eHSM command。准确寄存器、字段、访问权限和read-clear语义仍从SRC-0022绑定，不使用Vendor示例地址。

### 2. BootROM验证并暂存FMC Counter Candidate

1. FMC固定使用Vendor image type 1；BootROM调用Vendor BL验证时固定`check_version=0`，不得在BootROM阶段比较或写入stored SoC counter。
2. Vendor BL对FMC密码验证成功后，把已认证的16字节candidate及valid状态记录在BL RAM；BootROM不得使用未认证Header、Manifest、Host参数或通用64位Counter替代该candidate。
3. BootROM完成NGU Manifest/policy和loader源摘要/copy/目标回读摘要比较后，把同一candidate写入并commit唯一FMC Measurement Entry；只有Entry有效且`release_state=RELEASE_AUTHORIZED`才允许release FMC。
4. candidate的stored比较、必要单调写入和权威readback由FMC初始化时调用BL专用接口完成，不属于BootROM职责。

### 3. Vendor FW由GSP阶段加载

1. FMC不加载、启动或切换eHSM Vendor FW。
2. eHSM Vendor FW的定位、完整性验证、加载、启动和`firmware_done == 1 && firmware_err == 0`门禁由GSP初始化阶段负责。
3. GSP在Vendor FW ready前后首版均继续使用poll；本决策不启用interrupt。
4. Vendor FW失败时GSP不得启动依赖该FW的Runtime验证、SPDM或运行期安全服务。

### 4. Host重新下发职责

1. Device负责接收当前package、完成本次格式/范围/密码/counter/policy检查，并返回结构化结果；Device不实现上位机升级编排，也不自动重发同一eHSM command。
2. 当本次eHSM调用已得到确定完成响应后，认证失败、格式错误、低counter等结果可以完成buffer清理和Host ingress重新arm；上位机可自行决定发送另一个package。
3. Vendor timeout、异常BUSY或其他completion unknown不属于“可立即重新下发”的普通验证失败。此时service/slot按ADR-0011 quarantine，RAS批准并完成reset/recovery前不得再次访问Mailbox；上位机只能在Device恢复后重新发送。
4. Counter写入失败、掉电未决或其他不可逆操作状态未知同样必须fail-close，不得以“重新下发”绕过。

## 已接受的Global Counter方案

1. 全部SoC stage镜像固定使用Vendor type 1并共用一个16字节global `rollback_counter`；Vendor type 2/3在产品路径直接拒绝。Vendor type 0 eHSM FW使用独立counter域，二者没有数值相等关系。
2. FMC package给出本boot的SoC candidate。BootROM以`check_version=0`完成type 1验证后，由BL把认证candidate暂存在RAM；BootROM不读取、比较或写stored counter。
3. FMC是唯一global counter提交发起Owner。FMC初始化从有效FMC Measurement Entry取得`expected_candidate`并传回BL；BL必须先与RAM candidate逐字节exact-match，无candidate、candidate失效或不一致均在OTP改动前拒绝。
4. exact-match后由BL执行stored比较：
   - `candidate < stored`：拒绝，不接收GSP；
   - `candidate == stored`：不写，权威readback一致后形成`PROVEN_EQUAL`；
   - `candidate > stored`：按Vendor单向编码写入，权威readback一致后形成`PROVEN_UPDATED`。
5. 只有proof成立后FMC才接收GSP。GSP固定以`check_version=0`验证，且其认证`rollback_counter`必须等于已提交值；随后才允许完成Manifest/policy、loader源/目标双摘要、Measurement commit和release。
6. PMP、RMP、MMP和Die1 SoC镜像固定`check_version=0`且必须等于已提交SoC global counter，不得独立推进。eHSM FW由GSP按Vendor type 0流程验证、加载和启动，不参与SoC counter提交。
7. Counter timeout、acceptance unknown或readback不能证明时，FMC不得接收或release GSP、不得盲目重写，进入fail-close/RAS恢复路径。
8. counter已经提交但GSP验证/加载失败时，不得回退低值GSP；本boot只允许在前次事务确定完成且清理成功后重新接收相同已提交值的GSP，否则进入受限恢复或整机重启。
9. BootROM提交的FMC Measurement Entry是FMC取得expected candidate的唯一跨stage来源；Entry缺失、重复、CRC/commit失败或字段不一致均阻断BL commit API和GSP接收。

## ADR-0019后续补充

本文中的global security epoch统一改名为`rollback_counter[16]`。FMC在自身初始化时调用eHSM BL新增的staged-candidate exact-match commit/readback API；它发生在GSP接收之前。eHSM Vendor FW仍由GSP加载，但其启动副作用不参与SoC Counter提交证明。

## 影响

- BootROM不需要eHSM LCS command或counter read/update wrapper，只负责`check_version=0`验证、BL candidate暂存结果门禁和FMC Measurement生产。
- FMC不需要Vendor FW loader，但必须在初始化时调用BL staged-candidate commit client并验证proof。
- GSP需要Vendor FW加载/启动状态机，并在FW ready前阻止依赖服务。
- Device重新接收路径必须区分“确定完成的验证失败”和“completion unknown quarantine”。
- ADR-0019已关闭OPEN-CONFLICT-005/009：FMC主动调用eHSM BL新增16字节专用API；GSP Vendor FW加载Owner不重开。

## Review history

- 2026-07-23：负责人确认BootROM直接读取LCS、只比较counter不写；Vendor FW由GSP阶段加载；确定完成的验证失败后Device可重新接收，重发策略由上位机负责。
- 2026-07-23：负责人批准最终方向：GSP package作为全局security epoch锚点，FMC是唯一counter更新者；eHSM FW/PMP/RMP/MMP必须匹配已提交epoch，失败镜像局部隔离并等待Host重发。整包manifest/原子激活降为可选增强。
- 2026-07-23：负责人批准FMC从BootROM已commit的FMC Measurement条目取得自身epoch；冻结producer/consumer、epoch一致性和release门禁，不引入Handoff。
- 2026-07-24：CE-SEC-011确认Vendor counter实际提交点与本ADR的GSP Vendor FW加载Owner不兼容，登记OPEN-CONFLICT-009；本ADR保持accepted，但受影响实现暂停等待typed接口或Owner重裁决。
- 2026-07-24：负责人决定Counter细节延期；按16字节默认接口继续设计，不重开GSP加载Owner，具体绑定移至实现/EMU前门禁。
- 2026-07-28：后续裁决取代本文原“BootROM compare-only/GSP验证后更新”时序：BootROM固定`check_version=0`并由BL暂存FMC candidate；FMC初始化回传expected candidate，BL exact-match后提交/readback；proof成立后才接收同值GSP。
