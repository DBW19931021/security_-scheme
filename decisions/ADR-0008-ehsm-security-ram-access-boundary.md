# ADR-0008：eHSM对安全RAM的访问边界

- 状态：accepted
- 日期：2026-07-23
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018
- 相关证据：CE-SEC-004；Vendor Host `ehsm_ctx_intl_st`、Mailbox packet机制
- 相关冲突：OPEN-CONFLICT-006
- 补充关系：修正ADR-0007中“eHSM只允许访问Mailbox窄窗口”的限制

## 背景

ADR-0007为减少eHSM总线权限，曾要求eHSM只访问独立Mailbox子区和批准的业务buffer。项目负责人进一步明确：在NGU800P产品信任边界中，eHSM可以访问整个2 MiB安全RAM，不对eHSM设置Region级Firewall访问限制。

Vendor Host实现同时表明，Mailbox寄存器只传递一个64位packet地址；`ehsm_ctx_intl_st`内嵌packet、160字节command和20字节response，具体command还会携带其他输入/输出buffer地址。该机制本身不要求固定的独立Mailbox物理区。

## 决策

1. eHSM被视为可以访问整个2 MiB安全RAM的受信任master；SoC Firewall不为eHSM配置GSP、Measurement、Host ingress、plaintext、PMP/RMP/MMP之间的Region级访问限制。
2. 上述裁决只取消eHSM的硬件地址窗口限制，不取消软件System Address范围、长度、active descriptor、overflow、生命周期、PMA/barrier和command语义检查。产品不建立Local/System address-domain或转换。Host及其他非信任master的权限不变。
3. eHSM能够访问不等于业务command可以任意读写。NGU800P adapter仍只向eHSM提供当前command所需的有效地址和长度，并在command完成、失败或timeout处置完成后清理临时对象。
4. eHSM访问整个RAM意味着eHSM硬件/FW成为该2 MiB内GSP代码、Measurement和明文等资产的共同可信计算基的一部分；后续威胁模型、失效注入和发布说明必须明确该信任假设。

## Mailbox内存放置后续批准

负责人于2026-07-23批准取消独立64 KiB Mailbox物理Region。packet/cmd/rsp/channel context改为stage内固定`EHSM_CONTEXT_ARENA`：BootROM/FMC各在尾部启动复用区内的stage data/BSS设置专用`.ehsm_context_arena` linker section，GSP在`SEC_RAM_GSP_COMPLEX` data区设置同名专用section；不另建固定pool或Firewall Region。

首版禁止把eHSM context作为普通函数或RTOS task局部变量放在运行栈上，也不设同步叶子调用例外。BootROM/FMC/GSP均只使用stage-local静态`.ehsm_context_arena` slot；首版不注册interrupt、async、send-and-peek或流式init/update/finish路由，timeout/迟到响应闭环前slot不得清理或复用。

`SEC_RAM_EHSM_MAILBOX`从目标布局中删除；后续linker只为各stage提供arena section/pool。

> 2026-07-28最终裁决：ADR-0011冻结首版每stage一个64字节对齐、256字节静态context slot和最多一条在途事务，GSP通过唯一service串行化；只使用one-shot typed API，不分配`ehsm_session_st`；自动retry为0，timeout后service/slot quarantine。仍待集成的是最终PMA属性、每stage service channel、实际I/O descriptor/buffer和arena总容量。

## 影响

- 删除“eHSM只能访问Mailbox窄窗口”的项目约束和测试Expected。
- 保留Host隔离、W^X、owner切换、地址/长度检查和敏感材料清零。
- 采用stage-local arena，回收原P1示例中的64 KiB独立Mailbox容量；首版按一个256字节context slot加实际descriptor/buffer计算，不预留512字节session，也不按16个硬件channel预留16份并发context。
- timeout统一视为`acceptance_unknown`并quarantine整个service/slot；RAS批准并完成reset/recovery前不得回收或覆盖。

## Review history

- 2026-07-23：负责人批准eHSM full aperture。
- 2026-07-23：负责人批准取消独立Mailbox Region并采用stage-local固定`EHSM_CONTEXT_ARENA`；通用异步、流式及timeout未闭环context禁止放在普通函数栈。
- 2026-07-28：首版固定使用one-shot typed API并禁止流式API，不分配`ehsm_session_st`。
- 2026-07-23：ADR-0011补充冻结首版单context/单在途、256字节slot、GSP唯一service、零自动retry及timeout quarantine；最终PMA和arena总容量仍待集成。

## 参考

- `docs/03-architecture/security-ram-layout.md`
- `docs/04-interfaces/ehsm-mailbox.md`
- `evidence/code-investigations/CE-SEC-004-security-ram-map-and-mailbox.md`
- `source-vault/vendor-code/osr_eshm/ehsm_host-2.3.1-4019-2ee044d/src/types_internal.h`
- `source-vault/vendor-code/osr_eshm/ehsm_host-2.3.1-4019-2ee044d/src/api.c`
