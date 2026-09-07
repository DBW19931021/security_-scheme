# OSR eHSM 代码阅读与 Review 执行计划

## 目标

本文把项目组介绍和代码 review 拆成可执行计划。目标不是一次性读完所有源码，而是按交付件边界、运行链路和安全风险优先级推进，最终形成可对外汇报、可追踪、可继续迭代的 review 结论。

本计划默认使用简体中文沉淀文档；代码符号、路径、命令、API、寄存器名、宏名和协议字段名保留英文原文。

## 需要回答的问题

1. vendor 交付了什么：源码、预编译工具、测试资源、PDF、外部 API 的边界是什么。
2. eHSM 的启动、升级、验签、加密、OTP/key 管理链路怎么跑。
3. 哪些代码可以直接复用，哪些需要平台适配、二次确认或 vendor 澄清。
4. BL 作为安全启动信任根，有哪些安全不变量、攻击面、验证缺口和风险结论。

## 执行阶段

| 阶段 | 预计投入 | 主要输入 | 主要动作 | 产出 |
|---|---:|---|---|---|
| 第 0 阶段：交付件盘点 | 0.5 天 | 顶层目录、README、CMake、PDF、工具目录 | 区分源码/库/工具/测试资源；确认版本号、hash 清单和构建入口 | `00_vendor_delivery_inventory.md` |
| 第 1 阶段：总体架构 | 1 天 | BL/FW/Host/Tools/Tests | 梳理 Host、BL、FW、OTP/KMU、Crypto、Tools 的关系；画总体架构和数据流 | 架构图、组件职责表 |
| 第 2 阶段：BL 深读 | 3 到 5 天 | `ehsm_bl-*` 源码和 Bootloader TRM | 按启动、命令、镜像验证、升级、OTP/KMU、debug、平台驱动逐模块 review | `03_bootloader_deep_review.md` 逐步填充 |
| 第 3 阶段：FW/Host/Tools 对照 | 1 到 2 天 | `ehsm_fw-*`、`ehsm_host-*`、`tools/` | 对照 FW service、Host API、vendor 工具与 BL 消费路径 | 命令矩阵、工具产物对照表 |
| 第 4 阶段：测试和验证 | 1 到 2 天 | Python tests、demo、预置镜像、构建脚本 | 编译验证、hash 校验、测试覆盖矩阵、样例镜像解析 | 测试覆盖矩阵、风险验证结果 |
| 第 5 阶段：汇报材料收敛 | 0.5 到 1 天 | 前四阶段文档 | 整理项目组介绍、风险清单、vendor 问题、后续接入建议 | 汇报提纲和待确认问题表 |

## 详细阅读路线

### 0. 交付件边界

先读：

- `SW_changelist.md`
- `docs/OSR_eHSM_Bootloader_TRM_4019_1.1.pdf`
- `docs/OSR_eHSM_Firmware_TRM_4019_1.1.pdf`
- `docs/OSR_eHSM_Host_API_TRM_4019_v1.0.pdf`
- `tools/ehsm_image_tool-1.4.3/doc/ehsm_image_tool_spec.pdf`
- `tools/otptool-*/user_guide.pdf`
- `ehsm_bl-*/README.md`
- `ehsm_fw-*/README.md`
- `python_bootloader_tests-*/README.md`
- `python_firmware_tests-*/README.md`

要形成的结论：

- 交付物版本是否彼此匹配。
- vendor 提供了哪些可重编译源码，哪些只是预编译工具或资源。
- 项目组需要适配的接口在哪些文件中。

### 1. BL 启动主线

按顺序读：

1. `ehsm_bl-*/src/main.c`
2. `ehsm_bl-*/src/component/secure_boot.c`
3. `ehsm_bl-*/src/component/schedule.c`
4. `ehsm_bl-*/src/driver/mailbox.c`

要回答：

- reset 后 BL 的第一个可信状态是什么。
- 初始化失败是否进入安全失败状态。
- `BOOTLOADER_READY` 何时置位。
- `sch_start()` 进入命令循环前，OTP/KMU/selftest/mailbox 是否已经准备好。

### 2. BL 命令面

按顺序读：

1. `ehsm_bl-*/inc/mb.h`
2. `ehsm_bl-*/src/component/mbcmd_parser.c`
3. `ehsm_bl-*/src/component/dbgcmd_parser.c`
4. `ehsm_host-*/src/bl_mb.h`
5. `ehsm_host-*/src/api.c`

要回答：

- BL 暴露了哪些 `MB_CMD_ID_*`。
- 每个命令的输入结构、输出结构和返回路径是什么。
- 哪些命令应限制 lifecycle，哪些命令可能改写 OTP/REG/Flash/Debug 状态。
- Host API 和 BL `inc/mb.h` 是否存在 header 漂移。

### 3. Secure Boot Image 验证

按顺序读：

1. `ehsm_bl-*/src/component/fw_verify.h`
2. `ehsm_bl-*/src/component/fw_verify.c`
3. `tools/ehsm_image_tool-1.4.3/gen_images_demo.sh`
4. `tools/ehsm_image_tool-1.4.3/doc/ehsm_image_tool_spec.pdf`

要回答：

- 1024B image header 中每个字段的 offset、大小和安全含义。
- `naked`、`plain`、encrypted image 的限制是否和 lifecycle 匹配。
- RSA/ECC/SM2/CMAC 的签名范围是否覆盖应覆盖的数据。
- 解密使用的 AES/SM4 CBC、IV、padding、key id 是否和工具产物一致。
- version counter 是否能防 rollback。

### 4. Secure Upgrade Image 处理

按顺序读：

1. `ehsm_bl-*/src/component/fw_upgrade.h`
2. `ehsm_bl-*/src/component/fw_upgrade.c`
3. Python bootloader tests 中 upgrade image 资源和用例

要回答：

- 外层 upgrade image 和内层 boot image 的关系是什么。
- 外层验签/CMAC 和解密使用哪些 key id。
- 内层 boot image 是否重新生成 IV、重加密 code、重算签名/CMAC。
- 分块升级、掉电、复位、重复写入时是否有一致性策略。

### 5. OTP/KMU/key 管理

按顺序读：

1. `ehsm_bl-*/src/component/otp_key.c`
2. `ehsm_bl-*/src/component/otp_data.c`
3. `ehsm_bl-*/src/driver/otp.c`
4. `ehsm_bl-*/src/driver/kmu.c`
5. `osr_ehsm_external_driver_api_v1.1/include/otp_driver.h`

要回答：

- OTP logic key id 到 physical slot 的映射方式。
- key usage、key level、lifecycle 的检查点在哪里。
- OTP 默认值、写入、校验、重启生效的假设是什么。
- 外部平台 OTP driver 必须满足哪些原子性、写保护和读写语义。

### 6. Debug/Auth 和安全辅助机制

按顺序读：

1. `ehsm_bl-*/src/component/dbgauth.c`
2. `ehsm_bl-*/src/component/debug.c`
3. `ehsm_bl-*/src/component/selftest.c`
4. `fid_lib/`
5. `ehsm_bl-*/src/component/crypto_lib_api.*`

要回答：

- debug challenge 是否具备 freshness，是否有 replay 风险。
- debug open/close 是否受 lifecycle 和认证算法约束。
- FID/CFI 检查覆盖了哪些关键路径，哪些路径只靠普通返回码。
- selftest 失败是否能阻止 boot。

### 7. 平台/地址/外设边界

按顺序读：

1. `ehsm_bl-*/src/driver/mmap.c`
2. `ehsm_bl-*/src/driver/sysreg.c`
3. `ehsm_bl-*/src/driver/flash.c`
4. `ehsm_bl-*/src/driver/watchdog.c`
5. `docs/CPU/` 下 M130 资料

要回答：

- Host remote address 到 eHSM/SOC 地址的转换是否有范围检查。
- DMA 方向、读写权限和对齐要求是否清晰。
- sysreg 生命周期、安全状态、错误位是否可被不可信输入影响。
- 外部 flash driver 适配时是否需要擦写粒度、写保护和 verify 约束。

## Review 框架

每个模块用同一张表记录：

| 项 | 说明 |
|---|---|
| 模块职责 | 该文件/函数在 BL/FW/Host/Tools 中的位置 |
| 输入来源 | mailbox、shared memory、UART、OTP、sysreg、flash、tool image、测试资源 |
| 信任边界 | 哪些输入不可信，哪些数据来自信任根 |
| 安全不变量 | 必须始终成立的条件 |
| 关键检查 | 参数、长度、地址、对齐、lifecycle、version、algorithm、key usage |
| 失败路径 | 错误码、是否停止、是否上报、是否释放资源 |
| 攻击面 | 越界、rollback、naked 绕过、算法混用、key id 混用、fault injection、replay |
| 验证方式 | 静态阅读、CodeGraph、构建、测试镜像、Host demo、Python tests |
| 结论 | OK / 待验证 / 待 vendor 确认 / 风险 |

## 项目组介绍安排

建议 60 到 90 分钟：

| 时长 | 主题 | 材料 |
|---:|---|---|
| 5 分钟 | 交付件地图 | `00_vendor_delivery_inventory.md` |
| 5 分钟 | 构建和工具链 | BL/FW/Host CMake、Wing 工具链说明 |
| 10 分钟 | eHSM 总体架构 | 架构图、Host/BL/FW/OTP/KMU/Tools 关系 |
| 20 分钟 | BL 安全启动链路 | `03_bootloader_deep_review.md` 的启动和 image verify 图 |
| 10 分钟 | 镜像工具和 BL 验证关系 | `ehsm_image_tool` 参数、`fw_verify.c` 对照 |
| 10 分钟 | Host demo 如何触发 BL/FW | `demo/test.c`、`bl_demo.c`、Host API |
| 20 分钟 | Review 发现、风险和待确认问题 | 风险表、vendor 问题表 |
| 10 分钟 | 后续接入计划 | 外部 OTP/Flash driver、测试矩阵、owner |

## 文档设计

当前推荐文档结构：

```text
osr_eshm/docs/review/
  00_vendor_delivery_inventory.md
  01_review_execution_plan.md
  03_bootloader_deep_review.md

osr_eshm/openspec/specs/vendor-ehsm-review/
  spec.md
```

后续可继续补充：

```text
osr_eshm/docs/review/
  02_build_and_toolchain_review.md
  04_secure_boot_image_and_tools_review.md
  05_firmware_runtime_review.md
  06_host_api_and_demo_review.md
  07_test_coverage_matrix.md
  08_risk_and_vendor_questions.md
  diagrams/
    osr_ehsm_architecture.mmd
    bl_boot_flow.mmd
    image_verify_flow.mmd
    upgrade_flow.mmd
    mailbox_command_flow.mmd
```

## OpenSpec 沉淀要求

后续新增 review 结论时，应同步遵守 `openspec/specs/vendor-ehsm-review/spec.md`：

- 文档必须中文。
- BL review 必须覆盖启动、命令面、image verify、image upgrade、OTP/KMU、debug auth、平台驱动。
- 风险和结论必须关联代码路径、证据来源和验证方式。
- CPU/M130 相关内容必须优先引用 `docs/CPU/` 资料。
- 无法确认的行为必须进入 vendor 待确认问题表。

## 里程碑

| 里程碑 | 完成标准 |
|---|---|
| M1 交付件介绍可用 | `00_vendor_delivery_inventory.md` 和本文可支持 30 分钟交付件介绍 |
| M2 BL 主链路 review 可用 | `03_bootloader_deep_review.md` 完成 `main.c`、`secure_boot.c`、`schedule.c` 的逐函数结论 |
| M3 命令面矩阵可用 | `mbcmd_parser.c` 所有 command ID 有输入/输出/权限/风险记录 |
| M4 镜像安全链路可用 | `fw_verify.c` 和 `fw_upgrade.c` 的 header、签名、解密、version counter、key usage 已对照工具验证 |
| M5 风险清单可评审 | 所有高风险项有 owner、证据、验证结果或 vendor 待确认状态 |

## 当前下一步

- [ ] 从 `src/main.c` 和 `secure_boot.c` 开始填充 BL 第一轮 review。
- [ ] 建立 `mbcmd_parser.c` command handler 矩阵。
- [ ] 选取一个 `ehsm_image_tool` 生成的 boot image 做字段级解析。
- [ ] 对 `otp_driver.h`、`flash_driver.h` 和项目平台驱动接口做适配差异表。
