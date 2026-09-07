# ADR-0020：失败终态、Provisioning、Lifecycle、SPDM、更新、Multi-Die与发布原则

- Status: accepted
- Date: 2026-07-27
- Owner: 项目负责人
- Related: SRC-0024、OPEN-DESIGN-003、OPEN-DESIGN-011、OPEN-DESIGN-013～020、ADR-0010、ADR-0013、ADR-0017～0019
- Last reviewed: 2026-08-04

> 2026-08-04更新：SRC-0024把单个`Device Private Key`物理槽纠正为slot14，并冻结Table 34和完整16-slot顺序。软件和测试保留P-256与SM2两套能力，但每台设备由Provisioning Profile二选一，只持久化和使用一把设备私钥；不支持同机同时使用两套长期身份，也不允许跨曲线复用scalar。其余失败终态、Lifecycle、SPDM secure-session、更新、Multi-Die和发布原则不变。

## Context

完整主详设第10～16章已形成Lifecycle、Debug、Key/Certificate、SPDM、更新/OOB/Recovery、Multi-Die、RAS/审计/清零、构建发布和流片前验证的推荐原则。负责人批准其中可以冻结的产品方向，并要求对OPEN-DESIGN-014和OPEN-DESIGN-019中仍不清楚的内容继续解释后再裁决。

## Decision

1. `OPEN-DESIGN-003`关闭。security侧失败终态固定为：
   - BootROM eHSM/FMC失败：不release FMC，记录静态错误，撤销权限并尽力清零，请求RAS；RAS不可用时关闭普通中断并进入无限WFI fail-stop循环。
   - FMC验证GSP确定失败：不release GSP，仅允许对已证明可清理、无不可逆副作用的接收失败重新arm；不可逆状态未知时终止本boot。
   - GSP验证单个Runtime失败：隔离对应Runtime及其依赖者，保留不依赖它的安全控制面和Runtime。
   - eHSM timeout/late response或OTP/Counter状态未知：context/service或对象quarantine，不自动retry，等待权威查询、新boot或RAS批准恢复。
   - security只上报event和action request，不直接执行reset/watchdog；精确RAS event ID、通道和平台动作表作为OPEN-DESIGN-018的生成/集成输入。
2. `OPEN-DESIGN-011`关闭。采用单一受控provisioning/release matrix，逐设备/SKU和镜像绑定三套已批准算法Profile、Vendor Image Type、SoC/eHSM key域、boot/upgrade key ID、board binding和LCS mask。Manifest、OTP recipe、制包器、测试Expected和release Evidence从同一行配置生成；没有精确匹配行时默认拒绝。所有产品构建必须保留Profile 1/2/3三条实现路径，三套Profile均须通过完整启动链、更新链和负向测试；matrix只选择具体包使用的Profile和Key，不得用于SKU裁剪代码路径。
3. `OPEN-DESIGN-013`关闭。采用第10章typed Lifecycle/Debug service和challenge/token流程；LCS raw值及转换只来自RTL同步的baremetal生成头。NGU800P没有Debug scope，token不得携带scope、Die/core bitmap或分级授权；成功后只控制一个SoC全局Debug enable，Die0/Die1跟随同一开关。timeout、本地最大开放时长、reset、LCS变化、安全错误和显式close都必须关闭Debug；一次授权只消费一次。RMA固定由DEBUG LCS承载，但使用独立RMA授权和只读诊断白名单，普通Debug token不得替代；不新增RMA LCS且不关闭安全启动。DESTROY不可恢复且只保留平台批准的最小识别/报错能力。
4. `OPEN-DESIGN-015`保持开放，但首版安全选择已经冻结：SPDM 1.2只采用证书`KEY_EXCHANGE -> FINISH` secure session，拒绝PSK和无会话fallback；按ADR-0029，Cert0/1保存三张静态Issuer前缀，GSP为当前安全启动追加一级动态Firmware Alias Leaf，eHSM内部派生Alias Key并完成Issuer签Leaf和Alias签Report/transcript。软件实现和测试必须保留P-256与SM2两套能力并分别完成签名及证书公钥匹配。每台设备由Provisioning Profile选择其中一套，只持久化一把slot14 Device Issuer Private Key，Cert0/Cert1服务于同一Profile和同一设备身份。Measurement对外不返回地址，只返回summary block。transport、消息/完整链上限、block index和session wire细节由完整Profile输入冻结；在此之前状态为`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
5. `OPEN-DESIGN-016`关闭。正常更新只写inactive slot并读回校验；metadata固定使用A/B双副本，每份包含单调`sequence`、integrity和最后写入的`commit`，扫描时只接受完整有效记录并选择最大sequence，相同sequence内容冲突进入`RECOVERY_REQUIRED`。metadata提交后请求reset，由下一次BootROM/FMC/GSP真实安全启动作最终信任判定。OOB只验证外层授权并写固定FMC候选分区，不修改LCS、Counter、Key或Measurement。禁止回退到较低`rollback_counter`或未验证镜像；Counter已推进而候选不可用时，只能进入受限恢复等待相同或更高值的有效包。精确Flash分区、transport、token格式和控制器Owner作为平台生成/集成输入。
6. `OPEN-DESIGN-017`关闭。Die0是系统安全根；Die1镜像由Die0 GSP和Die0 eHSM完成验证、解密、受控传输和读回证明。Die1保持独立Measurement、release和失败实例；目标digest和Firewall最终状态证明完成后才能release。Die1失败只隔离Die1及其依赖者，不降低Die0策略。精确UCIe descriptor、master/target ID、地址、容量和reset/release寄存器来自RTL/平台Profile。
7. `OPEN-DESIGN-018`关闭。错误severity与RAS动作分离；保持单一eHSM Owner、自动retry为0、timeout quarantine、敏感日志最小化和受控secure-zero。priority、stack、queue depth、deadline、审计容量/时间源、RAS event ID及清零硬件参数由EMU测量和平台资料冻结，禁止以无证据常量进入产品。
8. `OPEN-DESIGN-019`部分裁决：产品和EMU构建禁止stub、simulated success、test key/cert/provider、伪RNG、零签名、silent fallback和未批准hardcode。测试mock只允许存在于明确隔离的host-unit target，且构建Evidence必须证明它们没有进入EMU/产品符号表和map。Registry/profile生成组织固定为`security_abi_registry.yaml`、`security_product_profiles.yaml`、`security_provisioning_release_matrix.yaml`、`security_ram_layout.yaml`四个项目权威源，由统一生成器输出C header、linker include、制包/测试schema和`security_abi_registry.json`审计快照；不使用巨型配置、不复制SRC-0022/Vendor权威常量、不生成密钥。正式toolchain/flags、KMS、SBOM、兼容矩阵和发布Owner仍待输入，因此OPEN-DESIGN-019保持部分开放。
9. `OPEN-DESIGN-020`关闭。Feature 001～020必须形成Source→Design→Code→Case→Evidence闭环；高严重度缺陷必须关闭，环境缺能力只能标记BLOCKED，不能标记PASS；waiver必须记录Owner、范围、期限和批准，流片前安全方案与release artifact必须由指定负责人签署。具体覆盖阈值、Owner姓名和签署名单在验证计划执行前补齐。
10. `OPEN-DESIGN-014`后续由ADR-0021、ADR-0025和ADR-0026完成软件设计裁决；Vendor交付、RTL绑定、Flash base及KMS/CA/MES外部接口作为独立实施输入管理。

## Consequences

- 关闭OPEN-DESIGN-003、011、013、016、017、018、020。
- OPEN-DESIGN-015保留为SPDM具体Profile和secure-session实现合同开放项，当前为`DESIGN_BLOCKED_BY_PROFILE_INPUT`。
- OPEN-DESIGN-019的registry/profile生成组织、no-stub与test隔离规则不再重开；只保留toolchain/KMS/SBOM/兼容矩阵和发布Owner外部输入。
- OPEN-DESIGN-014的软件设计由ADR-0026关闭；未到齐的Vendor、RTL和平台输入不得被描述为已实现。
- 本ADR只更新`security_-scheme`设计、约束和计划；不修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor代码，不执行Git操作。
