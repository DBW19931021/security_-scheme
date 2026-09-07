# CE-SEC-001：BootROM 与 eHSM 启动代码证据

## 调查身份

- 对应任务：`INV-SEC-001-bootrom-ehsm-startup`。
- 公司仓库：`REPO-GSP-FIRMWARE`，`../gsp-pmp-rmp-omp`。
- 清单参考点：`master` / `08b29c7b7a29ee9478c0c01d4d708beb0b77b5d9`。
- Vendor 参考：SRC-0018 中 eHSM Host `2.3.1-4019-2ee044d`、Bootloader `2.3.5-4019-72f8fdc`。
- 调查日期：2026-07-21。
- 调查范围：BootROM 构建和入口、`components/security` 验证路径、Board/启动代码、Vendor Host/BL 的 ready/self-test/Mailbox/timeout/reset 参考实现。
- 执行约束：只读；未执行 Git、构建或测试；未修改 `gsp-pmp-rmp-omp`、`baremetal` 或 Vendor 源码。
- 基线限制：清单记录公司仓库盘点时已有未提交修改。本报告证明的是调查时工作区文件内容，无法在禁止 Git 操作的约束下区分其与清单 commit 的差异。

## 结论摘要

| 编号 | 分类 | 结论 | 设计影响 |
|---|---|---|---|
| FACT-01 | `CODE_FACT` | BootROM 构建会编译安全组件和 demo 文件，但默认构建没有定义 `SECURE_DEMO`。 | 默认入口不会执行当前安全 demo。 |
| FACT-02 | `CODE_FACT` | 默认执行链为 `Reset_Handler -> pre_main -> main -> board_init/打印 -> return -> __exit`；未发现 eHSM 启动或 FMC handoff。 | 当前工作区不能作为生产安全启动链。 |
| FACT-03 | `CODE_FACT` | 手工启用的 demo 在 RAM 中构造包，并调用 `ehsm_verify_decrypt_stub()`；stub 仅解析、复制并写入模拟成功位。 | demo/stub 必须与生产 adapter 明确隔离并被替换。 |
| FACT-04 | `CODE_FACT` | 公司 BootROM 路径中未定位到 eHSM ready、自检、LCS/eFuse、Mailbox、timeout、reset 或 Vendor Host API 调用。 | DD-02 需要定义并实现完整适配契约。 |
| FACT-05 | `CODE_FACT` | demo 的 FMC load/entry 为硬编码元数据，验证输出落入 `demo_out`，没有按地址加载、范围校验或跳转。 | FMC 来源、加载、entry 和 handoff 都仍需冻结。 |
| FACT-06 | `VENDOR_IMPLEMENTATION` | Vendor Host 有 port/Mailbox/context/self-test/status/LCS/reset 接口和 busy/timeout 返回路径。 | 可以作为能力和 API 设计输入，不能直接代表 NGU800P SoC 集成。 |
| FACT-07 | `VENDOR_IMPLEMENTATION` / `GAP` | OSR m130 port 的 timer 创建恒为 0、timeout 判断恒为 false；demo ready 循环无超时，且以“BOOT_DONE 或 READY 任一置位”退出。 | 该 port/demo 不能直接复用为生产 timeout/ready 语义。 |
| FACT-08 | `CONFLICTING`（调查时）/ `CONFIRMED`（后续裁决） | Vendor BL bit18=`TRNG`，Host bit18=`SHA256`、bit19=`TRNG`；2026-07-22 Vendor回复确认Host错误。 | 当前按Bootloader定义解释，不再阻断；原始差异仍作为调查证据保留。 |

## 已确认代码事实

### FACT-01：构建入口包含安全骨架，但默认未打开安全 demo

- `solutions/bootrom/Makefile:13,39-49,67-70,97-100,146-151`：目标为 `bootrom`，依赖 `security` 组件，使用 `gcc_flash_irom.ld`；默认 CFLAGS 未定义 `SECURE_DEMO`。
- `solutions/bootrom/sub.mk:22-25`：`main.c` 和 `bootrom_secure_demo.c` 都被编译。
- `components/security/sub.mk:30-40`：BootROM 会把 `ehsm_stub.c`、`verify_flow.c`、manifest/policy/measurement 等加入安全组件，没有生产 eHSM adapter 源文件。
- `solutions/bootrom/app/include/bootrom_secure_demo.h:8`：唯一的 `SECURE_DEMO` 定义被注释；在目标仓库业务代码/构建配置中未找到其他定义。
- `solutions/bootrom/do_build.sh:115-122` 和 `boards/board_riscv_bootrom/sub.mk:25-34`：入口支持 qemu/emu/fpga/evb/prod board 宏，但没有按平台替换 eHSM adapter。

判断：demo“被编译”不等于 demo“被执行”，更不等于生产安全启动已接入。

### FACT-02：默认运行链结束于启动文件死循环

- `components/chip_riscv_bootrom/gcc_flash_irom.ld:19-34`：BootROM IROM 为 `0x100008000000`/`0x40000`，SRAM 为 `0x100008080000`/`0x40000`，入口为 `Reset_Handler`。
- `components/chip_riscv_c908_common/src/arch/c908vk-cp-xt_v2/startup.S:93-104,217-223`：入口进入 `Reset_Handler`，调用 `pre_main`；返回后落入 `__exit` 无限循环。
- `components/chip_riscv_c908_common/src/sys/pre_main.c:85-95`：弱定义 `pre_main()` 调用 `main()`。
- `solutions/bootrom/app/src/main.c:43-63`：默认只执行 `board_init()` 和打印；安全 demo 位于 `#ifdef SECURE_DEMO`，最终返回 0。
- `boards/board_riscv_bootrom/src/board_init.c:23-30`：`board_init()` 只做可选 UART 初始化。该文件 `:111-154` 的 `riscv_soc_start_cpu()` 仅在 QEMU/SMP 条件下释放次级 CPU，不能作为 eHSM/FMC release 证据。

默认调用链：

```text
Reset_Handler
  -> pre_main
    -> main
      -> board_init
      -> printf
      -> return 0
  -> __exit (无限循环)
```

### FACT-03：可选 demo 是内存构造包和软件 stub

- `solutions/bootrom/app/src/bootrom_secure_demo.c:10-19,35-62`：定义硬编码 FMC 元数据并在静态 RAM 数组中构造 synthetic package/manifest。
- 同文件 `:64-80`：重置 measurement，调用 `verify_image()`，打印带 `stub verify` 的结果并返回。
- `components/security/src/verify_flow.c:19-28,36-51,53-84`：直接调用 `ehsm_verify_decrypt_stub()`；成功后解析 manifest、检查 policy 并记录 measurement。
- `components/security/src/ehsm_stub.c:4-15,24-35,37-68`：使用可控 fail flag，实际成功动作是 `memcpy()`；随后把签名验证、解密和回滚位标为成功，并设置 `MEASUREMENT_SIMULATED_EHSM`。
- `components/security/include/security/ehsm_adapter.h:10-36`：公开接口仅有 stub 配置、stub result 和 stub 函数。

可选调用链：

```text
main [仅 SECURE_DEMO]
  -> bootrom_secure_demo_run
    -> build_demo_fmc_package
    -> verify_image
      -> ehsm_verify_decrypt_stub
        -> ehsm_parse_header
        -> memcpy(output)
      -> manifest_parse_after_ehsm_pass
      -> policy_check
      -> measurement_record
  -> 忽略返回值
  -> main return
  -> __exit
```

### FACT-04：公司路径未接入真实 eHSM 启动和传输

- 对 `solutions/bootrom`、`components/security`、BootROM board/chip glue 的生产源码和构建文件搜索，未定位到 `ehsm_driver_init_library`、`ehsm_ctx_init`、`ehsm_bl_self_test`、`ehsm_bl_get_self_test_result`、`ehsm_get_emu_status`、`ehsm_change_lifecycle` 或 `ehsm_port_reset_ehsm` 的调用。
- `components/security/include/security/security_types.h:15-25` 的状态枚举只有 header/length/address/verify/decrypt/rollback/policy/not-supported，尚无 ready、timeout、busy、reset、self-test、LCS 或 transport 错误。
- 与 ready/self-test/LCS 相关的匹配主要来自历史 docs、test/SPDM stub 或 manifest 字段，不能证明 BootROM 生产路径存在真实读取。

### FACT-05：当前没有 FMC 加载和 handoff

- `bootrom_secure_demo.c:13-19,54-61` 把 `0x1000080C0000` 同时写成 demo 的 load/entry，并填写版本、slot、rollback domain 等测试常量。
- `bootrom_secure_demo.c:66-74` 把验证输出放到静态 `demo_out[128]`。
- `ehsm_stub.c:59-68` 只检查输出数组容量并复制到该数组。
- 当前 BootROM `main.c` 没有消费 `manifest.load_addr`/`entry_addr`，没有 Flash/package 来源、目标地址许可表、entry 对齐校验、cache/fence、handoff 结构、FMC release 或 jump。

因此，硬编码地址只能视为 demo 元数据，不能视为已批准内存布局或已实现 handoff。

### FACT-06：Vendor Host 提供可参考的协议构件

- `source-vault/vendor-code/osr_eshm/ehsm_host-2.3.1-4019-2ee044d/src/api.c:228-268`：`ehsm_driver_init_library()` 初始化 port 和 Mailbox，`ehsm_ctx_init()` 设置 channel 以及 command/response buffer 地址。
- 同文件 `:147-187,270-292`：支持 interrupt、wait-and-poll、send-and-peek 三种模式；同步 poll 路径定义 timeout，异步路径返回 `EHSM_ERR_NEED_POLL`。
- `.../src/mailbox.c:61-105,108-153`：初始化 channel；发送前等待 note bit 清除，定义 timeout/busy；写 packet 地址前执行 cache、critical-section 操作；poll 读取 response note。
- `.../include/ehsmdrv/basic/port/ehsm_host_port.h` 定义 mailbox/shared-memory/OTP 平台参数、cache/barrier、interrupt、reset、status/error register 和 timer port hooks。
- `.../include/ehsmdrv/basic/bl_api.h:75-95` 公开启动自检和读取结果 API；`api.c:3534-3565` 形成对应 command。

这些是 eHSM/Core 交付快照的能力证据。NGU800P 的寄存器地址、channel、共享内存、cache 属性、timer、reset 和状态语义仍必须由 SoC/软件方案定义。

### FACT-07：Vendor OSR 示例不是可直接复用的生产实现

- `.../port/osr/m130/port/ehsm_host_port.c:109-114`：reset 使用 OSR FPGA magic 和固定系统寄存器。
- 同文件 `:116-177`：status/error 地址由 OSR `SYS_REG_BASE_ADDR` 派生。
- 同文件 `:205-222`：`ehsm_port_create_timer()` 恒返回 0，`ehsm_port_is_timeout()` 恒返回 false；port init 仅初始化 UART/CPU/mtime。
- `.../port/osr/m130/main.c:32-40`：demo reset 后用无 timeout 的忙等循环，条件是 `(BOOT_DONE | HSM_READY)` 任一 bit 置位即退出，而不是明确要求两者都满足。
- `.../port/osr/m130/inc/host_m130_cfg.h:14-22`：上述 status base/bit 定义为 OSR 平台值。

因此，即使 Host 通用层有 timeout 返回，当前 OSR port 也不能提供真实超时；ready 退出条件和地址同样不能直接移植到 NGU800P。

### FACT-08：自检位图在调查时存在已登记冲突，现已裁决

- Vendor BL `src/component/selftest.h:47-51`：SHA2 为 bit16，SHA3 为 bit17，TRNG 为 bit18。
- Vendor Host `include/ehsmdrv/basic/bl_api.h:32-35`：SHA2 为 bit16、SHA3 为 bit17、SHA256 为 bit18、TRNG 为 bit19。
- 该冲突已登记为 `CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP` / OPEN-CONFLICT-001。

后续裁决见 ADR-0003：当前4019交付采用Bootloader定义，Host定义错误；bit18/`0x40000`=`TRNG`，bit19/`0x80000`在Bootloader表中没有已确认含义，保持unknown/reserved。BootROM仍必须保存自检原始bitmap和命令返回码，不修改Vendor快照；原始Vendor回复材料待补录Source。

## 构建和平台影响

- 构建目标：`solutions/bootrom/Makefile` 的 `bootrom`。
- CPU/RTOS/toolchain：`do_build.sh` 使用 `c908_v2`、bare、GCC。
- Board：qemu/emu/fpga/evb/prod 仅设置不同 board 宏；本次未找到相应的 eHSM adapter 选择。
- 编译宏：`SECURE_DEMO` 默认未定义；手工定义只会打开 stub demo，不会生成生产启动链。
- 链接布局：IROM 与安全 SRAM 由 `gcc_flash_irom.ld` 固定；demo 中 `0x1000080C0000` 没有被本次证据证明为正式 FMC 区间。
- 版本差异：公司仓库工作区有既有修改；Vendor Host/BL 交付版本间存在自检位图漂移。

## 异常和错误路径

- 公司 stub 可返回 header/length/address/verify/decrypt/policy 错误；`verify_image()` 在失败时提前返回。
- `bootrom_secure_demo_run()` 会打印并返回错误，但 `main()` 对其使用 `(void)`，没有错误状态机、日志分级、reset、recovery 或 handoff 门禁代码。
- 默认和 demo 路径都没有执行 FMC，因此“没有在失败后进入 FMC”只是当前骨架缺少 handoff 的结果，不能当作生产 fail-close 已实现。
- Vendor 通用 Mailbox 定义 timeout/busy/poll，但 OSR m130 port 的 timeout 永不触发。
- 未确认 watchdog、最大重试次数、复位冷却、状态寄存器快照顺序、错误清除、持久审计或敏感缓冲区清零。

## 与目标设计的差异

| ID | 目标设计需求 | 代码现状 | 分类 | 影响 | 后续动作 |
|---|---|---|---|---|---|
| GAP-001 | BootROM 建立生产安全启动入口 | 默认是 hello-world，demo 默认关闭 | `GAP` | eHSM/FMC 链未开始 | 在 DD-01 冻结 BootROM 状态机和入口职责 |
| GAP-002 | 等待并确认 eHSM ready/boot 状态 | 公司路径无相关读取 | `GAP` | 无法建立安全前置条件 | DD-02 定义状态源、组合条件、timeout 和采样 |
| GAP-003 | 执行并解释自检 | 公司路径无实现；调查时Vendor位图冲突已裁决 | `GAP` | 协议映射已冻结，但公司生产实现仍缺失 | 按Bootloader映射实现并保留raw；v0.3更新用例093 |
| GAP-004 | 读取 LCS/eFuse 并实施 policy | 只有 manifest/lifecycle 骨架 | `GAP` | 生命周期门禁不存在 | 单独调查 SoC eFuse/LCS 接口和 Owner |
| GAP-005 | 真实 Mailbox/共享内存/cache adapter | 只有 stub | `GAP` | 验签/解密/rollback 都是模拟 | DD-02 定义 port/transport/buffer 契约 |
| GAP-006 | 有界 timeout/retry/reset/recovery | 公司路径无实现；Vendor OSR timer 为空 | `GAP` | 可能无限等待或无诊断 | 冻结参数来源和失败终态；实现真实 timer |
| GAP-007 | 从受控介质加载并验证 FMC 后 handoff | demo 静态包/静态输出，无 jump | `GAP` | 无法启动 FMC | DD-01/03 定义 source、range、load、entry、handoff |
| GAP-008 | 跨层错误映射与审计 | 现有 enum 过窄 | `GAP` | 失败无法定位/追踪 | DD-08 定义 namespace、stage、severity、raw code |

## 合理推断

- `INFERENCE-001`：`components/security` 当前是可用于完善 ABI/parser/policy/measurement 的骨架，但不能在替换 stub、接入真实 eHSM 和完成异常路径前提升为生产安全启动实现。
- `INFERENCE-002`（后续负责人批准方向）：BootROM/FMC/GSP优先移植/复用Vendor Host通用业务代码；OSR m130 port中的地址、magic、ready条件和timer必须全部视为平台示例并由NGU800P port核实/替换。
- `INFERENCE-003`（后续负责人批准约束）：生产代码需要在BootROM和Vendor Host通用层之间增加NGU800P port/adapter，并在编译期禁止EMU/产品target链接stub、simulated success或test provider；具体文件名和API待DD-02评审。

## 无法确认或冲突

- `UNKNOWN-001`：NGU800P eHSM reset/status/error 寄存器地址、bit 语义、read-clear/write-clear 规则和 reset domain。
- `UNKNOWN-002`：ready 必须满足的 bit 组合、采样顺序、稳定窗口、最大等待时间、retry/reset 次数和 watchdog 关系。
- `UNKNOWN-003`：Mailbox channel、MMIO base、共享内存窗口、64 位地址可见性、cache 属性、barrier 和 DMA/对齐约束。
- `UNKNOWN-004`：BootROM 获取 LCS/eFuse 的真实接口、授权、错误和不可逆资源边界。
- `UNKNOWN-005`：FMC 包的 Flash/ROM 来源、最大长度、load/entry 许可区、handoff ABI、cache/fence 和 jump 模式。
- `UNKNOWN-006`：失败终态应为 halt、watchdog reset、有限 eHSM reset、recovery 还是其组合。
- `RESOLVED-001`：原`CONFLICTING-001`自检bit18/bit19语义已按ADR-0003关闭；采用Bootloader定义，Host错误。

## 设计与追溯回填

- 已回填：`docs/03-architecture/secure-boot.md`、`docs/04-interfaces/ehsm-mailbox.md`、`docs/05-software-design/bootrom.md`。
- Feature：SEC-FEAT-001/002/005/019继续保持C1；SEC-FEAT-002不再因自检位图标记CONFLICTING，以本报告替换此前“少量入口glue”的泛化描述。
- 测试：关联当前工作簿用例038、051、093、094、095；v0.2保持不变，v0.3将用例093按bit18=`TRNG`、bit19=unknown/reserved高亮修改。
- 冲突：OPEN-CONFLICT-001已按ADR-0003关闭；原始status/bitmap仍必须采集，原始Vendor回复材料待补录。
- 下一调查：执行 `INV-SEC-002` 确认 FMC/GSP 验证、加载、measurement 和跳转链；另在 DD-02 创建 NGU800P eHSM port 参数/寄存器来源调查。

## 评审结论

- 调查问题均已给出代码事实、冲突或 UNKNOWN。
- 本报告可以进入详设评审，但不能作为代码已实现、已构建、已测试或硬件已验证的证明。

## 后续裁决记录

- 2026-07-22：负责人依据Vendor回复确认自检位图以Bootloader为准、Host错误；更新FACT-08后续状态、GAP-003、追溯和用例093规划。历史调查事实不被删除。
- 2026-07-22：负责人批准优先复用OSR Host通用业务代码及EMU/产品无stub/simulated success约束；现有test/stub流程仍只作为代码事实，不作为目标设计或最终oracle。
