# CE-SEC-006：eHSM Mailbox Vendor基线与NGU800P使用规则

## Evidence metadata

- Evidence ID：CE-SEC-006
- 日期：2026-07-23
- 父任务：INV-SEC-006
- 资料基线：SRC-0005、SRC-0012、SRC-0014、SRC-0018；NGU800P公司代码只读工作区
- 适用范围：OSR eHSM 4019及NGU800P D0 Mailbox集成
- 操作：只读复核Vendor PDF、Host/BL源码和NGU800P中断头文件；未修改代码仓，未构建/测试，未执行Git命令

## FACT-01：当前NGU800P基线为16个Mailbox channel

- `DOCUMENTED`：SRC-0012 PDF第39页§9.1说明eHSM最多支持16个Mailbox channel，每个方向各有两个info寄存器和note机制。
- `DOCUMENTED`：SRC-0005 PDF第115～116页§12.2～§12.3说明Mailbox最多16个channel，并使用`N=0～F`和`0x1000` channel stride描述寄存器。
- `VENDOR_IMPLEMENTATION`：SRC-0018 `ehsm_host-2.3.1-4019-2ee044d/port/osr/m130/port/ehsm_host_custom.h:6`定义`EHSM_PORT_MAILBOX_CHANNEL_COUNT 16u`。
- `CODE_FACT`：公司只读代码`components/chip_riscv_c908_common/include/ngu800p/ngu800p_ints.h:1496-1616`枚举`NGU800P_EHSM_O_MAILBOX_IRQ1`～`IRQ16`。

结论：16不再是Open question。具体channel分配、各stage并发数和arena slot数量仍属于软件详设参数。

## FACT-02：地址转换机制已存在于Host合同和eHSM BL代码

- `DOCUMENTED`：SRC-0014 PDF第29～30页规定Host逻辑地址应转换为eHSM可访问的物理/remote address；只有统一地址空间的平台才可采用identity mapping。
- `VENDOR_IMPLEMENTATION`：SRC-0018 Host `include/ehsmdrv/basic/port/ehsm_host_port.h:48-69`声明`ehsm_port_addr_to_raddr()`和`ehsm_port_raddr_to_addr()`；`src/api.c:149,266-267`用它转换packet/cmd/rsp地址。
- `VENDOR_IMPLEMENTATION`：OSR m130样例`port/osr/m130/port/ehsm_host_port.c:47-55`采用identity cast，这只是样例平台实现。
- `DOCUMENTED`：SRC-0005 PDF第89页§9.3.41～§9.3.42定义`SYS_SOC_MEM_BAL/BAH`及64位SoC总线基址。
- `VENDOR_IMPLEMENTATION`：eHSM BL `src/driver/mmap.c:78-104`的`mmap_remap_addr_u64/u32()`拆分64位remote address、设置`SYS_SOC_MEM_BA`并形成eHSM本地remap window；`mmap.c:106-145`和`src/component/schedule.c:78-104`用于remote data与Mailbox packet访问。

结论：地址转换是否存在不再开放。NGU800P需要实现的是C908 CPU pointer/local view到64位NoC/system remote address的具体Host port映射；eHSM内部继续使用Vendor remap机制。OPEN-CONFLICT-006中的C908 PC/linker视图不因此自动关闭。

## FACT-03：BootROM采用poll

- `VENDOR_IMPLEMENTATION`：Host Demo `demo/test.c:24`以`EHSM_DRV_MODE_WAIT_AND_POLL`初始化；`src/api.c:166-175`在该模式下循环调用`ehsm_mb_poll()`。
- `VENDOR_IMPLEMENTATION`：eHSM BL `src/component/schedule.c:146-176`轮询各Mailbox channel的note并处理命令。
- `CONFIRMED`：负责人明确BootROM使用poll；BL/BootROM业务路径没有提供需要移植的interrupt使用方式。

结论：BootROM poll不再开放。通用Host库支持interrupt不等于BootROM应使用interrupt；FMC/GSP可按其任务模型另行冻结。

## FACT-04：Ready/error位可由Vendor手册收敛

- `DOCUMENTED`：SRC-0005 PDF第34页状态表定义`hw_boot_done/error`、`bootloader_done/error`、`firmware_done/error`和`soc_verify_done/error`；第80页`SYS_HSM_STA0`表给出对应内部状态位。
- `DOCUMENTED`：SRC-0012 PDF第10～11页§2.1描述HW启动、BL启动/自检、状态置位和命令循环；其中明确`bootloader_done`只说明启动结束，仍需检查`bootloader_err`。
- `VENDOR_IMPLEMENTATION`：Host Demo `demo/common/utils.c:23-31`和`demo/test.c:51-52`的等待条件在任一`BOOT_DONE`/`HSM_READY`位出现时就退出，且没有deadline，弱于产品门禁要求。

Host可见低8位依次为：bit0 `hw_boot_done`、bit1 `hw_boot_err`、bit2 `bootloader_done`、bit3 `bootloader_err`、bit4 `firmware_done`、bit5 `firmware_err`、bit6 `soc_verify_done`、bit7 `soc_verify_err`；分别对应eHSM内部`SYS_HSM_STA0[16:23]`。

目标规则：每次保存raw status；任一`hw_boot_err`或`bootloader_err`立即fail-close并上报；只有done mask `0x05`全部置位且error mask `0x0A`为0时，BootROM才进入Mailbox命令阶段。Vendor未规定额外稳定窗口，当前不自行发明；有界deadline由项目策略冻结。

## FACT-05：MMIO属性与eHSM访问SoC RAM属性必须分开

- `DOCUMENTED`：SRC-0014及Host port头文件要求Mailbox MMIO aperture为non-cacheable，并要求平台针对共享内存实现相应cache flush/invalidate和barrier。
- `VENDOR_IMPLEMENTATION`：OSR FPGA样例`ehsm_host_port.c:57-60`的cache hook为空，只能证明该样例平台无需额外动作，不能定义NGU800P。
- `CONFIRMED`：负责人确认NGU800P的eHSM访问SoC RAM时不携带/不强制可由软件依赖的`non-cacheable`属性。

结论：Mailbox MMIO仍按Vendor合同使用non-cacheable映射；共享RAM的C908 cache属性、维护顺序和一致性由NGU800P port负责，不能假定eHSM master自动使其non-cacheable或自动coherent。

## FACT-06：剩余项是项目集成决策，不是Vendor事实问答

仍需B0-R2冻结：

1. NGU800P准确MMIO基址、寄存器绑定和C908视图。
2. 共享RAM cache属性、cache line、clean/invalidate/barrier与结构体对齐。
3. FMC/GSP driver mode。（后续由ADR-0010关闭：BootROM/FMC/GSP首版全程poll；interrupt仅为Vendor FW ready后的后续可选优化。）
4. command-specific deadline、busy/retry、幂等和late-response回收。
5. BootROM/FMC RAS report通道、RAS未ready时的持久记录和等待终态。（后续由ADR-0010部分关闭：早期失败顺序和fail-stop终态已冻结；实际RAS通道、deadline和slot offset仍是实现输入。）
6. 每stage channel分配、并发context和arena slot参数。

## 设计回填

- 更新`docs/04-interfaces/ehsm-mailbox.md`。
- 更新`docs/05-software-design/bootrom.md`。
- 部分收敛`requirements/open-questions.yaml`中的`OPEN-DESIGN-001`。
- 在`AGENTS.md`和`openspec/config.yaml`增加Vendor既有事实默认基线规则，避免重复向负责人询问当前资料已经回答的内容。
