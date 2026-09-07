# CE-SEC-004：安全RAM地址与eHSM Mailbox代码证据

## Evidence metadata

- Evidence ID：CE-SEC-004
- 日期：2026-07-22
- 父任务：INV-SEC-004
- 仓库：REPO-GSP-FIRMWARE、SRC-0018 Vendor代码快照
- 记录基线：`manifests/repositories.yaml`中的GSP commit `08b29c7b7a29ee9478c0c01d4d708beb0b77b5d9`；本轮未用Git重新核对
- 操作：只读文本调查；未修改代码，未构建/测试，未执行Git命令

## FACT-01：2 MiB RAM及地址视图已存在于当前地址头

- `components/chip_riscv_c908_common/include/ngu800p/config_bus_address_mapping.h:102-110`：local/config视图为`0x1000_0500_0000～0x1000_051F_FFFF`，大小`0x200000`。
- `components/chip_riscv_c908_common/include/ngu800p/subsys_address_mapping.h:175-184`：NoC/system视图为`0x1010_0500_0000～0x1010_051F_FFFF`，die0 remap为`0x1000_0500_0000`，die1 remap为`0x1000_4500_0000`。

结论：用户确认的local基址和2 MiB大小与当前地址头一致；Manifest/linker是否统一使用system视图仍需结合新目标裁决。

## FACT-02：当前BootROM/FMC/GSP不在该2 MiB中

- `components/chip_riscv_bootrom/gcc_flash_irom.ld:21-22`：BootROM代码在`0x1000_0800_0000` IROM，数据/栈在`0x1000_0808_0000`、大小256 KiB。
- `components/chip_riscv_fmc/gcc_flash_sram.ld:21`：FMC在`0x1000_080C_0000`、大小256 KiB。
- `components/chip_riscv_gsp/gcc_flash_sram.ld:21`：GSP在`0x1010_0808_0000`、大小256 KiB。

这与ADR-0004此前采用的GSP canonical地址一致，但与ADR-0006的新2 MiB统一RAM目标冲突。

## FACT-03：当前2 MiB已被其他固件链接占用

- `components/chip_riscv_omp/gcc_flash_sram.ld:21`：OMP使用`0x1010_0500_0000`起始512 KiB。
- `components/chip_riscv_rmp/gcc_flash_sram.ld:21`：RMP使用`0x1010_0508_0000`起始512 KiB。
- `components/chip_riscv_pmp/gcc_flash_sram.ld:21`：PMP使用`0x1010_0510_0000`起始512 KiB。
- 根`README.md:216-221`还把2 MiB描述为PMP/RMP/OMP/MMP四个512 KiB区；README顺序与三个现有linker不完全一致，且未找到独立MMP solution。

结论：新安全RAM目标不能与当前PMP/RMP/OMP布局同时成立，必须确认新目标是否替代当前实现、这些固件是否迁移，以及适用硬件/软件版本。

## FACT-04：Vendor Mailbox通过64位packet地址传递command/response

- SRC-0018 Host `include/ehsmdrv/basic/mailbox.h:7-23`：packed command为`cmd_id/cmd_id_inv + data[39]`，response为`ret_code + data[4]`，packet包含两个`raddr_t`命令/响应地址。
- `src/mailbox.c:6-39`：Mailbox base来自port宏，每channel寄存器跨度`0x1000`。
- `src/mailbox.c:108-135`：发送前等待note空闲并检查timeout，执行cache flush/invalidate和critical section，把64位packet地址拆成低/高32位后写入info寄存器并置note。
- `src/api.c:149-190`：支持interrupt、wait-and-poll、send-and-peek，timeout和response raw code向上返回。
- `include/ehsmdrv/basic/port/ehsm_host_port.h:15-35,42-86,165-220`：NGU800P port必须提供channel/base/shared-memory、地址转换、cache、interrupt、timer、barrier和critical section。

结论：可以按负责人决策follow Vendor机制，不另造Mailbox packet协议；NGU800P只需冻结port参数和内存权限。

## FACT-05：Vendor reset接口/示例不能进入production自动恢复

- Vendor port头声明`ehsm_port_reset_ehsm()`；Demo和lifecycle/OTP测试多处直接reset使状态生效。
- ADR-0006已规定产品安全软件只上报错误，reset由RAS策略决定。

结论：Vendor reset调用只保留为能力/测试事实。production adapter应替换为RAS错误/动作请求边界；baremetal测试需要通过批准fixture/RAS路径执行reset。

## GAP与冲突

| ID | 类型 | 内容 | 处理 |
|---|---|---|---|
| GAP-RAM-01 | TARGET_DESIGN vs CODE_FACT | BootROM/FMC/GSP目标迁入2 MiB，但当前linker仍在080x SRAM | OPEN-CONFLICT-006 |
| GAP-RAM-02 | ownership | 2 MiB当前被OMP/RMP/PMP占用，MMP记录也存在 | 确认迁移/删除/版本profile |
| GAP-RAM-03 | address domain | 新目标local为1000_0500，当前header给system视图1010_0500 | 建议Manifest/linker使用system canonical，port显式转换；待裁决 |
| GAP-MBOX-01 | platform input | NGU800P eHSM Mailbox具体base/channel/IRQ/cache属性尚未冻结 | B0-R2端口参数输入 |
| GAP-RAS-01 | early boot | BootROM/FMC失败时RAS上报通道和RAS未ready终态未定义 | 后续接口详设；不得自行reset |

## 设计回填

- 新增`docs/03-architecture/security-ram-layout.md`。
- 更新`docs/04-interfaces/ehsm-mailbox.md`和`docs/05-software-design/error-handling.md`。
- 新增ADR-0006及OPEN-CONFLICT-006；冲突关闭前不修改目标代码仓。
