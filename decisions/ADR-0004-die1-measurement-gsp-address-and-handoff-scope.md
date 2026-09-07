# ADR-0004：Die1 Measurement、GSP地址域与当前Handoff范围

> 2026-08-21后续裁决：ADR-0030删除Manifest。本文System Address、Measurement快照和“不采用版本化Handoff”的原则继续有效；下文的Manifest地址Owner现统一替换为Native Header offset1008 `load_addr`与typed-stage loader精确目标匹配，`entry_addr=load_addr`。

- 状态：accepted
- 日期：2026-07-22
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0020
- 相关冲突：OPEN-CONFLICT-002 / CONFLICT-SRC-0016-DIE1-MEASUREMENT；OPEN-CONFLICT-003 / CONFLICT-CODE-GSP-ADDRESS-DOMAIN
- 相关原则：W0-R1-05、W0-R1-06、W0-R1-08、W0-R1-11
- 替代关系：补充ADR-0003，并结束其中W0-R1-05的待评审状态

## 背景

负责人要求OPEN-CONFLICT-002和OPEN-CONFLICT-003采用评审包中的推荐方案，同时确认当前不需要新增版本化Handoff机制，应以现有芯片安全软件方案为准。当前方案中跨阶段需要保留的启动度量信息由Measurement Table承载，不因潜在未来需求新增一套阶段间共享ABI。

## 决策

### 1. OPEN-CONFLICT-002：采用Option A

1. Die1实际加载实例必须在内部Measurement Table形成独立记录。
2. 当前使用`fw_type=NGU_FW_TYPE_DIE1_FW`和`die_id=1`表达Die1实例；即使Die0/Die1使用相同映像、版本和digest，也不得丢失“Die1实例已被验证/加载”的可审计事实。
3. 重复映像可以产生相同digest，但实例标识、die_id和对应状态必须可区分。
4. SPDM对外呈现可以在后续批准的profile中定义逐条返回或聚合；若聚合，Verifier仍必须能够判断Die1实例的有效measurement及release状态，不能因压缩而丢失Die1实例语义。
5. 用例NGU800P-D0-SECURITY-107不再因本冲突阻塞；v0.3按本决策高亮更新Expected，v0.2保持不变。

### 2. OPEN-CONFLICT-003：采用Option A

> 2026-07-28最终裁决取代本节早期“canonical输入后转换”的实现方式。

1. GSP Native Header offset1008的`load_addr`不携带domain，固定解释为baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`定义的64位System Address；`entry_addr=load_addr`。
2. C908 PC、Header Overlay、typed-stage loader、linker和eHSM共享descriptor使用同一System Address数值；NGU800P产品软件不建立Local/System映射。
3. port/loader必须拒绝local/remap输入、旧080x地址、越界、溢出和不属于目标Region的span；Vendor回调只对active descriptor做范围校验并同值返回。
4. eHSM BL内部`SYS_SOC_MEM_BAL/BAH`与remap window属于Vendor内部实现，SoC侧不复制、不反向推导映射关系。
5. System Address视图不再是开放项；精确Region offset/容量、PMA和Firewall行为仍需平台输入及RTL/EMU Evidence。

### 3. W0-R1-05：当前不采用版本化Handoff

1. W0-R1-05状态从`pending_explanation_and_owner_review`改为`deferred_not_adopted_for_current_baseline`。
2. 当前设计不新增`security_handoff`结构、Handoff专用SRAM、commit协议、producer/consumer ABI或相应编码任务。
3. 跨阶段需要保留的镜像身份、版本、digest、die_id及方案规定的验证/释放度量信息统一由Measurement Table记录。
4. 镜像来源和System Address load/entry由已认证Header Overlay与typed-stage loader合同共同提供；错误/raw status由统一错误、日志和Evidence机制处理。不得为了替代Handoff而把无关控制状态或敏感数据塞入Measurement Table。
5. BootROM/FMC/GSP的具体调用、跳转和release流程继续以当前有效芯片安全软件方案及后续批准详设为准。
6. 只有后续出现明确、可复现的工程需求——例如下一阶段无法从Measurement Table、Header Overlay/typed-stage registry和批准接口重建必需可信状态——才可重新提出最小阶段交接机制；届时必须另行经过方案变更、ADR/OpenSpec和负责人批准。

### 4. 后续ADR-0006对绝对地址的影响

ADR-0006新增2 MiB安全RAM目标后，2026-07-28最终裁决冻结产品软件只使用`MANAGEMENT_NOC_S9_SRAM_BASE=0x1010_0500_0000`及其2 MiB范围。早期`0x1010_0808_0000`和`0x1000_0808_0000`仅保留为历史代码事实，不再是新D0地址；最终linker只等待精确Region容量、PMA和Firewall输入。

## 影响

- 当前W0不再等待R1-05，Handoff不再是DEV-SEC-002/005～007的设计或编码前提。
- Measurement Table成为当前方案跨阶段度量记录的唯一正式载体；需在DD-05冻结字段、owner、commit、完整性、reset和SPDM映射。
- OPEN-CONFLICT-002/003关闭；受影响Feature和用例可以继续进入详设，但代码修改仍需单独OpenSpec和实施任务授权。
- `docs/04-interfaces/boot-handoff.md`保留为已否决当前采用的历史提议，不进入当前基线和实现范围。

## 测试影响

- v0.3高亮修改用例107，要求可区分Die1实例，即使digest与Die0相同。
- v0.3和后续可执行测试覆盖System Address边界、local/remap及旧080x输入拒绝、溢出、越界和隐式alias；不再生成转换成功或地址domain场景。
- v0.2不覆盖修改；本ADR不表示相关用例已经执行或通过。

## 参考

- `sources/conflict-reports/CONFLICT-SRC-0016-DIE1-MEASUREMENT.md`
- `sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md`
- `docs/09-plans/W0首轮设计评审包.md`
- `docs/03-architecture/secure-boot.md`
- `docs/05-software-design/NGU800P安全软件详细设计.md`
- `decisions/ADR-0006-b0-r2-security-ram-mailbox-and-ras-reset.md`
- `sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md`
