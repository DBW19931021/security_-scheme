# CE-SEC-014：BootROM/FMC/GSP编码前落实性代码证据

## Evidence metadata

- Evidence ID：CE-SEC-014
- 日期：2026-07-27
- 父任务：TASK-SEC-SOC-FW-001
- 目标代码：`../gsp-pmp-rmp-omp`
- 基线说明：仓库清单此前记录`master@08b29...`；当前live worktree已有未提交修改。本轮不执行Git，因此不做diff、作者或归属判断。
- 操作：只读检查入口、公共security组件、构建source graph和linker；未修改代码、Vendor、baremetal或工作簿，未执行Git、构建和测试。

## FACT-01：三个产品入口尚未实现目标安全链

- `solutions/bootrom/app/src/main.c:43`进入`main()`，当前打印Hello World；`SECURE_DEMO`才调用演示入口。
- `solutions/bootrom/app/src/bootrom_secure_demo.c:10-19`使用硬编码旧地址/counter/profile，`:35`构造synthetic package，`:72-74`直接reset Measurement并调用旧verify flow。
- `solutions/fmc/app/src/main.c:41-55`仅为Hello World/bare helloworld。
- `solutions/gsp/app/src/main.c:27-56`主要由QEMU/SPDM测试宏控制；非测试分支`:62-63`循环打印。
- `solutions/gsp/app/src/pre_main.c:37-42`只创建普通`application_task`，优先级为`configMAX_PRIORITIES/2`，不是批准的最高优先级、生命周期唯一`security_service_task`。

结论：当前入口只能作为`CODE_FACT`和迁移起点，不能视为产品链或测试oracle。

## FACT-02：当前公共ABI与批准设计不一致

- `components/security/include/security/manifest.h:10`声明旧`MANIFEST_HEADER_SIZE 72`，但字段offset继续扩展；`:44`仍使用32位`version_counter`。
- `components/security/include/security/measurement.h:10`固定16个slot；`:20`记录`ehsm_status`，`:22`使用32位`version_counter`，`:40`包含sequence，`:45`返回内部表指针。
- `components/security/src/measurement.c:4`在私有BSS维护固定数组；reset只memset，record按slot覆盖；没有共享Region、实际entry count、CRC-32C、32位commit、finalize或跨stage snapshot。
- `components/security/src/verify_flow.c:21`调用`ehsm_verify_decrypt_stub`；`:56-82`从旧Manifest拼装旧Measurement；`:84`在verify flow内直接record，没有独立loader/readback digest/commit/release门禁。
- `components/security/include/security/security_types.h`仍包含旧镜像枚举/OMP/别名，且`:56`定义`MEASUREMENT_SIMULATED_EHSM`。当前RMP/MMP值与已批准Manifest registry不一致。

结论：必须按ADR-0019和ADR-0022替换旧ABI与职责，不能在旧结构上同时保留第二套产品语义。

## FACT-03：产品source graph尚未建立no-stub边界

- `components/security/sub.mk:37`无条件加入`ehsm_stub.c`。
- 同文件`:50-51`加入attestation stub，`:47`及`:79-88`纳入null crypto/platform相关实现。
- BootROM/FMC/GSP Makefile均包含`security`组件，并以whole-archive方式链接公共库。
- `solutions/bootrom/app/sub.mk:25`无条件编译`bootrom_secure_demo.c`。

结论：即使某些符号最终可能被链接器消除，当前生产source graph本身也不满足“产品/EMU禁止stub/test资产”的批准规则；必须先拆分生产/host-unit目标并建立符号/map负向检查。

## FACT-04：linker仍是旧080x布局

- BootROM linker使用`0x1000_0800_0000`一带。
- FMC linker使用`0x1000_080C_0000`一带。
- GSP linker使用`0x1010_0808_0000`一带。

结论：这些值与新`0x1000_0500_0000`/`0x1010_0500_0000`、2 MiB安全RAM目标不一致，不能反推最终load/entry、PC视图、容量或release地址。

## FACT-05：真实Vendor Host/NGU800P port绑定未进入产品仓

在目标产品源码中未定位到批准路径需要的真实`ehsm_init`、`ehsm_verify_image`、`ehsm_read_counter`、Vendor poll mode、地址转换、direct aperture/status、`bootloader_done/error`或`firmware_done/error`绑定。

结论：真实eHSM Host/port尚未集成；不得用现有stub、OSR样例固定地址或Demo顶层流程替代。

## FACT-06：loader、Measurement提交和release闭环尚不存在

当前旧verify flow在stub成功后直接写私有Measurement表。未定位到：

- 实际目标范围/重叠/权限检查；
- decrypt后payload重新Hash和load后readback digest；
- cache/fence及Firewall/PMP权限转换/lock/readback；
- ADR-0022 CRC/commit/final State/snapshot；
- BootROM release FMC、FMC release GSP、GSP release Runtime的真实接口；
- 统一RAS上报及RAS未ready的产品终态。

结论：当前实现不能证明“验证成功才加载、Measurement commit成功才release、失败不旁路”。

## 证据结论

1. 设计已足够拆出纯逻辑、平台适配和Vendor绑定三类任务。
2. 公共Manifest/Measurement/verify组件应按批准ABI重构，旧草案不具备产品兼容约束。
3. 三个stage均未达到产品集成就绪；BootROM/FMC/GSP真实链分别受平台、Vendor counter API和Runtime profile阻断。
4. 本轮未发现需改变已批准方案原则的新冲突，只有已经登记的实现输入缺口。

完整分级和任务顺序见`docs/09-plans/SOC安全固件编码前落实性审查-v1.md`。
