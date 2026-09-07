# CE-SEC-005：eHSM Context内存放置与生命周期代码证据

## Evidence metadata

- Evidence ID：CE-SEC-005
- 日期：2026-07-23
- 父任务：INV-SEC-005
- 仓库：SRC-0018 Vendor代码受控快照
- 记录基线：`ehsm_host-2.3.1-4019-2ee044d`
- 操作：只读文本调查；未修改代码，未构建/测试，未执行Git命令

## FACT-01：Vendor协议不要求固定独立Mailbox物理区

- `include/ehsmdrv/basic/mailbox.h:7-23`：`ehsm_mb_packet_st`只包含两个64位remote address；command为160字节，response为20字节。
- `src/api.c:266-267`：`ehsm_ctx_init()`把内嵌`cmd_buf`和`rsp_buf`的地址写入packet。
- `src/api.c:147-149`：发送时再把context内嵌packet的地址交给`ehsm_mb_send()`。
- `src/mailbox.c:108-135`：Mailbox寄存器只接收packet地址，不检查packet位于某个固定共享区。

结论：packet/cmd/rsp只需位于eHSM可寻址、cache行为正确且生命周期有效的RAM，不要求一个固定64 KiB物理Region。

## FACT-02：packet/cmd/rsp属于调用方context对象

- `src/types_internal.h:17-36`：`ehsm_ctx_intl_st`内嵌callback、packet、result指针、responded、cmd_buf和rsp_buf。
- `include/ehsmdrv/basic/api.h:14-17`：公开`ehsm_ctx_st`固定为`uint32_t _data[56]`，即224字节；同文件的`ehsm_session_st`为512字节，并明确要求在init/update/finish期间保持有效。
- `src/api.c:191-194`：每次调用只清理context中`result1`之后的可变部分，context本身由调用方长期持有。
- `src/api.c:18`：Vendor静态断言公开`ehsm_ctx_st`足以容纳内部context。

结论：独立分配的对象是完整224字节context，而不只是16字节packet；流式算法还可能需要独立512字节session。arena/pool应按context、session、对齐、cache line、channel和并发数预算，而不是预留固定64 KiB。

## FACT-03：通用context生命周期可能跨函数返回

- `src/api.c:52-53`：interrupt callback从packet地址减去成员offset反推出完整context地址。
- `src/api.c:147-189`：Host支持interrupt、wait-and-poll、send-and-peek；async interrupt和send-and-peek不保证发送函数返回时eHSM已经停止访问context。
- `src/api.c:341-386`及同类init/update/finish API：流式命令跨多次调用复用同一context并保存responded/result状态。
- timeout路径只返回错误；Vendor机制没有证明timeout时eHSM一定未执行或不再访问packet/cmd/rsp。

结论：把通用context声明为普通函数局部栈对象会产生use-after-return、late-response覆盖和callback反推失效风险。只有严格同步且可证明返回前eHSM停止访问的个别叶子调用才可以个案使用栈。

## FACT-04：cache属性可能要求独立section，但不等于独立物理Region

- `src/mailbox.c:108-135`：Vendor发送前调用cache flush/invalidate和critical section。
- `include/ehsmdrv/basic/port/ehsm_host_port.h`要求平台实现地址转换、cache、barrier和critical section。

结论：若NGU800P共享内存非一致，context arena需要精确clean/invalidate及barrier；若必须non-cacheable，可用stage内独立linker section或页属性实现，不必为Firewall单独切出64 KiB Region，也不应混入普通cached stack。

## 负责人裁决与评估结论

- ADR-0008：eHSM可以访问整个2 MiB安全RAM，独立Mailbox Region不再承担eHSM Firewall隔离作用。
- 已批准：BootROM/FMC在各自data/BSS或尾部启动复用区设置固定arena；GSP在data/BSS或固定pool设置arena。
- 已禁止：把通用interrupt/async/send-and-peek/流式及timeout未闭环context放在普通函数或RTOS任务栈上。严格同步叶子调用只有在证明返回前eHSM已停止访问时才允许个案例外。
- ADR-0011及2026-07-28后续冻结：首版每stage一个64字节对齐、256字节静态context slot和最多一条在途事务；GSP唯一service串行化；只使用one-shot typed API，不分配session；自动retry为0，timeout后service/slot quarantine。最终PMA属性、service channel、实际I/O descriptor/buffer及arena总容量仍待集成。

## 设计回填

- 更新`docs/03-architecture/security-ram-layout.md`。
- 更新`docs/04-interfaces/ehsm-mailbox.md`。
- 新增ADR-0008及OPEN-DESIGN-005。
- 物理放置、首版context slot/并发/retry/late-response规则已关闭；最终PMA和OPEN-CONFLICT-006剩余项关闭前，不修改目标代码仓或linker。
