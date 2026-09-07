# ADR-0005：B0-R1地址、Counter与Vendor BL边界

- 状态：accepted
- 日期：2026-07-22
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关冲突：OPEN-CONFLICT-004、OPEN-CONFLICT-005
- 相关合同：FW-C-001～FW-C-011
- 替代关系：补充ADR-0003/0004；不替代其中已接受结论

## 背景

任务二B0-R1复核发现，SRC-0016 Measurement示例的32位加载地址无法表达已批准的GSP 64位canonical地址；同时，eHSM native `Version_Counter`为16字节，而方案中的Measurement/global counter示例为32位。负责人还确认Wing-M130/eHSM BL应保持Vendor业务基线，不因C908产品软件开发而重新实现。

## 决策

### 1. Measurement加载地址

1. Measurement中的`load_addr`和`entry_addr`均保留并使用64位定宽类型；C wire ABI表达为`uint64_t`。
2. 两个地址不携带domain，固定解释为baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE`范围内的64位System Address；禁止截断、隐式alias、Local/System转换和local/remap输入。
3. 两个字段只记录loader成功执行后的只读审计快照，不能作为loader或jump的执行输入；Manifest/loader保持执行地址唯一Owner。
4. OPEN-CONFLICT-004关闭；字段offset、对齐和table整体版本继续在FW-C-007正常详设中冻结。

### 2. Version/rollback Counter宽度

1. 项目Version/rollback Counter逻辑宽度以Vendor native格式为准，统一为16字节；不得继续使用SRC-0016示例中的32位counter作为最终ABI，也不得截断或建立独立32位项目counter。
2. wire/持久化结构固定表达为`uint8_t counter[16]`，不依赖编译器是否支持`__int128`。
3. Manifest、Measurement、制包工具、eHSM adapter和counter service必须使用同一16字节逻辑值；具体字节序、比较算法、单向编码和Vendor命令映射仍需从Vendor正式文档/代码冻结。
4. 本ADR在2026-07-22批准时只关闭位宽选择；当时`image >= stored`还是`image >= stored + 1`、同版本重启、更新提交点、掉电恢复和counter耗尽策略尚未裁决，因此OPEN-CONFLICT-005曾保持部分开放。其当前状态以ADR-0019为准。

> 2026-07-23后续裁决：ADR-0012已冻结产品语义为`image >= stored`、同版本可重复启动；GSP package作为global security epoch锚点，FMC唯一更新，其他镜像匹配已提交epoch。
>
> 2026-07-27后续裁决：ADR-0019确认旧差异是32位而非32字节；项目统一命名`rollback_counter[16]`，并要求eHSM BL新增FMC专用update API。OPEN-CONFLICT-005/009均已关闭。
>
> 2026-07-28最终时序：FMC固定type 1；BootROM以`check_version=0`验证并由BL暂存认证candidate，不读取/比较/写stored counter。FMC初始化把Measurement中的expected candidate回传BL，BL exact-match后提交/readback；proof成立后才接收同值GSP。本条取代上面2026-07-23的GSP锚点/验证后更新时序。

### 3. Wing-M130/eHSM BL边界

1. Wing-M130/eHSM BL默认保持Vendor业务基线，不复制、不重写其内部业务流程。
2. 项目必须冻结Vendor BL/FW/Host配套版本、构建配置、自检位图、ready/error、Mailbox兼容和升级/回退边界。
3. 只有存在经过Evidence确认的NGU800P配置、接口、版本配套、安全缺陷或正式Vendor变更时，才创建单独的集成变更任务。
4. baremetal的`bl_demo`和Host测试代码只用于接口/能力验证，不等同于Wing-M130/eHSM BL产品固件。

## 后续未决

- 16字节counter物理资源标识、准确提交/status接口绑定、完整寿命/耗尽和不可逆存储恢复细节。
- OPEN-CONFLICT-009已被ADR-0019关闭；当前不重开“GSP加载Vendor FW”，而是要求FMC主动调用eHSM BL新增专用API。准确command/packing/LCS/status/readback/交付版本仍是实现和EMU前DoR。
- FW-C-001～011不是一次性ABI批准；各合同在对应B0轮次逐项冻结。

## 影响

- Measurement和counter相关设计可以按已批准宽度继续，但最终layout及代码任务仍受剩余问题阻断。
- 现有32位counter字段只能标记为旧方案示例或待迁移实现，不能作为产品ABI。
- Wing-M130/eHSM BL工作转为版本/配置/接口核对和差异集成，不进入无证据的业务重构。
- 本ADR不授权修改`gsp-pmp-rmp-omp`或`baremetal`。

## 参考

- `sources/conflict-reports/CONFLICT-MEASUREMENT-ADDRESS-WIDTH-OWNERSHIP.md`
- `sources/conflict-reports/CONFLICT-COUNTER-WIDTH-COMPARE-UPDATE.md`
- `sources/conflict-reports/CONFLICT-VENDOR-VERSION-COUNTER-COMMIT-SEQUENCE.md`
- `evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md`
- `docs/09-plans/SOC安全固件首轮详设评审包-启动链与核心合同.md`
- `docs/09-plans/NGU800P两项任务总计划.md`
