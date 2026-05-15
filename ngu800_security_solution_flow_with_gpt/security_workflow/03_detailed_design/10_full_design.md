# NGU800 安全方案详细设计说明书（整合版 V2.4）

版本：V2.4
状态：pending review（CR-0001 applied；CR-0003 applied；CR-0004 applied；CR-0005 applied；CR-0006 applied；CR-0007 applied；局部 reviewed，未升级为完整 reviewed baseline）
生成日期：2026-05-08

> 本文件为 NGU800 安全方案的完整详设与代码落地主入口。CR-0005 后，章节级详设与 `04_impl_design` 实现级分片的有效内容必须在本文可见；`04_impl_design` 仅作为编辑分片 / extracted implementation shard，不再作为独立事实源。本文按 CR-0001 统一章节顺序、标题编号、SEC1 加密口径和板级安全并入状态，按 CR-0003 同步 runtime image policy、board binding、JTAG/DMA/OOB、attestation report、manufacturing/RMA 阶段性裁决，按 CR-0004/CR-0005 同步 eHSM native header、OTP/key/counter source-conformance 和单文档代码落地规则，按 CR-0006 补充固件包格式、平台侧制作流程和设备侧 verify/decrypt 流程，并按 CR-0007 增强“生效约束 ID”的跳转链接与摘要可读性。V2.4 仍保持 pending review，不升级为完整 reviewed baseline。

## 约束链接说明

各章节的“生效约束 ID”均采用“可点击约束 ID + 一句话摘要”的形式。点击 ID 可跳转到 `security_workflow/01_constraints.md` 中的完整约束定义；摘要只用于快速阅读，不替代约束正文、CR、decision log 或 eHSM TRM。

## 文档目录

- [1. 设计基线摘要](#1-设计基线摘要)
- [2. 安全总体架构](#2-安全总体架构)
- [3. 安全启动详细设计](#3-安全启动详细设计)
- [4. 设备身份与远程度量证明设计](#4-设备身份与远程度量证明设计)
- [5. 安全调试与生命周期控制](#5-安全调试与生命周期控制)
- [6. 内外部接口设计](#6-内外部接口设计)
- [7. 板级安全设计](#7-板级安全设计)
- [8. Root of Trust、密钥体系与证书体系](#8-root-of-trust密钥体系与证书体系)
- [9. 制造、灌装、部署与 RMA](#9-制造灌装部署与-rma)
- [10. 实现级落地详设全集](#10-实现级落地详设全集)
- [11. 风险、依赖、冻结项与开放问题](#11-风险依赖冻结项与开放问题)
- [12. 附录](#12-附录)

---

# 1. 设计基线摘要


版本：v2.2
状态：评审版（可用于架构评审 / 方案冻结前阶段）

---

## 1.1 设计目标

本 Baseline 定义 NGU800 安全架构的核心裁决，包括：

- Root of Trust 定义
- Secure Boot 架构
- Host 与安全边界
- 密钥体系与生命周期
- 制造与密钥灌入流程
- 双算法支持策略

---

## 1.2 Baseline Summary

| Topic | Current Decision | Status |
|---|---|---|
| Root of Trust | eHSM | CONFIRMED |
| First Mutable Stage | FMC（安全抽象名：SEC1） | CONFIRMED |
| First Cryptographic Verifier | eHSM | CONFIRMED |
| SEC1/FMC Protection Policy | FMC 是一级固件，在安全链路抽象中等价 SEC1；FMC 来源为 NOR / 本地 Flash，正式安全启动路径必须签名 + 加密，验证与解密服务由 eHSM / 安全子系统提供 | CONFIRMED |
| SEC2/GSP Protection Policy | GSP 是二级安全管理固件，在安全链路抽象中等价 SEC2；GSP 在正式安全启动路径中必须签名 + 加密，验证、解密、measurement 与 release 由 FMC/GSP 调 eHSM 路径完成 | CONFIRMED |
| Runtime Image Policy | PM / RAS / Codec 等关键 runtime image 在 USER/PROD 默认签名 + 加密；signature-only 仅允许作为产品白名单例外 | ASSUMED / TBD |
| eHSM Image Container | FMC（SEC1）/ GSP（SEC2）的密码学 verify/decrypt container 采用 eHSM native secure boot image header；NGU 项目 metadata 放入 Code region manifest / policy table | CONFIRMED / TBD |
| Firmware Package Build/Verify Contract | 平台侧固件制作工具与设备侧 verify/decrypt 路径共享同一 eHSM native header + NGU protected manifest 契约；旧自定义 header 仅作流程意图参考 | CONFIRMED / TBD |
| OTP / Key / Counter Mapping | physical OTP/control/key/counter 以 eHSM TRM 为准；NGU `OTP-0..OTP-7`、`*_MIN_VER` 和 key name 仅为 logical alias / rollback domain | CONFIRMED / TBD |
| Algorithm Authority | secure boot / upgrade 验签算法 authority 来自 eHSM OTP/control field；NGU manifest 只记录 expected profile / audit profile | CONFIRMED |
| BootROM Role | 负责最小加载与编排，不负责复杂密码学校验 | CONFIRMED |
| SEC Role | 启动控制面与 release owner | CONFIRMED |
| Host Trust Model | 不可信，只投递 GSP（SEC2）及后续镜像 / 受保护包，不下发 FMC（SEC1） | CONFIRMED |
| Board / OOB Trust Model | BMC / OOB / 管理子系统可承载管理流程，但不进入 Root of Trust | CONFIRMED |
| Board Binding Stage Policy | V2.4 阶段 board binding 默认进入 attestation，不默认阻断 FMC（SEC1）；是否参与 GSP（SEC2）/runtime release decision 后续冻结 | ASSUMED / TBD |
| Manufacturing Baseline | 必须定义 key 注入、锁定、审计、生命周期推进 | CONFIRMED |

---

## 1.3 架构核心裁决

### 1.3.0 工程固件名与安全抽象名映射

本方案中的 `SEC1 / SEC2` 是安全启动链路抽象名；工程仓库中的实际 firmware / solution 名称以 `FMC / GSP` 为准。后续代码、manifest、打包工具和测试应优先使用工程名，同时保留安全抽象名作为别名，便于和历史方案、eHSM profile、CR 记录对齐。

| 安全抽象名 | 工程固件名 | 典型产物 | 角色 |
|---|---|---|---|
| SEC1 | FMC | `fmc-bm.bin` / `solutions/fmc` | 一级固件 / First Mutable Stage，由 BootROM 定位、eHSM verify+decrypt 后装载 |
| SEC2 | GSP | `gsp-bm.bin` / `solutions/gsp` | 二级安全管理固件，承接 Host/OOB 请求、后续 firmware verify/release、measurement/attestation/debug/RMA 编排 |

约束：

- 文档讨论安全链路策略时可继续使用 `FMC(SEC1)`、`GSP(SEC2)` 的双名写法。
- 本文后续历史章节若单独出现 `SEC1` 或 `SEC2`，除非上下文另有说明，均按 `FMC(SEC1)` 和 `GSP(SEC2)` 理解。
- 代码实现中的 `ngu_image_type` 优先使用 `NGU_IMAGE_FMC`、`NGU_IMAGE_GSP`；`NGU_IMAGE_SEC1`、`NGU_IMAGE_SEC2` 仅作为兼容别名。
- eHSM native header 的 `Image_Type` 仍不承载 FMC/GSP/PM/RAS/OMP/RMP 等 NGU 项目级类型；项目级类型必须来自受保护 NGU manifest / policy table。
- 当前 stub-backed 安全框架落地阶段只接入 `bootrom / fmc / gsp`；PM / RAS / OMP / RMP 等后续固件仍属于方案设计和组件级 policy 预留，不修改对应 solution 的入口流程或构建依赖。

### 1.3.1 Root of Trust

- Root Key 存储于 eFuse / OTP 安全区
- Root of Trust 由 eHSM 提供
- BootROM 不持有私钥

结论：
**eHSM = 唯一 Root of Trust**

---

### 1.3.2 First Verifier

- 所有签名验证由 eHSM 执行
- BootROM 不执行复杂验签
- 软件不得实现正式安全路径验签

---

### 1.3.3 Boot 控制权

- SEC 核（C908）为唯一 Boot 控制器
- 所有 MCU release 必须由 SEC 控制
- Host 不参与控制流程

---

## 1.4 Adopted vs Rejected Decisions

| Topic | Adopted | Rejected | Reason |
|---|---|---|---|
| First verifier | eHSM | BootROM / Host | 与 eHSM 能力边界和当前安全基线一致 |
| Host role | 仅投递 firmware | Host 参与执行放行 | Host 不可信 |
| BootROM crypto | 不做复杂校验 | BootROM 内嵌完整 crypto 验签路径 | 缩小攻击面，复用 eHSM |
| Image container | eHSM native header 作为 verify/decrypt container，NGU metadata 进入 protected manifest | NGU 自定义 physical FW header 与 eHSM header 并列 | 避免两套 physical ABI 和工具链冲突 |
| Firmware package flow | image packager 生成 eHSM native package，设备侧 eHSM 先 verify/decrypt，BootROM/SEC 再解析 NGU manifest 和 release policy | 平台工具生成旧 `header + Signed Region + signature + wrapped_cek + enc_payload` 作为最终 wire/storage 格式 | 保留制作/验证流程的可读性，同时不违背 eHSM TRM |
| OTP/key/counter | eHSM physical field + NGU logical alias / customization TBD | NGU 自定义 physical OTP 分区、32-bit per-image physical counter、未映射 key slot | follow `SRC-002/SRC-006/SRC-007`，降低 RTL/制造偏差 |
| Key ownership | 私钥不出 eHSM | 私钥落在 Host / 管理核 | 不满足安全边界 |
| Workflow | constraints → baseline → detailed → impl | raw inputs 直接生成 full design | 防止方案漂移 |

---

## 1.5 Secure Boot 架构

### 1.5.1 Boot Chain

BootROM → FMC（SEC1，NOR / 本地）→ GSP（SEC2，Host 下发）→ 子系统 FW

### 1.5.2 关键约束

- 所有 FW 必须验签
- FMC（SEC1）必须签名 + 加密；FMC 验证、解密和 key unwrap 必须由 eHSM / 安全子系统受控密码服务完成
- BootROM 只负责定位 FMC（SEC1）、调用受控接口、根据结果装载或拒绝启动，不直接实现复杂解密
- Host 不下发 FMC（SEC1）；Host 只投递 GSP（SEC2）及后续镜像 / 受保护包
- GSP（SEC2）在正式安全启动路径中必须签名 + 加密；GSP 解密失败必须阻断安全控制面启动
- PM、RAS、Codec 等后续关键 runtime image 在 USER/PROD 产品形态中默认签名 + 加密；若采用 signature-only，必须由产品安全策略显式允许并在 lifecycle / attestation / debug 状态中可见
- 未验签禁止执行
- 支持 Anti-rollback；物理承载先对齐 eHSM Version Counter / monotonic counter / owner 确认的等价机制
- FW Encrypt Branch 至少对 FMC（SEC1）+ GSP（SEC2）强制启用；per-image CEK / wrapped CEK 只有在 eHSM owner 明确支持后才能升级为 CONFIRMED
- FMC/GSP 的 eHSM native `Image_Type` 不承载 NGU 项目级 FMC/GSP/runtime image type；NGU `ngu_image_type` 放入 protected manifest / policy table
- FMC（SEC1）/ GSP（SEC2）sign+encrypt 不得走 eHSM NVM only verify；必须使用 verify+decrypt output path 和受控 RAM / staging / output buffer
- 平台侧固件制作工具必须生成 eHSM native package；NGU manifest、payload、版本、rollback domain、measurement slot 和 expected algorithm profile 必须被 verify/decrypt 成功后的受保护 Code region 覆盖
- 设备侧必须按“eHSM native verify/decrypt 成功 -> BootROM/SEC 解析 NGU manifest -> policy/release decision”的顺序执行，不得在 eHSM 成功前信任 manifest 内项目级字段

---

## 1.6 Host 边界

### 1.6.1 允许行为
- 传输 GSP（SEC2）及后续固件 / 受保护镜像包
- 发起请求
- 接收状态 / 失败报告

### 1.6.2 禁止行为
- 下发或替换 FMC（SEC1）
- 参与信任链
- 控制启动
- 访问密钥
- 直接访问安全域
- 直接 release 其他 MCU

---

## 1.7 Board / OOB / 管理子系统边界

### 1.7.1 输入采用策略

`SRC-005 管理子系统方案` 中的总体架构、模块职责、带外管理链路、电源/复位流程、单/双 Die 约束作为系统级流程输入采用。

涉及安全的部分采用以下裁决：

- BMC / OOB / 板级 MCU / 管理子系统不进入 Root of Trust。
- SMBus/I2C、I3C、PCIe VDM、SPI、UART、JTAG 只能作为受控管理或转发通道。
- JTAG、DMA、Flash 更新、电源复位、PowerBrake 等高权限能力必须经 lifecycle、debug auth、firewall、scope 和审计约束。
- 管理子系统文档中若出现未鉴权调试、直接访问安全子系统、直接访问 DRAM/Flash/寄存器空间或绕过 SEC/eHSM 的流程，不作为安全 baseline 采用。

### 1.7.2 Adopted vs Rejected

| Topic | Adopted | Rejected | Reason |
|---|---|---|---|
| 管理子系统总体架构 | 采用 `SRC-005 管理子系统方案` 的模块、链路和流程作为系统输入 | 忽略管理子系统集成 | 需要与板级、电源、复位、OOB 流程对齐 |
| OOB trust level | BMC/OOB/Sideband 不高于 Host | 将 BMC/OOB 默认视为可信根 | OOB 链路权限高且暴露面大，不能天然可信 |
| JTAG access | lifecycle + debug auth + scope + MUX 联合控制 | USER 态常开或板级 MUX 直通 | 防止绕过 secure boot、密钥和运行态隔离 |
| Management DMA | 仅访问普通白名单 buffer | 访问安全区、执行区、OTP/eHSM 私有区 | DMA 可绕过软件边界，必须硬隔离 |
| Power/reset control | 纳入安全状态机和审计 | 作为纯板级普通控制 | 复位/掉电会影响安全启动、恢复和 attestation 状态 |

---

## 1.8 eHSM 职责

- Root Key / Root Secret 使用
- 验签
- 加解密
- 密钥管理
- 生命周期控制
- 调试鉴权
- Counter / anti-rollback
- Attestation key 使用

---

## 1.9 Manufacturing / Provisioning Baseline

| Topic | Decision | Open Issue |
|---|---|---|
| Root Key 注入 | 通过制造安全通道灌入 OTP/eFuse | 工站对接细节待定 |
| Root Key 锁定 | MANU → USER 前必须锁定 | 读回校验策略待定 |
| 测试 Key 清理 | USER 前必须清理 | 测试证书链清理动作待定 |
| 审计日志 | 制造阶段必须记录 | 日志落点待定 |

---

## 1.10 双算法策略

- 方案必须同时支持国密和国际算法栈
- 实现层不得把算法写死到单一栈
- secure boot / upgrade 的算法 authority 来自 eHSM OTP/control field，例如 `SocBootAlg / SocUpgradeAlg`
- NGU manifest / report / mailbox 可记录 expected algorithm profile，用于一致性检查、审计和 attestation，不得覆盖 eHSM control field

---

## 1.11 Freeze Sensitive Items

| Item | Why Sensitive | Needed Before Freeze |
|---|---|---|
| eFuse bit 分配 | 直接影响 RTL / 制造灌装 | 需要字段级规划 |
| eHSM native header / NGU protected manifest ABI | 直接影响 BootROM / SEC / Host 对接 | eHSM native header follow TRM；NGU manifest ABI 需要结构级冻结 |
| Firmware package build/verify 契约 | 直接影响 image packager、BootROM、SEC verify flow 和 eHSM adapter 联调 | 需要冻结 manifest ABI、工具参数、golden vector、exact eHSM key ID / command mapping |
| Mailbox command model | 直接影响 FW / driver / eHSM API 对接 | 需要 req/resp 结构 |
| SPDM report fields | 直接影响 attestation 联调 | 需要字段定义 |
| board binding 策略 | 影响量产兼容性 | 需要策略裁决 |
| JTAG / OOB scope 控制 | 影响 USER 态调试暴露面 | 需要冻结 debug scope bitmap、MUX/CPLD 控制权和授权流程 |
| 管理子系统 DMA / mailbox / reset | 影响安全边界和状态一致性 | 需要冻结 firewall 白名单、可访问 buffer 和审计字段 |

---

## 1.12 Mermaid Architecture Diagram

```mermaid
graph TD
    BR[BootROM] --> SEC[SEC Core / C908]
    BR --> EH[eHSM]
    EH --> OTP[eFuse / OTP]
    SEC --> EH
    SEC --> FW[SEC1 / SEC2 / Other FW]
    Host --> SEC
    Host -. no trust .-> EH
```

---

## 1.13 Mermaid Sequence Diagram

```mermaid
sequenceDiagram
    participant BR as BootROM
    participant EH as eHSM
    participant SEC as SEC/C908
    participant H as Host

    BR->>SEC: Load early stage
    SEC->>EH: Verify SEC1 / later FW
    EH-->>SEC: PASS/FAIL
    H->>SEC: Deliver SEC2 / other FW
    SEC->>EH: Verify delivered FW
    EH-->>SEC: PASS/FAIL
    SEC->>FW: Release execution only if PASS
```

---

## 1.14 当前基线是否可进入详细设计

结论：
- 可以进入章节级详细设计
- 但不能跳过实现级设计直接进入代码开发

限制条件：
- eFuse / key / header / mailbox / SPDM 仍需在 `04_impl_design/` 中冻结
- manufacturing / provisioning 仍需细化到流程级
# 2. 安全总体架构

## 2.1 总体裁决

本章保留 Root of Trust 和系统信任边界的前置结论，详细密钥体系与证书体系后移到第 8 章统一展开。

- `[CONFIRMED]` eHSM 是唯一 Root of Trust 和首个密码学验证主体。
- `[CONFIRMED]` BootROM 是最早启动编排者，但不是密码学根，不持有 Root Private Key，不实现复杂验签或复杂解密。
- `[CONFIRMED]` SEC/C908 是安全启动、镜像接收、接口收敛、证明、debug/RMA、制造流程的统一控制面。
- `[CONFIRMED]` Host 不可信，只投递 GSP（SEC2）及后续镜像 / 受保护包，不下发 FMC（SEC1），不进入信任链，不拥有 release 权。
- `[CONFIRMED]` BMC / OOB / Sideband / 管理子系统可承载管理流程，但信任级别不高于 Host，不进入 Root of Trust。
- `[CONFIRMED]` FMC（SEC1）来源为 NOR Flash / 本地 Flash，在正式安全启动路径中必须签名 + 加密。

## 2.2 总体架构图

```mermaid
graph TD
    BR[BootROM] -->|locate FMC(SEC1) in NOR / local Flash| FLASH[NOR Flash]
    BR -->|VERIFY_FMC/VERIFY_SEC1: verify + decrypt mandatory| EH[eHSM]
    EH --> OTP[OTP / eFuse / Root / Counter / Lifecycle]
    BR -->|load controlled result| FMC[FMC / SEC1]
    FMC -->|Host channel + VERIFY_IMAGE| GSP[GSP / SEC2]
    Host[Host] -->|deliver GSP(SEC2) / runtime protected packages| FMC
    GSP -->|verify / measure / release| FW[PM / RAS / OMP / RMP / Other FW]
    GSP -->|Mailbox security services| EH
    BMC[BMC / OOB / Management] -->|controlled request / board flow| GSP
    BMC -. no RoT .-> EH
    Host -. no trust .-> EH
```

### 图下说明

1. FMC 是 First Mutable Stage，在安全链路抽象中等价 SEC1，来自 NOR Flash / 本地 Flash，不由 Host 下发。
2. BootROM 只做最小初始化、定位 FMC、调用 eHSM 受控接口、根据结果装载或拒绝启动。
3. eHSM 完成 FMC（SEC1）的验签、解密 / unwrap、rollback、吊销和 measurement 相关安全服务。
4. GSP 是后续运行期安全控制面，在安全链路抽象中等价 SEC2，负责 Host/OOB 请求收敛、后续固件验证、证明、debug/RMA 和制造流程编排。
5. Host、BMC、OOB、管理子系统都不是信任根，不能直接访问 eHSM、OTP/eFuse、Secure SRAM 或 release 目标核。

## 2.3 关键安全路径

| 安全路径 | 控制面 | 执行面 | 外部角色 | 当前裁决 |
|---|---|---|---|---|
| FMC（SEC1）启动 | BootROM 最小编排 | eHSM | Host 不参与 | FMC 必须签名 + 加密 |
| GSP（SEC2）/ Runtime FW | FMC / GSP | eHSM | Host 仅投递 | GSP 强制签名 + 加密；PM/RAS/Codec USER/PROD 默认签名 + 加密；signature-only 需产品白名单 |
| Attestation | GSP（SEC2） | eHSM | Verifier / Host 只验证 report | report 必须覆盖 measurement 和安全状态 |
| Debug / RMA | GSP（SEC2） | eHSM | BMC/OOB/工具只发起受控请求 | USER/PROD 默认关闭，需 challenge-response、scope、expire、audit |
| Board / OOB | GSP（SEC2） | eHSM / firewall / audit | BMC/OOB/管理子系统 | 总体流程可遵循，安全边界由安全方案裁决 |
| Manufacturing | GSP（SEC2） | eHSM / OTP/eFuse | 工站/Host/BMC 是链路 | MANU -> USER 必须灌装、锁定、清理、审计 |
---
# 3. 安全启动详细设计


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/01_boot.md`
> 当前状态：V1.0（基于当前约束、baseline 与输入资料收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 3.1 本章目标

本章定义 NGU800 在**安全启动模式**下的完整启动链设计，明确：

1. SoC BootROM、SEC1、SEC2、eHSM、Host 的职责边界
2. 安全启动与非安全启动的选择条件
3. SEC1 / SEC2 / 后续微核固件的验证与执行放行规则
4. 固件包格式、平台侧制作流程、设备侧验签解密流程
5. 反回滚、吊销、SEC1 强制解密、后续镜像按策略解密、失败处理与恢复入口
6. 与实现层文件的映射关系：
   - `04_impl_design/efuse_key_fw_header_design.md`
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/manufacturing_provisioning.md`

---

## 3.2 生效约束 ID

- [C-ROOT-01](../01_constraints.md#c-root-01) - Root of Trust 必须由 eHSM 承载
- [C-BOOT-01](../01_constraints.md#c-boot-01) - 所有可执行镜像执行前必须验签
- [C-BOOT-02](../01_constraints.md#c-boot-02) - Boot / release 顺序必须由安全核控制
- [C-BOOT-03](../01_constraints.md#c-boot-03) - BootROM 只做最小编排，不实现复杂加解密
- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
- [C-BOOT-06](../01_constraints.md#c-boot-06) - eHSM native header 与 NGU protected manifest 分层
- [C-BOOT-07](../01_constraints.md#c-boot-07) - SEC1 / SEC2 加密镜像必须走 verify+decrypt output path
- [C-BOOT-08](../01_constraints.md#c-boot-08) - 固件制作与设备侧 verify/decrypt 共享 eHSM-native 契约
- [C-EHSM-01](../01_constraints.md#c-ehsm-01) - OTP / key / counter 必须按 eHSM source-conformance 对齐
- [C-IF-01](../01_constraints.md#c-if-01) - 所有正式密码操作必须走 eHSM
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离
- [C-ACCESS-02](../01_constraints.md#c-access-02) - 访问控制必须使用 UserID + Firewall
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚
- [C-UPDATE-02](../01_constraints.md#c-update-02) - 必须支持受控升级 / 恢复
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程

---

## 3.3 生效 Baseline 决策

### 3.3.1 Root 与验证主体
- `[CONFIRMED]` Root of Trust = eHSM
- `[CONFIRMED]` First Cryptographic Verifier = eHSM
- `[CONFIRMED]` BootROM 不承担复杂密码学校验和密钥管理

### 3.3.2 启动控制权
- `[CONFIRMED]` SEC/C908 是唯一 boot control plane
- `[CONFIRMED]` Host 只具备镜像投递能力，不具备执行放行权
- `[CONFIRMED]` 所有微核 release 必须由 SEC 控制

### 3.3.3 镜像来源
- `[CONFIRMED]` SEC1 从 NOR Flash / Flash 获取
- `[CONFIRMED]` SEC2 及后续 PM / RAS / Codec 等固件由 Host 通过 PCIe 下发
- `[CONFIRMED]` 非安全启动路径应保留，但量产态是否开启必须受 lifecycle + OTP 策略控制

---

## 3.4 术语与阶段定义

| 术语 | 含义 |
|---|---|
| BootROM | SoC 最早执行的不可变启动代码，负责最小初始化与启动编排 |
| eHSM | 安全服务根，负责验证、密钥、OTP、lifecycle、debug auth、counter 等 |
| SEC1 | 安全最小 bring-up 固件，负责基础初始化与 Host 通道建立 |
| SEC2 | 完整安全控制面固件，负责后续固件接收、验证、升级、认证与调试控制 |
| Staging Buffer | Host 投递镜像的受控缓冲区 |
| Release | 允许某个目标核/固件开始执行的最终放行动作 |

### 3.4.1 启动阶段划分

| 阶段 | 名称 | 主执行体 | 主要动作 |
|---|---|---|---|
| A | SoC BootROM 早期启动 | BootROM | 最小平台初始化、读取 strap / lifecycle / secure boot 配置 |
| B | eHSM 自启动 | eHSM | ROM / Bootloader / 自检 / 生命周期恢复 / 密钥材料恢复 |
| C | SEC1 验证 | BootROM + eHSM | 定位 SEC1、请求验证、做版本/吊销/签名检查 |
| D | SEC1 装载与启动 | BootROM | 装载 SEC1 并跳转执行 |
| E | SEC1 基础初始化 | SEC1 | PCIe 初始化、Host 通道建立、共享缓冲区准备 |
| F | SEC2 与后续镜像处理 | SEC1/SEC2 + eHSM | 验证 SEC2、后续固件验证、测量、release |

---

## 3.5 设计要求

### 3.5.1 必须满足的安全目标

- `[CONFIRMED]` SEC1 必须在执行前经 eHSM 验证
- `[CONFIRMED]` SEC1 在正式安全启动路径中必须采用签名 + 加密保护，执行前必须由 eHSM / 安全子系统受控密码服务完成解密 / unwrap
- `[CONFIRMED]` 后续固件必须经 SEC1/SEC2 调用 eHSM 验证
- `[CONFIRMED]` Host 下发固件在执行前必须受控
- `[CONFIRMED]` 支持版本检查、防回滚、吊销
- `[CONFIRMED]` 支持设备认证与度量导出
- `[CONFIRMED]` 支持安全升级与安全调试
- `[CONFIRMED]` SEC1 在正式安全启动路径中必须签名 + 加密。
- `[CONFIRMED]` SEC2 在正式安全启动路径中必须签名 + 加密；SEC2 verify path 必须包含 signature verify、rollback check、revoke check、decrypt / unwrap、measurement 和 controlled release。
- `[ASSUMED]` PM / RAS / Codec 等关键 runtime image 在 USER/PROD 产品形态中默认签名 + 加密；若采用 signature-only，必须由产品安全策略显式允许并进入 image_type 白名单。

### 3.5.2 不得违反的边界

- BootROM 不得直接承担 SEC1 的复杂密码学校验或复杂解密
- Host 不得直接 release SEC2 或后续微核
- 普通非安全 Master 不得直接访问 eHSM、OTP、Secure SRAM
- 未验签通过的镜像不得进入执行态
- rollback floor 不得只依赖镜像内软件字段

---

## 3.6 架构图

```mermaid
graph TD
    PR[Power / Reset] --> BR[SoC BootROM]
    PR --> EH[eHSM ROM/BL/FW]

    BR --> CFG[secure_boot_enable / lifecycle / strap / control field]
    EH --> OTP[OTP / eFuse / Key / Lifecycle]

    BR -->|locate SEC1| FLASH[NOR Flash]
    BR -->|VERIFY_SEC1 via mailbox| EH
    EH -->|verify + decrypt PASS/FAIL| BR

    BR -->|load+jump| SEC1[SEC1]
    SEC1 -->|PCIe init / host channel| HOST[Host]
    HOST -->|deliver SEC2 + PM/RAS/Codec| STAGE[Staging Buffer]
    SEC1 -->|VERIFY_IMAGE| EH
    EH -->|PASS/FAIL| SEC2[SEC2]
    SEC2 -->|verify + measure + release| OTHERS[PM/RAS/Codec Cores]
```

### 图下说明

1. BootROM 是启动编排者，不是首个密码学验证者。
2. eHSM 在 SEC1 验证、SEC1 强制解密、后续镜像验证、反回滚、吊销检查中提供统一安全服务。
3. Host 只把 SEC2 及后续镜像投递到受控缓冲区，不拥有执行放行权。
4. SEC2 是后续运行期安全控制面，负责后续微核固件验证编排、度量汇总和放行。

---

## 3.7 时序图

```mermaid
sequenceDiagram
    participant BR as BootROM
    participant EH as eHSM
    participant FL as Flash
    participant S1 as SEC1
    participant H as Host
    participant S2 as SEC2
    participant MC as PM/RAS/Codec

    BR->>BR: 最小平台初始化
    BR->>BR: 读取 secure_boot_enable / lifecycle / strap
    BR->>EH: 拉起 eHSM / 等待 ready
    EH->>EH: ROM/BL/FW 自检、OTP装载、生命周期恢复
    BR->>FL: 定位 SEC1 镜像
    FL-->>BR: 返回 SEC1 镜像地址/内容
    BR->>EH: VERIFY_SEC1(addr,len,type,policy)
    EH->>EH: header / key_id / revoke / version / hash / signature / mandatory decrypt
    EH-->>BR: VERIFY_PASS / VERIFY_FAIL
    alt SEC1 验证通过
        BR->>S1: 装载并跳转
        S1->>S1: 基础初始化
        S1->>S1: PCIe 初始化 / Host 通道建立
        H->>S1: 下发 SEC2
        S1->>EH: VERIFY_IMAGE(SEC2)
        EH-->>S1: PASS / FAIL
        alt SEC2 验证通过
            S1->>S2: 装载并跳转
            S2->>H: 请求后续镜像
            H->>S2: 下发 PM/RAS/Codec 固件
            S2->>EH: VERIFY_IMAGE(PM/RAS/Codec)
            EH-->>S2: PASS / FAIL
            S2->>MC: 对通过校验的微核 release 执行
        else SEC2 验证失败
            S1->>S1: 记录错误并进入失败/恢复路径
        end
    else SEC1 验证失败
        BR->>BR: 记录错误并进入失败/恢复路径
    end
```

### 图下说明

1. SEC1 的首次密码学校验和强制解密发生在 BootROM 调 eHSM 的路径上。
2. 后续镜像验证责任转移到 SEC1/SEC2 调 eHSM 的路径。
3. release 是一个独立动作，必须发生在 verify pass 之后。
4. 任何验证失败都不能默默降级为“继续启动”，必须进入明确失败或恢复路径。

---

## 3.8 启动模式矩阵

| 模式 | secure_boot_enable | lifecycle | eHSM 参与 | 镜像要求 | 适用场景 |
|---|---|---|---|---|---|
| 安全启动 | 1 | MANU / USER / DEBUG-RMA | 必须 | 关键镜像必须验证；支持版本/吊销/反回滚 | 正式量产 / 制造验证 / 受控返修 |
| 非安全启动 | 0 或策略允许 | TEST / DEVE 为主 | 可不参与首阶段镜像验证 | 可允许受控绕过 | 实验室 bring-up / 特定开发调试 |
| Rescue / Recovery | 策略控制 | DEBUG/RMA 为主 | 必须 | 必须用受控 recovery trust / 特定 signer | 故障恢复 / 返修 |

### 3.8.1 模式选择规则

- `[CONFIRMED]` BootROM 启动后首先读取 `secure_boot_enable / lifecycle / strap / control field`
- `[CONFIRMED]` 非安全路径应保留，但不应默认允许量产态启用
- `[ASSUMED]` USER 生命周期下，非安全启动应由 OTP/eFuse + 策略态关闭
- `[ASSUMED]` Recovery 模式只能通过受控 lifecycle 和授权流程进入

---

## 3.9 可信镜像分类

| 镜像类型 | 来源 | 谁发起验证 | 谁执行验证 | 谁决定执行放行 | 反回滚检查 |
|---|---|---|---|---|---|
| SEC1 | NOR Flash | BootROM | eHSM | BootROM 跳转至 SEC1 | 跳转前检查；签名 + 加密强制 |
| SEC2 | Host/PCIe | SEC1 | eHSM | SEC1 / SEC2 受控跳转 | 执行前检查；签名 + 加密强制 |
| PM | Host/PCIe | SEC2 | eHSM | SEC2 release | 放行前检查；USER/PROD 默认签名 + 加密 |
| RAS | Host/PCIe | SEC2 | eHSM | SEC2 release | 放行前检查；USER/PROD 默认签名 + 加密 |
| Codec | Host/PCIe | SEC2 | eHSM | SEC2 release | 放行前检查；USER/PROD 默认签名 + 加密 |
| Recovery | 特殊路径 | SEC2 / Provisioning | eHSM | SEC2 / 受控状态机 | `[TBD]` 独立 image_type / signer / counter / decrypt policy |

### 3.9.1 当前建议

- `[CONFIRMED]` SEC1 的首次验证由 eHSM 完成
- `[CONFIRMED]` SEC1 必须签名 + 加密，解密由 eHSM / 安全子系统受控密码服务完成，解密失败必须阻止启动
- `[CONFIRMED]` SEC2 及后续镜像验证由 SEC1/SEC2 调 eHSM 完成
- `[CONFIRMED]` SEC2 必须签名 + 加密，解密失败必须阻断安全控制面启动
- `[CONFIRMED]` Host 不拥有执行放行权
- `[ASSUMED]` PM / RAS / Codec 在 USER/PROD 默认签名 + 加密；signature-only 只能作为产品策略白名单例外
- `[ASSUMED]` Recovery 镜像应使用专用 recovery trust anchor，并仅在受控 lifecycle 下允许
- `[TBD]` Recovery image 的 image_type、signer、trust anchor、rollback counter、decrypt policy 需要在详细设计冻结前关闭

---

## 3.10 镜像格式与 eHSM native header 分层

本章不重复完整实现级字段，正式结构以：

- `04_impl_design/efuse_key_fw_header_design.md`
- `04_impl_design/ehsm_source_conformance_matrix.md`

为准。CR-0004 接受后，章节级口径如下：

### 3.10.1 eHSM native header

- `[CONFIRMED]` SEC1 / SEC2 的密码学 verify/decrypt container 采用 eHSM native secure boot image header。
- `[CONFIRMED]` eHSM header 为 1KB plaintext image head，Code 从 offset 1024 开始，可明文或密文。
- `[CONFIRMED]` eHSM header 中的 `Image_Type` 保持 eHSM TRM 定义，不承载 NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 项目级类型。
- `[CONFIRMED]` `SocBootAlg / SocUpgradeAlg` 或等价 eHSM control field 是 secure boot / upgrade 的算法 authority。

### 3.10.2 NGU protected manifest

- `[CONFIRMED]` NGU 项目级 metadata 放入 eHSM Code region 的 protected manifest / policy table。
- manifest 记录 NGU `ngu_image_type`、policy、measurement slot、rollback domain、lifecycle mask、board binding policy、expected algorithm profile 等项目语义。
- manifest 由 BootROM / SEC 在 eHSM verify/decrypt 成功后解析；是否由 eHSM firmware / bootloader 直接解析保持 `[TBD]`。
- manifest ABI bit-level layout、是否必须位于 Code region 起始位置、extension 格式保持 `[TBD]`。

### 3.10.3 已废弃的旧口径

- `ngu_fw_min_hdr_t / ngu_fw_signed_hdr_t` 不再作为 physical wire/storage verification header。
- `algo_family / hash_algo / sig_algo / enc_algo` 不再作为镜像头内的算法 authority；如需要，可作为 manifest expected profile / audit profile。
- `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 不再作为 physical OTP 32-bit rollback counter，只作为 NGU logical rollback domain。
- `key_slot / wrapped_cek_*` 不得被写成 eHSM 已确认字段；per-image CEK / wrapped CEK 保持 `[TBD]`，除非 eHSM owner 后续确认。

### 3.10.4 当前项目建议

- `[CONFIRMED]` Host 下发镜像必须先进入 staging buffer
- `[CONFIRMED]` Verify path 必须能处理：
  - eHSM native header 解析
  - eHSM `Image_Type / Plain_Flag / Version_Counter / Code_Size` 检查
  - eHSM key ID / signer hash 检查
  - revoke bitmap 检查
  - eHSM Version Counter / NGU rollback domain 检查
  - hash / signature 校验
  - 对 SEC1 / SEC2 执行强制 verify+decrypt output path
  - eHSM verify/decrypt 成功后再解析 NGU manifest 并执行项目级 policy
- `[CONFIRMED]` SEC1 在正式安全启动路径中必须签名 + 加密
- `[CONFIRMED]` SEC2 在正式安全启动路径中必须“签名 + 加密”
- `[ASSUMED]` PM / RAS / Codec 等主要运行期固件在 USER/PROD 产品形态中默认“签名 + 加密”；若采用 signature-only，必须由产品安全策略显式允许并在 lifecycle / attestation / debug 状态中可见
- `[TBD]` signature-only 白名单的准入条件至少绑定 image_type、lifecycle、product SKU、debug state、release policy、rollback policy、是否包含敏感逻辑/数据

### 3.10.5 固件包物理布局与 SEC1 最终制品组成

CR-0006 后，本文把 `SRC-001 当前安全方案基线` 第 7 章中的“固件制作和设备侧解包流程”改写为 eHSM-native 口径。旧的 `header + Signed Region + signature + wrapped_cek + enc_payload` 只能作为流程意图参考，不再作为最终 wire/storage physical format。

SEC1 最终发布制品必须表达为一个 eHSM native secure image package。该 package 的物理外观由 eHSM TRM 决定，NGU 不再定义第二套 physical verification header；NGU 只在 eHSM protected Code region 内定义项目级 manifest 和 payload 语义。

为避免把签名“藏”在 header 这个词里，本文把 SEC1 package 按工程视角拆成三个逻辑部分：

1. `Part A: eHSM native header/control metadata`，明文，供 eHSM 解密前解析。
2. `Part S: eHSM signature/authentication material`，签名、公钥/证书引用、signer/key reference、认证算法相关材料；物理上可以是 1KB header 内的字段，也可以是 eHSM TRM 定义的 header-referenced authentication area。
3. `Part B: eHSM protected Code region`，SEC1 正式路径下存储态为密文，解密验签通过后明文为 `NGU manifest + SEC1 payload + padding/alignment`。

```mermaid
flowchart TB
    subgraph FILE[SEC1 secure image file / eHSM native package]
        H[Part A<br/>eHSM native header/control metadata<br/>plaintext, exact fields follow eHSM TRM]
        S[Part S<br/>signature/authentication material<br/>signature / signer / cert or key reference / auth metadata]
        C[Part B<br/>eHSM protected Code region<br/>ciphertext on storage, length = Code_Size]
    end

    subgraph PLAIN[Code region plaintext after eHSM PASS]
        M[NGU protected manifest<br/>SEC1 image type / policy / version / load / entry / measurement]
        P[SEC1 payload<br/>code / rodata / data init image]
        PAD[padding / alignment<br/>if required by eHSM or toolchain]
    end

    H --> S
    H --> C
    S -. authenticates at least full Code region .-> C
    C --> M
    M --> P
    P --> PAD
```

图下说明：

1. eHSM native header 是唯一 physical verification/decrypt container。
2. eHSM native header 明文存在，便于 eHSM 在解密前解析 `Valid_Flag / Image_Type / Plain_Flag / Naked_Flag / Code_Size / Version_Counter / key reference / algorithm profile / IV or nonce` 等字段；具体字段 offset 和长度以 eHSM TRM 为准。
3. 签名在最终固件中必须可见为 eHSM native authentication material。它不是 NGU 自定义 `signature` 尾随字段，也不放进解密后的 NGU manifest 里；它属于 eHSM native package 的认证材料，物理位置以 eHSM TRM 为准。
4. 若 eHSM TRM 将 signature/signature metadata 放在 1KB plaintext header 中，则 `Part S` 是 `Part A` 的一个字段区；若 TRM 使用 header 指针或扩展认证区，则 `Part S` 是 header 引用的认证块。无论哪种物理形式，方案和代码都必须把它作为 eHSM 原生认证材料处理。
5. eHSM protected Code region 是 SEC1 的强制保护对象。正式 SEC1 安全启动路径中，该区域必须被 eHSM 验签/认证保护，并且必须按 eHSM sign+encrypt profile 加密保护。
6. Code region 解密验签通过后的明文逻辑布局为 `NGU protected manifest + SEC1 payload + padding/alignment`。
7. `ngu_image_type`、rollback domain、measurement slot、lifecycle mask、expected algorithm profile、load address、entry address 等 NGU 项目级 release 字段不写入 eHSM native `Image_Type`，必须进入已受保护的 manifest。
8. 若后续需要 per-image CEK / wrapped CEK，只能作为 eHSM owner 确认后的 customization extension，不得由工具链自行定义为已冻结字段。

SEC1 制品按字节视角可表达为：

```text
SEC1 secure image file
┌────────────────────────────────────────────────────────────────────┐
│ Part A: eHSM native header / control metadata                      │
│         plaintext, 1KB, exact layout follows eHSM TRM               │
│                                                                    │
│         Typical semantics, not NGU-defined offsets:                 │
│         - Valid_Flag                                                │
│         - eHSM Image_Type                                           │
│         - Plain_Flag / Naked_Flag                                   │
│         - Code_Size                                                 │
│         - Version_Counter / anti-rollback related field             │
│         - key id / signer id / cert ref / key purpose               │
│         - algorithm profile                                         │
│         - IV / nonce / encryption metadata                          │
├────────────────────────────────────────────────────────────────────┤
│ Part S: eHSM signature / authentication material                    │
│         exact storage location follows eHSM TRM                     │
│         may be a field area inside the 1KB header, or a TRM-defined │
│         header-referenced authentication area                       │
│                                                                    │
│         Typical semantics, not NGU-defined offsets:                 │
│         - signature / authentication metadata                       │
│         - signer public key hash / signer id / cert chain reference │
│         - authenticated algorithm profile / key reference           │
│                                                                    │
│         Must authenticate at least full Code region                 │
├────────────────────────────────────────────────────────────────────┤
│ Part B: eHSM protected Code region                                  │
│         length = Code_Size                                          │
│         storage state: encrypted in formal SEC1 secure boot path     │
│         authentication state: covered by Part S                     │
│                                                                    │
│         Plaintext after eHSM PASS:                                  │
│         ┌──────────────────────────────────────────────────────┐   │
│         │ B1: NGU protected manifest                            │   │
│         │     project-level image description and release policy │   │
│         │     - magic / manifest_version                         │   │
│         │     - ngu_image_type = SEC1                            │   │
│         │     - security_flags = sign_required + decrypt_required│   │
│         │     - payload_offset / payload_size                    │   │
│         │     - load_addr / entry_addr                           │   │
│         │     - rollback_domain / version_counter                │   │
│         │     - measurement_slot                                 │   │
│         │     - lifecycle_mask                                   │   │
│         │     - expected_algorithm_profile                       │   │
│         │     - extension offset / extension size, if needed     │   │
│         ├──────────────────────────────────────────────────────┤   │
│         │ B2: SEC1 payload                                       │   │
│         │     actual SEC1 firmware code and data image            │   │
│         ├──────────────────────────────────────────────────────┤   │
│         │ B3: padding / alignment, if required                   │   │
│         └──────────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────────────────┘
```

签名、认证和加密覆盖范围必须按以下规则落地：

| 对象 | 是否必须被 eHSM 认证覆盖 | 是否必须被 eHSM 加密覆盖 | 工程原因 |
|---|---|---|---|
| eHSM native header | 按 eHSM TRM profile；若 TRM 指定部分字段进入认证/AAD，则必须 follow | No，header 需要在解密前可解析 | eHSM 需要先读取 header 才能选择 key/profile/check policy |
| NGU protected manifest | Yes，必须被完整认证保护 | Yes，SEC1 正式路径必须随 Code region 加密 | manifest 决定 `SEC1` 类型、policy、version、load/entry 和 release 条件 |
| SEC1 payload | Yes，必须被完整认证保护 | Yes，SEC1 正式路径必须加密 | payload 是最终执行对象，必须同时保护完整性和机密性 |
| padding/alignment | Yes，若计入 `Code_Size` 就必须被认证保护 | Yes，若计入加密 Code region 就必须被加密 | 防止解析边界、长度、填充区被利用 |

因此，NGU 对 SEC1 的最低认证覆盖要求是：

```text
AuthCoverage(SEC1) >= full Code region
                  = NGU protected manifest
                    + SEC1 payload
                    + padding/alignment counted by Code_Size
```

若 eHSM native header 中某些控制字段没有被 eHSM 认证/AAD 覆盖，BootROM / SEC 不得把这些字段作为 NGU 项目级安全决策依据；同等安全语义必须在已认证保护的 NGU manifest 中重复表达，并且只能在 eHSM PASS 后解析使用。

### 3.10.6 平台侧固件制作流程

平台侧固件制作工具的输入和输出必须围绕 eHSM native package 组织，而不是围绕旧 NGU custom header 组织。

```mermaid
flowchart TD
    ELF[SEC1 ELF / BIN<br/>build output] --> PAYLOAD[生成 SEC1 payload<br/>strip / objcopy / layout / alignment]
    META[release metadata<br/>image=SEC1 / version / policy / load / entry / measurement] --> MAN[生成 NGU protected manifest]
    PAYLOAD --> CODE[拼接 plaintext Code region<br/>manifest + SEC1 payload + padding]
    MAN --> CODE
    CODE --> PROF[选择 eHSM secure boot profile<br/>boot alg / key purpose / signer / version counter / encrypt policy]
    PROF --> ENC[eHSM packaging encrypt step<br/>encrypt protected Code region for SEC1]
    ENC --> AUTH[eHSM signing/auth step<br/>authenticate full Code region and bind TRM-defined metadata]
    AUTH --> HDR[生成 eHSM native header / metadata<br/>Valid_Flag / Flags / Code_Size / key/cert/signature metadata]
    HDR --> CHECK[发布前验收<br/>conformance / protected manifest / tamper vectors / rollback policy]
    CHECK --> OUT[SEC1 secure image package]
```

平台侧步骤要求：

1. 从 SEC1 构建输出生成 payload。payload 可以来自 ELF 转换、bin 拼接或 linker script 固定布局，但进入 package 前必须有确定的 `payload_size`、`load_addr`、`entry_addr` 和 alignment。
2. 确定 release metadata，包括 `ngu_image_type = SEC1`、版本、rollback domain、measurement slot、lifecycle mask、board binding policy、expected algorithm profile、sign/encrypt mandatory policy。
3. 生成 NGU protected manifest。manifest 的 bit-level ABI 仍为 `[TBD]`，但其语义必须与第 10 章实现级设计保持一致；SEC1 manifest 必须表达 `sign_required = 1` 和 `decrypt_required = 1`。
4. 将 `manifest + SEC1 payload + padding/alignment` 组成 plaintext Code region 输入；manifest 必须位于 Code region 起始处或 owner-confirmed parser 可确定的位置，并且必须被 `Code_Size` 覆盖。
5. 按 eHSM TRM / owner-confirmed profile 选择 `SocBootAlg / SocUpgradeAlg`、key purpose、signer、version counter、revoke policy、sign+encrypt profile。
6. 由 eHSM image packaging 工具或 owner-confirmed 等价流程对 Code region 执行 SEC1 加密保护，并生成 eHSM native header / metadata。NGU 工具不得生成第二套 physical verification header。
7. 由 eHSM image packaging / signing 工具按 TRM profile 对认证对象签名或生成认证材料。NGU 要求认证对象至少覆盖完整 Code region；若 eHSM profile 还要求 header field AAD，则必须 follow TRM。
8. 发布前必须检查：
   - eHSM header 字段符合 TRM；
   - eHSM `Image_Type` 未被误用为 NGU `SEC1/SEC2/runtime` 类型；
   - Code region 解密验签后的明文起始处或约定位置可解析出 NGU manifest；
   - manifest、payload、padding/alignment 均落入认证覆盖范围；
   - SEC1 package 的 manifest policy 明确要求 sign + encrypt；
   - manifest 中 expected algorithm profile 与 eHSM control field 不冲突；
   - version / rollback domain / measurement slot 与 release policy 一致；
   - sign+encrypt mandatory 镜像没有落入 NVM only verify profile。
9. 发布验收必须保留 golden vector 和 tamper vector。至少包括：修改 manifest `ngu_image_type`、修改 `entry_addr`、修改 `version_counter`、修改 SEC1 payload 字节、截断 Code region、篡改 `Code_Size` 后均应导致 eHSM verify 或 manifest policy 失败。

平台侧制作流程和设备侧处理流程的对应关系如下：

| 制作端动作 | 设备侧对应检查 / 动作 |
|---|---|
| 生成 manifest，写入 `ngu_image_type = SEC1` 和 sign+encrypt policy | eHSM PASS 后 BootROM 解析 manifest 并确认当前阶段只接受 SEC1 且必须 sign+encrypt |
| 拼接 `manifest + SEC1 payload + padding` 为 Code region | eHSM 按 `Code_Size` 对完整 Code region 执行认证和解密输出 |
| 选择 eHSM key/profile/version counter | eHSM 检查 key/signer/revoke/version counter/control field |
| 加密 Code region | eHSM 输出明文 Code region 到受控 buffer，解密失败则拒绝启动 |
| 对完整 Code region 生成认证材料 | eHSM 验签/认证失败则不输出可信 Code region |
| 发布验收生成 golden/tamper vector | BootROM/eHSM 适配层和 QEMU/stub 测试复用 vector 验证失败路径 |

### 3.10.7 设备侧 verify/decrypt 与 manifest policy 流程

设备侧必须先让 eHSM 完成 native package 的密码学验证与解密输出，再由 BootROM / SEC 解析 NGU manifest 并做项目级 policy/release 判断。

```mermaid
sequenceDiagram
    participant BR as BootROM/SEC
    participant EH as eHSM
    participant IMG as Firmware Package
    participant OUT as Controlled Output Buffer
    participant POL as NGU Manifest Policy

    BR->>IMG: 定位 eHSM native package
    BR->>BR: 检查镜像地址和 output buffer 白名单
    BR->>EH: bl_verify_image / soc_verify(package_addr, output_addr, profile)
    EH->>IMG: 解析 native header
    EH->>EH: 检查 Image_Type / Plain_Flag / Version_Counter / Code_Size
    EH->>EH: key / signer / revoke / rollback / signature or authentication check
    EH->>EH: decrypt protected Code region according to eHSM profile
    EH->>OUT: output plaintext Code region
    EH-->>BR: PASS / FAIL + status
    alt eHSM PASS
        BR->>OUT: 解析 NGU protected manifest
        BR->>POL: 检查 SEC1 type / sign+encrypt policy / lifecycle / rollback / load / entry
        POL-->>BR: policy pass / fail
        BR->>BR: measurement + load SEC1 payload + controlled jump
    else eHSM FAIL
        BR->>BR: 记录错误，拒绝 release
    end
```

设备侧步骤要求：

1. BootROM / SEC 只定位镜像、准备参数和受控 output buffer，不实现复杂签名、复杂解密或 key unwrap。
2. eHSM 对 native header 和 Code region 执行 TRM 定义的 verify/decrypt；SEC1 / SEC2 必须使用 verify+decrypt output path，不得使用 NVM only verify profile。
3. eHSM 返回 PASS 前，BootROM / SEC 不得信任 Code region 内的 NGU manifest，也不得使用 manifest 中的 load/entry/policy 字段。
4. eHSM PASS 后，受控 output buffer 中的 Code region 明文必须按 `manifest + payload + padding/alignment` 解析；BootROM / SEC 先解析 NGU manifest，再根据 `payload_offset / payload_size` 定位 SEC1 payload。
5. BootROM / SEC 解析 NGU manifest 后，至少检查：
   - `ngu_image_type` 是否符合当前启动阶段；
   - `security_policy_flags` 是否满足 SEC1/SEC2 mandatory sign+encrypt；
   - `payload_offset / payload_size` 是否完全落在 eHSM 输出 Code region 内；
   - `load_addr / entry_addr` 是否落入 SEC1 允许执行区；
   - `rollback_domain` 与 eHSM Version Counter / owner-confirmed rollback policy 是否一致；
   - `lifecycle_mask` 是否允许当前 lifecycle；
   - `measurement_slot` 是否有效；
   - `expected_algorithm_profile` 是否与 eHSM control field 一致；
   - board binding policy 是否只进入 attestation，或在后续 CR 冻结后参与 release decision。
6. 只有 eHSM PASS 且 manifest policy PASS 后，BootROM / SEC 才能记录 measurement，将 SEC1 payload 装载到受控执行区，并执行 controlled jump。
7. 若 eHSM FAIL、manifest policy FAIL、payload 边界错误、SEC1 entry 不在白名单执行区、或 sign+encrypt policy 不满足，BootROM / SEC 必须拒绝 release，并进入错误记录 / recovery policy。

---

## 3.11 校验规则

### 3.11.1 SEC1 校验规则

BootROM 向 eHSM Bootloader `bl_verify_image` 或等价 SEC1 early boot profile 发起 `VERIFY_SEC1` 请求时，至少执行：

1. eHSM native header 解析
2. eHSM `Image_Type / Plain_Flag / Version_Counter / Code_Size` 检查
3. `key_id / signer key slot` 检查
4. `revoke bitmap` 检查
5. eHSM Version Counter / NGU logical rollback domain 检查
6. `signer pubkey hash` 校验
7. `signature` 校验
8. 确认认证覆盖范围至少包含完整 Code region，即 `NGU protected manifest + SEC1 payload + padding/alignment counted by Code_Size`
9. Code region verify/decrypt output
10. eHSM 成功后由 BootROM / SEC 解析 NGU manifest 并记录 measurement 与保护策略状态

SEC1 解密失败必须返回明确错误码并阻止启动，BootROM 不得降级为未解密镜像继续执行。

### 3.11.2 后续镜像校验规则

SEC1/SEC2 向 eHSM Firmware `soc_verify` 或项目 wrapper 发起 `VERIFY_IMAGE` 时，至少执行：

1. eHSM native header 检查
2. eHSM `Image_Type` 与 expected profile 检查
3. eHSM verify/decrypt output
4. NGU manifest `ngu_image_type` 检查
5. `lifecycle_mask` 检查
6. `board_bind_flags` / Die binding 检查（如启用）
7. `signer_key_hash` / trust anchor 校验
8. rollback domain / policy 检查
9. SEC2 在正式安全启动路径中必须签名 + 加密，PM / RAS / Codec 等后续关键运行期固件在 USER/PROD 默认签名 + 加密
10. 通过后才允许 release

### 3.11.3 失败处理规则

- `[CONFIRMED]` 任一关键镜像验证失败，必须返回明确 `error_code`
- `[CONFIRMED]` 失败后必须记录错误并进入失败或恢复路径
- `[ASSUMED]` USER 量产态下，不允许自动降级到非安全启动继续运行
- `[ASSUMED]` DEBUG/RMA 可在授权后进入受控 rescue path

---

## 3.12 Staging Buffer 与 Host 交互规则

### 3.12.1 Host 的允许动作

Host 允许：
- 通过 PCIe 下发 SEC2 / PM / RAS / Codec 等镜像
- 配置 firmware descriptor
- 读取普通状态与版本信息
- 触发 mailbox doorbell / queue 交互（受控）

### 3.12.2 Host 的禁止动作

Host 不得：
- 直接 release 微核
- 修改 secure boot 状态
- 修改 lifecycle
- 修改 debug enable
- 修改 recovery 模式选择
- 直接访问 secure shared buffer / OTP / Secure SRAM
- 直接写 boot-critical 分区

### 3.12.3 DMA 访问要求

- `[CONFIRMED]` Host DMA 仅允许访问 firmware staging buffer 和普通数据缓冲区
- `[CONFIRMED]` Host DMA 不得访问：
  - SEC1 / SEC2 执行区
  - recovery 区
  - 证书/策略区
  - 安全共享缓冲区
  - 安全状态寄存器区

---

## 3.13 非安全启动规则

### 3.13.1 设计定位

- `[CONFIRMED]` 非安全启动必须保留，用于指定的开发和调试场景
- `[ASSUMED]` 非安全启动在量产 USER 生命周期下应默认关闭
- `[ASSUMED]` 非安全启动开启必须是显式策略，而不是失败后的隐式回退

### 3.13.2 最小规则

1. 非安全启动不得伪装成安全启动
2. 非安全启动路径必须在状态寄存器或证明路径中可见
3. 若进入非安全启动，不得产生“安全启动已通过”的错误状态
4. 非安全启动模式下的升级、调试、证明能力必须受更严格区分

### 3.13.3 与证明路径关系

- `[ASSUMED]` 若设备处于非安全启动路径，Attestation report 必须能体现：
  - secure_boot_state = disabled / bypass
  - 相应 measurement 策略可能降级
- `[ASSUMED]` Verifier 不应把非安全启动态报告判为量产可信设备态

---

## 3.14 失败处理与恢复路径

### 3.14.1 失败场景

| 场景 | 检测点 | 建议动作 |
|---|---|---|
| SEC1 验签或解密失败 | BootROM + eHSM | 停止跳转，记录错误码，进入失败/恢复路径 |
| SEC2 验签失败 | SEC1/SEC2 + eHSM | 拒绝装载，保持控制面不放行 |
| 后续微核验签失败 | SEC2 + eHSM | 拒绝对应微核 release |
| rollback 检查失败 | eHSM / counter path | 拒绝执行，记录 rollback error |
| eHSM 未 ready | BootROM / SEC 超时 | 进入受控失败处理，不得静默旁路 |
| mailbox / shared memory 错误 | SEC ↔ eHSM | 返回明确错误并停止危险路径 |

### 3.14.2 恢复规则

- `[CONFIRMED]` 升级失败时必须保证上一个 known-good 镜像仍可启动
- `[ASSUMED]` 建议对 SEC2 与主要运行期固件采用 A/B 槽位
- `[ASSUMED]` 恢复镜像应使用专用 recovery trust anchor 签名
- `[ASSUMED]` 恢复入口必须受 lifecycle 控制且可审计

---

## 3.15 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| SEC1 / SEC2 / 后续固件验证路径 | `04_impl_design/mailbox_if.md` |
| 镜像头、固件包制作流程、版本、rollback 字段 | `04_impl_design/efuse_key_fw_header_design.md` |
| 证明中 secure boot / rollback / lifecycle 状态反映 | `04_impl_design/spdm_report.md` |
| MANU→USER 冻结动作、恢复路径、RMA 策略 | `04_impl_design/manufacturing_provisioning.md` |

---

## 3.16 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| SEC1 验证与解密调用边界 | 直接影响 BootROM / eHSM 接口冻结 | 已基本收敛 | 冻结 `VERIFY_SEC1` 参数模型、强制解密标志和输出 buffer 约束 |
| release owner 语义 | 直接影响 SEC / Host / 微核控制权 | 已基本收敛 | 冻结 release 状态机 |
| rollback counter 映射 | 影响 OTP / 升级 / 证明一致性 | 部分收敛 | 冻结 image_type → counter_id |
| non-secure boot 在 USER 是否完全关闭 | 影响产品策略和客户模式 | 未完全冻结 | 需产品/安全评审裁决 |
| SEC2 decrypt / release policy | 影响后续安全控制面启动 | 已收敛 | 冻结 SEC2 key slot、wrapped CEK、错误码和 release 状态机 |
| 固件制作工具契约 | 影响 image packager、BootROM、SEC verify flow、eHSM adapter 联调 | 部分收敛 | 冻结 manifest ABI、工具参数、golden vector、exact eHSM key ID / command mapping |
| runtime signature-only 白名单 | 影响产品 SKU 与 attestation 策略 | 未完全冻结 | 冻结 image_type / lifecycle / SKU / debug / release / rollback 条件 |
| recovery trust model | 影响升级与返修路径 | 未完全冻结 | 冻结 signer / lifecycle 条件 |

---

## 3.17 开放问题

1. 除 SEC1/SEC2 外，哪些非敏感运行期镜像允许在特定产品阶段采用 signature-only 白名单？
2. Recovery 是否独立 image_type + 独立 signer / trust anchor / rollback counter / decrypt policy？
3. SEC1 是否只负责把 SEC2 拉起，还是在首版中继续承担一部分运行期安全控制？
4. 非安全启动在 TEST/DEVE 之外是否允许保留特定维护入口？
5. 双Die / 板级绑定策略是否需要在 boot 阶段强制参与 verify decision？
6. `ngu_image_manifest_t` ABI、image packager CLI、golden vector 和 exact eHSM command 参数如何冻结？

---

## 3.18 本章结论

本章已将 NGU800 安全启动收敛到当前可评审的正式口径：

- BootROM 是启动编排者，不是首个密码学验证者
- eHSM 是首个密码学验证主体
- SEC1 从 NOR Flash 获取并在执行前经 eHSM 验证和强制解密
- SEC2 与后续固件由 Host 投递、由 SEC1/SEC2 调 eHSM 验证；SEC2 在正式安全启动路径中必须签名 + 加密
- 固件制作工具和设备侧 verify/decrypt path 必须共享 eHSM native header + NGU protected manifest 契约
- Host 没有执行放行权
- release 必须晚于 verify pass
- anti-rollback、吊销、SEC1/SEC2 强制解密、后续镜像按策略解密和失败/恢复路径必须进入启动链
- 非安全启动应保留，但必须受生命周期和策略显式控制

后续若 `mailbox_if.md`、`efuse_key_fw_header_design.md`、`manufacturing_provisioning.md` 冻结字段变更，本章必须同步更新。
---
# 4. 设备身份与远程度量证明设计


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/03_attestation.md`
> 当前状态：V1.0（基于当前约束、baseline、实现级接口与现有方案资料收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 4.1 本章目标

本章定义 NGU800 的设备身份、远程度量证明与 SPDM 相关报告设计，明确：

1. 设备身份根、证明私钥与签名职责归属
2. 认证主体（SEC2 / eHSM）的职责分工
3. measurement_table 的拥有者、生成者和维护路径
4. 报告头、度量块、状态块、证书链块、签名块的章节级结构
5. nonce / session / challenge 的绑定关系
6. Verifier 的最小校验流程
7. 与实现层文件的映射关系：
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/efuse_key_fw_header_design.md`
   - `04_impl_design/manufacturing_provisioning.md`

---

## 4.2 生效约束 ID

- [C-ROOT-01](../01_constraints.md#c-root-01) - Root of Trust 必须由 eHSM 承载
- [C-KEY-01](../01_constraints.md#c-key-01) - 私钥不得导出
- [C-KEY-02](../01_constraints.md#c-key-02) - Key 使用必须绑定 lifecycle
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-BOOT-01](../01_constraints.md#c-boot-01) - 所有可执行镜像执行前必须验签
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-IF-01](../01_constraints.md#c-if-01) - 所有正式密码操作必须走 eHSM
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程

---

## 4.3 生效 Baseline 决策

### 4.3.1 身份与签名边界
- `[CONFIRMED]` 设备证明私钥不得离开 eHSM
- `[CONFIRMED]` eHSM 是最终签名执行主体
- `[CONFIRMED]` Host 只接收 report，不接触证明私钥

### 4.3.2 控制面与服务面
- `[CONFIRMED]` SEC2 是设备认证统一执行体，对外作为认证/证明控制面
- `[CONFIRMED]` SEC2 负责汇总 measurement_table、管理状态并对外响应 SPDM 相关请求
- `[CONFIRMED]` eHSM 负责签名能力和必要的密钥操作

### 4.3.3 证明内容
- `[CONFIRMED]` report 必须覆盖从安全启动到运行态最关键的可信对象
- `[CONFIRMED]` report 至少包含：report header、measurement 集合、cert chain、signature block
- `[CONFIRMED]` measurement 至少应覆盖 BootROM/immutable identity、SEC 固件、其他微核固件、安全状态和平台标识
- `[CONFIRMED]` SEC1 measurement 必须对应 verify + decrypt 成功后的受控镜像状态，不能只记录未解密包体存在性

---

## 4.4 设计要求

### 4.4.1 本章必须回答的问题

1. 设备证明“谁来组织、谁来签名”？
2. measurement_table 由谁维护、何时更新？
3. challenge / nonce / session 如何绑定到报告？
4. 报告里哪些字段必须被签名覆盖？
5. 证书链首版采用什么模式？
6. lifecycle / debug / secure boot / anti-rollback 是否必须进入报告？
7. Verifier 至少应做哪些校验？
8. 国密 / 国际算法如何在 report 中共存？

### 4.4.2 不得违反的边界

- Host 不得持有设备证明私钥
- 不能只返回“签名结果”而不返回签名所覆盖的可信状态
- 不能只覆盖 firmware hash 而忽略 lifecycle / debug / challenge 绑定
- 不能把调试授权成功等价成“设备可信”
- 不能把非安全启动态的设备伪装成量产可信态

---

## 4.5 认证架构

### 4.5.1 当前项目裁决

当前项目推荐口径为：

> **SEC2 作为设备认证的统一执行体，eHSM 作为签名与密钥服务提供者。**

含义如下：

- SEC2 负责：
  - 验证后续微核固件
  - 更新 measurement_table
  - 维护对外认证状态
  - 响应 SPDM 证书、Challenge、Measurements 请求
  - 组装完整证明报告的数据对象
- eHSM 负责：
  - challenge 相关安全服务
  - attestation key 使用
  - 签名计算
  - 必要的 key / cert / lifecycle / counter 状态支撑

### 4.5.2 角色分工表

| 角色 | 职责 | 不允许做的事 |
|---|---|---|
| Host / Verifier | 发起 challenge / nonce / session；接收并验证报告 | 直接接触证明私钥 |
| SEC2 | 认证控制面；汇总度量；组织 report；对外响应 | 私自伪造签名 |
| eHSM | 最终签名、key 使用、挑战生成、状态支持 | 接受非 SEC 的不受控证明请求 |
| OTP / eFuse | 提供设备根、lifecycle、counter、平台标识基础信息 | 被 Host 直接读取敏感根材料 |

---

## 4.6 架构图

```mermaid
graph TD
    V[Verifier / Host] -->|Challenge / Nonce / Request| SEC2[SEC2]
    SEC2 -->|Mailbox GEN_ATTEST_REPORT| EH[eHSM]
    EH --> OTP[OTP / eFuse / Root / Lifecycle / Counter]
    SEC2 --> MT[measurement_table]
    SEC2 --> ST[secure boot / debug / version state]
    EH -->|sign / key use| REP[Signed Report]
    SEC2 -->|organize blocks| REP
    REP --> V
```

### 图下说明

1. SEC2 是认证控制面，对外可表现为 SPDM Responder。
2. eHSM 是签名和密钥使用的执行面。
3. measurement_table 由启动链各阶段产生，最终由 SEC2 汇总维护并对外输出。
4. OTP/eFuse 提供 device identity seed、lifecycle、counter、chip/platform 标识等基础状态。

---

## 4.7 时序图

```mermaid
sequenceDiagram
    participant V as Verifier/Host
    participant S2 as SEC2
    participant EH as eHSM
    participant MT as measurement_table

    V->>S2: Challenge / Nonce / Get Measurements
    S2->>S2: 收敛 session / challenge / policy
    S2->>MT: 读取 SEC1/SEC2/微核/状态度量
    S2->>EH: GEN_ATTEST_REPORT(req)
    EH->>EH: 选择 attestation key / 绑定 nonce / 签名
    EH-->>S2: signed report or signed region
    S2->>S2: 组装完整 report（若采用 SEC2 组装模式）
    S2-->>V: report + cert chain / anchor info
    V->>V: 校验签名、nonce、状态、measurement
```

### 图下说明

1. report 的“数据组织”和“最终签名”可以分工，但私钥使用必须在 eHSM 内部。
2. 证明价值在于“签名覆盖的度量对象”，而不是单纯返回一个签名本身。
3. challenge / nonce 必须绑定到本次 report，防止重放。

---

## 4.8 身份与证书模型

### 4.8.1 身份对象

| 对象 | 内容 | 生成/维护位置 | 使用位置 |
|---|---|---|---|
| Device UID | 设备唯一标识 | OTP/eFuse | eHSM / 报告 |
| Device Identity | UID + product info + lifecycle | SEC2 组装 | Host / Verifier |
| Device Identity Key | 设备证明私钥 | eHSM | 签名 report |
| Attestation Cert | 设备证明证书 | 制造/灌装阶段 | 报告 / Verifier |
| Debug Auth Cert | 调试授权证书 | 制造/售后流程 | eHSM / 调试鉴权 |

### 4.8.2 当前项目建议

- `[CONFIRMED]` 首版可以 Device Identity Key 为主，不强制首版启用 Alias Key
- `[ASSUMED]` 预留 Alias / Session-bound key 扩展位
- `[CONFIRMED]` Attestation Cert 与 Debug Auth Cert 应在语义上区分，不应简单混用

### 4.8.3 证书模型建议

首版优先支持两种模式：

#### 模式 A：Hash Anchor + 可选 Chain
- OTP/eFuse 中固化 attestation root hash / signer hash
- 报告中带 signer/anchor 标识
- 如需要，可附带 cert chain blob

#### 模式 B：Full Chain
- 报告中直接携带完整证书链
- verifier 侧直接做整链校验

当前建议：
- `[CONFIRMED]` 结构上必须支持 cert chain block
- `[ASSUMED]` 首版可采用“Hash Anchor + 可选 Chain Blob”作为最小可行方案
- `[TBD]` 是否强制报告内嵌完整 cert chain 需结合客户接入模式冻结

---

## 4.9 度量原则

### 4.9.1 核心原则

设备远程认证的价值不在于返回一个签名本身，而在于签名所覆盖的**实际可信对象集合**。

因此报告必须能够让验证方判断：

- 设备是谁
- 当前运行的关键固件是什么
- 当前 lifecycle / debug / secure boot / anti-rollback 状态是什么
- 当前状态是否符合预期策略

### 4.9.2 度量生成责任

- BootROM 阶段：记录 immutable identity / ROM version 等早期信息
- SEC1 装载阶段：记录 SEC 固件早期版本 / 测量信息
- SEC1 解密阶段：记录 SEC1 已按强制加密策略完成 eHSM / 安全子系统受控解密的状态摘要
- SEC2 运行阶段：统一汇总并维护 measurement_table
- eHSM：提供签名、密钥、counter、lifecycle 等支持

### 4.9.3 当前裁决

- `[CONFIRMED]` measurement_table 的拥有者和统一维护者是 SEC2
- `[CONFIRMED]` BootROM / SEC1 / SEC2 / 后续微核验证路径产生的度量最终进入统一 measurement_table
- `[ASSUMED]` 首版 measurement_table 可由 SEC2 维护在受控内存中，并在 attestation 时组织输出

---

## 4.10 度量内容

### 4.10.1 建议至少覆盖的对象

| Measurement | Producer | Signed | Notes |
|---|---|---|---|
| BootROM version / immutable identity | BootROM | Yes | 启动阶段写入指定安全内存 |
| SEC FW hash / version / rollback | BootROM + SEC2 | Yes | SEC1/SEC2 验证时记录 |
| image protection policy / decrypt_applied | BootROM + SEC2 + eHSM | Yes | 至少反映 SEC1/SEC2 强制签名 + 加密、PM/RAS/Codec 默认策略及 signature-only 例外 |
| Aux FW hash / version / rollback | SEC2 | Yes | PM / RAS / Codec 等 |
| lifecycle / debug / secure_boot / anti_rollback | SEC2 + eHSM | Yes | 统一维护状态 |
| chip_id / device_uuid / die info | eHSM + SEC2 | Yes | 平台实例身份 |
| board binding state / board_bind_result | SEC2 + eHSM | Yes | `[ASSUMED]` V2.4 默认进入证明，不默认阻断 SEC1 |

### 4.10.2 当前项目推荐 measurement 集合

1. BootROM / immutable identity
2. SEC1
3. SEC2
4. PM / RAS / Codec 微核集合
5. lifecycle state
6. debug state
7. secure boot enable
8. image confidentiality policy / image protection policy（至少覆盖 SEC1 + SEC2）
9. anti-rollback enable
10. chip_id / device_uuid
11. board binding / die binding（如启用）

---

## 4.11 报告内容与格式

### 4.11.1 报告逻辑布局

```text
Report Header
+ Identity Block
+ Nonce / Session Binding Block
+ Measurement Block(s)
+ Lifecycle / Debug Block
+ Firmware Version Block
+ Cert Chain Block
+ Signature Block
```

### 4.11.2 当前项目要求

- `[CONFIRMED]` report 至少包含：
  - report header
  - measurement 集合
  - cert chain / anchor 信息
  - signature block
- `[CONFIRMED]` lifecycle/debug/secure_boot/anti_rollback 状态必须可导出
- `[CONFIRMED]` nonce / challenge 必须绑定
- `[ASSUMED]` session_id / transcript_hash 在 SPDM 会话场景下建议绑定

---

## 4.12 报告头结构

### 4.12.1 章节级推荐结构

```c
typedef struct {
    uint32_t magic;
    uint32_t header_len;
    uint32_t total_len;
    uint8_t  report_uuid[16];
    uint8_t  device_uuid[16];
    uint8_t  requester_nonce[32];
    uint32_t lifecycle_state;
    uint32_t debug_state;
    uint32_t session_id;
    uint32_t hash_algo;
    uint32_t sig_algo;
    uint32_t cert_format;
    uint32_t block_count;
    uint32_t signed_region_offset;
    uint32_t signed_region_len;
} ngu_attest_report_header_t;
```

### 4.12.2 字段说明

| 字段 | 说明 |
|---|---|
| `magic` | 固定魔数，便于解析 |
| `header_len` | 报告头长度 |
| `total_len` | 整体报告长度 |
| `report_uuid` | 本次报告唯一标识 |
| `device_uuid` | 设备实例标识 |
| `requester_nonce` | challenge / nonce |
| `lifecycle_state` | 当前生命周期 |
| `debug_state` | 当前 debug 状态 |
| `session_id` | 会话绑定信息 |
| `hash_algo / sig_algo` | 算法表达 |
| `cert_format` | cert chain/anchor 表达方式 |
| `block_count` | block 数量 |
| `signed_region_offset/len` | 签名覆盖区域 |

### 4.12.3 当前裁决

- `[CONFIRMED]` `requester_nonce` 必须进入签名覆盖范围
- `[CONFIRMED]` lifecycle_state / debug_state 不得只在外部上下文中推测，必须显式进入报告
- `[ASSUMED]` `report_uuid` 用于审计与缓存去重，首版建议保留

---

## 4.13 度量结构

### 4.13.1 Measurement Block Header

```c
typedef struct {
    uint16_t block_type;
    uint16_t block_version;
    uint32_t block_len;
    uint32_t flags;
    uint32_t reserved0;
} ngu_measurement_block_header_t;
```

### 4.13.2 固件度量信息结构

```c
typedef struct {
    uint32_t fw_type;
    uint8_t  hash[32];
    uint32_t fw_version;
    uint32_t rollback_counter;
    uint32_t reserved0;
} ngu_meas_fw_info_t;
```

### 4.13.3 状态度量结构

```c
typedef struct {
    uint32_t lifecycle_state;
    uint32_t debug_state;
    uint32_t secure_boot_enable;
    uint32_t anti_rollback_enable;
    uint8_t  chip_id[16];
    uint32_t reserved0;
} ngu_meas_state_t;
```

### 4.13.4 当前建议

- `[CONFIRMED]` firmware hash、version、rollback counter 应成组表达
- `[CONFIRMED]` lifecycle/debug/secure_boot/anti_rollback 状态应成组表达
- `[ASSUMED]` digest 长度可按 `algo_family/hash_algo` 扩展到 32B / 48B

---

## 4.14 度量集合

### 4.14.1 推荐 slot/组件集合

| Slot / Component | 是否首版必须 | 说明 |
|---|---|---|
| SEC1 | 是 | 启动链关键项 |
| SEC1 image protection policy | 是 | 反映 SEC1 已强制签名 + 加密且未走非安全降级路径 |
| SEC2 image protection policy | 是 | 反映 SEC2 已强制签名 + 加密且 decrypt_applied 成功 |
| PM 微核 | 默认纳入 | `[ASSUMED]` USER/PROD 默认 sign+encrypt，产品分阶段实现可区分强制项与可选项 |
| RAS 微核 | 默认纳入 | `[ASSUMED]` USER/PROD 默认 sign+encrypt，产品分阶段实现可区分强制项与可选项 |
| Codec 微核 | 默认纳入 | `[ASSUMED]` USER/PROD 默认 sign+encrypt，产品分阶段实现可区分强制项与可选项 |
| lifecycle state | 是 | 量产可信判断必要 |
| debug policy bitmap / state | 是 | 调试状态可信判断必要 |
| board policy / binding digest | 可选 | 板级策略/绑定 |

### 4.14.2 当前裁决

- `[CONFIRMED]` SEC1 / SEC2 / lifecycle / debug / secure_boot / anti_rollback 是首版必须覆盖的核心度量项
- `[CONFIRMED]` lifecycle、debug state、secure_boot state、rollback state 必须进入 report，并被 report signature 覆盖
- `[CONFIRMED]` SEC1 / SEC2 的 image protection policy 必须可被 report 或 measurement flags 表达
- `[ASSUMED]` image protection policy、decrypt_applied、image_type policy 字段进入 report 或 measurement flags
- `[ASSUMED]` PM / RAS / Codec 首版默认纳入；如产品分阶段实现，可在 verifier 策略中区分强制项与可选项
- `[ASSUMED]` board binding 默认进入 attestation report 或扩展证明数据，不默认阻断 SEC1 verify/decrypt/release
- `[TBD]` board binding 是否参与 SEC2/runtime release decision 需与板级安全策略一起冻结

---

## 4.15 签名覆盖范围

### 4.15.1 必须签名覆盖的内容

- report header
- identity block
- challenge / nonce / session 绑定信息
- measurement block 集合
- lifecycle / debug / secure_boot / anti_rollback 状态
- firmware version / rollback 信息
- cert chain metadata（若存在）

### 4.15.2 不允许的实现

- 只对 measurement digest 做签名
- 把 lifecycle/debug 状态放在签名区之外
- verifier 通过“外部上下文猜测” challenge 绑定关系
- 由 Host 侧重新拼装后再要求 verifier 验签

### 4.15.3 当前裁决

- `[CONFIRMED]` nonce/challenge 绑定是必须项
- `[CONFIRMED]` 报告不能只证明“跑的是什么”，还必须证明“当前状态是什么”
- `[ASSUMED]` session / transcript hash 在 SPDM session 模式下建议纳入签名覆盖

---

## 4.16 双算法报告映射

### 4.16.1 国密路径

| 用途 | 建议算法 |
|---|---|
| Hash | SM3 |
| Signature | SM2 |
| Cert Key | SM2 |

### 4.16.2 国际路径

| 用途 | 建议算法 |
|---|---|
| Hash | SHA-256 / SHA-384 |
| Signature | ECDSA-P256 / P-384 / RSA-3072 |
| Cert Key | P-256 / P-384 / RSA |

### 4.16.3 结构层要求

报告结构中必须显式表达：
- `algo_family`
- `hash_algo`
- `sig_algo`
- `cert_format`

### 4.16.4 当前裁决

- `[CONFIRMED]` verifier 逻辑不得把算法栈写死为单一实现
- `[CONFIRMED]` report 结构必须对国密/国际算法共存友好
- `[ASSUMED]` 首版产品可只启用一种主路径，但字段不能缺失

---

## 4.17 Verifier 最小校验步骤

Verifier 至少必须执行：

1. 检查 `report_version / magic / total_len`
2. 检查 `algo_family / hash_algo / sig_algo / cert_format`
3. 提取并匹配 cert chain / anchor
4. 检查 signer 身份是否可信
5. 检查 `requester_nonce` 是否与 challenge 一致
6. 若存在 session_id / transcript 绑定，检查是否一致
7. 校验签名
8. 检查 measurement 集合是否满足策略
9. 检查 lifecycle / debug / secure_boot / anti_rollback 状态是否满足预期
10. 检查 firmware version / rollback counter 是否不低于策略门限
11. 若启用 board/die binding，检查 binding 是否匹配

### 4.17.1 当前裁决

- `[CONFIRMED]` verifier 不能只做“签名对不对”的校验
- `[CONFIRMED]` verifier 必须同时校验状态和策略
- `[ASSUMED]` 首版 verifier 可按产品线策略区分“必须项”和“建议项”

---

## 4.18 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| report 字段级结构 / binding / measurement / signature | `04_impl_design/spdm_report.md` |
| attestation mailbox 命令 / challenge / report 获取接口 | `04_impl_design/mailbox_if.md` |
| attestation key / anchor / identity seed 上游对象 | `04_impl_design/efuse_key_fw_header_design.md` |
| manufacturing / cert / anchor / lifecycle 冻结关系 | `04_impl_design/manufacturing_provisioning.md` |

---

## 4.19 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| Device Identity vs Alias Key 首版策略 | 影响 report / cert / verifier 复杂度 | 部分收敛 | 冻结首版 key model |
| cert chain 内嵌策略 | 影响 report 大小与 verifier 部署方式 | 未完全冻结 | 冻结首版 cert 模式 |
| measurement 必选集合 | 影响安全策略与兼容性 | 部分收敛 | 冻结首版最小 measurement set |
| image protection policy / decrypt_applied 字段 | 影响 verifier 是否能识别 SEC1/SEC2 强制加密和后续镜像 signature-only 例外 | 部分收敛 | 冻结 report 字段位置和 flags 编码 |
| session/transcript 绑定粒度 | 影响 SPDM 集成深度 | 未完全冻结 | 冻结首版 binding 模式 |
| board/die binding report 策略 | 影响板级/多Die 产品 | 部分收敛 | V2.4 默认进入 attestation；是否参与 SEC2/runtime release decision 后续冻结 |
| PowerBrake / PG / FAULT / reset event | 影响证明与板级故障解释 | 未完全冻结 | 冻结进入主 report 还是扩展 event log |

---

## 4.20 开放问题

1. 首版是否仅 Device Identity Key 签名即可满足客户接入，还是必须同步规划 Alias Key？
2. report 是否必须默认内嵌完整 cert chain？
3. measurement_table 最终是否全部由 SEC2 自维护，还是部分由 eHSM 动态拉取？
4. report 中 image protection policy / decrypt_applied / board_bind_result 是作为 measurement flags、lifecycle block 字段，还是独立 policy block 表达？
5. 双Die 场景是单 report 汇总还是主/从 Die 分别证明？
6. debug 授权状态是否需要带时间窗口/过期信息进入 report？
7. PowerBrake / PG / FAULT / reset event 是否进入主 report，还是进入扩展 event log？

---

## 4.21 本章结论

本章已将 NGU800 的设备身份与远程度量证明设计收敛到当前可评审的正式口径：

- SEC2 是认证控制面，eHSM 是签名与密钥执行面
- 设备证明私钥不得离开 eHSM
- measurement_table 由启动链产生、由 SEC2 汇总维护
- SEC1 measurement 必须反映 verify + decrypt 成功后的受控镜像状态
- 报告必须覆盖身份、挑战绑定、关键固件度量、lifecycle/debug/secure_boot/anti_rollback 状态
- image protection policy / decrypt_applied / board_bind_result 是首版 report 的策略表达方向，但字段位置仍需冻结
- Verifier 必须同时校验签名、状态和策略，而不是只校验签名
- 国密与国际算法必须在报告结构层共存

后续若 `spdm_report.md`、`mailbox_if.md`、`manufacturing_provisioning.md` 或 key/cert 基线字段冻结有变化，本章必须同步更新。
---
# 5. 安全调试与生命周期控制


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/04_lifecycle_debug.md`
> 当前状态：V1.0（基于当前约束、baseline、实现级接口与现有方案资料收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 5.1 本章目标

本章定义 NGU800 的生命周期统一模型、安全调试策略、调试授权路径、状态切换规则和运行态控制边界，重点明确：

1. 项目生命周期状态与 eHSM 原生状态的统一映射
2. 不同 lifecycle 下的启动策略、调试策略、升级策略和接口开放范围
3. 安全调试必须经过 challenge-response / debug auth 的控制要求
4. 调试范围（scope / bitmap）与自动关闭策略
5. lifecycle 对 OTP/eFuse、非安全启动、镜像接收和 provisioning 的 gating 关系
6. 生命周期切换的受控条件与不可逆路径
7. 与实现层文件的映射关系：
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/manufacturing_provisioning.md`
   - `04_impl_design/efuse_key_fw_header_design.md`

---

## 5.2 生效约束 ID

- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
- [C-KEY-02](../01_constraints.md#c-key-02) - Key 使用必须绑定 lifecycle
- [C-DEBUG-01](../01_constraints.md#c-debug-01) - USER 态默认关闭未授权调试
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-IF-01](../01_constraints.md#c-if-01) - 所有正式密码操作必须走 eHSM
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离
- [C-ACCESS-02](../01_constraints.md#c-access-02) - 访问控制必须使用 UserID + Firewall
- [C-BOARD-03](../01_constraints.md#c-board-03) - JTAG 必须受 lifecycle、debug auth、scope 和 MUX 控制
- [C-BOARD-04](../01_constraints.md#c-board-04) - 管理 DMA / mailbox / 中断 / 复位必须隔离和审计
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明

---

## 5.3 生效 Baseline 决策

### 5.3.1 生命周期控制
- `[CONFIRMED]` lifecycle 必须控制 debug、OTP/eFuse 访问、非安全启动和镜像接受范围
- `[CONFIRMED]` USER/PROD 下 SEC1 / SEC2 解密 key / FW_KEK 使用必须受 lifecycle gating，且不可由 debug/RMA 普通请求关闭 SEC1/SEC2 强制解密策略
- `[CONFIRMED]` USER 生命周期必须关闭未授权 debug
- `[CONFIRMED]` USER 生命周期应强制安全启动

### 5.3.2 调试控制
- `[CONFIRMED]` 调试必须走 challenge-response / debug auth
- `[CONFIRMED]` debug enable 必须支持鉴权后开启与自动关闭
- `[CONFIRMED]` 量产态 debug 默认关闭，不得由普通软件直接打开

### 5.3.3 权责边界
- `[CONFIRMED]` 所有安全关键操作必须通过 SEC2 控制路径执行
- `[CONFIRMED]` eHSM 提供 debug auth、lifecycle 切换、control field 修改等能力
- `[CONFIRMED]` Host 不得直接修改 lifecycle、secure boot、debug enable 等关键状态

---

## 5.4 设计要求

### 5.4.1 本章必须回答的问题

1. 项目生命周期如何统一命名和编码？
2. 每个 lifecycle 允许哪些启动模式、调试模式、升级模式？
3. debug 是怎么打开的，谁来授权、谁来执行？
4. 调试授权能开到什么粒度，是否需要 scope/bitmap？
5. lifecycle 切换由谁发起、由谁裁决、由谁落盘？
6. 哪些切换是单向不可逆的？
7. USER / RMA / DEST 下哪些能力必须彻底关闭？
8. attestation 报告中哪些状态必须反映当前 lifecycle/debug 策略？

### 5.4.2 不得违反的边界

- Host / 普通 CPU / 非安全 master 不得直接打开 debug
- USER 生命周期不得保留无限制调试
- lifecycle 回退不得由普通软件路径完成
- 非安全启动不得在 USER 态隐式回退或默认可用
- 调试授权不得绕过 eHSM challenge / auth 机制

---

## 5.5 生命周期统一模型

### 5.5.1 输入口径整理

当前输入资料存在两类命名口径：

- 项目口径：`TEST / DEVE / MANU / USER / DEBUG / DEST`
- eHSM/native 口径：`TEST / DEVELOP / MANUFACTURE / USER / DEBUG / DESTROY`

为便于系统级实现与文档统一，本章采用“统一态 + 映射表”的方式表达。

### 5.5.2 统一编码建议

| Enc | Unified State | eHSM Mapping | Project Mapping | 启动策略 | 调试策略 | 更新/Provisioning 策略 |
|---|---|---|---|---|---|---|
| `0x00` | TEST | TEST | TEST | 可安全 / 非安全启动 | 实验室开放调试 | 允许基础测试 |
| `0x01` | DEV | DEVELOP | DEVE | 可安全 / 非安全启动 | 允许开发调试 | 允许开发升级 |
| `0x02` | MANUFACTURE | MANUFACTURE | MANU | 优先安全启动 | 有限受控调试 | 允许 provisioning / 冒烟验证 |
| `0x03` | PROD | USER | USER | 强制安全启动 | 默认关闭，仅授权临时开 | 仅受控升级 |
| `0x04` | RMA | DEBUG | DEBUG / RMA | 仅允许签名 rescue / 受控启动 | 授权后有限开放 | 维修升级 |
| `0x05` | DECOMMISSIONED | DESTROY | DEST | 不允许正常启动 | 关闭 | 不允许 |

### 5.5.3 当前裁决

- `[CONFIRMED]` `TEST -> DEV -> MANUFACTURE -> PROD` 为主单向路径
- `[CONFIRMED]` `DECOMMISSIONED / DEST` 为不可逆终态
- `[ASSUMED]` `RMA` 在工程上等价映射到 eHSM 的 `DEBUG` 或 `DEBUG/RMA` 子模式
- `[TBD]` 是否保留独立 `DEBUG` 与 `RMA` 子状态编码，需最终与 eHSM 接口实现冻结

---

## 5.6 架构图

```mermaid
graph TD
    HOST[Host / BMC / OOB] -->|受控请求| SEC2[SEC2]
    SEC2 -->|DEBUG_AUTH / CHANGE_LIFECYCLE| EH[eHSM]
    EH --> OTP[OTP/eFuse Lifecycle / Control Bits]
    EH --> DBG[Debug Auth / Challenge / Scope Control]
    SEC2 --> STATE[Secure Boot / Debug / Update / Non-secure Boot Policy]
    STATE --> REPORT[Attestation Report State Block]
```

### 图下说明

1. lifecycle 与 debug 的最终裁决在 eHSM + OTP/eFuse 语义层完成。
2. SEC2 是统一控制面，对外收敛 Host/BMC/OOB 的请求。
3. Attestation report 必须反映当前 lifecycle/debug 状态，而不是只报告 firmware hash。

---

## 5.7 时序图

```mermaid
sequenceDiagram
    participant H as Host/Service Tool
    participant SEC as SEC2
    participant EH as eHSM
    participant OTP as OTP/eFuse

    H->>SEC: debug request / lifecycle request
    SEC->>SEC: 参数白名单检查 / 当前状态检查
    alt Debug 授权
        SEC->>EH: GET_CHALLENGE
        EH-->>SEC: challenge
        H->>SEC: auth blob / cert / signature
        SEC->>EH: DEBUG_AUTH
        EH->>EH: challenge-response / policy check / scope check
        EH-->>SEC: granted / denied + scope
        SEC-->>H: result
    else Lifecycle 切换
        SEC->>EH: CHANGE_LIFECYCLE
        EH->>OTP: update lifecycle / control field / lock path
        EH-->>SEC: success / fail
        SEC-->>H: result
    end
```

### 图下说明

1. debug 开启必须经过 challenge-response 或等价鉴权。
2. lifecycle 修改必须由受控命令触发，并最终落到 OTP/eFuse / control field。
3. Host 不能直接改 debug enable 或 lifecycle 寄存器。

---

## 5.8 各生命周期策略矩阵

| 生命周期 | Secure Boot | Non-secure Boot | Debug | OTP/eFuse 写操作 | Firmware Update | Provisioning | Recovery / Rescue |
|---|---|---|---|---|---|---|---|
| TEST | 可开可关 | 允许 | 高权限开放 | 受控允许 | 允许 | 允许最小测试 | 可选 |
| DEV | 推荐开启 | 允许 | 允许开发授权 | 受控允许 | 允许 | 视需要 | 可选 |
| MANU | 必须优先安全启动 | 受策略限制 | 有限受控 | 允许正式灌装 | 允许 | 必须允许 | 冒烟/恢复可用 |
| USER/PROD | 强制开启 | 默认关闭 | 默认关闭，仅临时授权 | 禁止正式写入根材料 | 仅受控升级 | 禁止 | 仅授权 rescue |
| RMA | 受控签名启动 | 默认关闭 | 授权后有限开放 | 仅限受控维修流程 | 允许维修升级 | 禁止常规 provisioning | 允许 |
| DEST | 不允许 | 不允许 | 关闭 | 禁止 | 禁止 | 禁止 | 禁止 |

### 5.8.1 当前裁决

- `[CONFIRMED]` USER 生命周期强制安全启动，默认关闭未授权 debug
- `[CONFIRMED]` USER/PROD 下 SEC1 必须保持签名 + 加密，SEC1 解密由 eHSM / 安全子系统受控密码服务完成
- `[CONFIRMED]` MANU 生命周期必须允许 provisioning 和冒烟验证
- `[CONFIRMED]` RMA 只允许受控 rescue / 维修升级
- `[ASSUMED]` DEV/TEST 阶段允许更宽松的 boot/debug 策略，但不能与量产态混淆

---

## 5.9 调试模型

### 5.9.1 调试分类

当前项目建议把调试能力至少分为以下几类：

| Debug Capability | 含义 | 量产态建议 |
|---|---|---|
| CPU halt / single-step | 核心停机、单步 | 禁止，需授权 |
| trace visibility | 跟踪可见性 | 禁止，需授权 |
| secure memory visibility | 安全内存可见 | 禁止，需授权 |
| interconnect debug windows | 片上互连调试窗口 | 禁止，需授权 |
| board-assisted debug access | 板级辅助调试入口 | 禁止，需授权 |
| JTAG / CPLD / MUX access | 板级 JTAG、CPLD/MUX 选择、边界扫描、GPU/CPU/Flash/DRAM/安全子系统调试 | USER 默认关闭，需授权、scope、时限和审计 |

### 5.9.2 调试位图

- `[CONFIRMED]` eHSM 支持 SoC debug authorization，并带大位图控制能力
- `[ASSUMED]` NGU800 应按子系统定义 debug bit 域
- `[TBD]` 最终 bit-level 映射（例如 129bit 端口位图）需在实现阶段冻结
- `[TBD]` `SRC-005 管理子系统方案` 中涉及的 JTAG 目标（CPU、GPU、DRAM、Flash、安全子系统、板级 MCU、边界扫描）需映射到 SoC debug scope 或板级二级 scope

### 5.9.3 当前建议

调试授权建议至少输出以下信息：

```c
typedef struct {
    uint32_t debug_enable_state;
    uint32_t granted_scope_words;
    uint32_t expire_policy;
    uint64_t scope_bitmap_addr;
} ngu_debug_auth_state_t;
```

说明：
- `granted_scope_words`：位图长度
- `expire_policy`：自动关闭策略 / 时间窗口策略
- `scope_bitmap_addr`：在受控共享内存中传递位图

### 5.9.4 板级 JTAG 调试补充规则

基于 `SRC-005 管理子系统方案`，JTAG 具备接入 GPU JTAGBUS、寄存器空间、DRAM、Flash、安全子系统、CPU 调试单元和板级 MCU 的能力。该能力在生命周期与调试模型中按最高风险调试入口处理：

- `[CONFIRMED]` USER/PROD 生命周期下，JTAG / CPLD / MUX 默认关闭。
- `[CONFIRMED]` JTAG 打开必须复用 debug auth 路径，不允许由 BMC/OOB/板级 MCU 直接打开。
- `[CONFIRMED]` 授权必须包含 target scope、访问类型、时间窗口和自动关闭策略。
- `[CONFIRMED]` 异常复位、生命周期切换、授权超时或安全错误事件必须关闭 JTAG scope 并清零 MUX 配置。
- `[ASSUMED]` ATE/SLT/EVB 阶段可使用更宽松 JTAG 策略，但 MANU -> USER 前必须锁定或清理测试路径。

---

## 5.10 调试开启流程

### 5.10.1 最小流程

1. Host / 服务工具发起 debug request
2. SEC2 进行当前 lifecycle、白名单和目标 scope 的预检查
3. SEC2 向 eHSM 发起 `GET_CHALLENGE`
4. eHSM 返回 challenge
5. 请求方提交 debug auth blob / 证书 / 签名
6. SEC2 调用 `DEBUG_AUTH`
7. eHSM 完成：
   - challenge-response 校验
   - cert / anchor 校验
   - lifecycle 策略检查
   - scope bitmap 检查
8. 成功后返回授权范围和失效策略
9. SEC2 按授权结果开启受限 debug 能力
10. 到期或关闭后，执行 `CLOSE_DEBUG`

### 5.10.2 当前裁决

- `[CONFIRMED]` challenge-response 是必须路径
- `[CONFIRMED]` 调试开启必须有 scope 控制，不能只有“开/关”二元语义
- `[CONFIRMED]` debug enable 必须支持自动关闭
- `[ASSUMED]` 授权结果中应带时间窗口或显式失效策略

---

## 5.11 调试关闭与自动回收

### 5.11.1 必须支持的收口场景

| 场景 | 行为 |
|---|---|
| 显式关闭 | `CLOSE_DEBUG` |
| 生命周期切换 | 若切到 USER / DEST，必须自动收口 |
| 异常复位 | 需恢复默认关闭态 |
| 授权超时 | 到期自动关闭 |
| 安全错误事件 | 可强制关闭调试 |

### 5.11.2 当前裁决

- `[CONFIRMED]` debug enable 不能是“开了就一直开”
- `[CONFIRMED]` USER 态下即使临时授权打开，也必须可自动关闭
- `[ASSUMED]` 异常复位后建议恢复到“未授权 debug 关闭”的保守状态

---

## 5.12 lifecycle 切换规则

### 5.12.1 推荐转换图

```text
TEST -> DEV -> MANUFACTURE -> PROD
PROD -> RMA (authorized)
ANY -> DECOMMISSIONED (irreversible)
```

### 5.12.2 转换条件

| 转换 | 条件 |
|---|---|
| TEST -> DEV | 实验室控制、基础 bring-up 完成 |
| DEV -> MANUFACTURE | 工程收敛、准备正式灌装 |
| MANUFACTURE -> PROD | 密钥/证书灌装完成 + smoke validation 通过 |
| PROD -> RMA | 授权 + challenge-response + 必要安全擦除前置 |
| ANY -> DECOMMISSIONED | 不可逆销毁路径 |

### 5.12.3 当前裁决

- `[CONFIRMED]` `MANUFACTURE -> PROD` 之前必须完成密钥/证书灌装和冒烟验证
- `[CONFIRMED]` `PROD -> RMA` 需要授权并满足安全前置条件
- `[CONFIRMED]` `DECOMMISSIONED / DEST` 是不可逆终态
- `[ASSUMED]` `RMA -> PROD` 恢复时应重新做量产安全检查与状态归档

---

## 5.13 lifecycle 与接口 gating

### 5.13.1 命令级 gating

| 命令 | TEST | DEV | MANU | USER | RMA | DEST |
|---|---|---|---|---|---|---|
| VERIFY_IMAGE | Y | Y | Y | Y | Y | N |
| GET_CHALLENGE | Y | Y | Y | 受控 | Y | N |
| DEBUG_AUTH | Y | Y | 受控 | 默认 N / 受策略 | Y | N |
| CLOSE_DEBUG | Y | Y | Y | Y | Y | N |
| CHANGE_LIFECYCLE | Y | Y | Y | 受限 | 受限 | N |
| READ_COUNTER | Y | Y | Y | Y | Y | N |
| INCREASE_COUNTER | Y | Y | Y | Y | 受控 | N |
| GEN_ATTEST_REPORT | 可选 | 可选 | 可选 | Y | 可选 | N |
| PROVISION_ROOT_MATERIAL | N | N | Y | N | N | N |

### 5.13.2 当前裁决

- `[CONFIRMED]` provisioning 命令只允许在 MANU 或等价受控阶段执行
- `[CONFIRMED]` USER 阶段默认禁止 DEBUG_AUTH 成功开启调试，除非策略明确允许受限授权
- `[CONFIRMED]` DEST 阶段不允许正常安全服务路径

---

## 5.14 lifecycle 与启动策略的关系

### 5.14.1 启动策略要求

- `[CONFIRMED]` lifecycle 必须控制 secure / non-secure boot 的可用性
- `[CONFIRMED]` USER 阶段应强制安全启动
- `[CONFIRMED]` USER/PROD 阶段 SEC1 必须保持签名 + 加密，SEC1 decrypt required 不得被 caller 或调试状态关闭
- `[ASSUMED]` TEST/DEV 阶段可允许非安全启动，用于 bring-up 和开发
- `[CONFIRMED]` RMA 仅允许受控 signed rescue boot，不得恢复成普通开放调试启动

### 5.14.2 当前裁决

- 非安全启动不是失败时的默认旁路，而是受 lifecycle + control field + strap 共同控制的显式模式
- attestation 报告中必须反映 `secure_boot_state`

---

## 5.15 lifecycle 与 attestation 的关系

### 5.15.1 必须进入报告的状态

report 中至少必须反映：

- `lifecycle_state`
- `debug_state`
- `secure_boot_state`
- `image_confidentiality_policy`（至少表达 SEC1 强制签名 + 加密策略）
- `anti_rollback_state`

### 5.15.2 当前裁决

- `[CONFIRMED]` verifier 不能只校验签名，还必须检查 lifecycle/debug 状态
- `[CONFIRMED]` 量产可信判断必须包含：
  - lifecycle 是否为 USER/PROD 或允许上线状态
  - debug 是否关闭或处于允许模式
  - secure boot 是否开启
  - rollback 策略是否满足门限

---

## 5.16 lifecycle 与 provisioning / RMA 的关系

### 5.16.1 Provisioning
- `[CONFIRMED]` Root / signer / debug / attestation / counter / control field 的正式灌装只允许在 MANU
- `[CONFIRMED]` MANU → USER 之前必须完成 key/anchor 锁定与测试 trust 清理

### 5.16.2 RMA
- `[CONFIRMED]` RMA 是受权返修路径，不是常驻状态
- `[CONFIRMED]` 返修调试必须 challenge-response 后有限开放
- `[CONFIRMED]` RMA / DEBUG 不得绕过 SEC1 加密策略；rescue image 必须使用专用 signer / recovery trust，并由 eHSM / 安全子系统受控验证与解密或按 recovery policy 处理
- `[ASSUMED]` RMA 结束后应恢复量产安全状态，并重新形成报告/审计记录

---

## 5.17 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| debug auth / lifecycle 命令 / gating / 错误码 | `04_impl_design/mailbox_if.md` |
| lifecycle/debug 状态块进入证明报告 | `04_impl_design/spdm_report.md` |
| MANU→USER / RMA 流程、审计与恢复 | `04_impl_design/manufacturing_provisioning.md` |
| control bits / lifecycle encoding / OTP字段 | `04_impl_design/efuse_key_fw_header_design.md` |
| SEC1 / SEC2 解密 key / FW_KEK lifecycle gating | `04_impl_design/efuse_key_fw_header_design.md` / `04_impl_design/manufacturing_provisioning.md` |
| JTAG scope bitmap / CPLD-MUX gating / 板级调试授权闭环 | `04_impl_design/mailbox_if.md` / `[TBD] firewall_access_rules` |

---

## 5.18 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| 生命周期统一编码 | 影响 OTP / report / command 接口 | 部分收敛 | 冻结最终编码表 |
| DEBUG 与 RMA 是否独立编码 | 影响命令 gating 与审计模型 | 未完全冻结 | 冻结状态机 |
| debug scope bitmap bit-level 定义 | 影响 FW / RTL / verifier / 工具 | 未完全冻结 | 冻结端口位图 |
| runtime signature-only 白名单 | 影响 USER/PROD 策略和 attestation 可见性 | 未完全冻结 | SEC2 已强制加密；仍需冻结 PM/RAS/Codec 等哪些非敏感镜像允许 signature-only |
| JTAG scope 与 CPLD/MUX 控制权 | 影响板级调试是否能绕过 eHSM 授权 | 未完全冻结 | 冻结 JTAG target scope、MUX 控制寄存器和关闭策略 |
| USER 下调试授权策略 | 影响量产与售后边界 | 未完全冻结 | 冻结是否允许短时授权 |
| DEST 阶段允许保留哪些状态查询能力 | 影响退役与审计 | 未完全冻结 | 冻结销毁态策略 |

---

## 5.19 开放问题

1. `DEBUG` 与 `RMA` 首版是否合并成一个统一态，还是保留子状态？
2. 首版 USER 态是否允许短时授权 debug，还是完全禁止？
3. debug scope bitmap 最终按子系统、功能类还是资源域来编码？
4. `RMA -> PROD` 恢复是否必须重新跑一次最小 attestation / smoke validation？
5. PM / RAS / Codec 或其他 runtime image 中哪些非敏感镜像允许进入 signature-only 白名单？
6. DEST 阶段是否允许只读状态查询用于审计收尾？
7. 板级 JTAG MUX / CPLD 的授权控制由 SEC 直接写寄存器，还是由板级 MCU 受控代理执行？

---

## 5.20 本章结论

本章已将 NGU800 的生命周期与安全调试控制收敛到当前可评审的正式口径：

- lifecycle 统一模型已建立，并与 eHSM / 项目命名对齐
- USER/PROD 态必须强制安全启动并默认关闭未授权 debug
- debug 开启必须经过 challenge-response / debug auth / scope 控制 / 自动关闭
- lifecycle 切换必须通过受控命令面执行，且不可由 Host 或普通软件直接修改
- attestation 报告必须反映 lifecycle/debug/secure boot/anti-rollback 状态
- provisioning 和 RMA 都必须被 lifecycle 严格 gating，并形成审计闭环

后续若 `mailbox_if.md`、`spdm_report.md`、`manufacturing_provisioning.md` 或 OTP/lifecycle 字段冻结有变化，本章必须同步更新。
---
# 6. 内外部接口设计


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/06_interface.md`
> 当前状态：V1.0（基于当前约束、baseline 与实现级接口文件收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 6.1 本章目标

本章定义 NGU800 的内外部安全接口边界，重点明确：

1. Host / BMC / OOB-MCU / Sideband 与安全子系统的边界
2. SEC/C908、eHSM、BootROM、管理子系统之间的接口职责划分
3. Mailbox + Shared Memory 的项目适配模型
4. Verify、Lifecycle、Debug Auth、Counter、Attestation、Provisioning 等接口的通用格式
5. 外部访问控制、地址白名单、生命周期限制和错误模型
6. 与实现层文件的映射关系：
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/efuse_key_fw_header_design.md`
   - `04_impl_design/manufacturing_provisioning.md`

---

## 6.2 生效约束 ID

- [C-IF-01](../01_constraints.md#c-if-01) - 所有正式密码操作必须走 eHSM
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离
- [C-ACCESS-02](../01_constraints.md#c-access-02) - 访问控制必须使用 UserID + Firewall
- [C-BOARD-01](../01_constraints.md#c-board-01) - 管理子系统流程可参考，安全边界由安全方案裁决
- [C-BOARD-02](../01_constraints.md#c-board-02) - 带外管理通道不得绕过安全策略
- [C-BOARD-03](../01_constraints.md#c-board-03) - JTAG 必须受 lifecycle、debug auth、scope 和 MUX 控制
- [C-BOARD-04](../01_constraints.md#c-board-04) - 管理 DMA / mailbox / 中断 / 复位必须隔离和审计
- [C-BOOT-01](../01_constraints.md#c-boot-01) - 所有可执行镜像执行前必须验签
- [C-BOOT-02](../01_constraints.md#c-boot-02) - Boot / release 顺序必须由安全核控制
- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
- [C-DEBUG-01](../01_constraints.md#c-debug-01) - USER 态默认关闭未授权调试
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程

---

## 6.3 生效 Baseline 决策

### 6.3.1 安全服务调用边界
- `[CONFIRMED]` 所有正式安全服务必须通过受控 Mailbox 或定义好的安全接口
- `[CONFIRMED]` SEC/C908 是唯一安全控制面 caller
- `[CONFIRMED]` eHSM 是唯一安全服务执行面

### 6.3.2 Host 边界
- `[CONFIRMED]` Host 只具备投递能力，不进入信任链
- `[CONFIRMED]` Host 不得直接访问 eHSM、OTP、Secure SRAM
- `[CONFIRMED]` Host 不得直接放行执行

### 6.3.3 生命周期与授权
- `[CONFIRMED]` USER 生命周期必须关闭未授权 debug
- `[CONFIRMED]` DEBUG/RMA 相关接口必须通过 challenge-response 或等价鉴权
- `[CONFIRMED]` 制造相关接口只能在受控 provisioning 生命周期窗口内使用

---

## 6.4 设计要求

### 6.4.1 本章必须回答的问题

1. Host、BMC、OOB-MCU 是否允许直接调用 eHSM？
2. SEC/C908 与 eHSM 之间通过什么接口交互？
3. 真实命令包放在寄存器还是共享内存？
4. 哪些接口是 Boot 期必须具备的？
5. 哪些接口只能在 MANU / DEBUG / RMA 阶段打开？
6. Address / length / lifecycle / permission 检查由谁负责？
7. 错误码、超时、busy、并发语义如何统一？
8. 哪些接口属于“安全服务接口”，哪些仅是普通数据链路？

### 6.4.2 不得违反的边界

- Host 不得直接调用 eHSM 私有命令面
- 普通 Master 不得直接操作 OTP/eFuse 安全区
- Mailbox 请求不得绕过 SEC 的参数白名单与生命周期检查
- Verify / Debug Auth / Lifecycle / Provisioning 不得在不合法 lifecycle 下开放
- 共享内存地址不得由 Host 任意指定到安全域

---

## 6.5 架构图

```mermaid
graph TD
    H[Host / Driver / Verifier] -->|PCIe / Doorbell / Data Buffer| SEC[SEC / C908]
    BMC[BMC / OOB-MCU / Sideband] -->|受控请求| SEC
    BR[BootROM] -->|早期编排| SEC
    SEC -->|Mailbox Req + Shared Memory Ptr| EH[eHSM]
    EH -->|Mailbox Resp + Result| SEC
    EH --> OTP[OTP / eFuse]
    EH --> ALG[Verify / Key / Debug Auth / Counter / Attestation]
    SEC --> FW[SEC2 / PM / RAS / Codec / Recovery]
    H -. no direct access .-> EH
    H -. no direct access .-> OTP
```

### 图下说明

1. 外部世界（Host / BMC / OOB-MCU）与 eHSM 之间没有直接信任链接口。
2. 所有正式安全服务调用必须先进入 SEC/C908 控制面。
3. Mailbox 传递“命令与包地址”，共享内存传递“真实包体”。
4. OTP/eFuse 只被 eHSM 直接使用，不向外暴露敏感内容。

---

## 6.6 时序图

```mermaid
sequenceDiagram
    participant H as Host/BMC
    participant SEC as SEC/C908
    participant MB as Mailbox
    participant EH as eHSM
    participant SM as Shared Memory

    H->>SEC: 投递请求/镜像/nonce
    SEC->>SEC: 参数白名单检查、lifecycle检查、权限检查
    SEC->>SM: 写入请求包
    SEC->>MB: INFO[0/1]写包地址 + NOTE doorbell
    MB->>EH: 中断/事件
    EH->>SM: 读取请求包
    EH->>EH: 执行 verify / key / auth / counter / attestation / provisioning
    EH->>SM: 写回响应包
    EH->>MB: 响应 NOTE
    MB->>SEC: response ready
    SEC->>SM: 读取响应包
    SEC-->>H: 返回结果或状态
```

### 图下说明

1. Host 的请求在安全语义上必须经 SEC 收敛，不允许直达 eHSM。
2. eHSM 不负责“理解 Host 业务语义”，只负责执行已受控的安全服务请求。
3. 共享内存必须受地址白名单和 cache 一致性规则保护。

---

## 6.7 接口分层

### 6.7.1 分层模型

| 层级 | 名称 | 典型对象 | 作用 |
|---|---|---|---|
| L0 | 物理/链路层 | PCIe / IRQ / Doorbell / SMBus / Sideband | 承载数据和中断 |
| L1 | SEC 收敛层 | SEC/C908 | 权限检查、生命周期检查、参数封装 |
| L2 | 安全服务接口层 | Mailbox + Shared Memory | 调用 eHSM 安全服务 |
| L3 | 安全执行层 | eHSM | verify / key / auth / counter / lifecycle / attestation |
| L4 | 结果消费层 | Host / Verifier / Driver / Tool | 接收结果、做后续处理 |

### 6.7.2 项目裁决

- `[CONFIRMED]` 安全语义控制点在 L1（SEC 收敛层）
- `[CONFIRMED]` eHSM 是 L3 安全执行层
- `[CONFIRMED]` Host / BMC / OOB-MCU 只在 L0 / L4，不直接跨到 L3

---

## 6.8 内部接口划分

### 6.8.1 BootROM ↔ SEC
BootROM 与 SEC 的关系是启动编排关系，不作为通用安全服务接口对外暴露。

BootROM 可承担：
- 早期平台初始化
- 读取 strap / lifecycle / secure boot 配置
- 触发或等待 eHSM ready
- 把控制流转移到 SEC1/SEC

BootROM 不应承担：
- 完整 Mailbox 服务管理
- 长期运行态安全服务代理
- 完整调试鉴权 / 证明 / provisioning 服务

### 6.8.2 SEC ↔ eHSM
这是项目中**唯一正式安全服务接口面**。

SEC 负责：
- 参数合法性检查
- 生命周期检查
- 请求包封装
- token 管理
- 超时与 busy 处理
- 响应解包和上报

eHSM 负责：
- 真实安全操作执行
- OTP / key / counter / lifecycle / verify / auth / attestation 服务
- 返回结构化结果和错误码

### 6.8.3 SEC ↔ Host / BMC / OOB
SEC 对外提供的是**受控代理接口**，而不是“把 eHSM 原生命令透传给外部”。

Host/BMC/OOB 可请求：
- 固件投递
- 受控 verify 流程触发
- challenge / report 获取
- 状态查询
- manufacturing/provisioning 受控步骤（仅 MANU）

Host/BMC/OOB 不可请求：
- 直接写 OTP 安全区
- 直接开关 debug
- 直接改 lifecycle
- 直接导出私钥或敏感 key blob

---

## 6.9 外部接口分类

### 6.9.1 Host / PCIe 类接口

| 接口类别 | 用途 | 安全级别 |
|---|---|---|
| Firmware Download | SEC2 / PM / RAS / Codec 固件投递 | 高 |
| Status Query | 查询当前状态 / 错误码 / 版本 | 中 |
| Attestation Request | challenge / nonce / report 获取 | 高 |
| Debug Auth Proxy | 调试鉴权代理 | 高 |
| Provisioning Proxy | 制造阶段灌装代理 | 最高 |

### 6.9.2 BMC / OOB / Sideband 类接口

当前项目中，BMC / OOB / Sideband 的信任级别应默认为：

- `[CONFIRMED]` **不高于 Host**
- `[ASSUMED]` 可作为受控链路承载者
- `[ASSUMED]` 不应天然视为 Root of Trust 的扩展部分

因此：
- BMC / OOB 可以作为桥接者，但不能默认直接控制安全策略
- SMBus / sideband 只能用于受控状态查询、受控命令转发或板级管理，不应直接成为 Root / lifecycle / debug 的绕过路径

### 6.9.3 管理子系统新增接口口径

基于 `SRC-005 管理子系统方案`，管理子系统相关接口按以下口径纳入本章：

| 接口 / 机制 | `SRC-005 管理子系统方案` 中的用途 | 安全接口裁决 |
|---|---|---|
| SMBus/I2C | 低速带外管理、传感器、电源管理、alert、master notify | 允许作为受控管理链路，高权限命令必须经 SEC/eHSM |
| I3C | 高带宽带外业务、固件更新、高频器件状态采集 | 允许作为受控链路，更新/调试/provisioning 必须鉴权 |
| PCIe VDM | 带外数据 over PCIe，当前暂考虑不支持 | 若后续启用，按 Host 不可信链路处理 |
| UART | 带外调试通道，当前暂考虑不支持 | 若后续启用，按 debug 接口处理，默认 USER 关闭 |
| JTAG | 接入 GPU/CPU/DRAM/Flash/安全子系统/板级 MCU | 不作为普通接口开放，必须经 lifecycle + debug auth + scope + MUX gating |
| SPI/QSPI | NOR Flash、板级 MCU 接口 | 影响固件存储时必须执行签名校验、写保护和 lifecycle gating |
| AXI DMA | 子系统内部和低速外设数据搬运 | 只能访问 firewall 白名单 buffer，禁止访问安全域 |
| mailbox / 中断 | CPU 子系统消息和中断协作 | 只能作为协作机制，安全服务必须经 SEC 收敛 |
| 互斥寄存器 | 多 CPU 共享资源互斥 | 不能替代权限检查，不能作为安全访问授权 |
| 电源/复位/PowerBrake | 板级电源、上下电、复位、故障响应 | 影响安全状态时必须进入状态机和审计 |

当前裁决如下：

- `[CONFIRMED]` 管理子系统总体链路和系统流程作为接口集成输入采用。
- `[CONFIRMED]` 管理子系统接口不改变“SEC/C908 是唯一安全 caller，eHSM 是唯一安全执行面”的基线。
- `[CONFIRMED]` JTAG、DMA、Flash 更新、电源复位等高权限接口不得绕过 lifecycle、debug auth、firewall 和审计。
- `[ASSUMED]` SMBus/I2C、I3C 等低速/高速 OOB 链路可承载状态查询和受控请求转发，但首版不直接承载高权限安全命令。

---

## 6.10 Mailbox 通用模型

本章不重复实现级全部细节，正式字段冻结以：

- `04_impl_design/mailbox_if.md`

为准。这里给出章节级口径。

### 6.10.1 设计原则

1. Mailbox 寄存器只传递“包地址 + doorbell / 状态”
2. 完整请求 / 响应包放在共享内存
3. SEC 是唯一 caller
4. eHSM 是唯一 callee
5. Host 不得直接调用 eHSM Mailbox

### 6.10.2 通道建议

| 通道 | 用途 |
|---|---|
| CH0 | 通用控制面：verify / lifecycle / debug auth / key / counter / UTC |
| CH1 | 长耗时镜像服务（可选） |
| CH2 | Attestation / SPDM 扩展（可选） |
| CH3~15 | 预留 |

### 6.10.3 首版最小要求

- `[CONFIRMED]` CH0 必须实现
- `[ASSUMED]` CH1 / CH2 可按首版复杂度决定是否启用
- `[CONFIRMED]` 多通道若未实现，不得在软件层伪装“已支持”

---

## 6.11 Mailbox 通用头

### 6.11.1 Request Header

```c
typedef struct {
    uint16_t cmd_id;
    uint16_t hdr_ver;
    uint32_t total_len;
    uint32_t token;
    uint32_t caller_id;
    uint32_t lifecycle_state;
    uint32_t flags;
    uint32_t payload_off;
    uint32_t payload_len;
    uint32_t resp_buf_off;
    uint32_t resp_buf_len;
} ngu_mb_req_hdr_t;
```

### 6.11.2 Response Header

```c
typedef struct {
    uint16_t cmd_id;
    uint16_t hdr_ver;
    uint32_t total_len;
    uint32_t token;
    uint32_t status;
    uint32_t err_code;
    uint32_t detail0;
    uint32_t detail1;
    uint32_t detail2;
    uint32_t detail3;
} ngu_mb_resp_hdr_t;
```

### 6.11.3 章节级规则

- `[CONFIRMED]` `token` 必须用于请求/响应配对
- `[CONFIRMED]` 所有长度字段必须由 SEC 先做边界检查
- `[CONFIRMED]` `caller_id` 必须固定为 SEC/C908 安全调用面
- `[ASSUMED]` `lifecycle_state` 可作为快速拒绝提示，但最终仍以 eHSM 当前状态/OTP 为准

---

## 6.12 命令表（项目当前建议）

| Cmd ID | 命令名 | 主要用途 | 允许 caller |
|---|---|---|---|
| 0x0001 | VERIFY_SEC1 | SEC1 验签 + 强制解密 + rollback + measurement | SEC / BootROM 早期受控路径 |
| 0x0002 | VERIFY_IMAGE | eHSM native image verify/decrypt + NGU manifest policy check；SEC2 解密强制 | SEC |
| 0x0003 | VERIFY_AND_MEASURE | 验签、按策略解密并更新 measurement | SEC |
| 0x0020 | GET_CHALLENGE | 获取 challenge | SEC |
| 0x0021 | DEBUG_AUTH | 调试鉴权 | SEC |
| 0x0022 | CLOSE_DEBUG | 关闭调试 | SEC |
| 0x0023 | CHANGE_LIFECYCLE | 切换生命周期 | SEC |
| 0x0040 | READ_COUNTER | 读取 rollback counter | SEC |
| 0x0041 | INCREASE_COUNTER | 提升 rollback counter | SEC |
| 0x0060 | KEY_DERIVE | 密钥派生 | SEC |
| 0x0080 | GEN_ATTEST_REPORT | 生成证明报告 | SEC |
| 0x00A0 | PROVISION_ROOT_MATERIAL | 制造灌装 | SEC（MANU only） |

### 6.12.1 当前裁决

- `[CONFIRMED]` Host 不得直接作为这些命令的 caller
- `[CONFIRMED]` `VERIFY_SEC1` 的 decrypt 不可由 caller 关闭，BootROM 只消费 eHSM / 安全子系统返回的受控结果
- `[CONFIRMED]` Provisioning 相关命令不得在 USER 生命周期可用
- `[CONFIRMED]` Verify、Debug、Lifecycle、Counter、Attestation 是首批必须支持的命令族

---

## 6.13 Verify Image 结构

章节级最小结构如下，完整定义以 `mailbox_if.md` 为准。

CR-0004 接受后，`VERIFY_SEC1 / VERIFY_IMAGE` 是 NGU wrapper/profile，不直接替代 eHSM 原生命令。SEC1 early boot 应映射到 eHSM Bootloader `bl_verify_image` 或等价 ROM path；SEC2/runtime load 应映射到 eHSM Firmware `soc_verify` 或项目 wrapper。

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint64_t image_addr;
    uint32_t image_len;
    uint32_t ehsm_image_type_expected;
    uint32_t ngu_image_type_expected;
    uint32_t verify_policy;
    uint32_t expected_lcs_mask;
    uint32_t decrypt_required;
    uint32_t rollback_required;
    uint32_t expected_algorithm_profile;
    uint32_t measurement_slot;
    uint32_t jump_on_pass;
    uint64_t dst_addr;
} ngu_mb_verify_image_req_t;
```

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t ehsm_version_counter_checked;
    uint32_t ngu_rollback_domain;
    uint32_t signer_key_ref;
    uint32_t measurement_slot;
    uint32_t rollback_checked;
    uint32_t decrypt_applied;
    uint32_t decrypt_result;
    uint32_t policy_state;
} ngu_mb_verify_image_resp_t;
```

### 6.13.1 字段级章节规则

- `ehsm_image_type_expected` 必须保持 eHSM TRM 定义，不承载 NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 项目级类型。
- `ngu_image_type_expected` 必须来自 NGU manifest / SEC policy table，用于 release policy、measurement slot 和 attestation 映射。
- `VERIFY_SEC1` 必须表达 mandatory decrypt、rollback policy、measurement slot、输出 buffer / destination 约束和 result code。
- `VERIFY_IMAGE(SEC2)` 必须表达 mandatory decrypt profile；decrypt failure / policy mismatch 必须阻断安全控制面启动。
- `expected_algorithm_profile` 只能用于一致性检查和审计，不得覆盖 eHSM `SocBootAlg / SocUpgradeAlg` 或等价 control field。
- exact key ID / key level / key purpose 映射必须来自 `04_impl_design/ehsm_source_conformance_matrix.md`，不得由 Host 请求字段指定。
- per-image CEK / wrapped CEK 不作为已冻结字段；若后续需要，必须通过 eHSM customization CR 增补。
- `jump_on_pass` 不得让 Host 间接控制跳转
- `dst_addr` 必须满足 SEC 地址白名单；SEC1 解密结果只允许进入 BootROM / SEC 认可的受控执行区或 staging 区
- `rollback_checked` 必须对 verifier / SEC 可见，不得隐式假设已完成
- `decrypt_result` 必须能够区分未执行、成功、失败和 policy mismatch

---

## 6.14 外部访问控制规则

### 6.14.1 Host / BMC / OOB 的统一限制

| 对象 | 允许 | 不允许 |
|---|---|---|
| Host | 投递镜像、发起 challenge、读取结果 | 直接调 eHSM、直接 release、直接写安全区 |
| BMC | 受控转发、板级管理、状态获取 | 直接覆盖 Root / lifecycle / debug 策略 |
| OOB-MCU / 板级 MCU | 板级辅助控制、受控桥接、电源/复位流程执行 | 直接成为安全根、直接打开 debug、直接改 lifecycle |
| SMBus / I2C / I3C / Sideband | 受控状态传输、简单触发、管理请求转发 | 直接承载高权限安全命令 |
| JTAG / CPLD / MUX | 授权后按 scope 打开受限调试路径 | USER 常开、板级直通、绕过 eHSM debug auth |
| 管理子系统 DMA | 访问普通白名单 buffer | 访问 eHSM、OTP/eFuse、Secure SRAM、SEC 执行区、证书/策略区 |
| 电源 / 复位 / PowerBrake | 执行受控电源和故障响应流程 | 绕过 secure boot 失败处理、绕过审计或造成未解释安全状态 |

### 6.14.2 地址与长度检查

- `[CONFIRMED]` 所有地址参数必须由 SEC 先做白名单检查
- `[CONFIRMED]` eHSM 侧必须再次做范围检查
- `[CONFIRMED]` 共享内存不得指向 Secure SRAM / OTP / eHSM 私有区
- `[CONFIRMED]` Host DMA 不得访问安全执行区和安全共享区

### 6.14.3 Cache / 一致性规则

- `[CONFIRMED]` SEC 在 doorbell 前必须完成 cache flush / clean（若共享内存可 cache）
- `[CONFIRMED]` SEC 读取响应前必须做 invalidate / barrier
- `[ASSUMED]` 首版共享缓冲区应优先选用便于一致性管理的内存区域，而不是复杂跨域区域

---

## 6.15 生命周期限制矩阵

| Command | TEST | DEVE | MANU | USER | DEBUG/RMA | DEST |
|---|---|---|---|---|---|---|
| VERIFY_IMAGE | Y | Y | Y | Y | Y | N |
| GET_CHALLENGE | Y | Y | Y | 受控 | Y | N |
| DEBUG_AUTH | Y | Y | 受控 | 默认关闭/受策略 | Y | N |
| CHANGE_LIFECYCLE | Y | Y | Y | 受限 | 受限 | N |
| READ_COUNTER | Y | Y | Y | Y | Y | N |
| INCREASE_COUNTER | Y | Y | Y | Y | 受控 | N |
| GEN_ATTEST_REPORT | 可选 | 可选 | 可选 | Y | 可选 | N |
| PROVISION_ROOT_MATERIAL | N | N | Y | N | N | N |

### 6.15.1 当前裁决

- `[CONFIRMED]` USER 下 provisioning 接口必须关闭
- `[CONFIRMED]` DEBUG/RMA 接口必须受 challenge-response 或等价授权控制
- `[ASSUMED]` DEST 生命周期下不再允许正常安全服务路径

---

## 6.16 错误码与并发语义

### 6.16.1 错误码原则

错误码至少应区分：

- 无效命令
- 非法状态
- 生命周期不允许
- 权限不足
- 地址越界
- 验签失败
- 解密失败
- eHSM key reference / key policy 无效
- policy mismatch
- rollback 失败
- auth 失败
- busy
- timeout
- 内部错误

### 6.16.2 并发语义原则

- `[CONFIRMED]` 首版 CH0 建议单 outstanding
- `[CONFIRMED]` 请求处理中再次提交同通道请求应返回 `BUSY`
- `[CONFIRMED]` 对 `BUSY` 可重试
- `[CONFIRMED]` 对 `VERIFY_FAIL / AUTH_FAIL / INVALID_LCS / ACCESS_DENY` 不得盲重试

---

## 6.17 Attestation / SPDM 与接口的关系

### 6.17.1 对外表现
Attestation 在外部看起来像：

- Host / Verifier 发 challenge / nonce
- 返回 report blob

### 6.17.2 对内实现
对内必须通过：

- `GEN_ATTEST_REPORT`
- 可选 `GET_CHALLENGE`
- 可选 `READ_COUNTER`

并且：
- 私钥不离开 eHSM
- 报告签名由 eHSM 完成
- SEC 只负责请求封装和结果转发

---

## 6.18 Provisioning 与接口的关系

### 6.18.1 对外表现
制造工站需要看见的是：

- provisioning request
- provisioning result
- lock / verify / lifecycle change result
- audit result

### 6.18.2 对内实现
这些能力必须复用受控 Mailbox 路径，而不是额外旁路：

- `PROVISION_ROOT_MATERIAL`
- `CHANGE_LIFECYCLE`
- `READ_COUNTER`
- 必要时 `DEBUG_AUTH`

### 6.18.3 章节级规则

- `[CONFIRMED]` Provisioning Tool 不得绕过 SEC 直接写 eHSM
- `[CONFIRMED]` MANU→USER 动作必须是可审计的步骤集合
- `[CONFIRMED]` 进入 USER 前必须清理测试 trust / debug 白名单

---

## 6.19 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| Mailbox req/resp / command ID / 状态机 | `04_impl_design/mailbox_if.md` |
| Attestation 报告字段 / binding / cert / signature | `04_impl_design/spdm_report.md` |
| eHSM native header / NGU manifest / rollback / signer reference | `04_impl_design/efuse_key_fw_header_design.md` |
| Provisioning / MANU→USER / RMA | `04_impl_design/manufacturing_provisioning.md` |

---

## 6.20 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| CH0/CH1/CH2 首版启用策略 | 影响 RTL / FW / driver 划分 | 部分收敛 | 冻结首版最小通道集合 |
| NOTE 位语义 | 影响 RTL / FW 中断与状态机 | 未完全冻结 | 冻结 req/rsp/ack 规则 |
| 共享内存最终落点 | 影响缓存、一致性和安全边界 | 未完全冻结 | 冻结 buffer 区域 |
| Provisioning 命令最终参数 | 影响工站和 SEC 对接 | 部分收敛 | 冻结 request/response 结构 |
| BMC / OOB / SMBus 默认信任级别 | 影响板级链路设计 | 未完全冻结 | 冻结是否允许某些桥接能力 |
| JTAG / CPLD / MUX 控制 | 影响 USER 态调试暴露面 | 未完全冻结 | 冻结 debug auth 到板级 MUX 的控制路径 |
| 管理子系统 DMA 白名单 | 影响安全域隔离 | 未完全冻结 | 冻结 UserID、firewall region、buffer 范围 |
| 电源/复位安全状态 | 影响启动、恢复和证明一致性 | 未完全冻结 | 冻结哪些事件进入安全状态机和审计 |

---

## 6.21 开放问题

1. 首版是否只启用 CH0，还是同步启用 CH1 做大镜像路径？
2. 请求/响应共享内存是共用一块还是分离管理？
3. BMC / OOB 是否允许在某些产品形态下承担 provisioning 代理角色？
4. Sideband / SMBus 是否需要支持 challenge / status 等轻量接口？
5. Attestation 报告是否首版默认内嵌完整 cert chain？
6. JTAG MUX / CPLD 的控制寄存器是否由 SEC 直接控制，还是由板级 MCU 代理执行？
7. 管理子系统 DMA 的 UserID 和 firewall region 如何划分？
8. PowerBrake、PG/FAULT、复位类事件是否进入 attestation 报告或仅进入本地审计？

---

## 6.22 本章结论

本章已将 NGU800 内外部接口设计收敛到当前可评审的正式口径：

- 安全服务接口边界：SEC/C908 是唯一 caller，eHSM 是唯一安全执行者
- Host / BMC / OOB / SMBus 只能作为受控请求发起者或链路承载者，不能直接进入信任链
- JTAG、管理子系统 DMA、电源/复位等高权限接口必须经 lifecycle、debug auth、firewall 和审计约束
- Mailbox + Shared Memory 是正式安全服务接口模型
- Verify、Lifecycle、Debug、Counter、Attestation、Provisioning 构成首批必须定义的接口族
- 地址检查、生命周期限制、错误码、busy/timeout 语义必须在实现层明确
- 章节级接口口径必须与 `mailbox_if.md`、`spdm_report.md`、`manufacturing_provisioning.md` 和 `efuse_key_fw_header_design.md` 同步维护

后续若实现级接口字段冻结有变化，本章必须同步更新。
---
# 7. 板级安全设计


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/05_board_security.md`
> 当前状态：V1.0（基于当前约束、baseline 与 `SRC-005 管理子系统方案` 增量收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 7.1 本章目标

本章定义 NGU800 的板级安全和管理子系统安全边界，重点明确：

1. BMC / OOB-MCU / 板级 MCU / 管理子系统与安全子系统之间的信任关系。
2. SMBus/I2C、I3C、PCIe VDM、SPI、UART、JTAG 等带外管理通道的安全约束。
3. JTAG、DMA、mailbox、中断、互斥寄存器、电源/复位控制等高权限能力的安全控制策略。
4. 管理子系统文档中总体架构和流程的采用范围，以及安全设计不足时的替代裁决。
5. 与实现层文件的映射关系：
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/manufacturing_provisioning.md`
   - `04_impl_design/efuse_key_fw_header_design.md`

---

## 7.2 生效约束 ID

- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离
- [C-ACCESS-02](../01_constraints.md#c-access-02) - 访问控制必须使用 UserID + Firewall
- [C-DEBUG-01](../01_constraints.md#c-debug-01) - USER 态默认关闭未授权调试
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-BOARD-01](../01_constraints.md#c-board-01) - 管理子系统流程可参考，安全边界由安全方案裁决
- [C-BOARD-02](../01_constraints.md#c-board-02) - 带外管理通道不得绕过安全策略
- [C-BOARD-03](../01_constraints.md#c-board-03) - JTAG 必须受 lifecycle、debug auth、scope 和 MUX 控制
- [C-BOARD-04](../01_constraints.md#c-board-04) - 管理 DMA / mailbox / 中断 / 复位必须隔离和审计
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚

---

## 7.3 生效 Baseline 决策

### 7.3.1 管理子系统输入采用策略

- `[CONFIRMED]` `SRC-005 管理子系统方案` 中的管理子系统总体架构、模块职责、带外管理链路、电源/复位流程、单/双 Die 约束作为系统流程输入采用。
- `[CONFIRMED]` `SRC-005 管理子系统方案` 中涉及安全的内容必须经过安全基线二次裁决。
- `[CONFIRMED]` 若管理子系统流程与 eHSM Root of Trust、SEC 统一控制面、lifecycle gating 或 debug auth 基线冲突，以安全基线为准。

### 7.3.2 板级信任边界

- `[CONFIRMED]` BMC / OOB-MCU / 板级 MCU / 管理子系统不进入 Root of Trust。
- `[CONFIRMED]` BMC / OOB / Sideband 的信任级别不高于 Host。
- `[CONFIRMED]` 板级链路可承载管理请求、状态查询、故障定位和工装流程，但不得直接访问 eHSM、OTP/eFuse、Secure SRAM、Root/anchor 或 lifecycle 控制。

### 7.3.3 高风险入口

- `[CONFIRMED]` JTAG 在 USER/PROD 默认关闭。
- `[CONFIRMED]` JTAG 接入 GPU、CPU、DRAM、Flash、安全子系统或板级 MCU 前，必须经过 challenge-response / debug auth。
- `[CONFIRMED]` 管理子系统 DMA、mailbox、中断、互斥访问和电源复位控制必须受 firewall、白名单、lifecycle 和审计约束。

---

## 7.4 设计要求

### 7.4.1 本章必须回答的问题

1. 管理子系统文档中的总体架构和流程哪些可以遵循？
2. 哪些带外管理通道只能作为链路，不能作为安全服务入口？
3. BMC / OOB / 板级 MCU 是否可以直接控制 eHSM、lifecycle、debug 或 provisioning？
4. JTAG 是否允许访问 GPU 寄存器、DRAM、Flash、安全子系统和 CPU 调试单元？
5. JTAG MUX / CPLD / 板级控制单元由谁授权、谁收口、谁审计？
6. 管理子系统 DMA 可以访问哪些 buffer，不能访问哪些安全区域？
7. 电源、上下电、复位、PowerBrake 等控制信号如何进入安全状态机？
8. 单 Die / 双 Die、board binding / die binding 是否影响证明和镜像验证策略？

### 7.4.2 不得违反的边界

- BMC / OOB / 板级 MCU 不得成为 Root of Trust 的扩展部分。
- 管理子系统不得直接修改 lifecycle、secure boot、debug enable、rollback counter、Root/anchor。
- JTAG 不得在 USER/PROD 量产态常开。
- JTAG MUX / CPLD 不得提供绕过 eHSM debug authorization 的直通路径。
- 管理子系统 DMA 不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、recovery 区、证书/策略区。
- 电源/复位控制不得绕过安全启动失败处理和审计。

---

## 7.5 管理子系统输入摘要

`SRC-005 管理子系统方案` 当前纳入以下系统级输入：

| 输入主题 | 文档口径 | 安全采用策略 |
|---|---|---|
| 带外管理通道 | BMC、OAM 模组、模组 MCU、GPU、板级 MCU/GPU 之间存在 SMBus/I2C、I3C、PCIe、UART、JTAG 等链路 | 总体链路关系可遵循，所有安全服务必须经 SEC/eHSM 收敛 |
| SMBus/I2C | 支持 SMBus/I2C，最大 1MHz，支持 slave，alert 可选，可配置临时转 master 发 master notify | 可作为低速管理链路，不得承载高权限安全命令直达 |
| I3C | 支持最高 12.5MHz，支持 slave/master，满足高性能带外业务需求 | 可作为带外业务链路，安全命令必须受认证、白名单和生命周期控制 |
| PCIe VDM | 暂考虑不支持 | 若后续启用，必须纳入 Host/OOB 不可信模型 |
| UART | 暂考虑不支持 | 若后续启用，默认视为调试接口，需 lifecycle/debug gating |
| JTAG | 可接入 BMC、UBB、OAM、板级 MCU/GPU，可访问 GPU JTAGBUS、寄存器、DRAM、Flash、安全子系统、CPU 调试单元 | 该能力不能直接按普通功能开放，必须由安全侧重定义授权、scope、MUX 和审计 |
| CPU 子系统 DMA | 管理子系统内部考虑通用 AXI DMA，每 CPU 分配独立 DMA 通道，低速外设绑定物理通道 | DMA 必须受 firewall 白名单限制 |
| mailbox / 中断 / 互斥 | CPU 子系统预留 mailbox、中断和互斥访问机制 | 只能作为受控协作机制，不得作为安全旁路 |
| 电源/复位 | 板级 MCU 管理 GPU 电源开关、上下电顺序、异常响应和定位 | 影响安全状态时必须进入安全状态机和审计 |
| 单/双 Die | 带内管理单/双 Die 仅一个 PCIe 物理通道；带外管理对外仅 DIE0 出 OOB 接口 | 需要与 die binding、证明报告和跨 Die 访问策略联动 |

---

## 7.6 架构图

```mermaid
graph TD
    BMC[BMC / OOB Host] -->|SMBus/I2C / I3C / PCIe / Sideband| MCU[板级 MCU / 管理子系统]
    MCU -->|受控管理请求| SEC[SEC / C908]
    SEC -->|Mailbox + Shared Memory| EH[eHSM]
    EH --> OTP[OTP / eFuse / Lifecycle / Counter]

    MCU -->|Power / Reset / PG / Fault| PWR[电源与复位控制]
    MCU -->|DMA / Mailbox / Interrupt| MGMT[管理子系统内部资源]

    JTAG[JTAG / CPLD / MUX] -->|scope-gated debug| DBG[CPU / GPU / Flash / DRAM / 安全子系统]
    SEC -->|debug auth result / scope| JTAG
    EH -->|challenge-response / policy| SEC

    BMC -. no direct trust .-> EH
    MCU -. no direct access .-> OTP
    JTAG -. no bypass .-> EH
```

### 图下说明

1. BMC/OOB/板级 MCU 可以承载管理流程，但不直接进入 eHSM 或 OTP/eFuse。
2. JTAG 的物理接入能力来自板级链路，但授权、scope 和生命周期裁决必须来自 SEC/eHSM。
3. 电源、复位、DMA、mailbox 和中断都可能影响安全状态，不能作为纯普通外设看待。
4. 管理子系统总体流程遵循 `SRC-005 管理子系统方案`，安全边界由 `01_constraints.md` 和 `02_baseline.md` 裁决。

---

## 7.7 时序图

```mermaid
sequenceDiagram
    participant Tool as BMC/OOB/Service Tool
    participant MCU as 板级MCU/管理子系统
    participant SEC as SEC/C908
    participant EH as eHSM
    participant MUX as JTAG MUX/CPLD
    participant Target as GPU/CPU/Flash/安全子系统

    Tool->>MCU: 管理请求 / debug request / power request
    MCU->>SEC: 转发受控请求
    SEC->>SEC: 检查 lifecycle / command whitelist / target scope
    alt 安全调试请求
        SEC->>EH: GET_CHALLENGE / DEBUG_AUTH
        EH->>EH: 校验证书、签名、scope、lifecycle
        EH-->>SEC: granted / denied + scope + expire policy
        alt 授权通过
            SEC->>MUX: 配置受限 JTAG scope
            MUX->>Target: 打开受限调试路径
            SEC->>SEC: 记录审计事件
        else 授权失败
            SEC-->>MCU: deny
        end
    else 普通管理请求
        SEC->>SEC: 地址/权限/状态检查
        SEC-->>MCU: result / status
    end
```

### 图下说明

1. 管理工具不能直接打开 JTAG MUX，必须经 SEC/eHSM 授权。
2. 授权结果必须包含 scope 和失效策略，不允许只返回“允许调试”。
3. 失败路径必须保持默认关闭态并记录审计。

---

## 7.8 带外通道安全策略

| 通道 | 管理用途 | 安全风险 | 安全要求 |
|---|---|---|---|
| SMBus/I2C | 低速状态查询、传感器、电源管理、OOB 管理 | 总线易被桥接或伪造管理请求 | 命令白名单、状态只读优先，高权限命令必须经 SEC/eHSM |
| I3C | 高性能带外业务、固件更新、高频状态采集 | 带宽更高，可能承载更大攻击面 | 数据路径隔离，更新/调试/provisioning 必须鉴权 |
| SPI/QSPI | NOR Flash、板级 MCU 接口 | 可影响固件存储和启动介质 | Flash 更新必须验签，写操作需 lifecycle gating |
| PCIe VDM | 带外数据 over PCIe（当前暂不支持） | 若启用可能混入 Host 数据面 | 默认关闭；启用前纳入 Host 不可信模型 |
| UART | 调试输入输出（当前暂不支持） | 常被作为开发后门 | 默认关闭；启用必须按 debug 接口管控 |
| JTAG | GPU/CPU/DRAM/Flash/安全子系统调试 | 最高风险，可直接绕过运行态保护 | USER 默认关闭，需 challenge-response、scope bitmap、MUX gating、审计 |

---

## 7.9 JTAG 安全设计

### 7.9.1 `SRC-005 管理子系统方案` JTAG 能力输入

`SRC-005 管理子系统方案` 描述 JTAG 需要支持以下能力：

- 可接入 BMC，链路形态为 BMC -> UBB -> OAM -> 板级 MCU/GPU。
- 可接入 GPU 芯片 JTAGBUS，访问寄存器空间和 DRAM。
- 可接入 GPU 芯片安全子系统，定位安全子系统问题。
- 可接入 CPU 调试单元，定位 CPU 固件问题。
- 可访问 GPU Flash，定位固件问题和更新固件。
- 可接入板级 MCU，定位板级 MCU 问题和更新 MCU 固件。
- 可用于 GPU 芯片边界扫描和板级 MCU 边界扫描。
- EVB/SLT 板提供 JTAG 接口，ATE/LB 板复用 DFT JTAG IO。

### 7.9.2 安全裁决

上述能力不作为默认开放能力继承。安全方案裁决如下：

- `[CONFIRMED]` USER/PROD 生命周期下 JTAG 默认关闭。
- `[CONFIRMED]` JTAG 访问 GPU 寄存器、DRAM、Flash、安全子系统、CPU 调试单元前必须通过 debug auth。
- `[CONFIRMED]` JTAG 控制必须输出 scope bitmap，至少区分 CPU、GPU、Flash、DRAM、安全子系统、板级 MCU、边界扫描。
- `[CONFIRMED]` JTAG MUX / CPLD / 板级控制单元必须接受 SEC/eHSM 授权结果，不得有常开或板级直通模式。
- `[CONFIRMED]` JTAG 授权必须有自动关闭、异常复位关闭、生命周期切换关闭和审计记录。
- `[ASSUMED]` ATE/LB/EVB/SLT 阶段可允许更宽松的 JTAG 策略，但进入 USER 前必须清理或锁定测试路径。

### 7.9.3 JTAG scope 建议

| Scope | 默认 USER | 允许阶段 | 授权要求 |
|---|---|---|---|
| CPU halt / single-step | 关闭 | DEV / RMA | debug auth + time limit |
| GPU register access | 关闭 | DEV / RMA | debug auth + target whitelist |
| DRAM access | 关闭 | DEV / RMA | debug auth + memory range whitelist |
| Flash access / update | 关闭 | MANU / RMA | signed image + debug/update auth |
| 安全子系统访问 | 关闭 | RMA 特批 | eHSM debug auth + narrow scope + audit |
| 板级 MCU 调试 | 关闭 | DEV / MANU / RMA | board debug auth + audit |
| Boundary scan | 关闭 | ATE / SLT / MANU | manufacturing lifecycle gating |

---

## 7.10 DMA / Mailbox / 中断 / 互斥访问策略

### 7.10.1 DMA 策略

`SRC-005 管理子系统方案` 提到 CPU 子系统采用通用 AXI DMA，内部 CPU 分配独立 DMA 通道，低速外设绑定物理 DMA 通道。安全要求如下：

- `[CONFIRMED]` DMA 只能访问普通 staging buffer、普通数据 buffer 和经 firewall 显式允许的区域。
- `[CONFIRMED]` DMA 不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、recovery 区、证书/策略区和安全共享缓冲区。
- `[CONFIRMED]` DMA 访问必须带 UserID 或等价 master 标识，并经 firewall 策略检查。
- `[ASSUMED]` 低速外设 DMA 通道应默认最小权限，按外设绑定固定访问范围。

### 7.10.2 Mailbox / 中断策略

- `[CONFIRMED]` 管理子系统 mailbox 和中断只作为协作机制，不得直接成为 eHSM 安全服务入口。
- `[CONFIRMED]` 安全服务请求必须进入 SEC/C908 收敛层，由 SEC 执行参数、地址、lifecycle、权限检查。
- `[CONFIRMED]` 对安全状态有影响的中断必须可屏蔽、可追踪、可审计。

### 7.10.3 互斥访问策略

`SRC-005 管理子系统方案` 中 CPU 子系统互斥访问机制可用于普通共享资源协调，但安全设计要求如下：

- `[CONFIRMED]` 互斥寄存器不能替代权限检查。
- `[CONFIRMED]` 互斥成功不代表具备访问安全资源的权限。
- `[ASSUMED]` 若互斥寄存器用于管理固件与安全服务协作，必须增加 caller、resource_id、lifecycle 和 timeout 语义。

---

## 7.11 电源、上下电和复位安全策略

`SRC-005 管理子系统方案` 描述板级 MCU 负责 GPU 电源开关、上下电顺序管理、非主电源电流检测、电源初始化配置、电源异常响应和定位。安全设计要求如下：

- `[CONFIRMED]` 影响 GPU 芯片、SEC、eHSM、Flash、DRAM 或安全状态的复位/掉电/PowerBrake 信号必须进入安全状态机。
- `[CONFIRMED]` USER/PROD 下不得通过板级复位流程绕过 secure boot 或 rollback 检查。
- `[CONFIRMED]` 异常复位后 debug 默认关闭，JTAG scope 清零。
- `[CONFIRMED]` 电源异常、PowerBrake、PG/FAULT 事件若影响 attestation 可信状态，必须进入状态记录或报告摘要。
- `[ASSUMED]` 板级 MCU 可执行电源策略动作，但高安全影响动作需由 SEC 状态机确认或记录。

---

## 7.12 单 Die / 双 Die 与板级绑定

`SRC-005 管理子系统方案` 描述单 Die / 双 Die 场景下：

- 带内管理单/双 Die 封装物理通道都只有一个 PCIe（DIE0 出）。
- 除 DRAM 地址空间外，两 Die 地址空间需要 BAR 地址分别映射。
- 带外管理对外只呈现一个管理设备，硬件接口对外只 DIE0 出 OOB 接口。
- 原则上不建议外部感知内部单 Die / 双 Die 差异。
- CE 调度、Profiling、功耗管理可能涉及跨 Die 或双 Die 资源。

安全设计要求如下：

- `[CONFIRMED]` 单 Die / 双 Die 差异不应暴露为安全策略绕过路径。
- `[CONFIRMED]` 跨 Die 访问必须经过地址映射、权限和 firewall 检查。
- `[ASSUMED]` board binding / die binding 应进入 attestation measurement 或状态摘要。
- `[TBD]` 双 Die 场景是否需要主/从 Die 分别出具证明，或由主 Die 汇总证明，需与 attestation 方案联动冻结。
- `[ASSUMED]` board binding / die binding 默认进入 attestation measurement、状态摘要或扩展证明数据。
- `[CONFIRMED]` board binding 不默认阻断 SEC1 verify/decrypt/release。
- `[TBD]` board binding 是否参与 SEC2/runtime image release decision，需与产品形态和制造流程一起冻结。

---

## 7.13 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| OOB 请求、JTAG 授权代理、状态查询接口 | `04_impl_design/mailbox_if.md` |
| board/die binding、debug state、电源/复位异常状态进入报告 | `04_impl_design/spdm_report.md` |
| MANU/ATE/SLT 阶段 JTAG 策略、USER 前测试路径清理 | `04_impl_design/manufacturing_provisioning.md` |
| lifecycle、debug enable、JTAG disable、control bits | `04_impl_design/efuse_key_fw_header_design.md` |
| DMA / firewall / UserID / 地址白名单 | `[TBD] firewall_access_rules` |

---

## 7.14 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| JTAG scope bitmap | 影响 USER 态调试暴露面 | 未冻结 | 冻结 CPU/GPU/DRAM/Flash/安全子系统/板级 MCU scope |
| JTAG MUX / CPLD 控制权 | 影响是否存在板级直通绕过路径 | 未冻结 | 冻结由 SEC/eHSM 授权结果驱动的控制方式 |
| BMC / OOB provisioning 代理 | 影响制造链攻击面 | 未冻结 | 冻结是否允许 OOB 承担 provisioning proxy |
| 管理子系统 DMA 白名单 | 影响安全内存隔离 | 未冻结 | 冻结可访问 buffer、UserID、firewall 策略 |
| 电源/复位安全状态 | 影响 secure boot 和 attestation 一致性 | 未冻结 | 冻结哪些事件进入安全状态机和报告 |
| board/die binding | 影响镜像验证、证明和量产兼容性 | 未冻结 | 冻结首版是否启用及字段位置 |

---

## 7.15 开放问题

1. JTAG scope bitmap 最终由 eHSM 原生位图直接承载，还是由 SEC 做 SoC 级二次映射？
2. CPLD / JTAG MUX 的控制寄存器由谁写入，是否需要硬件锁定防止板级直通？
3. OOB/BMC 作为 provisioning transport proxy 时，命令格式、认证、审计、失败回滚和 rate limit / lockout 如何定义？
4. 管理子系统 DMA 的 UserID 和 firewall region 如何划分？
5. 电源异常、PowerBrake、PG/FAULT 是否进入 attestation report，还是只进入本地审计？
6. 双 Die 场景下，board/die binding 是单 report 汇总还是双 Die 分别证明？
7. EVB/SLT/ATE 阶段 JTAG 测试路径如何在 USER 前锁定和审计？

---

## 7.16 本章结论

本章将 `SRC-005 管理子系统方案` 纳入板级安全设计，并形成以下安全裁决：

- 管理子系统总体架构和系统流程原则上遵循。
- BMC / OOB / 板级 MCU / 管理子系统不进入 Root of Trust。
- SMBus/I2C、I3C、PCIe VDM、SPI、UART、JTAG 等带外链路只能作为受控链路，不能直接进入安全执行面。
- JTAG 文档中描述的高权限访问能力不能按默认功能开放，必须经 lifecycle、debug auth、scope bitmap、MUX gating 和审计控制。
- 管理子系统 DMA、mailbox、中断、互斥访问、电源复位控制必须纳入 firewall、状态机和审计策略。
- 单 Die / 双 Die、board binding / die binding 需要与镜像验证、attestation 和制造流程联动冻结。

后续若 `SRC-005 管理子系统方案` 补充字段级接口、JTAG MUX 控制、DMA region、OOB provisioning 或电源复位状态机，本章及 `06_interface.md`、`10_full_design.md`、实现级文档必须同步更新。
---
# 8. Root of Trust、密钥体系与证书体系


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/02_key_cert.md`
> 当前状态：V1.0（基于当前约束、baseline 与输入资料收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 8.1 本章目标

本章定义 NGU800 的 Root of Trust、密钥体系、证书体系和双算法映射口径，明确：

1. Root of Trust 的归属和边界
2. UDS / Root Secret / DRK / 各分支业务密钥的层级关系
3. 固件验签、固件解密、设备证明、调试鉴权所依赖的 key branch
4. 证书链 / trust anchor / signer hash 的项目采用策略
5. 国密与国际算法栈在 key / cert / report / FW header 中的统一承载方式
6. 与实现层文件的映射关系：
   - `04_impl_design/efuse_key_fw_header_design.md`
   - `04_impl_design/spdm_report.md`
   - `04_impl_design/manufacturing_provisioning.md`
   - `04_impl_design/mailbox_if.md`

---

## 8.2 生效约束 ID

- [C-ROOT-01](../01_constraints.md#c-root-01) - Root of Trust 必须由 eHSM 承载
- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
- [C-IF-01](../01_constraints.md#c-if-01) - 所有正式密码操作必须走 eHSM
- [C-KEY-01](../01_constraints.md#c-key-01) - 私钥不得导出
- [C-KEY-02](../01_constraints.md#c-key-02) - Key 使用必须绑定 lifecycle
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离
- [C-ACCESS-02](../01_constraints.md#c-access-02) - 访问控制必须使用 UserID + Firewall
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚

---

## 8.3 生效 Baseline 决策

### 8.3.1 Root of Trust
- `[CONFIRMED]` Root of Trust = eHSM
- `[CONFIRMED]` Root Key / Root Secret 仅由 eHSM 使用
- `[CONFIRMED]` BootROM 不持有 Root Private Key

### 8.3.2 私钥边界
- `[CONFIRMED]` 私钥不得离开 eHSM
- `[CONFIRMED]` Host / 普通核 / 管理核 不得直接访问私钥
- `[CONFIRMED]` 所有正式安全路径 crypto 必须走 eHSM

### 8.3.3 双算法策略
- `[CONFIRMED]` 方案必须同时覆盖国密与国际算法两套栈
- `[CONFIRMED]` eHSM `SocBootAlg / SocUpgradeAlg` 或等价 control field 是 secure boot / upgrade 算法选择 authority
- `[CONFIRMED]` NGU manifest / mailbox / report 可携带 `expected_algorithm_profile`，但只能用于一致性检查、审计和 attestation，不得覆盖 eHSM control field
- `[ASSUMED]` 首版实现可按产品形态选择默认主算法栈，但结构上不得丢失双栈能力

---

## 8.4 设计要求

### 8.4.1 本章必须回答的问题

1. Root Secret / UDS 放在哪里，谁使用？
2. Device Root Key（DRK）如何从根种子派生？
3. 固件验签根和固件解密根如何区分？
4. Attestation 与 Debug Auth 是否复用一套根？
5. 固件验签采用“OTP 固化公钥摘要”还是“完整 cert chain”？
6. 设备证明采用“Device Identity Key”还是“Alias Key / Session Key”？
7. 国密 / 国际算法如何在同一套结构中共存？
8. 制造灌装阶段具体写什么、锁什么、清理什么？

### 8.4.2 不得违反的边界

- BootROM 不得成为密钥管理中心
- Host 不得持有或缓存设备私钥
- 证书链策略不得脱离 Root / OTP / lifecycle 约束单独定义
- 任何 key branch 都不得绕过 lifecycle gating
- 量产 USER 态不得保留测试 trust anchor 或测试 signer

---

## 8.5 架构图

```mermaid
graph TD
    OTP[OTP / eFuse
UDS / Root Secret / Control Bits / Signer Hash / Counter] --> EH[eHSM]
    EH --> DRK[Device Root Key / DRK]
    DRK --> FWV[FW Verify Branch]
    DRK --> FWE[FW Encrypt Branch]
    DRK --> ATT[Attestation Branch]
    DRK --> DBG[Debug Auth Branch]

    FWV --> S1[SEC1 Verify]
    FWV --> S2[SEC2 Verify]
    FWV --> OTH[PM/RAS/Codec FW Verify]

    ATT --> DI[Device Identity Key]
    ATT --> ALIAS[Alias / Session Key Optional]

    DBG --> DAUTH[Challenge / Debug Auth]

    EH --> CERT[Signer Hash / Cert Anchor / Cert Chain]
    Host[Host] -. no private key .-> CERT
```

### 图下说明

1. OTP/eFuse 保存的是**根材料、控制位、signer anchor、counter**，而不是让普通软件直接读取的明文密钥仓库。
2. eHSM 是唯一合法的 key usage 执行面。
3. DRK 是项目内部逻辑层次，不要求一定以明文字段形式存在，但要求在设计语义上作为各分支 key 的共同上游。
4. 固件验签、设备证明、调试鉴权在工程上建议分成不同 key branch，避免权限耦合。

---

## 8.6 时序图

```mermaid
sequenceDiagram
    participant OTP as OTP/eFuse
    participant EH as eHSM
    participant SEC as SEC/C908
    participant FW as FW Verify Path
    participant V as Verifier
    participant DBG as Debug Client

    OTP->>EH: 提供 UDS / Root Secret / signer anchor / lifecycle / counter
    EH->>EH: 派生 DRK
    EH->>EH: 派生 FW Verify / Encrypt / Attestation / Debug branches

    SEC->>EH: VERIFY_IMAGE(req)
    EH->>FW: 使用 FW Verify Branch 校验镜像
    FW-->>EH: PASS / FAIL
    EH-->>SEC: verify result

    V->>SEC: challenge / nonce
    SEC->>EH: GEN_ATTEST_REPORT
    EH->>EH: 使用 Attestation Branch 组织并签署 report
    EH-->>SEC: signed report
    SEC-->>V: report

    DBG->>SEC: debug auth request
    SEC->>EH: DEBUG_AUTH
    EH->>EH: 使用 Debug Auth Branch 校验授权
    EH-->>SEC: granted / denied
```

### 图下说明

1. 所有 key branch 都从 Root / UDS 语义上派生，而不是离散孤立存在。
2. 固件验签、设备证明、调试鉴权通过不同 branch 可降低权限串扰。
3. Host / Verifier / Debug Client 都不能直接操作私钥，只能通过 SEC → eHSM 的受控路径发起请求。

---

## 8.7 Root of Trust 设计

### 8.7.1 定义

本项目中的 Root of Trust 由以下三部分共同构成：

| 组件 | 职责 |
|---|---|
| OTP / eFuse | 持久保存根种子、控制位、signer anchor、counter、lifecycle 状态 |
| eHSM | 使用根种子，提供 crypto / verify / key / lifecycle / debug auth 服务 |
| BootROM | 最早启动编排者，负责把控制流程带到安全验证路径，但不是密码学根 |

### 8.7.2 当前裁决

- `[CONFIRMED]` Root of Trust 的“根使用权”归 eHSM
- `[CONFIRMED]` BootROM 属于启动链的 earliest code，但不等于密码学根
- `[CONFIRMED]` Root Secret / Root Key 材料不应作为软件可读资产暴露
- `[ASSUMED]` 若硬件实现上存在部分 Root 材料对 BootROM 的最小可见形式，也不得被视为可复用私钥材料

### 8.7.3 与启动链的关系

Root of Trust 的责任不是“替 BootROM 做所有事情”，而是：

1. 提供信任基础（OTP / Root Secret / signer anchor）
2. 提供首个密码学验证能力（eHSM）
3. 约束后续所有执行放行和生命周期行为

---

## 8.8 密钥对象表

### 8.8.1 关键密钥对象

| Key Object | 作用 | 是否可导出 | 推荐存储 / 使用位置 | 生命周期限制 |
|---|---|---|---|---|
| UDS / Root Secret | 根种子 | 否 | OTP/eFuse → eHSM 使用 | 全生命周期受控 |
| DRK | 设备根派生密钥 | 否 | eHSM 内部 | 全生命周期受控 |
| FW Verify Root | 固件验签根 | 否（私钥）/是（公钥或摘要） | eHSM / cert anchor | USER 必须受控 |
| FW Encrypt Key / KEK | 固件机密性保护，至少对 SEC1 + SEC2 强制启用 | 否 | eHSM | SEC1/SEC2 强制；PM/RAS/Codec USER/PROD 默认启用，signature-only 例外按产品白名单 |
| Image CEK / wrapped CEK | `[TBD]` 若后续确需 per-image CEK，由 eHSM owner 确认 extension / customization 后定义 | 否（CEK 明文不得离开 eHSM） | 不作为首版已冻结镜像头字段 | 与 eHSM key policy、NGU `ngu_image_type`、lifecycle 绑定；具体机制待冻结 |
| Attestation Seed | 设备证明上游种子 | 否 | eHSM | USER / DEBUG/RMA 受控 |
| Device Identity Key | 设备证明私钥 | 否 | eHSM | 不得导出 |
| Alias / Session Key | 证明扩展私钥 | 否 | eHSM | `[ASSUMED]` 首版可选 |
| Debug Auth Seed / Key | 调试鉴权 | 否 | eHSM | DEBUG/RMA 受控 |
| Signer Hash / Anchor | 固件验签锚点 | 可读摘要 | OTP/eFuse / cert block | USER 必须冻结 |

### 8.8.2 当前项目建议

- `[CONFIRMED]` UDS / Root Secret 为最上游根材料
- `[CONFIRMED]` 固件验签、设备证明、调试鉴权不应直接共用同一把外部暴露身份，而应在语义上分 branch
- `[ASSUMED]` 首版可先在实现上减少 branch 数量，但结构设计必须预留分支能力

---

## 8.9 密钥层级（Key Hierarchy）

### 8.9.1 推荐逻辑层级

```text
UDS / Root Secret
    ↓ KDF
Device Root Key (DRK)
    ↓───────────────┬───────────────────┬───────────────────┬───────────────────┐
    ↓               ↓                   ↓                   ↓
FW Verify Branch    FW Encrypt Branch   Attestation Branch  Debug Auth Branch
```

### 8.9.2 设计理由

#### FW Verify Branch
用于：
- SEC1 / SEC2 / 后续微核镜像签名校验
- signer hash / anchor 匹配
- 吊销 / 版本 / trust chain 判定

#### FW Encrypt Branch
用于：
- 镜像解密
- FW_KEK / image protect key 路径；exact eHSM key ID / key level / key purpose 以 eHSM TRM 和 source-conformance matrix 为准
- `NGU800:FW:ENC` 语义标签
- `[CONFIRMED]` 对 SEC1 为强制启用，SEC1 verify/decrypt output path 必须由 eHSM / 安全子系统受控密码服务完成
- `[CONFIRMED]` 对 SEC2 强制启用 FW Encrypt Branch。
- `[ASSUMED]` 对 PM、RAS、Codec 等后续关键固件在 USER/PROD 产品形态默认启用 FW Encrypt Branch；signature-only 例外按产品安全策略和 `ngu_image_type` 白名单冻结。
- `[TBD]` `NGU800:WRAP:CEK` / per-image wrapped CEK 是否存在，取决于 eHSM owner 是否确认 image/container extension
- `[TBD]` 除 SEC1/SEC2 外，哪些非敏感运行期镜像允许 signature-only，需由产品安全策略冻结

#### Attestation Branch
用于：
- Device Identity Key
- Alias / Session-bound attestation key
- 签署 report / attestation response

#### Debug Auth Branch
用于：
- challenge-response
- 调试授权校验
- scope / time / lifecycle 相关鉴权

### 8.9.3 当前裁决

- `[CONFIRMED]` FW Verify 和 Attestation 不能混为一条“无边界通用签名私钥”
- `[CONFIRMED]` FW Encrypt Branch 至少对 SEC1 + SEC2 强制启用；NGU `ngu_image_type = SEC1/SEC2` 必须映射到 eHSM owner-confirmed FW decrypt key policy 和 lifecycle policy
- `[TBD]` exact eHSM key ID / key level / key purpose 需要由 eHSM owner 确认，不得由 NGU 自行冻结物理 key slot
- `[CONFIRMED]` Debug Auth 必须有独立控制面，不能简单复用普通 attestation 成功即开 debug
- `[ASSUMED]` DRK 是否在硬件实现中显式存在为中间寄存态不重要，重要的是语义上 branch 上游唯一且受控

---

## 8.10 证书体系设计

### 8.10.1 当前项目面临的两种模型

| 模型 | 描述 | 优点 | 风险 |
|---|---|---|---|
| Hash Anchor 模型 | OTP 中保存 signer hash / root hash；镜像或报告中带 signer/cert 信息 | 实现轻、适合首版 | 灵活度受限 |
| Full Cert Chain 模型 | 镜像 / report 中直接携带完整 cert chain | 标准化程度高，适合长期扩展 | 体积大、实现复杂 |

### 8.10.2 当前建议

#### 固件验签路径
- `[CONFIRMED]` 首版优先采用 **OTP 固化 signer hash / trust anchor** 模型
- `[ASSUMED]` 可预留镜像中携带 cert chain blob 的能力
- `[TBD]` 是否直接首版全面切到 X.509 需看项目证书基础设施成熟度

#### 设备证明路径
- `[CONFIRMED]` report 中必须支持：
  - Hash Anchor
  - 可选 Cert Chain Block
- `[ASSUMED]` 首版 verifier 可本地预置 trust anchor，通过 report 中的 signer / anchor hash 完成快速定位
- `[TBD]` 是否要求 report 默认内嵌完整 cert chain，需结合客户接入方式和 SPDM verifier 能力冻结

### 8.10.3 证书对象表

| Cert / Anchor Object | 用途 | 建议位置 |
|---|---|---|
| FW Signer Hash Slot0 | 固件验签国密 signer 锚点 | OTP/eFuse |
| FW Signer Hash Slot1 | 固件验签国际 signer 锚点 | OTP/eFuse |
| Debug Auth Anchor | 调试授权锚点 | OTP/eFuse |
| Attestation Root Hash | 设备证明锚点 | OTP/eFuse |
| Optional Cert Chain Blob | 报告 / 镜像附带链 | 镜像 / report block |

---

## 8.11 推荐 KDF Label

> 说明：本节给出项目内部建议语义标签，不代表必须锁死到某一种 KDF 标准实现。
> 若后续采用 HKDF-SM3 / HKDF-SHA256 / 项目自定义 KDF，只要 label 语义保持稳定即可。

| Label | 用途 |
|---|---|
| `NGU800:DRK` | 从 UDS / Root Secret 派生设备根密钥 |
| `NGU800:FW:VERIFY` | 固件验签 branch |
| `NGU800:FW:ENC` | 固件加密 / 解密 branch |
| `NGU800:ATTEST:DEV` | 设备证明 Device Identity Key |
| `NGU800:ATTEST:ALIAS` | Alias / Session 证明 key |
| `NGU800:DEBUG:AUTH` | 调试鉴权 |
| `NGU800:REPORT:BIND` | 报告绑定（nonce / session 相关） |
| `NGU800:WRAP:CEK` | `[TBD]` 镜像 CEK wrap / unwrap，仅在 eHSM owner 确认 per-image CEK extension 后启用 |

### 8.11.1 使用规则

- `[CONFIRMED]` 不同业务场景必须使用不同 Label
- `[CONFIRMED]` 不得用同一个 Label 既做固件验签根又做调试鉴权
- `[CONFIRMED]` SEC1 的 FW Encrypt 派生必须绑定 NGU `ngu_image_type = SEC1`、eHSM owner-confirmed key policy、lifecycle policy 和 rollback domain，避免解密能力被跨镜像复用
- `[TBD]` 若后续启用 wrapped CEK，必须补充 eHSM customization CR 并冻结 manifest/header 扩展、unwrap command 和 anti-replay 绑定项
- `[ASSUMED]` 若国密和国际算法的 KDF 内核不同，label 语义仍应保持一致

---

## 8.12 双算法映射

### 8.12.1 国密路径

| 用途 | 建议算法 |
|---|---|
| Hash | SM3 |
| Signature | SM2 |
| Encryption | SM4 |
| KDF | SM3-based KDF / HKDF-SM3 compatible |

### 8.12.2 国际路径

| 用途 | 建议算法 |
|---|---|
| Hash | SHA-256 / SHA-384 |
| Signature | ECDSA P-256 / P-384 或 RSA-3072 |
| Encryption | AES-256-GCM / AES-CTR + MAC |
| KDF | HKDF-SHA256 / HKDF-SHA384 |

### 8.12.3 结构体层要求

以下结构中可携带算法 profile 字段用于审计或一致性检查，但不得替代 eHSM control field：

- NGU protected manifest
- Attestation Report Header
- Mailbox request/response 中涉及签名 / hash / enc 的命令
- Provisioning blob metadata

物理 secure boot / upgrade 算法选择以 eHSM TRM 定义的 `SocBootAlg / SocUpgradeAlg` 或等价 control field 为准。

### 8.12.4 当前裁决

- `[CONFIRMED]` 所有项目结构必须保留双算法表达能力
- `[ASSUMED]` 首版产品出货可以只启用其中一套主路径，但不能把结构设计做死
- `[TBD]` 各产品线国密/国际算法默认选择策略需在产品规划层冻结

---

## 8.13 与启动 / 证明 / 调试路径的关系

### 8.13.1 启动路径
- 固件验签 branch 为 SEC1 / SEC2 / PM / RAS / Codec 等镜像提供验证能力
- 固件加密 branch 至少为 SEC1 提供强制解密能力，并为 SEC2 / 后续关键运行期固件提供按策略启用的机密性保护能力
- rollback floor 需与 OTP counter 绑定
- signer hash / revoke / lifecycle mask 必须进入 verify decision

### 8.13.2 证明路径
- Attestation branch 负责 report 签名
- report 中必须带出 secure boot / lifecycle / debug / firmware version 摘要
- verifier 不能只看签名而不看状态

### 8.13.3 调试路径
- Debug auth branch 独立于普通 attestation
- 进入 RMA / DEBUG 时，challenge-response 必须基于独立授权链
- 不得把“报告签名成功”直接视为“调试可开放”

---

## 8.14 制造、灌装与密钥体系的关系

### 8.14.1 制造阶段必须完成的 key / anchor 对象

- UDS / Root Secret
- FW_KEK / image protect key 或其 eHSM 内部派生种子
- FW signer hash / trust anchor
- Debug auth anchor
- Attestation anchor / identity seed
- eHSM Version Counter 初始状态 / owner-confirmed rollback policy
- secure boot / FW encrypt / debug / attestation / rollback / algorithm control field

### 8.14.2 USER 前必须完成的动作

1. 锁定 Root / anchor 区
2. 锁定 SEC1 / SEC2 解密相关 eHSM key policy / FW_KEK 策略 / signer anchor / rollback policy
3. 清理测试 key / 测试 cert / 测试 debug trust
4. 开启 secure boot
5. 开启 anti-rollback
6. 关闭未授权 debug
7. 推进 lifecycle 到 USER
8. 留存审计日志

### 8.14.3 当前裁决

- `[CONFIRMED]` 制造阶段必须定义 key 注入、锁定、审计，不得停留在抽象口号
- `[CONFIRMED]` USER 生命周期下不允许残留测试信任锚
- `[ASSUMED]` 优先采用“Seed/UDS 注入 + eHSM 内部派生”的模式
- `[TBD]` 是否首版支持全量 cert chain 灌装取决于工站和证书服务准备度

---

## 8.15 与实现层的映射关系

| 本章主题 | 对应实现层文件 |
|---|---|
| Root / UDS / DRK / signer hash / control bits | `04_impl_design/efuse_key_fw_header_design.md` |
| Device Identity / report / cert block | `04_impl_design/spdm_report.md` |
| provisioning / lock / lifecycle / audit | `04_impl_design/manufacturing_provisioning.md` |
| key derive / verify / debug auth 命令面 | `04_impl_design/mailbox_if.md` |

---

## 8.16 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| UDS / Root Secret 注入模式 | 影响制造链和 Root 暴露面 | 部分收敛 | 冻结“直接注入”还是“seed 派生” |
| signer hash vs full cert chain | 影响镜像格式、证明格式、制造工站 | 部分收敛 | 冻结首版采用模型 |
| runtime signature-only 白名单 | 影响 FW Encrypt Branch、镜像头和产品策略 | 未完全冻结 | 冻结除 SEC1/SEC2 外哪些非敏感镜像允许 signature-only |
| Attestation 是否首版启用 Alias Key | 影响 report / cert / verifier 复杂度 | 未完全冻结 | 冻结首版 identity model |
| Debug Auth 与 Attestation 的锚点关系 | 影响调试授权链路 | 未完全冻结 | 冻结是否独立 anchor |
| 双算法默认策略 | 影响产品线和测试矩阵 | 未完全冻结 | 冻结产品策略 |

---

## 8.17 开放问题

1. DRK 是否需要在工程文档中显式作为中间对象对外暴露，还是只保留语义层定义？
2. FW Verify 与 Attestation 是否共享部分上游派生材料但逻辑分离，还是完全独立 branch？
3. Attestation 首版是否仅 Device Identity Key 签名就够，还是必须同步规划 Alias Key？
4. 固件验签首版是否只用 OTP signer hash，不携带完整 cert chain？
5. Debug auth 的 anchor 是否和 attestation anchor 完全独立？
6. 除 SEC1/SEC2 外，PM / RAS / Codec 或其他 runtime image 中哪些非敏感镜像允许在特定产品阶段采用 signature-only？

---

## 8.18 本章结论

本章已将 NGU800 的 Root、密钥体系与证书体系收敛到当前可评审的正式口径：

- Root of Trust = eHSM，BootROM 不是密码学根
- UDS / Root Secret 是最上游根材料
- 固件验签、设备证明、调试鉴权必须在逻辑上分 branch
- FW Encrypt Branch 至少对 SEC1 + SEC2 强制启用，SEC1/SEC2 解密必须由 eHSM / 安全子系统受控密码服务完成
- 私钥不得离开 eHSM
- signer hash / trust anchor / cert chain 需要按项目首版策略冻结
- 国密与国际算法必须在结构层共存
- 制造阶段必须定义 key 注入、锁定、清理和生命周期推进动作

后续若 `efuse_key_fw_header_design.md`、`spdm_report.md`、`manufacturing_provisioning.md`、`mailbox_if.md` 冻结字段变更，本章必须同步更新。
---
# 9. 制造、灌装、部署与 RMA


> 文档定位：NGU800 / NGU800P 章节级正式详设
> 章节文件：`security_workflow/03_detailed_design/07_manufacturing_rma.md`
> 当前状态：V1.0（基于当前约束、baseline 与实现级制造设计收敛）
> 设计标记口径：`[CONFIRMED] / [ASSUMED] / [TBD]`

---

## 9.1 本章目标

本章定义 NGU800 在制造、灌装、量产冻结、部署与返修（RMA）阶段的安全设计，重点明确：

1. 制造阶段与生命周期状态的映射关系
2. Root / UDS / signer anchor / debug anchor / attestation anchor / counter 的灌装对象与顺序
3. SEC1 强制加密所需 FW_KEK / image protect key 的灌装、锁定和 USER 前冻结要求
4. OTP / eFuse 写入、校验、锁定、审计的控制要求
5. MANU → USER 的冻结动作集合
6. 量产部署后的状态约束
7. RMA / DEBUG 场景下的授权、调试、恢复与重新冻结规则
8. 与实现层文件的映射关系：
   - `04_impl_design/manufacturing_provisioning.md`
   - `04_impl_design/efuse_key_fw_header_design.md`
   - `04_impl_design/mailbox_if.md`
   - `04_impl_design/spdm_report.md`

---

## 9.2 生效约束 ID

- [C-ROOT-01](../01_constraints.md#c-root-01) - Root of Trust 必须由 eHSM 承载
- [C-KEY-01](../01_constraints.md#c-key-01) - 私钥不得导出
- [C-KEY-02](../01_constraints.md#c-key-02) - Key 使用必须绑定 lifecycle
- [C-DEBUG-01](../01_constraints.md#c-debug-01) - USER 态默认关闭未授权调试
- [C-DEBUG-02](../01_constraints.md#c-debug-02) - DEBUG / RMA 必须认证、限权、审计
- [C-HOST-01](../01_constraints.md#c-host-01) - Host 不可信，只能投递镜像、请求服务、读取结果
- [C-ATT-01](../01_constraints.md#c-att-01) - 必须支持设备认证与远程度量证明
- [C-UPDATE-01](../01_constraints.md#c-update-01) - 必须支持防回滚
- [C-BOOT-04](../01_constraints.md#c-boot-04) - SEC1 / SEC2 正式路径必须签名 + 加密
- [C-MFG-01](../01_constraints.md#c-mfg-01) - 必须定义 Root Key 灌装与锁定流程
- [C-ACCESS-01](../01_constraints.md#c-access-01) - 安全子系统资源必须隔离

---

## 9.3 生效 Baseline 决策

### 9.3.1 制造控制面
- `[CONFIRMED]` Provisioning 流程必须经 SEC/C908 控制面收敛
- `[CONFIRMED]` eHSM 是唯一 Root 材料写入、锁定、lifecycle 状态变更的安全执行面
- `[CONFIRMED]` Host / BMC / 工站不进入信任链，只是链路承载者或受控请求发起者

### 9.3.2 量产冻结
- `[CONFIRMED]` 进入 USER 前必须完成 secure boot、anti-rollback、debug 关闭、测试 trust 清理
- `[CONFIRMED]` Root / signer / debug / attestation 相关敏感对象必须完成锁定
- `[CONFIRMED]` SEC1 / SEC2 解密相关 eHSM key policy / FW_KEK 策略 / signer anchor / eHSM Version Counter 或 owner-confirmed rollback policy 必须在 USER 前锁定
- `[TBD]` exact eHSM key ID、control bit、Version Counter 到 NGU logical rollback domain 的映射必须由 source-conformance matrix 跟踪，不得在制造章节自行冻结
- `[CONFIRMED]` USER 生命周期不得默认开放未经授权的调试路径

### 9.3.3 RMA / DEBUG
- `[CONFIRMED]` 返修或 DEBUG 场景必须经过授权
- `[CONFIRMED]` 调试开启必须通过 challenge-response 或等价鉴权机制
- `[ASSUMED]` RMA 处理完成后，应恢复量产安全状态并形成审计闭环

---

## 9.4 设计要求

### 9.4.1 本章必须回答的问题

1. 制造阶段到底写哪些对象、按什么顺序写？
2. Root / UDS / signer / debug / attestation / counter 之间的先后关系是什么？
3. FW_KEK / image protect key 与 SEC1 signer anchor / rollback counter 如何在 USER 前锁定？
4. MANU 验证启动要检查哪些项目？
5. USER 冻结时必须关闭或清理哪些对象？
6. 量产出厂后哪些状态必须可被证明？
7. RMA / DEBUG 如何合法开启，又如何恢复？
8. 审计日志至少要记录哪些事件？
9. 制造工具、Host/BMC、SEC、eHSM 的职责边界在哪里？

### 9.4.2 不得违反的边界

- 不得允许工站或 Host 直接操作 eHSM 私有执行面
- 不得允许 Root / UDS / 私钥明文以普通软件资产形式长期存在
- 不得在 USER 生命周期保留测试 signer / 测试 cert / 测试 debug 白名单
- 不得在 USER 生命周期保留 SEC1/SEC2 解密绕过策略或可被普通软件关闭的 SEC1/SEC2 decrypt_required
- 不得在失败时报告“USER 冻结完成”
- 不得把 RMA/DEBUG 当成长期常开模式

---

## 9.5 架构图

```mermaid
graph TD
    HSM[Factory HSM / KMS] --> TOOL[Provisioning Tool / 工站]
    TOOL --> HOST[Host / BMC / 工装链路]
    HOST --> SEC[SEC / C908]
    SEC --> EH[eHSM]
    EH --> OTP[OTP / eFuse]
    EH --> CFG[Control Bits / Lifecycle / Counter]
    EH --> KEY[Root / Anchor / Attestation / Debug Objects]

    SEC --> BOOT[MANU 验证启动]
    BOOT --> USER[USER 冻结]
    USER --> DEPLOY[量产部署]
    USER --> RMA[RMA / DEBUG 授权返修]
    RMA --> RESTORE[恢复量产安全状态]
```

### 图下说明

1. 工厂 HSM/KMS 是制造密钥材料的上游管理端，但不直接替代设备内部 Root of Trust。
2. SEC/C908 是制造流程的控制面，eHSM 是真正执行 Root / OTP / lifecycle / lock 操作的安全执行面。
3. USER 冻结不是单条命令，而是一组必须全部成功的冻结动作集合。
4. RMA 路径是受控旁路，只能临时开放，并且必须回收。

---

## 9.6 时序图

```mermaid
sequenceDiagram
    participant TOOL as Provisioning Tool
    participant HOST as Host/BMC
    participant SEC as SEC/C908
    participant EH as eHSM
    participant OTP as OTP/eFuse

    TOOL->>HOST: 下发 provisioning 计划与 blob
    HOST->>SEC: 受控转发 provisioning 请求
    SEC->>SEC: 参数白名单检查 / lifecycle 检查
    SEC->>EH: PROVISION_ROOT_MATERIAL / 写 signer / 写 debug anchor / 写 counter
    EH->>OTP: 写入目标区
    EH-->>SEC: 写入结果
    SEC->>EH: 校验 / 锁定 / 读状态
    EH-->>SEC: verify / lock result
    SEC->>SEC: 执行 MANU 验证启动
    SEC->>EH: CHANGE_LIFECYCLE(USER)
    EH->>OTP: 更新 lifecycle / lock 路径
    EH-->>SEC: USER freeze result
    SEC-->>HOST: 返回冻结结果与审计状态
    HOST-->>TOOL: 工站记录结果
```

### 图下说明

1. Provisioning Tool 不直接向 eHSM 发命令，而是通过 Host/BMC 链路与 SEC/C908 协作。
2. 生命周期推进前，必须先完成写入校验和锁定。
3. USER 冻结前必须先做 MANU 验证启动，确保量产条件已满足。

---

## 9.7 生命周期与制造阶段映射

### 9.7.1 生命周期总体映射

| 生命周期 | 制造/部署语义 | 典型用途 | 默认安全策略 |
|---|---|---|---|
| TEST | 裸片 / 封测 / 初测 | 基础功能验证 | 可开放测试路径，不得等价于量产 |
| DEVE | 开发板 / EVB 调试 | 软件 bring-up、接口联调 | 可有限开放调试 |
| MANU | 正式制造 / 板级灌装 | 写入根材料、建立量产前状态 | 基础安全校验生效 |
| USER | 量产交付 | 面向客户部署 | 强制 secure boot、关闭未授权 debug |
| DEBUG / RMA | 受权返修 / 厂商分析 | 故障定位、临时调试 | 仅授权开启 |
| DEST | 销毁 | 退役 / 擦除 | 不再允许正常使用 |

### 9.7.2 制造阶段推荐细分

| 阶段 ID | 阶段名称 | 主要动作 |
|---|---|---|
| MFG-0 | 封测初测 | 测试路径、基础 bring-up、早期健康检查 |
| MFG-1 | 板级 bring-up | 电源、时钟、基础接口、主从Die连通 |
| MFG-2 | Provisioning 准备 | 工站认证、算法栈选择、blob 准备 |
| MFG-3 | Root / Anchor Provisioning | 写 UDS / Root / signer / debug / attest |
| MFG-4 | Control Bit Provisioning | 写 secure boot / debug / attestation / rollback 控制位 |
| MFG-5 | 校验与锁定 | 写入校验、锁位、状态确认、审计 |
| MFG-6 | MANU 验证启动 | 带验证的近量产启动检查 |
| MFG-7 | USER 冻结 | 清理测试 trust、推进 USER、关闭未授权 debug |
| MFG-8 | 出厂验收 | 形成最终记录、出厂状态确认 |

---

## 9.8 灌装交付项

### 9.8.1 必须灌装对象

| 对象 | 是否首版必须 | 说明 |
|---|---|---|
| UDS / Root Secret | 是 | 根种子 / Root 材料上游 |
| Root Key / Root KEK 材料 | 视模式 | 可直接写入，或由 UDS 内部派生 |
| FW Signer Hash / Trust Anchor | 是 | 支撑 SEC1 / SEC2 / 运行期 FW 验签 |
| FW_KEK / Image Protect Key | 是 | 支撑 SEC1 强制加密镜像的 CEK unwrap / 解密策略 |
| Debug Auth Anchor | 是 | 支撑 DEBUG/RMA 调试鉴权 |
| Attestation Seed / Anchor | 是 | 支撑设备证明 |
| Rollback Counter 初值 / 版本地板 | 是 | 支撑 anti-rollback |
| Secure Boot / Debug / Attestation / Rollback 控制位 | 是 | 建立量产策略 |
| Board Binding 信息 | 可选 | 视产品线策略启用 |
| Die Binding 信息 | 双Die 推荐 | 主从Die 一致性约束 |

### 9.8.2 禁止残留的对象

| 对象 | 原因 |
|---|---|
| 测试 signer key / 测试 cert anchor | USER 前必须清理 |
| 测试 debug 白名单 | USER 前必须清理 |
| 非量产 bypass 配置 | USER 前必须关闭 |
| 明文可导出的 Root 私钥 | 根本不允许存在于最终流程 |

---

## 9.9 灌装顺序设计

### 9.9.1 推荐顺序

```text
(1) 读取 lifecycle 与 OTP 当前状态
    ↓
(2) 校验设备处于允许 provisioning 的状态
    ↓
(3) 写入 UDS / Root Secret / Root 材料
    ↓
(4) 写入 FW signer hash / trust anchor
    ↓
(5) 写入 FW_KEK / image protect key 策略或其 eHSM 内部派生种子
    ↓
(6) 写入 Debug auth anchor
    ↓
(7) 写入 Attestation seed / anchor
    ↓
(8) 写入 counter 初值 / rollback floor
    ↓
(9) 写入 secure boot / FW encrypt / debug / attestation / rollback 控制位
    ↓
(10) 校验写入结果
    ↓
(11) 锁定 key / FW_KEK 策略 / anchor / control bits
    ↓
(12) 执行 MANU 验证启动
    ↓
(13) 执行 USER 冻结
```

### 9.9.2 顺序理由

- Root 材料必须先于 signer / attest / debug anchor 生效，否则没有可信根。
- counter 初值必须在正式量产前建立，否则 anti-rollback 没有基线。
- control bits 必须在信任锚完成注入后再打开，避免出现“策略已要求 secure boot，但锚尚未就绪”的中间态。
- 锁定位只能在校验通过后执行，否则可能把错误数据永久锁死。

---

## 9.10 Provisioning 接口设计口径

### 9.10.1 项目接口裁决

- `[CONFIRMED]` 制造相关动作必须复用 SEC/C908 → eHSM 的受控路径
- `[CONFIRMED]` Provisioning Tool 不得直接进入 eHSM 内部命令面
- `[CONFIRMED]` 制造相关命令只能在允许的 lifecycle 下可用

### 9.10.2 命令族

本章对应的实现级接口，以 `04_impl_design/mailbox_if.md` 为准，核心包括：

- `PROVISION_ROOT_MATERIAL`
- `CHANGE_LIFECYCLE`
- `READ_COUNTER`
- `INCREASE_COUNTER`（受策略控制）
- `GET_CHALLENGE`
- `DEBUG_AUTH`

### 9.10.3 章节级规则

1. `PROVISION_ROOT_MATERIAL` 只能在 MANU 或受控 provisioning 状态可用
2. `CHANGE_LIFECYCLE(USER)` 必须晚于写入校验和锁定
3. `DEBUG_AUTH` 在制造态仅用于必要的 bring-up / RMA，不得作为长期打开调试的替代
4. 所有 provisioning blob 的地址和长度必须受 SEC 白名单和 eHSM 范围检查双重保护

---

## 9.11 校验策略

### 9.11.1 写入后校验

每类 provisioning 写入后，至少需要以下检查：

1. 命令返回状态成功
2. 若目标区允许读回，则做读回一致性校验
3. 若目标区不允许直接读回，则通过：
   - eHSM 内部状态确认
   - 试运行校验
   - challenge / verify / report 侧间接确认
4. 状态必须进入工站审计记录

### 9.11.2 MANU 验证启动最小检查项

| 检查项 | 说明 |
|---|---|
| SEC1 / SEC2 验签 | 核心启动链验证 |
| SEC1 解密 / unwrap | 验证 SEC1 签名 + 加密策略、FW_KEK / wrapped CEK 和输出 buffer 约束 |
| rollback counter 读取 | 反回滚路径验证 |
| lifecycle / control bits 读取 | 状态验证 |
| challenge / report 最小链路 | 证明能力基础验证 |
| debug 默认状态检查 | 验证未授权 debug 未默认放开 |

### 9.11.3 错误处理原则

- `[CONFIRMED]` 任一关键对象写入失败，不得继续推进 USER 冻结
- `[CONFIRMED]` 锁定失败必须视为 provisioning 失败
- `[ASSUMED]` 校验失败后设备可停留在 MANU / 故障态，而不是进入“半冻结 USER”状态

---

## 9.12 锁定策略

### 9.12.1 必须锁定的对象

| 对象 | 锁定时机 | 说明 |
|---|---|---|
| Root / UDS 区 | 写入并校验通过后 | 防止重复覆盖 |
| signer hash / trust anchor 区 | 写入校验后 | 防止验签锚被替换 |
| FW_KEK / image protect key 策略 | 写入校验后 | 防止 SEC1 解密策略被替换或降级 |
| debug auth anchor 区 | 写入校验后 | 防止调试授权根被替换 |
| attestation anchor 区 | 写入校验后 | 防止证明身份根被替换 |
| control bits 区 | USER 冻结前 | 防止量产策略回退 |
| lifecycle 回退路径 | USER 推进后 | 防止回退到开发态 |

### 9.12.2 锁定原则

- `[CONFIRMED]` 锁定动作必须显式执行，不得假设“默认已锁”
- `[CONFIRMED]` 锁定结果必须可审计
- `[CONFIRMED]` 锁定失败不得推进生命周期
- `[ASSUMED]` 若部分区支持一次性写入后天然只读，仍需在工程文档中显式标记为“已锁语义”

---

## 9.13 USER 冻结动作

### 9.13.1 必须完成的动作集合

进入 USER 前，必须完成：

1. `SECURE_BOOT_EN = 1`
2. `DEBUG_AUTH_EN = 1`
3. `JTAG_FORCE_DISABLE = 1`
4. `ANTI_ROLLBACK_EN = 1`
5. `FW_ENCRYPT_EN = 1`，且至少覆盖 SEC1 + SEC2
6. Root / signer / debug / attestation / SEC1/SEC2 解密相关 eHSM key policy / FW_KEK 策略完成锁定
7. 测试 signer / 测试 cert / 测试 debug 白名单全部清理
8. 如启用 attestation，则 `ATTEST_EN = 1`
9. 推进 lifecycle 到 USER
10. 锁定 lifecycle 回退路径
11. 生成冻结完成的审计记录

### 9.13.2 事务性要求

这些动作不一定由单条命令完成，但在流程语义上必须视为**一个事务性步骤集合**：

- 任何一步失败，都不得报告“USER 冻结成功”
- 失败后必须进入 MANU 故障处理流程
- 不得形成“部分已冻结、部分未冻结”的不可解释状态

---

## 9.14 部署阶段设计

### 9.14.1 量产部署后的默认状态

在 USER 生命周期下，默认应满足：

| 项目 | 期望状态 |
|---|---|
| Secure Boot | 开启 |
| SEC1 Image Protection | 签名 + 加密强制开启 |
| Anti-Rollback | 开启 |
| 未授权 Debug | 关闭 |
| 测试 Signer / Trust | 已清除 |
| Attestation | 按产品策略开启 |
| Provisioning 接口 | 关闭 |
| Lifecycle 回退 | 不允许 |

### 9.14.2 部署后可允许的操作

- 受控 firmware update
- attestation / report generation
- 状态查询
- 受策略控制的 challenge / 认证类请求

### 9.14.3 部署后禁止的操作

- 再次 provisioning Root 材料
- 覆盖 signer anchor
- 恢复测试 trust
- 直接打开 debug
- 回退 lifecycle 到开发态

---

## 9.15 RMA 规则

### 9.15.1 RMA 基本原则

RMA / DEBUG 不是普通用户态能力，而是：

> **受授权、可审计、可恢复的返修旁路**

必须满足：

1. 先鉴权，后开放
2. 权限受 scope 和时间窗口约束
3. 维修后必须恢复量产安全状态
4. 全程可审计

### 9.15.2 推荐流程

```text
接收返修设备
    ↓
校验工单 / 设备身份 / 厂商授权
    ↓
发起 challenge / debug auth
    ↓
在受限 scope 下开放调试能力
    ↓
读取故障信息 / 维修 / 受控刷写恢复
    ↓
重新恢复量产镜像和状态
    ↓
关闭调试能力
    ↓
恢复 USER 安全状态
    ↓
生成 RMA 审计结案记录
```

### 9.15.3 RMA 约束

- `[CONFIRMED]` 不得因为进入 RMA 就长期常开 debug
- `[CONFIRMED]` 不得跳过 challenge / auth 直接开调试口
- `[CONFIRMED]` 返修完成后不得带着测试 trust 或开放调试出厂
- `[CONFIRMED]` RMA 不得长期开放 SEC1/SEC2 解密绕过路径；rescue / recovery 镜像必须使用专用 signer / recovery trust，并保持 eHSM 受控解密或受控 recovery policy
- `[ASSUMED]` RMA 完成后，建议重新生成与当前状态一致的最小 report / status 记录，用于归档

---

## 9.16 审计模型

### 9.16.1 必须记录的事件

| Audit Event | 至少应记录的内容 |
|---|---|
| PROVISION_START | 设备 ID、工站 ID、时间、操作员、工单 |
| ROOT_WRITE | 写入对象类型、target slot、结果 |
| FW_KEK_WRITE | FW_KEK / image protect key 策略写入、slot、结果 |
| ANCHOR_WRITE | signer/debug/attest anchor 类型、结果 |
| CTRL_BITS_WRITE | 控制位变化前后、结果 |
| LOCK_APPLY | 锁定对象、结果 |
| MANU_BOOT_VERIFY | 验证启动结果、错误码 |
| USER_FREEZE | lifecycle 变化前后、结果 |
| RMA_AUTH | challenge / auth 结果、scope |
| RMA_CLOSE | 恢复状态、结案结果 |
| PROVISION_END | 总结果、日志索引 |

### 9.16.2 审计要求

- `[CONFIRMED]` 审计日志不得记录明文私钥或 Root 材料
- `[CONFIRMED]` 审计日志必须至少可关联：
  - 设备
  - 工站
  - 时间
  - 操作员 / 工单
  - 结果
- `[ASSUMED]` 审计日志应支持导出到制造后台系统或至少可离线归档

---

## 9.17 与其他章节 / 实现层的映射关系

| 本章主题 | 对应章节 / 实现层 |
|---|---|
| lifecycle 与冻结语义 | `03_detailed_design/04_lifecycle_debug.md` |
| Root / signer / control bits 灌装对象 | `04_impl_design/efuse_key_fw_header_design.md` |
| Provisioning 命令与 req/resp | `04_impl_design/mailbox_if.md` |
| report 中 lifecycle / debug / board 状态表达 | `04_impl_design/spdm_report.md` |
| 具体流程状态机 / 审计要求 | `04_impl_design/manufacturing_provisioning.md` |

---

## 9.18 冻结敏感项

| Item | Why Sensitive | Current Status | Needed Before Freeze |
|---|---|---|---|
| Root 注入模式（直接 Root vs Seed/UDS） | 影响制造链安全暴露面 | 部分收敛 | 冻结首版模式 |
| runtime signature-only 白名单 | 影响 FW_KEK 规划和量产镜像封装 | 未完全冻结 | SEC2 已强制加密；冻结除 SEC1/SEC2 外的 signature-only 白名单 |
| OTP 是否支持读回校验 | 影响校验策略 | 未完全冻结 | 冻结可读回区和不可读回区策略 |
| Provisioning 链路承载方式 | 影响工站 / Host / BMC 选型 | 未完全冻结 | 冻结首版工装路径 |
| 双Die 灌装是否联动事务 | 影响 OAM / 双Die 产品制造 | 未完全冻结 | 冻结联动策略 |
| RMA 恢复后是否强制再验收 | 影响售后流程与安全闭环 | 未完全冻结 | 冻结返修交付规则 |

---

## 9.19 开放问题

1. 首版是否完全采用“Seed/UDS 注入 + eHSM 内部派生”，还是保留直接 Root 材料写入模式？
2. 不可读 OTP 区域的校验策略最终采用“状态确认”还是“试运行校验”？
3. BMC / OOB-MCU 在某些产品形态下是否允许承担 provisioning 桥接角色？
4. 双Die 产品是按单设备事务灌装，还是主/从 Die 分步灌装？
5. RMA 结束后，是否要求强制重新生成 attestation / 状态摘要并归档？
6. PM / RAS / Codec 或其他 runtime image 中哪些非敏感镜像允许进入 signature-only 白名单？

---

## 9.20 本章结论

本章已将 NGU800 的制造、灌装、部署与 RMA 路径收敛到当前可评审的正式口径：

- 制造必须通过 SEC/C908 控制面与 eHSM 安全执行面完成
- UDS / Root / signer / debug / attestation / eHSM Version Counter / control field 的灌装顺序必须固定
- SEC1 / SEC2 解密相关 eHSM key policy / FW_KEK 策略 / signer anchor / rollback policy 必须在 USER 前锁定
- 锁定、校验和 USER 冻结必须显式化、事务化、可审计
- 量产部署后必须保持 secure boot、anti-rollback、未授权 debug 关闭和测试 trust 清理
- RMA 是受授权、可恢复、可审计的旁路，不得成为常开调试模式

后续若 `manufacturing_provisioning.md`、`mailbox_if.md`、`efuse_key_fw_header_design.md` 或生命周期策略冻结字段变化，本章必须同步更新。
---
# 10. 实现级落地详设全集

> 本章是 CR-0005 后的代码落地完整实现级详设。
> `security_workflow/04_impl_design/*.md` 的有效正文已按原始文件全量嵌入本章，仅调整标题层级以适配 `10_full_design.md` 的章节编号。
> 若本章与 `04_impl_design` 分片发生冲突，以 accepted CR、`00_project/decision_log.md`、official TRM 和本章为准；分片文档不得覆盖本章。

## 10.1 本章使用规则

| 规则 | 说明 |
|---|---|
| 代码落地入口 | Firmware、driver、工具链、测试和验证计划优先读取本章。 |
| 分片角色 | `04_impl_design` 保留为编辑分片 / extracted implementation shard，便于局部维护和 Review，不作为独立事实源。 |
| 同步要求 | 修改 `04_impl_design` 分片时，必须同步回本章；修改本章实现级字段时，也必须同步对应分片或显式记录分片待同步。 |
| 安全裁决优先级 | accepted CR / decision_log / official TRM / 本章 / 分片文档。 |
| TBD 保留 | 合并动作不关闭 `[TBD]`，不把未冻结字段升级为 `[CONFIRMED]`。 |
| 字段冻结 | C-like 结构、command ID、OTP/control/key/counter mapping 只有在 accepted CR、decision_log 或官方资料支持时才可作为冻结字段。 |

### 10.1.1 嵌入来源

| 本章小节 | 嵌入来源 | 说明 |
|---|---|---|
| 10.2 | `security_workflow/04_impl_design/README.md` | 04_impl_design README 与事实源说明 |
| 10.3 | `security_workflow/04_impl_design/efuse_key_fw_header_design.md` | eFuse / Key / eHSM FW Header 对齐设计 |
| 10.4 | `security_workflow/04_impl_design/ehsm_source_conformance_matrix.md` | eHSM Source-Conformance Matrix |
| 10.5 | `security_workflow/04_impl_design/mailbox_if.md` | Mailbox Interface 实现级设计 |
| 10.6 | `security_workflow/04_impl_design/manufacturing_provisioning.md` | Manufacturing / Provisioning 实现级设计 |
| 10.7 | `security_workflow/04_impl_design/spdm_report.md` | SPDM / Attestation Report 实现级设计 |

<!-- Embedded from security_workflow/04_impl_design/README.md; keep full content synchronized. -->
## 10.2 04_impl_design

本目录存放实现级详设编辑分片 / extracted implementation shards。

CR-0005 后，代码落地、评审和 ChatGPT 方案审查的完整详设主入口是：

```text
security_workflow/03_detailed_design/10_full_design.md
```

本目录下的分片正文必须全量同步到 `10_full_design.md` 第 10 章。若本目录分片与 `10_full_design.md` 冲突，以 accepted CR、`00_project/decision_log.md`、official TRM 和 `10_full_design.md` 为准；分片不得作为独立事实源覆盖主详设。

<!-- Embedded from security_workflow/04_impl_design/efuse_key_fw_header_design.md; keep full content synchronized. -->
## 10.3 NGU800 eFuse / Key / eHSM FW Header 对齐设计（CR-0004/CR-0006 Applied）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级详设（CR-0004 / CR-0006 accepted-apply 后修订）
适用范围：NGU800 / NGU800P 安全启动、密钥体系、反回滚、制造灌装
主要来源：`CR-0004`、`CR-0006`、`SRC-001 当前安全方案基线` 第 6/7 章、`SRC-006 eHSM Firmware TRM`、`SRC-007 eHSM Bootloader TRM`

---

### 10.3.1 设计目标

本文档不再定义一套与 eHSM 并列的 physical FW Header、physical OTP/eFuse layout 或 physical key slot。

本文档收敛以下对象：

1. eHSM native secure boot image header 的项目使用方式
2. NGU 项目级 manifest / logical policy 与 eHSM native header 的分层
3. eHSM OTP/control/key/counter 与 NGU logical alias 的映射
4. SEC1 / SEC2 sign+encrypt 的 verify+decrypt output path
5. 平台侧固件制作、设备侧 verify/decrypt 与 NGU manifest policy 的落地流程
6. CR-0004 / CR-0006 后续仍需冻结的 eHSM customization / ABI / key ID / OTP bit / tooling 问题

---

### 10.3.2 Source-Conformance 原则

### 10.3.3 eHSM-native 优先

`SRC-006 eHSM Firmware TRM` 与 `SRC-007 eHSM Bootloader TRM` 已定义的字段均作为 physical fact：

- secure boot image header
- `Image_Type / Plain_Flag / Naked_Flag / Version_Counter`
- `SocBootAlg / SocUpgradeAlg` 等 control field
- OTP key ID / level / purpose
- SOC FW / eHSM FW Version Counter
- `bl_verify_image / soc_verify / fw_upgrade` 等命令语义

### 10.3.4 NGU 字段分类

任何 NGU 字段必须归入以下类别之一：

| Mapping Type | 含义 |
|---|---|
| `eHSM-native` | eHSM TRM 已定义的 physical field / command |
| `NGU-logical-alias` | NGU 文档阅读视图，不表达 physical offset / key slot |
| `manifest-extension` | 放入 eHSM Code region 的 NGU protected manifest |
| `eHSM-customization-TBD` | 需要 eHSM owner / TRM / firmware customization 支撑 |
| `NGU-SoC-integration-TBD` | 属于 SoC/RTL/board 集成字段，非 eHSM physical field |

---

### 10.3.5 eHSM Native Secure Boot Image Header

### 10.3.6 Header 布局

`[CONFIRMED]` SEC1 / SEC2 的密码学 verify/decrypt container 采用 eHSM native secure boot image header。

| eHSM Field | Offset | Size | Mapping Type | NGU 使用规则 |
|---|---:|---:|---|---|
| `Signature` | 0 | 256 | `eHSM-native` | 签名/MAC 值，长度由 eHSM control field 的算法选择决定 |
| `Public_Key` | 256 | 320 | `eHSM-native` | 非对称算法公钥字段；CMAC 模式无效 |
| `Encrypt_IV` | 576 | 16 | `eHSM-native` | AES/SM4 CBC/CMAC 相关 IV；具体语义按 eHSM TRM |
| `Valid_Flag` | 592 | 4 | `eHSM-native` | eHSM image valid marker |
| `Image_Type` | 596 | 1 | `eHSM-native` | eHSM TRM 定义，不写入 NGU SEC1/SEC2/runtime 编码 |
| `Plain_Flag` | 597 | 1 | `eHSM-native` | Code region 是否明文；SEC1/SEC2 正式路径必须为密文 profile |
| `Naked_Flag` | 598 | 1 | `eHSM-native` | 裸镜像仅用于 TRM 允许的开发生命周期 |
| `Reserved` | 599 | 5 | `eHSM-native` | 必须按 TRM 置 0；不得擅自承载 NGU 字段 |
| `Code_Size` | 604 | 4 | `eHSM-native` | Code region 大小 |
| `Version_Counter` | 608 | 16 | `eHSM-native` | eHSM 128-bit one-way counter |
| `Public_Key_Ext` | 624 | 400 | `eHSM-native` | RSA3072 扩展字段 |
| `Code` | 1024 | `Code_Size` | `eHSM-native` | 明文或密文 Code region；NGU manifest 放在此区域 |

### 10.3.7 旧 NGU Header 状态

以下旧结构不再作为 physical wire/storage format：

- `ngu_fw_min_hdr_t`
- `ngu_fw_signed_hdr_t`

它们只能作为历史草案或工具侧中间结构参考；任何代码、ROM、SEC、Host 工具或文档不得把它们作为安全启动镜像的 physical verification header。

---

### 10.3.8 NGU Protected Manifest

### 10.3.9 放置原则

`[CONFIRMED]` NGU 项目级 metadata 放入 eHSM Code region 的 protected manifest / policy table。

推荐布局：

```text
[eHSM Native Image Header, 1KB, plaintext]

[Code region, verified/decrypted by eHSM]
  ngu_image_manifest_t or equivalent manifest extension
  actual firmware payload
```

### 10.3.10 Manifest 语义

manifest 可承载以下 NGU 项目级逻辑：

| Manifest Concept | Status | 说明 |
|---|---|---|
| `ngu_image_type` | `[CONFIRMED]` | NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 项目级类型 |
| `security_policy_flags` | `[CONFIRMED]` | sign/encrypt/rollback/measurement/release policy 的项目表达 |
| `rollback_domain` | `[CONFIRMED]` | NGU logical rollback domain，不是 physical OTP 32-bit counter |
| `measurement_slot` | `[CONFIRMED]` | measurement / attestation slot |
| `lifecycle_mask` | `[CONFIRMED]` | NGU policy gate；最终生命周期 authority 仍在 eHSM/OTP |
| `product_sku_mask` | `[ASSUMED]` | 产品策略 |
| `board_binding_policy` | `[ASSUMED]` | board binding 默认进入 attestation；是否参与 release 仍 TBD |
| `expected_algorithm_profile` | `[CONFIRMED]` | 仅做一致性检查/审计，不覆盖 eHSM control field |
| `payload_digest` | `[ASSUMED]` | 用于 manifest 内 payload 描述；具体 hash profile 跟随 eHSM/产品策略 |

### 10.3.11 Manifest ABI 开放项

以下内容保持 `[TBD]`：

1. `ngu_image_manifest_t` bit-level ABI。
2. manifest 是否必须位于 Code region 起始位置。
3. manifest extension / TLV 格式。
4. eHSM firmware / bootloader 是否直接解析 manifest。
5. manifest 中 board binding 是否参与 SEC2/runtime release decision。

#### 固件包格式与制作/验证流程（CR-0006）

##### 旧 current_plan 格式的处理

`SRC-001 当前安全方案基线` 第 6/7 章中描述的：

```text
header + Signed Region + signature + wrapped_cek + enc_payload
```

在 CR-0006 后只作为“制作和验证流程意图”的参考，不作为 NGU800 最终 physical wire/storage format。

CR-0004 / CR-0006 后的正式包格式是：

```text
[eHSM Native Secure Boot Image Header, 1KB plaintext]

[eHSM Code region, verified/decrypted by eHSM]
  NGU protected manifest / policy table
  actual firmware payload
```

规则：

- eHSM native header 是唯一 physical verification/decrypt header。
- NGU 不再定义 `common_firmware_header` / `signed_region_v1` 作为设备侧 wire/storage ABI。
- Signed Region 中原本想表达的 `firmware_type / version / rollback / key_id / payload_hash / policy`，必须迁移到 eHSM native field、NGU protected manifest 或 eHSM owner-confirmed key/counter mapping。
- `wrapped_cek` 仅在 eHSM owner 后续确认 per-image CEK / wrap extension 时才可作为 `eHSM-customization-TBD` 进入实现；当前不得被工具链写死。

##### 固件包布局图

```mermaid
flowchart LR
    subgraph PACKAGE[eHSM Native Package]
        HDR[eHSM Native Header<br/>Signature / Public_Key / Encrypt_IV<br/>Valid_Flag / Image_Type / Plain_Flag<br/>Code_Size / Version_Counter]
        CODE[Code Region<br/>covered by eHSM verify/decrypt]
    end

    subgraph CODE_REGION[Code Region Contents]
        MAN[NGU Protected Manifest<br/>image_type / policy / rollback_domain<br/>measurement_slot / lifecycle_mask / digest]
        PAY[Actual Payload<br/>SEC1 / SEC2 / PM / RAS / Codec]
    end

    HDR --> CODE
    CODE --> MAN
    CODE --> PAY
```

图下说明：

1. `Image_Type / Plain_Flag / Version_Counter / Code_Size` 由 eHSM TRM 定义。
2. `ngu_image_type / security_policy_flags / rollback_domain / measurement_slot` 由 NGU manifest 表达。
3. payload digest 应由 manifest 或 eHSM owner-confirmed metadata 表达；具体 hash profile 跟随 eHSM control field / product profile。
4. load address / entry point 若进入 manifest，必须等 eHSM PASS 后才能被 BootROM / SEC 使用。

##### 平台侧固件制作流程

```mermaid
flowchart TD
    A[输入 payload / image class / product profile] --> B[生成 NGU protected manifest]
    B --> C[manifest + payload 组成 Code region]
    C --> D[选择 eHSM boot/upgrade profile<br/>SocBootAlg / SocUpgradeAlg / key purpose / counter]
    D --> E[调用 eHSM image packaging<br/>或 owner-confirmed 等价工具]
    E --> F[生成 native header + protected Code region]
    F --> G[发布前 conformance check]
    G --> H[输出正式固件包]
```

平台侧制作规则：

| Step | 工具动作 | 输出 / 检查 |
|---|---|---|
| 1 | 读取 payload、image class、版本、产品 profile | 明确 `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 的 NGU 项目级类型 |
| 2 | 生成 NGU protected manifest | `ngu_image_type`、policy、rollback domain、measurement slot、lifecycle mask、expected algorithm profile、payload digest |
| 3 | 拼接 eHSM Code region | `manifest + payload`，manifest 必须处于 verify/decrypt 保护范围 |
| 4 | 选择 eHSM profile | 算法 authority 来自 eHSM control field；工具只能选择 owner-confirmed profile |
| 5 | 生成 eHSM native header | Header 字段必须逐项通过 source-conformance matrix |
| 6 | 执行签名/加密封装 | 按 eHSM TRM / owner-confirmed tool 完成，不由 NGU 工具发明 physical crypto metadata |
| 7 | 发布前检查 | 检查 `Image_Type` 未误用、`Plain_Flag` 符合 sign+encrypt policy、Version Counter / rollback domain 一致、manifest 可解析 |

##### 设备侧 verify/decrypt 流程

```mermaid
sequenceDiagram
    participant BOOT as BootROM/SEC
    participant EHSM as eHSM
    participant PKG as eHSM Native Package
    participant OUT as Controlled Output Buffer
    participant MAN as NGU Manifest

    BOOT->>PKG: locate package
    BOOT->>BOOT: address whitelist / output buffer check
    BOOT->>EHSM: bl_verify_image or soc_verify
    EHSM->>PKG: parse native header
    EHSM->>EHSM: check Image_Type / Plain_Flag / Version_Counter / Code_Size
    EHSM->>EHSM: verify signature / key / revoke / rollback
    EHSM->>OUT: verify+decrypt Code region
    EHSM-->>BOOT: PASS or FAIL
    alt PASS
        BOOT->>OUT: parse protected manifest
        BOOT->>MAN: check NGU image type / lifecycle / policy / measurement slot
        MAN-->>BOOT: policy result
        BOOT->>BOOT: measure and controlled release
    else FAIL
        BOOT->>BOOT: record error and block release
    end
```

设备侧规则：

1. BootROM / SEC 不得在 eHSM PASS 前信任 NGU manifest。
2. BootROM / SEC 不得自行执行正式安全路径签名验证、复杂解密或 key unwrap。
3. SEC1 / SEC2 必须走 eHSM verify+decrypt output path；NVM only verify 不适用于 sign+encrypt 镜像。
4. output buffer / destination address 必须由 BootROM / SEC 预先白名单，eHSM 侧再次检查。
5. eHSM PASS 后，BootROM / SEC 才能解析 NGU manifest 并执行项目级 policy：
   - `ngu_image_type` 与当前启动阶段匹配；
   - `security_policy_flags` 满足 SEC1/SEC2 mandatory sign+encrypt；
   - `rollback_domain` 与 eHSM Version Counter / owner-confirmed rollback policy 一致；
   - `lifecycle_mask` 允许当前 lifecycle；
   - `measurement_slot` 合法；
   - `expected_algorithm_profile` 与 eHSM control field 不冲突；
   - board binding policy 按 CR-0003/后续 CR 裁决执行。

##### 工具链交付物

首版 image packager 至少应输出以下可审查产物：

| Artifact | 用途 | 状态 |
|---|---|---|
| eHSM native package | 正式发布固件包 | `[CONFIRMED direction]` |
| package manifest dump | 供 review / CI 检查 NGU manifest 语义 | `[ASSUMED]` |
| source-conformance report | 检查 header、manifest、key/counter/profile 来源 | `[ASSUMED]` |
| golden vector | BootROM / SEC / eHSM adapter 联调用例 | `[TBD exact format]` |
| policy check report | 检查 sign+encrypt、rollback、lifecycle、measurement slot | `[ASSUMED]` |

注意：这些工具链交付物不改变 eHSM physical ABI；它们用于工程检查和联调。

---

### 10.3.12 Image Type 分层

| 概念 | 承载位置 | Authority | 状态 |
|---|---|---|---|
| eHSM `Image_Type` | eHSM native header offset 596 | eHSM TRM | `[CONFIRMED]` |
| NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` | NGU protected manifest | NGU SEC/Boot policy | `[CONFIRMED]` |
| eHSM firmware 直接理解 NGU image type | eHSM customization | eHSM owner | `[TBD]` |

规则：

- 不得把 NGU `IMAGE_TYPE_SEC1 / SEC2 / PM / RAS / Codec / Recovery` 直接写成 eHSM native `Image_Type` 编码。
- eHSM `Image_Type` 只用于 eHSM 已定义的镜像/key profile。
- NGU `ngu_image_type` 用于 release policy、measurement slot、runtime image policy 和 attestation 映射。

---

### 10.3.13 Algorithm Authority

`[CONFIRMED]` secure boot / upgrade 的算法 authority 来自 eHSM OTP/control field，例如：

- `EhsmCodeVerifyAlg`
- `EhsmCodeUpgradeAlg`
- `SocBootAlg`
- `SocUpgradeAlg`

规则：

- NGU manifest 可记录 `expected_algorithm_profile`。
- `expected_algorithm_profile` 仅用于一致性检查、审计、attestation。
- 若 manifest expected profile 与 eHSM control field 冲突，必须失败或记录 policy mismatch。
- NGU 不得通过 header/manifest 覆盖 eHSM control field 的算法选择。

---

### 10.3.14 OTP / eFuse Logical View

### 10.3.15 NGU logical view

`[CONFIRMED]` `OTP-0..OTP-7` 仅保留为 NGU logical view，不表达 physical OTP/eFuse offset。

| NGU Logical View | 语义 | Mapping Type | Physical Authority |
|---|---|---|---|
| OTP-0 lifecycle view | lifecycle / lock / destroy state | `NGU-logical-alias` | eHSM lifecycle / control field / SoC integration |
| OTP-1 control view | secure boot / debug / algo / rollback policy | `NGU-logical-alias` | eHSM FW Control / SOC Control / hardware control field |
| OTP-2 root material view | root / UDS / DRK upstream | `NGU-logical-alias` | eHSM OTP key layout / secure storage |
| OTP-3 anchor view | signer / debug / attest anchor | `NGU-logical-alias` | eHSM key ID / purpose / owner-confirmed anchor slot |
| OTP-4 rollback view | rollback domain / version state | `NGU-logical-alias` | eHSM Version Counter / customization TBD |
| OTP-5 identity view | UID / device identity seed | `NGU-logical-alias` | eHSM UID / key layout / attestation design |
| OTP-6 board/die binding view | board / die binding policy | `NGU-SoC-integration-TBD` | SoC/board integration |
| OTP-7 non-secure config view | non-secure readonly config | `NGU-SoC-integration-TBD` | SoC/board integration |

### 10.3.16 Control Fields

旧的 `SECURE_BOOT_EN / DEBUG_AUTH_EN / JTAG_FORCE_DISABLE / FW_ENCRYPT_EN / ATTEST_EN / ANTI_ROLLBACK_EN` 不再写成 NGU 自定义 physical OTP bit。

| NGU Logical Field | Mapping Type | 说明 |
|---|---|---|
| `SECURE_BOOT_EN` | `eHSM-native` / `NGU-SoC-integration-TBD` | 对齐 eHSM / hardware control field |
| `DEBUG_AUTH_EN` | `eHSM-native` / `NGU-SoC-integration-TBD` | 对齐 eHSM debug auth / lifecycle policy |
| `JTAG_FORCE_DISABLE` | `NGU-SoC-integration-TBD` | 依赖 SoC/board JTAG MUX / firewall / lifecycle |
| `FW_ENCRYPT_EN` | `manifest-extension` / policy | SEC1/SEC2 mandatory encrypt 是项目策略，不应伪装为单一 physical bit |
| `ATTEST_EN` | `NGU-logical-alias` / `NGU-SoC-integration-TBD` | 证明服务 enable policy |
| `ANTI_ROLLBACK_EN` | `eHSM-native` / policy | 对齐 eHSM Version Counter / update policy |

---

### 10.3.17 Version Counter / Rollback

`[CONFIRMED]` eHSM TRM 定义 eHSM FW 和 SOC FW Version Counter，长度为 16 字节，采用单向计数语义。

| NGU Old Field | New Status | Physical Mapping |
|---|---|---|
| `SEC1_MIN_VER` | `NGU-logical-alias` | eHSM SOC FW Version Counter / owner-confirmed rollback policy |
| `SEC2_MIN_VER` | `NGU-logical-alias` | eHSM SOC FW Version Counter / owner-confirmed rollback policy |
| `PMP_MIN_VER / RMP_MIN_VER / OMP_MIN_VER / MMP_MIN_VER` | `NGU-logical-alias` | per-image rollback customization TBD |

规则：

- NGU `*_MIN_VER` 不得写成 physical OTP 32-bit counter。
- V1 可使用 eHSM SOC FW Version Counter 作为物理 rollback 基础候选。
- 若要求 SEC1 / SEC2 / PM / RAS / Codec 独立 rollback counter，必须通过 eHSM customization 或 owner-confirmed monotonic counter service 冻结。

---

### 10.3.18 Key Hierarchy / Key Slot Mapping

`[CONFIRMED]` eHSM TRM 已定义 OTP key ID / level / purpose。NGU key name 是 logical alias，不是新增 physical key slot。

| NGU Key Concept | Mapping Type | eHSM Mapping Direction | Status |
|---|---|---|---|
| UDS / Root Secret | `eHSM-native` / logical root | Chip Root Key / Device Root Key / secure storage | `[CONFIRMED]` |
| FW Verify Key | `NGU-logical-alias` | Soc FW Verify Key / Soc Upgrade Verify Key / owner mapping | `[TBD exact ID]` |
| FW Encrypt Key / FW_KEK | `NGU-logical-alias` | Soc Encrypt Key / Soc Upgrade Encrypt Key / owner mapping | `[TBD exact ID]` |
| Debug Auth Key / Seed | `NGU-logical-alias` | Soc Debug Verify Key / User Auth Key / owner mapping | `[TBD exact ID]` |
| Attestation Key / Seed | `NGU-logical-alias` | Soc Private Key / Secret Key / owner mapping | `[TBD exact ID]` |
| Image CEK / wrapped CEK | `eHSM-customization-TBD` | per-image CEK extension requires eHSM support | `[TBD]` |

规则：

- 私钥、root secret、FW_KEK 明文不得离开 eHSM / 受控安全环境。
- exact key ID 不得由 SEC/Host/Tool 自行发明。
- manufacturing provisioning 必须使用 eHSM install / change lifecycle / change control field 等官方或 owner-confirmed command。

---

### 10.3.19 SEC1 / SEC2 Verify+Decrypt Path

### 10.3.20 Deployment Rule

`[CONFIRMED]` SEC1 / SEC2 sign+encrypt 不得使用 eHSM NVM only verify。

| Path | eHSM TRM 语义 | NGU 采用 |
|---|---|---|
| RAM deploy / output buffer | verify + decrypt to RAM / output address | SEC1 / SEC2 mandatory |
| NVM only verify | only verify, image cannot be encrypted | 不适用于 SEC1 / SEC2 sign+encrypt |

### 10.3.21 Command Mapping

| NGU Profile | eHSM Direction | Status |
|---|---|---|
| `VERIFY_SEC1` | eHSM Bootloader `bl_verify_image` 或等价 ROM path | `[ASSUMED]` early boot mapping，需 BootROM/eHSM 集成确认 |
| `VERIFY_IMAGE(SEC2)` | eHSM Firmware `soc_verify` 或 SEC wrapper | `[CONFIRMED direction]` runtime/load direction，接口字段以 mailbox_if 为准 |
| `fw_upgrade` | eHSM upgrade image -> boot image | `[CONFIRMED]` manufacturing/update path reference |

### 10.3.22 Output Buffer Rule

- output buffer / destination address 必须由 BootROM / SEC 预先白名单。
- eHSM 侧必须再次做范围检查。
- Host / OOB / 管理子系统 DMA 默认不得访问 output buffer、SEC1/SEC2 执行区、manifest policy area、measurement_table 安全写区。
- decrypt fail / policy mismatch / address invalid 必须阻断 boot 或 runtime release。

---

### 10.3.23 Manufacturing / Provisioning 对齐

制造灌装必须改为 eHSM command / field mapping：

| Manufacturing Concept | eHSM Mapping | Status |
|---|---|---|
| root / key material install | `install_random_key` / `install_encrypt_key` / owner-confirmed provisioning command | `[CONFIRMED direction]` |
| lifecycle change | `change_lifecycle` | `[CONFIRMED direction]` |
| control field change | `change_control_field` | `[CONFIRMED direction]` |
| version counter init / check | eHSM Version Counter / command policy | `[TBD exact process]` |
| OTP readback validation | eHSM readback/status/attested validation | `[TBD]` |

---

### 10.3.24 Source-Conformance Matrix

字段级 mapping 见：

- `security_workflow/04_impl_design/ehsm_source_conformance_matrix.md`

任何后续涉及 header、OTP、key、counter、manufacturing command 的变更，必须同步更新 matrix。

---

### 10.3.25 Open Issues

| Issue | Status | Owner |
|---|---|---|
| `ngu_image_manifest_t` bit-level ABI | `[TBD]` | Security Owner / SEC FW |
| eHSM firmware / bootloader 是否解析 NGU manifest | `[TBD]` | eHSM Owner |
| exact OTP/control bit mapping | `[TBD]` | eHSM / RTL Owner |
| exact NGU key alias -> eHSM key ID mapping | `[TBD]` | eHSM / Security Owner |
| per-image rollback counter | `[TBD]` | eHSM / Product Security |
| per-image CEK / wrapped CEK | `[TBD]` | eHSM Owner |
| recovery image policy | `[TBD]` | Security Owner |
| board binding release decision | `[TBD]` | Security / Board Owner |
| image packager CLI / golden vector / package conformance report | `[TBD]` | Tooling Owner / SEC FW Owner |

---

### 10.3.26 结论

CR-0004 应用后，本文档的实现级基线为：

- eHSM native secure boot image header 是 SEC1 / SEC2 的密码学 verify/decrypt container。
- NGU 项目级 metadata 进入 Code region protected manifest，不再定义并列 physical header。
- 平台侧固件制作工具和设备侧 verify/decrypt 流程必须共享 eHSM native header + NGU protected manifest 契约。
- eHSM `Image_Type`、算法 control field、Version Counter、OTP key layout 均按 eHSM TRM 作为 physical fact。
- `OTP-0..OTP-7`、`*_MIN_VER`、NGU key names 仅保留为 logical view / logical alias。
- SEC1 / SEC2 sign+encrypt 必须使用 verify+decrypt output path，不使用 NVM only verify。
- 未冻结的 ABI / key ID / counter / wrapped CEK / manifest parser 不得被实现写死。

<!-- Embedded from security_workflow/04_impl_design/ehsm_source_conformance_matrix.md; keep full content synchronized. -->
## 10.4 eHSM Source-Conformance Matrix（CR-0004）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级 source gate
适用范围：FW header、OTP/eFuse、control field、version counter、key slot、secure boot command、manufacturing command
主要来源：`CR-0004`、`SRC-006 eHSM Firmware TRM`、`SRC-007 eHSM Bootloader TRM`

---

### 10.4.1 使用规则

任何实现级 physical field 必须满足以下条件之一：

1. 映射到 eHSM TRM / Host API / Bootloader TRM 已定义字段或命令。
2. 映射到 accepted CR / decision log。
3. 明确标记为 `manifest-extension`、`NGU-logical-alias`、`eHSM-customization-TBD` 或 `NGU-SoC-integration-TBD`。

不得把 Codex 草案、旧 NGU 建议字段或未确认推断写成 `[CONFIRMED] physical field`。

---

### 10.4.2 Header / Image Format

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| Physical secure boot image header | `ngu_fw_min_hdr_t`, `ngu_fw_signed_hdr_t` | eHSM 1KB Image Head | `eHSM-native` | `SRC-006`, `SRC-007`, `CR-0004` | `[CONFIRMED]` | 旧 NGU struct 不再作为 physical wire/storage format |
| Signature | `sig_off / sig_len` | `Signature` offset 0 size 256 | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 长度由 eHSM algorithm control field 决定 |
| Public key | `cert_off / cert_len`, signer fields | `Public_Key`, `Public_Key_Ext` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | cert chain 是否进入 manifest/report 另行冻结 |
| Encryption IV | `nonce_iv_*` | `Encrypt_IV` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 语义按 eHSM TRM |
| eHSM image type | old `image_type` mixed use | `Image_Type` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | 不承载 NGU SEC1/SEC2/runtime 编码 |
| NGU project image type | `IMAGE_TYPE_SEC1/SEC2/...` | NGU manifest `ngu_image_type` | `manifest-extension` | `CR-0004` | `[CONFIRMED]` | release/measurement/attestation 使用 |
| Algorithm fields | `algo_family/hash_algo/sig_algo/enc_algo` | `SocBootAlg/SocUpgradeAlg` or equivalent control field | `eHSM-native` + `manifest-extension` | `SRC-006`, `CR-0004` | `[CONFIRMED]` | eHSM control field 是 authority；manifest 仅 expected profile |
| Code size | `payload_len/ciphertext_len` | `Code_Size` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NGU manifest 可再描述 payload |
| Protected NGU metadata | old signed header fields | Code region manifest | `manifest-extension` | `CR-0004` | `[CONFIRMED] / [TBD ABI]` | manifest ABI 未冻结 |
| per-image wrapped CEK | `wrapped_cek_*` | no confirmed native field | `eHSM-customization-TBD` | `CR-0004` | `[TBD]` | 需 eHSM owner 确认 |

---

### 10.4.3 OTP / Control / Counter

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| OTP logical partition | `OTP-0..OTP-7` | eHSM OTP/control/key/counter layout | `NGU-logical-alias` | `CR-0004` | `[CONFIRMED]` | 不表达 physical offset |
| Lifecycle | `LIFECYCLE_STATE` | eHSM lifecycle / hardware lifecycle field | `eHSM-native` / `NGU-SoC-integration-TBD` | `SRC-006`, `SRC-007` | `[CONFIRMED] / [TBD bit]` | exact encoding 需 owner/RTL 确认 |
| Secure boot enable | `SECURE_BOOT_EN` | eHSM / hardware control field | `eHSM-native` / `NGU-SoC-integration-TBD` | `SRC-006`, `CR-0004` | `[CONFIRMED direction]` | exact bit 未冻结 |
| Algorithm select | `algo_family` | `SocBootAlg`, `SocUpgradeAlg`, `EhsmCodeVerifyAlg`, `EhsmCodeUpgradeAlg` | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NGU 不覆盖 |
| Anti-rollback counter | `SEC1_MIN_VER`, `SEC2_MIN_VER`, `*_MIN_VER` | eHSM SOC FW / eHSM FW Version Counter | `eHSM-native` + `NGU-logical-alias` | `SRC-006`, `CR-0004` | `[CONFIRMED] / [TBD per-image]` | old fields are logical rollback domains |
| JTAG force disable | `JTAG_FORCE_DISABLE` | SoC/board JTAG MUX + lifecycle/debug auth | `NGU-SoC-integration-TBD` | `CR-0003`, `CR-0004` | `[TBD bit]` | 不写成 eHSM physical OTP bit |
| Attestation enable | `ATTEST_EN` | eHSM / SEC policy | `NGU-logical-alias` / `NGU-SoC-integration-TBD` | `CR-0004` | `[TBD bit]` | report/service policy |

---

### 10.4.4 Key Slot / Key Purpose

| NGU Concept | Current / Old Field | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|---|
| Chip Root | `UDS / Root Secret` | Chip Root Key / secure storage | `eHSM-native` | `SRC-006` | `[CONFIRMED role] / [TBD provisioning flow]` | provisioning flow 待 eHSM/ATE 联调冻结 |
| Device Root | `DRK` | Device Root Key | `eHSM-native` / `NGU-logical-alias` | `SRC-006` | `[CONFIRMED]` | DRK may be semantic only |
| FW verify | `FW Verify Key` | Soc FW Verify Key / Soc Upgrade Verify Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | owner must freeze mapping |
| FW decrypt | `FW Encrypt Key / FW_KEK` | Soc Encrypt Key / Soc Upgrade Encrypt Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | SEC1/SEC2 mandatory use |
| Debug auth | `Debug Auth Seed / Key` | Soc Debug Verify Key / User Auth Key | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | debug branch independent from attestation |
| Attestation | `Attestation Seed / Device Identity Key` | Soc Private Key / Secret Key / owner mapping | `NGU-logical-alias` | `SRC-006` | `[TBD exact ID]` | attestation model TBD |

---

### 10.4.5 Secure Boot / Upgrade Commands

| NGU Profile | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|
| `VERIFY_SEC1` | Bootloader `bl_verify_image` or equivalent ROM path | `eHSM-native` wrapper | `SRC-007`, `CR-0004` | `[ASSUMED]` | exact BootROM callable path TBD |
| `VERIFY_IMAGE(SEC2)` | Firmware `soc_verify` | `eHSM-native` wrapper | `SRC-006`, `CR-0004` | `[CONFIRMED direction]` | NGU wrapper adds manifest policy |
| encrypted SEC1/SEC2 deploy | RAM deploy / output buffer | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED]` | NVM only verify not allowed for encrypted images |
| NVM only verify | NVM deploy | `eHSM-native` | `SRC-006`, `SRC-007` | `[CONFIRMED not for SEC1/SEC2 encrypted]` | image cannot be encrypted |
| FW upgrade | `fw_upgrade` / `bl_fw_upgrade` | `eHSM-native` wrapper | `SRC-006`, `SRC-007` | `[CONFIRMED direction]` | output boot image |

---

### 10.4.6 Manufacturing Commands

| NGU Concept | eHSM Native Field / Command | Mapping Type | Source | Status | Notes |
|---|---|---|---|---|---|
| Key install | `install_random_key`, `install_encrypt_key` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | exact slot mapping TBD |
| Lifecycle change | `change_lifecycle` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | SEC is control plane |
| Control field change | `change_control_field` | `eHSM-native` | `SRC-006` | `[CONFIRMED direction]` | exact bit mapping TBD |
| OTP readback / validation | eHSM status / readback / attested validation | `eHSM-native` / `eHSM-customization-TBD` | `CR-0004` | `[TBD]` | sensitive fields may be non-readable |

---

### 10.4.7 Open Items

| Item | Status | Blocking Area |
|---|---|---|
| manifest ABI bit-level layout | `[TBD]` | SEC FW / tools |
| eHSM manifest parser | `[TBD]` | eHSM firmware |
| exact OTP/control bit mapping | `[TBD]` | RTL / eHSM |
| exact key ID mapping | `[TBD]` | eHSM / security owner |
| per-image rollback counter | `[TBD]` | product update policy |
| per-image CEK / wrapped CEK | `[TBD]` | eHSM customization |
| recovery image policy | `[TBD]` | recovery / RMA |

<!-- Embedded from security_workflow/04_impl_design/mailbox_if.md; keep full content synchronized. -->
## 10.5 NGU800 Mailbox 接口实现级设计（V1.0）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级详设
适用范围：NGU800 / NGU800P 安全子系统（SEC/C908 ↔ eHSM）
定位：供 RTL、SEC FW、eHSM 适配层、Driver、Host 代理层对齐使用

---

### 10.5.1 设计目标

本文档在 NGU800 当前基线下，定义安全子系统 Mailbox 的实现级接口模型，明确：

1. Mailbox 的调用边界与拥有者
2. 硬件寄存器与共享内存的分工
3. 请求/响应包格式
4. 关键命令的 request / response 结构
5. 生命周期限制、错误码模型、超时重试策略
6. 与 Host / PCIe / SEC / eHSM 的边界关系

本文档目标不是替代 eHSM 原始 TRM，而是在 NGU800 项目中形成一个可直接实施的“项目适配层定义”。

---

### 10.5.2 设计输入与裁决口径

### 10.5.3 已采用的输入事实

- eHSM 与 SoC 之间通过 Mailbox 进行命令/响应交互，通过 SOC_MEM 进行数据传递。
- eHSM 侧会从 SOC_MEM 取入参数据并把结果写回 SOC_MEM。
- 安全子系统相对封闭，外部系统不能直接访问 eHSM。
- 在项目架构中，eHSM 只接受来自 C908 的任务，其他 Core 或 Master 没有访问通道。
- eHSM Mailbox 硬件支持最多 16 个通道；每个方向有 2 个 data 寄存器和 1 个 note/status 寄存器。
- eHSM 固件/Bootloader 已存在 verify、debug auth、lifecycle、counter、UTC 等命令族能力。
- CR-0004 后，`VERIFY_SEC1 / VERIFY_IMAGE` 是 NGU wrapper/profile，底层必须映射到 eHSM Bootloader / Firmware 原生命令和 native image header。

### 10.5.4 项目裁决

即使通用 eHSM 文档描述了“主机向 eHSM 发送命令”流程，本项目不直接采用“Host 直接调用 eHSM”模式，而采用：

> **Host → SEC/C908 → eHSM**

即：
- Host 只能通过 PCIe + 中断 / 标志位把镜像或请求送到受控缓冲区
- SEC/C908 负责做访问控制、生命周期判断、参数封装
- eHSM 只处理来自 SEC/C908 的安全服务请求

---

### 10.5.5 总体架构

### 10.5.6 逻辑关系

```mermaid
graph TD
    H[Host / BMC / 外部软件] -->|PCIe / IRQ / Flag| SEC[SEC / C908]
    SEC -->|Mailbox Command + Shared Memory Ptr| EH[eHSM]
    EH -->|Mailbox Response + Shared Memory Result| SEC
    SEC -->|状态 / 结果| H
    EH --> OTP[eFuse / OTP]
    EH --> ALG[HASH / PKE / SKE / TRNG / Counter / UTC]
```

### 10.5.7 角色分工

| 角色 | 职责 | 不允许做的事 |
|---|---|---|
| Host | 投递镜像 / 请求；读取结果 | 直接调用 eHSM；直接 release 执行；直接访问 key/OTP |
| SEC/C908 | 唯一 Mailbox caller；参数封装；生命周期/权限检查；状态机控制 | 绕过 eHSM 进行正式安全路径验签 |
| eHSM | 安全服务执行者；verify / key / lifecycle / auth / counter / UTC | 接收非 C908 发来的任务 |
| RTL/Mailbox HW | 提供通道、寄存器、note、中断 | 承担协议级安全语义判断 |

---

### 10.5.8 硬件寄存器使用模型

### 10.5.9 Mailbox 硬件资源

每个 Mailbox 通道按硬件能力包含：

#### 下行（SoC → eHSM）
- `MB_S2H_INFO[0]`
- `MB_S2H_INFO[1]`
- `MB_S2H_NOTE`

#### 上行（eHSM → SoC）
- `MB_H2S_INFO[0]`
- `MB_H2S_INFO[1]`
- `MB_H2S_NOTE`

此外项目侧必须使用：
- `eHSM_STATUS_0`
- `eHSM_STATUS_1`
- `MB_INT_ALL`
- `MB_HSM_CFG0`
- `MB_HSM_CFG1`

### 10.5.10 本项目对 INFO 寄存器的使用约定

#### 约定原则
由于硬件每个方向只有 2 个 data 寄存器，本项目不把完整请求包直接塞入寄存器，而采用：

> **寄存器只传“包地址 + 控制信息”，真实包体放在共享内存。**

#### 推荐映射

| 寄存器 | 含义 |
|---|---|
| `MB_S2H_INFO[0]` | `pkt_addr_l`：请求包物理地址低 32 位 |
| `MB_S2H_INFO[1]` | `pkt_addr_h`：请求包物理地址高 32 位 |
| `MB_S2H_NOTE` | doorbell / 请求有效标记 |
| `MB_H2S_INFO[0]` | `rsp_addr_l`：响应包地址低 32 位（可选与请求相同） |
| `MB_H2S_INFO[1]` | `rsp_addr_h`：响应包地址高 32 位 |
| `MB_H2S_NOTE` | response ready / 完成标记 |

说明：
- 若请求和响应共用同一片共享内存包体，则 `MB_H2S_INFO[0/1]` 可返回同地址。
- `NOTE` 位的置位/清除语义必须在 RTL / FW 联调时统一，不得双方各自假设。

---

### 10.5.11 通道分配建议

### 10.5.12 通道总体原则
虽然硬件支持最多 16 通道，但首版项目不建议“命令一类一个通道”，避免把接口复杂度前移到 RTL/FW 交互层。

建议首版采用：

| 通道号 | 用途 | 说明 |
|---|---|---|
| CH0 | 通用控制面通道 | verify / lifecycle / debug auth / key / counter / UTC |
| CH1 | 长耗时镜像服务通道 | 大镜像 verify / decrypt / install |
| CH2 | 预留 | attestation / SPDM 扩展 |
| CH3~15 | 预留 | 后续扩展 |

首版最小可行实现：
- **CH0 必须实现**
- CH1 可按镜像路径复杂度决定是否首版启用

---

### 10.5.13 共享内存包格式

### 10.5.14 通用请求头

```c
typedef struct {
    uint16_t cmd_id;            /* 命令号 */
    uint16_t hdr_ver;           /* header版本 */
    uint32_t total_len;         /* 整个请求包长度 */
    uint32_t token;             /* 请求-响应配对 token */
    uint32_t caller_id;         /* 固定为 SEC/C908 caller id */
    uint32_t lifecycle_state;   /* 当前发起时 lifecycle */
    uint32_t flags;             /* 同步/异步、是否跳转、是否更新计数等 */
    uint32_t payload_off;       /* 负载偏移 */
    uint32_t payload_len;       /* 负载长度 */
    uint32_t resp_buf_off;      /* 响应区域偏移，可与请求同包 */
    uint32_t resp_buf_len;      /* 响应缓冲长度 */
} ngu_mb_req_hdr_t;
```

### 10.5.15 通用响应头

```c
typedef struct {
    uint16_t cmd_id;            /* 与请求一致 */
    uint16_t hdr_ver;           /* header版本 */
    uint32_t total_len;         /* 整个响应包长度 */
    uint32_t token;             /* 与请求配对 */
    uint32_t status;            /* SUCCESS / FAIL / BUSY / RETRY 等 */
    uint32_t err_code;          /* 细粒度错误码 */
    uint32_t detail0;           /* 附加返回值 */
    uint32_t detail1;           /* 附加返回值 */
    uint32_t detail2;           /* 附加返回值 */
    uint32_t detail3;           /* 附加返回值 */
} ngu_mb_resp_hdr_t;
```

### 10.5.16 头字段约束

- `caller_id` 在本项目中必须固定代表 SEC/C908 安全调用面。
- `lifecycle_state` 不是最终授权依据，但 eHSM 可用其做快速拒绝；最终仍以 eHSM 侧状态/OTP 为准。
- `token` 必须由 SEC 生成，避免并发时响应错配。
- 所有长度字段必须由 SEC 先做边界检查，再提交到 eHSM。

---

### 10.5.17 命令 ID 规划（项目建议）

### 10.5.18 命令分组

| 范围 | 组别 |
|---|---|
| 0x0001 ~ 0x001F | Boot / Verify |
| 0x0020 ~ 0x003F | Debug / Auth / Lifecycle |
| 0x0040 ~ 0x005F | Counter / UTC / Status |
| 0x0060 ~ 0x007F | Key Service |
| 0x0080 ~ 0x009F | Attestation / SPDM |
| 0x00A0 ~ 0x00BF | Provisioning / Manufacturing |
| 0x00C0 ~ 0x00FF | Vendor Reserved |

### 10.5.19 首版强制命令

| Cmd ID | Name | 说明 |
|---|---|---|
| 0x0001 | VERIFY_FMC / VERIFY_SEC1 | FMC（SEC1）验签 + 强制解密 + rollback + measurement |
| 0x0002 | VERIFY_IMAGE | eHSM native image verify/decrypt + NGU manifest policy check；GSP（SEC2）解密强制 |
| 0x0003 | VERIFY_AND_MEASURE | 验签、按策略解密并写入 measurement |
| 0x0020 | GET_CHALLENGE | 获取 challenge |
| 0x0021 | DEBUG_AUTH | 调试鉴权 |
| 0x0022 | CLOSE_DEBUG | 关闭调试 |
| 0x0023 | CHANGE_LIFECYCLE | 切换生命周期 |
| 0x0040 | READ_COUNTER | 读取计数器 |
| 0x0041 | INCREASE_COUNTER | 提升计数器 |
| 0x0042 | GET_UTC | 获取 UTC |
| 0x0043 | SET_UTC | 设置 UTC |
| 0x0060 | KEY_DERIVE | 密钥派生 |
| 0x0061 | IMPORT_WRAP_KEY | 导入封装密钥（如策略允许） |
| 0x0080 | GEN_ATTEST_REPORT | 生成证明报告 |
| 0x00A0 | PROVISION_ROOT_MATERIAL | 制造灌装阶段专用 |

---

### 10.5.20 关键命令结构体

### 10.5.21 VERIFY_FMC / VERIFY_SEC1 / VERIFY_IMAGE

`VERIFY_FMC` 是工程命名，安全抽象上等价 `VERIFY_SEC1`。该 profile 可在实现上复用 `VERIFY_IMAGE` 包格式，但策略不可被 caller 降级。该 profile 不定义新的 eHSM physical header，而是映射到 eHSM Bootloader `bl_verify_image` 或等价 ROM path：

- eHSM native header 必须是 physical verification container
- NGU `FMC/SEC1` 类型来自 manifest / SEC policy，不写入 eHSM `Image_Type`
- `decrypt_required` 固定为 1
- `rollback_policy` 固定启用
- `measurement_slot` 必须有效
- 输出地址只能落在 BootROM / SEC 认可的受控执行区或 staging 区
- Host 不得直接调用该命令，BootROM / SEC 也不得关闭 FMC（SEC1）解密

`VERIFY_IMAGE` 中 `image_type == GSP/SEC2` 时也必须采用 mandatory decrypt profile：

- 底层优先映射到 eHSM Firmware `soc_verify` 或 owner-confirmed wrapper
- `decrypt_required` 固定为 1
- `rollback_policy` 固定启用
- decrypt failure / policy mismatch 必须阻断 GSP（SEC2）release

PM / RAS / Codec 等 runtime image 在 USER/PROD 默认走 verify + decrypt profile；signature-only 只能由产品策略和 image_type 白名单允许。

#### Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint64_t image_addr;
    uint32_t image_len;
    uint32_t ehsm_image_type_expected; /* eHSM native Image_Type profile */
    uint32_t ngu_image_type_expected;  /* FMC(SEC1) / GSP(SEC2) / PMP / RMP / OMP / MMP / RECOVERY */
    uint32_t verify_policy;     /* verify only / verify+decrypt / verify+measure */
    uint32_t expected_lcs_mask; /* 允许的 lifecycle */
    uint32_t decrypt_required;  /* FMC(SEC1)/GSP(SEC2) 时必须为 1 */
    uint32_t rollback_policy;   /* rollback required / optional / recovery policy */
    uint32_t expected_algorithm_profile; /* audit/check only, not algorithm authority */
    uint32_t measurement_slot;
    uint32_t jump_on_pass;      /* 仅对 eHSM FW 或特定路径有效 */
    uint64_t dst_addr;          /* 解密输出地址，0 表示原地/策略定义 */
} ngu_mb_verify_image_req_t;
```

#### Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t ehsm_version_counter_checked;
    uint32_t ngu_rollback_domain;
    uint32_t signer_key_ref;
    uint32_t measurement_slot;
    uint32_t rollback_checked;  /* 0/1 */
    uint32_t decrypt_applied;   /* 0/1 */
    uint32_t decrypt_result;    /* none / success / fail / policy_mismatch */
    uint32_t policy_state;      /* applied policy summary */
} ngu_mb_verify_image_resp_t;
```

#### 约束
- `ehsm_image_type_expected` 保持 eHSM TRM 定义；`ngu_image_type_expected` 来自 NGU manifest / policy table。
- `VERIFY_FMC / VERIFY_SEC1` 时，`decrypt_required` 不可被 caller 关闭；若 eHSM native header / manifest policy 不满足加密要求，必须返回 policy mismatch / key error。
- `VERIFY_IMAGE(GSP/SEC2)` 时，`decrypt_required` 不可被 caller 关闭；若 eHSM native header / manifest policy 不满足加密要求，必须返回 policy mismatch / key error 并阻断安全控制面启动。
- PM / RAS / Codec 若进入 signature-only 白名单，响应中的 `policy_state` 必须能反映该例外路径，供 measurement / attestation 使用。
- `expected_algorithm_profile` 只能用于一致性检查和审计，不得覆盖 eHSM `SocBootAlg / SocUpgradeAlg` 或等价 control field。
- eHSM key slot / key ID mapping 必须来自 eHSM TRM 或 `ehsm_source_conformance_matrix.md`，不得由 wrapper 自行发明。
- per-image CEK / wrapped CEK 不作为已冻结字段；若后续需要，必须通过 eHSM customization CR 增补。
- `VERIFY_FMC / VERIFY_SEC1` 必须完成 eHSM native header 解析、key_id / signer 检查、revoke bitmap、eHSM Version Counter / NGU rollback domain、signer hash / trust anchor、signature、Code region verify/decrypt output 和 measurement 记录。
- `VERIFY_FMC / VERIFY_SEC1` 的认证覆盖契约必须满足 `AuthCoverage >= full Code region`，其中 full Code region 至少包含 `NGU protected manifest + FMC payload + padding/alignment counted by Code_Size`。
- `VERIFY_FMC / VERIFY_SEC1` 的解密输出必须是受控 output buffer 中的 plaintext Code region；BootROM 只能在 eHSM PASS 后解析 manifest，并依据 `payload_offset / payload_size` 定位 FMC payload。
- `VERIFY_FMC / VERIFY_SEC1` 不得从未认证保护的 header 字段中直接读取 NGU 项目级 release 决策，例如 `FMC/GSP/runtime` 项目类型、load/entry 地址、lifecycle mask 或 measurement slot；这些语义必须来自已认证保护的 manifest。
- `VERIFY_IMAGE` 对 GSP（SEC2）使用同等覆盖原则；runtime image 若允许 signature-only 例外，仍必须保证 manifest 和 payload 位于 eHSM 认证覆盖范围内。
- QEMU / stub / golden vector 必须覆盖 tamper case：篡改 manifest `ngu_image_type`、`entry_addr`、`version_counter`、payload 字节、`Code_Size` 或截断 Code region 后不得 release。
- `jump_on_pass` 只允许在明确受控路径启用；不允许让 Host 通过该字段间接控制跳转。
- `dst_addr` 必须由 SEC 预先做地址白名单检查；FMC（SEC1）解密结果只能进入 BootROM / SEC 认可的受控执行区或 staging 区。

---

### 10.5.22 GET_CHALLENGE

#### Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t auth_type;         /* eHSM debug / SOC debug / USER control */
    uint32_t nonce_len_req;     /* 请求 challenge 长度 */
} ngu_mb_get_challenge_req_t;
```

#### Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t challenge_len;
    uint8_t  challenge[64];
    uint8_t  uid_hash[32];
} ngu_mb_get_challenge_resp_t;
```

---

### 10.5.23 DEBUG_AUTH

#### Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t auth_type;         /* eHSM debug / SOC debug */
    uint32_t scope_words;       /* 端口位图长度（word） */
    uint64_t sig_addr;          /* 签名/MAC 地址 */
    uint32_t sig_len;
    uint64_t scope_bitmap_addr; /* SOC debug 端口位图地址 */
    uint64_t cert_or_keyblob_addr;
    uint32_t cert_or_keyblob_len;
} ngu_mb_debug_auth_req_t;
```

#### Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t granted;
    uint32_t granted_scope_words;
    uint32_t expire_policy;
} ngu_mb_debug_auth_resp_t;
```

---

### 10.5.24 CHANGE_LIFECYCLE

#### Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t from_lcs;
    uint32_t to_lcs;
    uint32_t auth_type;
    uint64_t auth_blob_addr;
    uint32_t auth_blob_len;
} ngu_mb_change_lcs_req_t;
```

#### Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t old_lcs;
    uint32_t new_lcs;
} ngu_mb_change_lcs_resp_t;
```

---

### 10.5.25 COUNTER 命令

#### READ_COUNTER Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t counter_id;
} ngu_mb_read_counter_req_t;
```

#### READ_COUNTER Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t counter_id;
    uint64_t counter_val;
} ngu_mb_read_counter_resp_t;
```

#### INCREASE_COUNTER Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t counter_id;
    uint64_t increment;
} ngu_mb_inc_counter_req_t;
```

#### INCREASE_COUNTER Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t counter_id;
    uint64_t counter_val_new;
} ngu_mb_inc_counter_resp_t;
```

---

### 10.5.26 GEN_ATTEST_REPORT

#### Request

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint64_t nonce_addr;
    uint32_t nonce_len;
    uint32_t session_flags;
    uint32_t measurement_mask;
    uint64_t out_report_addr;
    uint32_t out_report_max_len;
} ngu_mb_gen_att_report_req_t;
```

#### Response

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t report_len;
    uint32_t sig_algo;
    uint32_t hash_algo;
} ngu_mb_gen_att_report_resp_t;
```

---

### 10.5.27 状态机与握手语义

### 10.5.28 请求路径

```mermaid
sequenceDiagram
    participant SEC as SEC/C908
    participant MB as Mailbox
    participant EH as eHSM
    participant SM as Shared Memory

    SEC->>SM: 写请求包
    SEC->>MB: 写 INFO[0/1] = pkt addr
    SEC->>MB: 置位 NOTE / doorbell
    MB->>EH: 中断/事件
    EH->>SM: 读请求包
    EH->>EH: 执行命令
    EH->>SM: 写响应包
    EH->>MB: 写返回 INFO[0/1] + 置位 response NOTE
    MB->>SEC: 中断/事件
    SEC->>SM: 读响应包
```

### 10.5.29 NOTE 语义（项目建议）

- `REQ_VALID`：SEC 已写好请求包，可处理
- `REQ_BUSY`：eHSM 已接收，处理中
- `RSP_VALID`：eHSM 已写好响应
- `RSP_DONE_ACK`：SEC 已完成响应消费

若硬件 NOTE 仅提供单 bit，则用“置位触发 + 软件状态字段”方式实现，不在 NOTE bit 上塞太多语义。

---

### 10.5.30 Cache / DMA / 内存一致性规则

### 10.5.31 请求写入责任
- SEC 负责把请求包写入共享内存。
- 若共享内存位于 cacheable 区域，SEC 必须在 doorbell 前完成 cache flush / clean。
- eHSM 侧如通过 DMA/AXI 读取共享内存，必须保证读到 flush 后数据。

### 10.5.32 响应读取责任
- eHSM 写完响应包并置 response ready 后，SEC 在读取前必须做 invalidate / barrier（如区域可 cache）。

### 10.5.33 地址合法性
- `pkt_addr` / `dst_addr` / `scope_bitmap_addr` 等所有地址必须由 SEC 先做白名单检查。
- eHSM 侧应再做一次范围检查，防止越界或越权访问。
- 管理子系统 DMA / Host DMA / OOB DMA 对安全资源默认拒绝，只允许访问 firewall 显式白名单 staging/data buffer。
- DMA 不得访问 eHSM internal memory、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、recovery 区、cert/policy/metadata 安全区、measurement_table 安全写区域、debug/lifecycle/rollback 控制寄存器。
- `[TBD]` 具体 UserID、firewall region、地址范围、错误隐藏策略和审计字段由 RTL/实现设计冻结。

---

### 10.5.34 生命周期限制矩阵

| Command | TEST | DEVE | MANU | USER | DEBUG/RMA | DEST |
|---|---|---|---|---|---|---|
| VERIFY_IMAGE | Y | Y | Y | Y | Y | N |
| GET_CHALLENGE | Y | Y | Y | 受控 | Y | N |
| DEBUG_AUTH | Y | Y | 受控 | 默认 N / 受策略 | Y | N |
| CLOSE_DEBUG | Y | Y | Y | Y | Y | N |
| CHANGE_LIFECYCLE | Y | Y | Y | 受限 | 受限 | N |
| READ_COUNTER | Y | Y | Y | Y | Y | N |
| INCREASE_COUNTER | Y | Y | Y | Y | 受控 | N |
| GEN_ATTEST_REPORT | 可选 | 可选 | 可选 | Y | 可选 | N |
| PROVISION_ROOT_MATERIAL | N | N | Y | N | N | N |

---

### 10.5.35 错误码模型（项目建议）

| Code | 名称 | 含义 |
|---|---|---|
| 0x0000 | EHSM_OK | 成功 |
| 0x0001 | EHSM_ERR_INVALID_CMD | 无效命令 |
| 0x0002 | EHSM_ERR_INVALID_HDR | 头格式非法 |
| 0x0003 | EHSM_ERR_INVALID_LEN | 长度非法 |
| 0x0004 | EHSM_ERR_INVALID_STATE | 状态不允许 |
| 0x0005 | EHSM_ERR_INVALID_LCS | 生命周期不允许 |
| 0x0006 | EHSM_ERR_ACCESS_DENY | 权限不足 |
| 0x0007 | EHSM_ERR_ADDR_RANGE | 地址越界 |
| 0x0008 | EHSM_ERR_VERIFY_FAIL | 验签失败 |
| 0x0009 | EHSM_ERR_DECRYPT_FAIL | 解密失败 |
| 0x000A | EHSM_ERR_ROLLBACK_FAIL | 版本回退 |
| 0x000B | EHSM_ERR_AUTH_FAIL | 鉴权失败 |
| 0x000C | EHSM_ERR_BUSY | 通道忙 |
| 0x000D | EHSM_ERR_TIMEOUT | 超时 |
| 0x000E | EHSM_ERR_NOT_SUPPORTED | 功能未实现 |
| 0x000F | EHSM_ERR_INTERNAL | 内部错误 |
| 0x0010 | EHSM_ERR_KEY_REF_INVALID | eHSM key reference / key policy 无效或与 image policy 不匹配 |
| 0x0011 | EHSM_ERR_POLICY_MISMATCH | 请求策略、镜像头策略或 lifecycle policy 不一致 |

---

### 10.5.36 超时 / 重试 / 并发语义

### 10.5.37 最小要求
- CH0 首版建议单 outstanding。
- 若请求处理中，重复提交同通道请求应返回 `EHSM_ERR_BUSY`。
- `token` 必须在单通道范围内唯一，直到响应被消费。

### 10.5.38 重试规则
- 对 `BUSY` 可重试
- 对 `VERIFY_FAIL / AUTH_FAIL / INVALID_LCS / ACCESS_DENY` 不可盲重试
- 对 `TIMEOUT` 需先读 `eHSM_STATUS` 再决定是否恢复通道

---

### 10.5.39 与 Host 的关系（项目强规则）

1. Host 不直接向 eHSM 发送安全命令
2. Host 只能把镜像或请求交给 SEC 控制面
3. SEC 负责：
   - 参数白名单检查
   - 生命周期检查
   - 权限检查
   - 请求封装
4. eHSM 只接受来自 C908/SEC 的安全任务

---

### 10.5.40 RTL / FW / Driver 影响

### 10.5.41 RTL 侧
- 需要冻结 NOTE 位语义或支持软件状态字段协同
- 需要冻结 INFO[0/1] 的读写时序
- 需要明确中断清除时机
- 需要明确 shared memory 地址位宽（32/64）

### 10.5.42 SEC FW 侧
- 需要实现请求包封装层
- 需要实现 cache flush / invalidate
- 需要实现 token 管理与 timeout 管理
- 需要实现 command dispatch 封装

### 10.5.43 Driver 侧
- 需要实现寄存器访问封装
- 需要实现同步/异步等待接口
- 需要实现错误码到上层错误模型的映射

---

### 10.5.44 当前冻结建议

当前建议先冻结以下内容：

1. `INFO[0/1] = packet address low/high`
2. `NOTE` 仅做 doorbell/response ready，不承载复杂多状态
3. CH0 为首版唯一强制实现通道
4. Host 不直接调用 eHSM
5. VERIFY / DEBUG_AUTH / CHANGE_LIFECYCLE / COUNTER / ATTEST 五类命令结构先冻结
6. 所有地址参数必须双重检查（SEC + eHSM）

---

### 10.5.45 开放问题

1. 是否首版启用 CH1 专门承载大镜像 verify/decrypt
2. 请求/响应共用一片共享内存还是分离
3. 共享内存最终落在管理子系统 IRAM、DDR buffer 还是 firewall 划出的 share memory
4. `PROVISION_ROOT_MATERIAL` 是否对外暴露为单独命令，还是仅制造态内部接口
5. Debug port 129 bit 的最终位图映射由谁冻结
6. OOB/BMC provisioning transport proxy 的 request authentication、anti-replay、audit、failure rollback、rate limit / lockout 字段如何冻结
7. Runtime signature-only 白名单如何在 `verify_policy` / `policy_state` / attestation report 中编码

---

### 10.5.46 结论

本文件已经把 Mailbox 从“概念交互”推进到“实现级接口”层面，明确了：

- 调用边界：SEC/C908 是唯一 caller，eHSM 是唯一安全执行者
- 硬件分工：寄存器传地址，共享内存传包体
- 协议骨架：req/resp 通用头 + 关键命令结构
- 安全规则：生命周期限制、Host 隔离、地址检查、错误码、超时重试
- 工程影响：RTL / FW / driver 的冻结点

后续可在此基础上继续补：
- command ID 最终值
- debug port 位图冻结
- request/response 头字段最终 bit-level 对齐
- 与 eHSM 原生命令表的一一映射表

<!-- Embedded from security_workflow/04_impl_design/manufacturing_provisioning.md; keep full content synchronized. -->
## 10.6 NGU800 制造 / 灌装 / Provisioning / RMA 实现级设计（V1.0）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级详设
适用范围：NGU800 / NGU800P 制造、灌装、板级 bring-up、量产冻结、返修/RMA
定位：供安全架构评审、SEC/C908 FW、eHSM 适配层、制造工站、Provisioning Tool、测试团队对齐使用

---

### 10.6.1 设计目标

本文档用于把 NGU800 当前方案中的制造与灌装链路，收敛到**流程级 / 字段级 / 接口级 / 审计级**，重点明确：

1. 制造阶段和生命周期阶段的对应关系
2. Root Secret / Root Key / signer hash / debug anchor 的灌装对象与顺序
3. OTP / eFuse 写入、锁定、校验、审计要求
4. MANU → USER 的冻结动作
5. RMA / DEBUG 的授权与恢复规则
6. 工站、SEC/C908、eHSM、Host/BMC 在制造阶段的职责边界
7. 与 `efuse_key_fw_header_design.md`、`mailbox_if.md`、`spdm_report.md` 的对接关系

本文档不是制造作业指导书本身，而是 NGU800 项目实现层的**制造安全控制基线**。

---

### 10.6.2 设计输入与项目裁决

### 10.6.3 当前项目已采用的事实

当前项目已经明确或基本收敛的事实包括：

- eHSM 是安全服务根和首个密码学验证主体
- Root Secret / Root Key 不应离开 eHSM 使用域
- SEC1 在正式安全启动路径中必须签名 + 加密，SEC1 解密 key / FW_KEK 使用必须受 lifecycle gating
- 生命周期必须受 OTP / eFuse 与 eHSM 联合控制
- CR-0004 已确认 physical OTP/control/key/counter 排布优先按 eHSM TRM；本文件中的 OTP/eFuse 名称均为 NGU logical view，不能理解为新增 physical offset
- USER 生命周期必须关闭未授权 debug
- Host 不进入信任链，只能作为受控投递方
- 制造阶段必须定义 key 注入、锁定、审计和生命周期推进
- 方案必须同时考虑国密与国际算法双栈

### 10.6.4 项目裁决

当前项目对制造链路采用以下裁决：

> **制造工站通过受控 Provisioning 路径与 SEC/C908 交互，由 SEC 调 eHSM 完成 OTP / eFuse 写入与状态变更。**

即：
- 工站 / Provisioning Tool 不直接操作 Root of Trust 决策逻辑
- 工站不应直接触达 eHSM 私有执行面
- Root Secret / Root Key / 调试锚点 / signer anchor 的最终写入与锁定由 SEC + eHSM 路径完成
- MANU → USER 的冻结动作必须是原子化、可审计的步骤集合，而不是人工口头流程

---

### 10.6.5 生命周期与制造阶段映射

### 10.6.6 生命周期总体映射

| 生命周期 | 制造阶段语义 | 目标 | 默认安全策略 |
|---|---|---|---|
| TEST | 晶圆 / 封测 / 初始 bring-up | 验证硬件基本功能 | 可开放测试路径，不得等价于量产 |
| DEVE | 开发板 / EVB 调试 | 软件 bring-up / 调试 | 可有限开放 debug |
| MANU | 正式制造 / 板级生产 / 工站灌装 | 注入根材料、配置控制位、建立量产基础 | 启用基础安全校验 |
| USER | 量产交付 | 面向客户交付 | 关闭未授权 debug、锁定根材料 |
| DEBUG / RMA | 返修 / 厂商授权分析 | 故障定位与返修 | 仅限授权开启 |
| DEST | 销毁 | 退役 / 数据清除 | 不再允许正常启动 |

### 10.6.7 制造阶段建议分解

为方便工程落地，建议把制造过程进一步分解为：

| 阶段 ID | 阶段名称 | 主要动作 |
|---|---|---|
| MFG-0 | 裸片 / 封测初测 | 测试路径、基础检查、非量产 trust |
| MFG-1 | 板级 bring-up | 板卡电源、时钟、接口连通性验证 |
| MFG-2 | 安全灌装准备 | 建立工站认证、选择算法栈、装载 provisioning agent |
| MFG-3 | Root Material Provisioning | 注入 Root / UDS / anchor / counter 初值 |
| MFG-4 | 安全控制位配置 | 写 secure boot / debug / attestation / anti-rollback 控制位 |
| MFG-5 | 校验与锁定 | 写入结果校验、锁位、审计归档 |
| MFG-6 | MANU 验证启动 | 在 MANU 策略下进行带验证启动 |
| MFG-7 | USER 冻结 | 清除测试 trust、推进 USER、关闭未授权 debug |
| MFG-8 | 出厂验收 | 生成验收报告、归档审计记录 |

---

### 10.6.8 角色与职责边界

### 10.6.9 角色分工

| 角色 | 职责 | 不允许做的事 |
|---|---|---|
| Provisioning Tool / 工站 | 组织灌装步骤、提交请求、记录审计、接收结果 | 直接持有设备证明私钥、直接控制 eHSM 内部策略 |
| Host / BMC（制造场景） | 作为链路承载方、传输工站请求、获取状态 | 参与 Root of Trust 决策、直接写 Root 密钥到最终安全区 |
| SEC / C908 | 唯一 provisioning 控制面；参数校验；流程编排；调用 eHSM；状态收敛 | 绕过 eHSM 直接完成正式安全路径密钥使用 |
| eHSM | OTP/eFuse 写入控制、锁定、lifecycle、counter、debug auth、key 服务执行 | 接受非 SEC 的非受控制造命令 |
| OTP / eFuse | 按 eHSM TRM 持久保存生命周期、control field、Root 材料、key ID / level / purpose、Version Counter | 被 Host 或普通核直接改写，或被 NGU 自定义 physical layout 覆盖 |

### 10.6.10 强边界规则

1. Provisioning Tool 可以发起“灌装动作”，但不能直接触达 eHSM 内部寄存器语义。
2. Host/BMC 在制造阶段仍然不进入信任链，只是链路承载者。
3. SEC/C908 必须是唯一 provisioning caller。
4. eHSM 必须是唯一 Root 材料写入与锁定执行者。
5. OTP / eFuse 的最终控制位不得通过普通非安全路径直接改写。

---

### 10.6.11 制造对象清单

### 10.6.12 必须灌装对象

| 对象 | 是否必需 | 说明 |
|---|---|---|
| UDS / Root Secret | 必需 | 根种子 / 根材料 |
| Root Key / Root KEK 材料 | 必需 | 可直接写入或由 UDS 派生 |
| FW signer hash / trust anchor | 必需 | 支撑固件验签 |
| FW_KEK / Image Protect Key | 必需 | 支撑 SEC1/SEC2 强制加密镜像解密策略；exact eHSM key ID mapping 仍需冻结 |
| Debug auth anchor | 必需 | 支撑 RMA / DEBUG 调试鉴权 |
| Attestation anchor / identity seed | 必需 | 支撑设备证明 |
| 版本计数初值 | 必需 | 支撑 anti-rollback；物理承载优先映射 eHSM Version Counter，不再定义多个 32-bit physical counter |
| Secure boot / debug / attestation 控制位 | 必需 | 建立量产策略 |
| Board binding 信息 | 可选 | 按产品策略启用 |
| 主 / 从 Die binding 信息 | 双Die 推荐 | 支撑多Die一致性约束 |

### 10.6.13 不允许在制造阶段永久保留的对象

| 对象 | 原因 |
|---|---|
| 测试 signer key / 测试证书锚点 | 进入 USER 前必须清除 |
| 测试 debug 白名单 | 进入 USER 前必须清除 |
| 非量产 secure boot bypass 配置 | 进入 USER 前必须关闭 |
| 明文导出的 Root 私钥 | 根本不允许存在于最终流程 |

---

### 10.6.14 OTP / eFuse 灌装内容与顺序

### 10.6.15 推荐灌装顺序

建议顺序如下：

```text
(1) 读取当前生命周期和 OTP 状态
    ↓
(2) 验证设备处于允许灌装状态（MANU 或受控 provisioning 态）
    ↓
(3) 写入 UDS / Root Secret / Root Key 材料
    ↓
(4) 写入 FW signer hash / trust anchor
    ↓
(5) 写入 FW_KEK / image protect key 策略或其 eHSM 内部派生种子
    ↓
(6) 写入 debug auth anchor
    ↓
(7) 写入 attestation seed / anchor
    ↓
(8) 写入 eHSM Version Counter 初始状态 / owner-confirmed rollback policy
    ↓
(9) 写入 secure boot / FW encrypt / debug / attestation / algorithm 控制位
    ↓
(10) 读回校验或等价校验
    ↓
(11) 锁定 key / FW_KEK 策略 / 控制位 / 生命周期回退路径
    ↓
(12) 执行 MANU 验证启动
    ↓
(13) 执行 USER 冻结
```

### 10.6.16 为什么不能乱序

- Root 材料必须先于 signer / attestation 生效，否则后续校验没有可信根。
- eHSM Version Counter / rollback policy 必须在正式启动前建立，否则 anti-rollback 没有约束基线。
- 控制位必须在写入关键 anchor 后再打开，避免系统处于“要求安全启动但尚未具备信任锚”的中间态。
- 锁定位必须在校验通过后再写，避免把错误内容永久锁死。

---

### 10.6.17 Provisioning 命令模型

### 10.6.18 建议与 Mailbox 对接

制造阶段不建议引入完全独立的新链路，而建议复用受控 Mailbox / Shared Memory 模型。

对应 `mailbox_if.md` 中建议命令：

- `PROVISION_ROOT_MATERIAL`
- `CHANGE_LIFECYCLE`
- `READ_COUNTER`
- `INCREASE_COUNTER`（制造初始写入时按策略使用）
- `GET_CHALLENGE`
- `DEBUG_AUTH`（仅 RMA/DEBUG 场景）

### 10.6.19 Provisioning 请求结构建议

```c
typedef struct {
    ngu_mb_req_hdr_t hdr;
    uint32_t provision_type;        /* ROOT / SIGNER_HASH / DEBUG_ANCHOR / ATTEST / CTRL_BITS */
    uint32_t algo_family;           /* GM / INTL */
    uint32_t write_flags;           /* write / verify / lock / advance_lcs */
    uint64_t blob_addr;             /* 写入包地址 */
    uint32_t blob_len;              /* 写入包长度 */
    uint32_t target_ref;            /* eHSM key ID / control field / logical alias reference */
} ngu_mb_provision_req_t;
```

### 10.6.20 Provisioning 响应结构建议

```c
typedef struct {
    ngu_mb_resp_hdr_t hdr;
    uint32_t provision_type;
    uint32_t write_result;          /* success / partial / verify_fail */
    uint32_t lock_result;           /* 0/1 */
    uint32_t lcs_after;             /* 若发生生命周期推进 */
} ngu_mb_provision_resp_t;
```

### 10.6.21 关键约束

- `write_flags` 不得允许任意组合；必须由 SEC 侧先做合法性白名单检查。
- `target_ref` 必须映射到 eHSM key ID / control field / Version Counter 或已接受 CR 中的 logical alias。
- Provisioning Tool 不得把 `OTP-0..OTP-7` 当成 physical offset。
- `blob_addr/blob_len` 必须满足共享内存白名单和长度边界检查。
- provisioning 命令必须只允许在受控 lifecycle 下执行。

### 10.6.22 eHSM 命令映射

CR-0004 后，制造命令必须优先映射到 eHSM 已定义命令或 owner-confirmed wrapper：

| Manufacturing Action | eHSM Mapping Direction | 状态 |
|---|---|---|
| 安装随机/派生 key | `install_random_key` / owner-confirmed key install path | `[CONFIRMED direction]` |
| 安装加密 key blob | `install_encrypt_key` / owner-confirmed encrypted key install path | `[CONFIRMED direction]` |
| 生命周期推进 | `change_lifecycle` | `[CONFIRMED direction]` |
| control field 更新 | `change_control_field` | `[CONFIRMED direction]` |
| 版本计数 / rollback 状态 | eHSM Version Counter / owner-confirmed counter command | `[TBD exact process]` |
| OTP readback / 验收 | readback / status / attested validation | `[TBD]` |

---

### 10.6.23 Root Material Provisioning 细化

### 10.6.24 支持两种注入模式

#### 模式 A：直接写入根材料
- 工站侧准备 Root Secret / Root KEK 材料
- 通过受控通道写入 OTP/eFuse 安全区
- 适合工厂中心化生成密钥模型

#### 模式 B：写入种子 / 设备标识后由 eHSM 内部派生
- 工站只写入 UDS / seed / identity seed
- Root 派生由 eHSM 内部完成
- 更利于减少明文根材料在工站流转

### 10.6.25 当前项目建议

当前优先建议：

> **优先采用“Seed / UDS 注入 + eHSM 内部派生”的模式。**

原因：
1. 更符合“私钥不出 eHSM”的长期方向
2. 减少制造链路中的明文高敏根材料暴露
3. 更利于后续 attestation / key hierarchy 一致化

若项目现实约束要求直接写入 Root 材料，也必须满足：
- 工站 HSM/KMS 保护
- 不落明文磁盘
- 写入后立即锁定
- 全流程审计

---

### 10.6.26 控制位配置策略

### 10.6.27 必须配置的控制位

| 控制位 | 建议 USER 前状态 | 说明 |
|---|---|---|
| `SECURE_BOOT_EN` logical policy | 1 | 映射 eHSM / hardware control field，exact bit TBD |
| `DEBUG_AUTH_EN` logical policy | 1 | 映射 eHSM debug auth / lifecycle policy，exact bit TBD |
| `JTAG_FORCE_DISABLE` SoC integration policy | 1 | USER 默认关闭 JTAG；属于 SoC/board integration，非 NGU 自定义 eHSM OTP bit |
| `FW_ENCRYPT_EN` logical policy | 1（至少覆盖 SEC1 + SEC2） | SEC1/SEC2 强制签名 + 加密；PM/RAS/Codec USER/PROD 默认签名 + 加密 |
| `ATTEST_EN` logical policy | 1 | 启用设备证明；exact control mapping TBD |
| `ANTI_ROLLBACK_EN` logical policy | 1 | 启用反回滚；物理承载对齐 eHSM Version Counter / owner-confirmed counter |
| `DUAL_ALGO_EN` product policy | 1 | 允许双算法栈共存；算法 authority 仍来自 eHSM control field |

### 10.6.28 控制位写入原则

- 控制位写入必须晚于根材料 / signer anchor 写入
- USER 前必须确保控制位与实际信任锚一致
- 不得出现“开启 secure boot，但信任锚还未完成注入”的中间状态

---

### 10.6.29 校验与锁定策略

### 10.6.30 校验策略

推荐按以下顺序校验：

1. 校验写入命令执行返回状态
2. 读回校验（若策略允许）
3. 若不可读回，则通过 eHSM 内部状态校验 / 试运行校验
4. 进行一轮 MANU 策略下的验证启动
5. 校验 debug / lifecycle / attestation / rollback 状态是否符合预期

### 10.6.31 锁定对象

| 对象 | 锁定时机 | 说明 |
|---|---|---|
| Root Secret / Root Key 区 | 灌装校验通过后 | 防止重复覆盖 |
| signer hash / anchor 区 | 校验通过后 | 防止验签根被替换 |
| FW_KEK / image protect key 策略 | 灌装校验通过后 | 防止 SEC1/SEC2 解密策略被替换或降级 |
| debug anchor 区 | 校验通过后 | 防止调试授权根被替换 |
| 控制位区 | USER 冻结前 | 防止量产策略回退 |
| lifecycle 回退路径 | USER 推进后 | 防止回退到开发态 |

### 10.6.32 锁定规则

- 锁定必须是 provisioning 流程中的显式步骤，不允许“假设已经锁定”
- 锁定动作本身必须被审计记录
- 若锁定失败，不得继续推进 USER 生命周期

---

### 10.6.33 MANU 验证启动

### 10.6.34 目标

在进入 USER 前，必须先在 MANU 策略下完成一次“接近量产条件”的验证启动，用于确认：

- secure boot 可正常工作
- signer anchor 可正确校验
- anti-rollback 路径可正常读取
- mailbox / verify / counter 基本命令可工作
- attestation 基本能力可用

### 10.6.35 最小验证项

| 验证项 | 说明 |
|---|---|
| 验签 SEC1 / SEC2 | 核心启动链验证 |
| SEC1 verify/decrypt output | 验证 SEC1 eHSM native header、manifest policy、FW decrypt key mapping 和输出 buffer 约束 |
| SEC2 verify/decrypt output | 验证 SEC2 eHSM native header、manifest policy、FW decrypt key mapping 和安全控制面 release 约束 |
| 读取 eHSM Version Counter / rollback state | 反回滚链路验证 |
| 读取 lifecycle | 生命周期状态验证 |
| 生成 challenge / 或最小 report | 证明路径基本可用 |
| debug 默认策略检查 | 验证未授权 debug 未被放开 |

---

### 10.6.36 MANU → USER 冻结动作

### 10.6.37 必须冻结的动作集合

进入 USER 前，必须完成以下动作：

1. `SECURE_BOOT_EN = 1`
2. `DEBUG_AUTH_EN = 1`
3. `JTAG_FORCE_DISABLE = 1`
4. `ANTI_ROLLBACK_EN = 1`
5. `FW_ENCRYPT_EN = 1`，且至少覆盖 SEC1 + SEC2
6. Root Key / UDS / signer anchor / SEC1/SEC2 解密相关 eHSM key policy / FW_KEK 策略完成锁定
7. 测试 signer / 测试证书链 / 测试调试白名单全部清除
8. 如启用 attestation，则 `ATTEST_EN = 1`
9. 将生命周期推进到 USER
10. 锁定生命周期回退路径
11. 生成冻结完成审计记录

### 10.6.38 原子性要求

这些动作在工程实现上不一定要单条命令原子完成，但在**流程语义上必须被当成一个事务性步骤集合**处理：

- 任何一步失败，都不得报告“已完成 USER 冻结”
- 出错后必须进入可恢复的 MANU 故障处理分支
- 不得进入“部分已冻结、部分未冻结”的不明状态

---

### 10.6.39 Audit / 审计模型

### 10.6.40 审计事件建议

| Audit Event | 必须记录内容 |
|---|---|
| PROVISION_START | 设备 ID、工站 ID、时间、操作员 |
| ROOT_WRITE | 写入对象类型、slot、结果 |
| ANCHOR_WRITE | signer/debug/attest anchor 类型、结果 |
| CTRL_BITS_WRITE | 控制位变化前后、结果 |
| LOCK_APPLY | 锁定对象、结果 |
| MANU_BOOT_VERIFY | 验证启动结果 |
| USER_FREEZE | 生命周期变化前后、结果 |
| PROVISION_END | 总结果、失败码、日志索引 |

### 10.6.41 审计要求

- 审计日志不得包含明文 Root 私钥材料
- 可以记录 hash / ID / slot / result，但不能记录敏感明文
- 审计日志至少需可关联：
  - 设备
  - 工站
  - 时间
  - 操作员 / 工单
  - 结果

---

### 10.6.42 RMA / DEBUG 流程

### 10.6.43 基本原则

RMA / DEBUG 不是普通制造路径，而是**受授权的返修分析路径**。

必须满足：

1. 不直接破坏 USER trust state
2. 必须先经过授权
3. 必须记录审计日志
4. 必须在维修后恢复量产安全状态

### 10.6.44 建议流程

```text
接收返修设备
    ↓
校验工单 / 设备身份 / 厂商授权
    ↓
执行 challenge / debug auth
    ↓
按策略打开受限 debug 能力
    ↓
读取故障信息 / 维修 / 刷写恢复
    ↓
重新写回量产镜像
    ↓
恢复 USER 安全状态
    ↓
记录 RMA 审计结案
```

### 10.6.45 RMA 约束

- 不得因为进入 RMA 就默认长期开放 debug
- 不得跳过 challenge / auth
- 不得允许返修后继续带测试 trust 出厂
- 不得长期开放 SEC1/SEC2 解密绕过路径；RMA / rescue 镜像必须使用专用 signer / recovery trust，并保持 eHSM 受控解密或受控 recovery policy

---

### 10.6.46 与其他实现文件的对接

### 10.6.47 与 `efuse_key_fw_header_design.md`
- 该文件定义“灌什么”
- 本文件定义“怎么灌、何时锁、何时推进生命周期”

### 10.6.48 与 `mailbox_if.md`
- 该文件定义 provisioning 命令如何走 SEC → eHSM
- 本文件定义这些命令在哪些阶段被允许执行

### 10.6.49 与 `spdm_report.md`
- 该文件定义 report 如何表达 lifecycle/debug/board 状态
- 本文件定义这些状态在制造 / USER / RMA 阶段如何变化

### 10.6.50 与 `05_code_rules.md`
- 本文件的流程冻结点必须转化为 MUST / MUST NOT 开发规则

---

### 10.6.51 RTL / FW / Tool 影响

### 10.6.52 RTL 侧
- 需要支持 OTP/eFuse 写入控制位与锁位语义
- 需要支持生命周期状态持久化与回退限制
- 需要为 provisioning 命令保留合法状态机支撑

### 10.6.53 SEC FW 侧
- 需要实现 provisioning state machine
- 需要实现写入顺序控制、失败回滚、阶段状态记录
- 需要实现 MANU 验证启动和 USER 冻结动作编排

### 10.6.54 eHSM 适配层
- 需要支持 Root / anchor / counter / lifecycle / debug auth 的命令封装
- 需要支持写入后校验和锁定状态读取

### 10.6.55 Provisioning Tool
- 需要实现设备识别、工站认证、blob 管理、日志审计
- 需要显式区分“写入成功”“锁定成功”“USER 冻结完成”

---

### 10.6.56 当前冻结建议

当前建议优先冻结以下内容：

1. 制造阶段推荐顺序（MFG-0 ~ MFG-8）
2. Root / signer / debug / attestation / counter / ctrl_bits 的灌装顺序
3. MANU 验证启动的最小检查项
4. USER 冻结动作集合
5. 审计事件最小集合
6. RMA 需先 challenge / auth，再开受限 debug

---

### 10.6.57 开放问题

1. Root Material 首版是否完全采用 seed/UDS 注入，而非直接 Root Key 注入
2. OTP/eFuse 是否支持对部分区做读回校验，哪些区仅支持状态校验
3. Provisioning Tool 与 SEC 的承载链路最终经由 PCIe、BMC 还是独立工装接口
4. 双Die 产品的主 / 从 Die 灌装是独立还是联动事务
5. USER 冻结失败时，允许停留在 MANU，还是进入显式故障态
6. RMA 完成后恢复 USER 状态时，是否强制重新生成 attestation 相关状态摘要
7. OOB/BMC 作为 provisioning transport proxy 时的认证、审计、失败回滚和 rate limit / lockout 策略

---

### 10.6.58 结论

本文件已经把 NGU800 的制造 / 灌装 / Provisioning 路径推进到实现级设计，明确了：

- 生命周期与制造阶段的映射
- 灌装对象、顺序、校验和锁定策略
- MANU 验证启动与 USER 冻结动作
- RMA / DEBUG 的授权与恢复规则
- 审计模型和工程职责边界

到此为止，NGU800 当前阶段的“方案 → 实现”主链已经具备：

```text
约束
→ baseline
→ 实现级 eFuse / Key / FW Header
→ 实现级 Mailbox
→ 实现级 SPDM Report
→ 实现级 Manufacturing / Provisioning
→ Code Rules
→ Traceability
```

后续应继续把这些实现级设计反填充进章节级详设，并同步约束后续代码开发与测试验证。

<!-- Embedded from security_workflow/04_impl_design/spdm_report.md; keep full content synchronized. -->
## 10.7 NGU800 SPDM / Attestation Report 实现级设计（Starter v0.1）

> CR-0005 source-of-truth notice:
> 本文件是 `10_full_design.md` 第 10 章的编辑分片 / extracted implementation shard，不再作为独立事实源。
> 代码落地、评审和 ChatGPT 方案审查应优先读取 `security_workflow/03_detailed_design/10_full_design.md`。
> 修改本文件时，必须同步主详设第 10 章；若发生冲突，以 accepted CR、decision_log、official TRM 和 `10_full_design.md` 为准。

状态：实现级设计起始稿
目标：定义设备证明报告的字段级结构和签名覆盖范围

---

### 10.7.1 设计目标

本文件用于收敛：

- 设备身份字段
- report header
- measurement block
- lifecycle/debug block
- cert chain block
- signature block
- nonce/session 绑定关系

---

### 10.7.2 Report Header 建议

`algo_family / hash_algo / sig_algo` 在 report 中表示证明报告自身的签名 / hash profile，不是 secure boot / upgrade 的算法 authority。secure boot / upgrade 算法选择仍以 eHSM `SocBootAlg / SocUpgradeAlg` 或等价 control field 为准。

```c
typedef struct {
    uint16_t report_version;
    uint16_t algo_family;
    uint16_t hash_algo;
    uint16_t sig_algo;
    uint32_t report_len;
    uint32_t nonce_len;
    uint32_t session_binding_flags;
    uint32_t measurement_count;
    uint32_t lifecycle_state;
    uint32_t debug_state;
    uint32_t secure_boot_state;
    uint32_t image_confidentiality_policy;
    uint32_t rollback_state;
    uint32_t board_bind_result;
    uint32_t event_log_policy;
} ngu_att_report_hdr_t;
```

---

### 10.7.3 Identity Block

```c
typedef struct {
    uint8_t  device_id[32];
    uint8_t  die_id[32];
    uint8_t  board_id_hash[32];
    uint8_t  signer_id[32];
} ngu_att_identity_block_t;
```

---

### 10.7.4 Measurement Block

```c
typedef struct {
    uint32_t slot_id;
    uint32_t ehsm_image_type;
    uint32_t ngu_image_type;
    uint32_t image_version;
    uint32_t ehsm_version_counter_checked;
    uint32_t ngu_rollback_domain;
    uint32_t rollback_checked;
    uint32_t decrypt_applied;
    uint32_t image_policy_state;
    uint8_t  measurement_hash[48];
    uint32_t flags;
} ngu_att_measurement_block_t;
```

建议 measurement 至少覆盖：
- SEC1
- SEC2
- PM / RAS / Codec 等关键 runtime image
- lifecycle/debug 状态

SEC1 / SEC2 对应 measurement 必须反映 verify + decrypt 成功后的受控镜像状态，不能只记录未解密包体存在性。`flags` 至少需要能表达 `SIGN_VERIFIED`、`DECRYPT_APPLIED`、`ROLLBACK_CHECKED`、`POLICY_MATCHED` 和 `SIGNATURE_ONLY_EXCEPTION` 等语义。

字段语义：
- `ehsm_image_type` 来自 eHSM native header，保持 eHSM TRM 定义。
- `ngu_image_type` 来自 NGU protected manifest / policy table，用于 SEC1 / SEC2 / PM / RAS / Codec / Recovery 等项目级证明语义。
- `ehsm_version_counter_checked` 与 `ngu_rollback_domain` 分别表达物理计数器检查结果和 NGU 逻辑 rollback domain，不得把 `*_MIN_VER` 当成 physical OTP counter。

---

### 10.7.5 Lifecycle / Debug Status Block

```c
typedef struct {
    uint32_t lifecycle_state;
    uint32_t debug_enable_state;
    uint32_t anti_rollback_state;
    uint32_t secure_boot_state;
    uint32_t rollback_state;
    uint32_t image_confidentiality_policy; /* 至少表达 SEC1/SEC2 强制签名 + 加密策略 */
    uint32_t board_bind_result;            /* [ASSUMED] board binding 默认进入证明 */
} ngu_att_lifecycle_block_t;
```

---

### 10.7.6 Certificate Chain Block

```c
typedef struct {
    uint32_t cert_format;
    uint32_t cert_chain_len;
    uint32_t cert_chain_off;
} ngu_att_cert_chain_block_t;
```

---

### 10.7.7 Signature Block

```c
typedef struct {
    uint32_t sig_format;
    uint32_t sig_len;
    uint32_t sig_off;
} ngu_att_sig_block_t;
```

---

### 10.7.8 签名覆盖范围

必须覆盖：
- Report Header
- Identity Block
- Measurement Blocks
- Lifecycle/Debug Block
- Secure boot / image protection policy fields
- Rollback state
- Board binding result if present
- Nonce / Session Binding 信息

私钥不得离开 eHSM。

字段状态：
- `[CONFIRMED]` measurement、lifecycle、debug_state、secure_boot_state、rollback_state 必须被签名覆盖。
- `[ASSUMED]` image protection policy、decrypt_applied、`ngu_image_type` policy 和 board_bind_result 进入 report 或 measurement flags。
- `[TBD]` PowerBrake / PG / FAULT / reset event 进入主 report 还是扩展 event log。

---

### 10.7.9 GM / 国际算法映射

| algo_family | hash_algo | sig_algo |
|---|---|---|
| GM | SM3 | SM2 |
| INTL | SHA-256 / SHA-384 | ECDSA / RSA |

---

### 10.7.10 当前阶段结论

本文件已经给出 report 字段级方向。
后续应结合真实 verifier 需求进一步细化 slot、证书模型、image protection policy / decrypt_applied / `ehsm_image_type` / `ngu_image_type` / board_bind_result 编码、event log 策略和 session 绑定语义。

---
# 11. 风险、依赖、冻结项与开放问题

## 11.1 本次 CR 已收敛事项

| 项目 | 收敛结论 | 来源 |
|---|---|---|
| SEC1 加密 | `[CONFIRMED]` SEC1 在正式安全启动路径中必须签名 + 加密 | `CR-0001`; `C-BOOT-04` |
| SEC1 来源 | `[CONFIRMED]` SEC1 来自 NOR Flash / 本地 Flash，不由 Host 下发 | `SRC-003 启动方案`; baseline |
| SEC1 解密执行面 | `[CONFIRMED]` SEC1 解密 / unwrap 必须由 eHSM / 安全子系统受控密码服务完成 | `CR-0001`; boot/key/interface/impl |
| BootROM 边界 | `[CONFIRMED]` BootROM 不实现复杂验签或复杂解密 | `C-BOOT-03`; `C-BOOT-04` |
| 板级安全 | `[CONFIRMED]` 板级安全设计已作为正式章节并入 master | `05_board_security.md`; `CR-0001` |
| 章节顺序 | `[CONFIRMED]` Root / Key / Cert 详细设计后置到 boot、attestation、debug、interface、board 后 | `CR-0001` |
| V2.4 状态 | `[CONFIRMED]` V2.4 保持 pending review，不升级为完整 reviewed baseline | `CR-0003` |
| SEC2 加密 | `[CONFIRMED]` SEC2 正式安全启动路径必须 sign + encrypt | `CR-0003`; `C-BOOT-05` |
| Runtime image 默认策略 | `[ASSUMED]` PM/RAS/Codec USER/PROD 默认 sign + encrypt；signature-only 为 `[TBD]` 白名单例外 | `CR-0003` |
| Board binding 阶段策略 | `[ASSUMED]` 默认进入 attestation，不默认阻断 SEC1；是否参与 SEC2/runtime release decision 保持 `[TBD]` | `CR-0003` |
| eHSM native 固件包契约 | `[CONFIRMED]` 平台侧固件制作工具与设备侧 verify/decrypt 路径共享 eHSM native header + NGU protected manifest 契约 | `CR-0006`; `C-BOOT-08` |

## 11.2 仍需冻结的开放问题

| Open Item | Blocking Area | 当前状态 | 需要的决策 |
|---|---|---|---|
| 除 SEC1/SEC2 外，哪些非敏感 runtime image 允许 signature-only | Boot / Key / Product Policy | `[OPEN]` | SEC2 已强制加密；冻结 PM/RAS/Codec 或其他 runtime image 的白名单和准入条件 |
| Recovery image 独立策略 | Boot / Recovery / RMA | `[OPEN]` | 冻结 image_type、signer、trust anchor、rollback counter、decrypt policy |
| X.509 full cert chain 是否首版强制 | Key / Cert / Attestation | `[TBD]` | 冻结证书基础设施成熟度与 report / image 携带方式 |
| report 中 image protection policy / decrypt_applied / board_bind_result 字段位置 | Attestation / SPDM | `[OPEN]` | 冻结放在 measurement flags、lifecycle block 还是独立 policy block |
| board binding 是否参与 SEC2/runtime release decision | Board / Boot / Manufacturing | `[TBD]` | V2.4 默认进入 attestation、不阻断 SEC1；后续冻结 release decision |
| JTAG scope bitmap 与 CPLD/MUX 控制权 | Debug / Board / RTL | `[TBD]` | 冻结 scope 位图、MUX 控制寄存器、关闭策略 |
| DMA / firewall / UserID / 地址白名单 | Board / Interface / RTL | `[TBD]` | 冻结管理子系统 DMA 可访问 buffer、UserID 和 firewall region |
| OOB/BMC provisioning transport proxy 字段级设计 | Manufacturing / Board | `[OPEN]` | 允许 transport proxy，但需冻结命令格式、认证、审计、失败回滚、rate limit / lockout |
| PowerBrake / PG / FAULT / reset 是否进入主 report | Board / Attestation | `[OPEN]` | 冻结哪些电源/复位事件进入安全状态机、审计、主 report 或扩展 event log |
| RMA 完成后是否强制重新生成 attestation / 状态摘要 | RMA / Attestation | `[OPEN]` | 冻结返修交付闭环要求 |
| Image packager CLI / manifest ABI / golden vector | Tooling / BootROM / SEC FW / eHSM Adapter | `[OPEN]` | 冻结 `ngu_image_manifest_t` bit-level ABI、packager CLI、source-conformance report 和联调用 golden vector |

## 11.3 依赖项

| Dependency | 影响 | 当前处理 |
|---|---|---|
| eHSM 字段级 TRM / key policy 细节 | FW_KEK、per-image CEK extension、Version Counter、OTP/control field 字段冻结 | 当前按 source-conformance matrix 跟踪；未获 eHSM owner 确认的项保持 `[TBD]` |
| 管理子系统字段级接口 | OOB、JTAG、DMA、power/reset 安全控制字段 | 当前遵循总体架构和流程，安全边界以安全基线裁决 |
| 产品安全策略 | runtime signature-only 白名单、非安全启动例外、debug 售后策略 | 当前强制 SEC1/SEC2；PM/RAS/Codec 默认 sign+encrypt，signature-only 白名单保留策略冻结项 |
| 制造工站 / HSM / KMS | Root/UDS/FW_KEK/provisioning 审计流程 | 当前定义流程级要求，工站接口需进一步冻结 |

## 11.4 旧待补清单处理结果

原 master 中关于板级安全的待补项已关闭：板级安全设计已作为第 7 章正式并入 master。仍未冻结的 board binding、JTAG scope、DMA/firewall/UserID、OOB provisioning proxy 等，不再作为“待补章节”，统一保留在本章开放问题和 `00_project/open_questions.md` 中追踪。
---
# 12. 附录

## 12.1 本整合版来源文件

| 章节 | 来源文件 |
|---|---|
| 1. 设计基线摘要 | `security_workflow/02_baseline.md` |
| 2. 安全总体架构 | `01_constraints.md`、`02_baseline.md`、`CR-0001` 收敛口径 |
| 3. 安全启动详细设计 | `security_workflow/03_detailed_design/01_boot.md` |
| 4. 设备身份与远程度量证明设计 | `security_workflow/03_detailed_design/03_attestation.md` |
| 5. 安全调试与生命周期控制 | `security_workflow/03_detailed_design/04_lifecycle_debug.md` |
| 6. 内外部接口设计 | `security_workflow/03_detailed_design/06_interface.md` |
| 7. 板级安全设计 | `security_workflow/03_detailed_design/05_board_security.md` |
| 8. Root of Trust、密钥体系与证书体系 | `security_workflow/03_detailed_design/02_key_cert.md` |
| 9. 制造、灌装、部署与 RMA | `security_workflow/03_detailed_design/07_manufacturing_rma.md` |
| 10. 实现级落地详设全集 | `security_workflow/04_impl_design/*.md`，按 CR-0005 全量嵌入 |
| 11. 风险、依赖、冻结项与开放问题 | 各章节开放问题、`00_project/open_questions.md`、`CR-0001` 至 `CR-0006` |

## 12.2 编号整理规则

- 本 master 采用单一阿拉伯数字多级标题体系。
- 源章节原始一级标题不再重复出现，避免 `# 二、...` 与 `# 5. ...` 并列。
- 源章节正文、实现级分片正文和表格尽量原样保留，仅按整合版章节号重编号。
- Root of Trust 核心基线保留在第 1 / 2 章，Root / Key / Cert 详细设计后置到第 8 章。
- CR-0005 后，第 10 章是代码落地的完整实现级详设入口；`04_impl_design` 仅为编辑分片 / extracted implementation shard。
- CR-0006 后，固件包制作与设备侧 verify/decrypt 流程必须在第 3 章和第 10 章同时可见，且不得回退到旧 NGU 自定义 physical header。

## 12.3 CR 执行记录

| CR ID | 主题 | 执行结果 |
|---|---|---|
| `CR-0001-sec1-encryption-fw-protection-master-sync` | SEC1 加密、固件保护链、Master 章节重排与板级安全并入 | 已按 CR 落地到源章节、实现级文件、master、导出版和追踪记录 |
| `CR-0004-ehsm-native-header-otp-layout-alignment` | eHSM native header、OTP/key/counter source-conformance | 已按 CR 落地到主详设和实现级分片，并在第 10 章完整可见 |
| `CR-0005-single-full-design-code-landing-spec` | `10_full_design.md` 作为唯一代码落地详设入口 | 已将 `04_impl_design` 分片正文全量嵌入第 10 章，并将分片定位为非独立事实源 |
| `CR-0006-firmware-package-build-verify-flow` | 固件包格式、平台侧制作流程与设备侧 verify/decrypt 流程 | 已补充固件包布局图、制作流程图、设备侧验证时序图，并同步 code rules / traceability / open questions |
