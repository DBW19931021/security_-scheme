---
title: "NGU800 安全软件方案（精简版，取消 FMC A/B，采用单分区 + OOB MCU 恢复刷写）"
author: ""
date: "2026-05-27"
lang: zh-CN
---

# 修订说明

> 当前方案源：2026-06-03 起，`security_inputs/current_plan/芯片安全软件方案_2.0.pdf` 为当前收敛方案基线；除后续特殊说明外，本文按该版解释。

本文基于《芯片安全软件方案_2.0》和《NGU800 安全方案详细设计说明书》重新整理，版式和精简度参考历史《安全软件方案》。本版只生成 Markdown 文档，保留关键 Mermaid 架构图、流程图、时序图和状态机图。

本次修订将 FMC 防变砖策略从“BootROM 管理 FMC 固定分区 自动 fallback”调整为：

> **首版不在 BootROM 中实现 FMC A/B 分区和 slot metadata 状态机，采用单 FMC 固定分区 + OOB MCU 通过 QSPI 恢复刷写的方案。**

该调整的核心目标是降低 BootROM 复杂度，使 BootROM 只保留固定地址取包、调用 eHSM verify/decrypt、最小 manifest 检查、measurement 和跳转逻辑。FMC 损坏、刷写中断、版本不一致或校验失败时，设备进入 OOB 恢复状态，由纳入板级安全边界的 OOB MCU 通过 QSPI 对 NOR Flash 中的固定 FMC 分区执行授权重刷写。OOB MCU 刷写不替代 BootROM/eHSM 的启动可信裁决，FMC 是否可执行仍由下一次上电或复位后的 BootROM + eHSM 决定。

本版采用以下整理原则：

1. 以 `芯片安全软件方案_2.0.pdf` 和 NGU800 最新详细设计为事实源；旧版《安全方案》仅作为历史章节组织和表述颗粒度参考。
2. 正文使用确定性方案口径；未冻结内容集中放入最后一章“冻结项”。
3. 保留详细设计中的关键图形化流程，并将原 FMC A/B 相关图替换为“单 FMC 分区 + OOB MCU 恢复刷写”相关图。
4. 启动链以 `BootROM -> FMC(SEC1) -> GSP(SEC2) -> Runtime FW` 为主线；Host 只投递 GSP 及后续镜像，不通过 SoC 安全启动链路下发或替换 FMC。
5. OOB MCU 纳入板级安全边界，可作为受控带外刷写主体通过 QSPI 更新 NOR Flash 中的固定 FMC 分区；OOB MCU 不持有 eHSM 根材料，不替代 eHSM 验证结果，不直接放行 FMC。
6. 证书体系采用静态 Device Attestation Cert + measurement report 的证明模式，不把动态 Alias 证书链作为首版强制路径。

# 1. 概述

本方案面向 NGU800 的全生命周期安全能力建设，覆盖安全启动、固件验签与解密、安全服务接口、生命周期与安全 Debug、设备远程认证、制造灌装、RMA 以及板级/OOB 安全边界。

## 1.1 总体目标

- 建立以 eHSM 为唯一 Root of Trust 的安全启动与安全服务体系。
- 保证 FMC(SEC1)、GSP(SEC2) 和关键 Runtime 固件在执行前完成验签、解密、反回滚和 release policy 检查。
- 将 Host/BMC 等外部管理实体限定为不可信请求方或链路承载方；将 OOB MCU/板级安全 MCU 纳入板级安全边界，作为受控带外刷写、电源复位和管理代理，但不进入 eHSM Root of Trust。
- 采用单 FMC 固定分区，降低 BootROM 中的 slot 选择、metadata 状态机、boot confirm 和 fallback 逻辑复杂度。
- 将 FMC 损坏恢复能力转移到 OOB MCU + QSPI 板级恢复通道；允许 Host/BMC/运维工具通过 OOB 通路重发 FMC 包并触发重刷写。
- 建立可导出的 measurement table 和 attestation report，使验证方能够判断设备身份、固件版本、生命周期、Debug 状态、安全启动状态和 OOB 恢复状态。
- 建立制造灌装、USER 冻结、密钥轮换、RMA 和审计流程，支撑量产部署和售后闭环。

## 1.2 核心裁决

| 项目 | 裁决 |
| --- | --- |
| Root of Trust | eHSM 是唯一 Root of Trust；Root Secret、key usage、counter、lifecycle、debug auth 和 attestation key 使用均由 eHSM 控制。 |
| BootROM 角色 | BootROM 是最早启动编排者，负责最小初始化、读取固定 FMC 分区、调用 eHSM verify/decrypt、最小 manifest 检查、measurement 和跳转；不实现复杂密码库，不持有私钥，不实现 FMC A/B 状态机。 |
| First Mutable Stage | FMC 是 SEC1，对应 `fmc-bm.bin` / `solutions/fmc`，来自本地 NOR Flash 固定分区，不由 Host 通过 SoC 安全启动链路下发。 |
| Runtime Security Control | GSP 是 SEC2，对应 `gsp-bm.bin` / `solutions/gsp`，负责 Host/OOB 请求收敛、后续固件验证、measurement、attestation、debug/RMA 和 provisioning 编排。 |
| Host 边界 | Host 只投递 GSP 和 Runtime 固件包、发起请求、读取状态；FMC 损坏恢复时，Host/BMC 可通过 OOB MCU 发起 FMC 重刷写；Host 不直接写 eHSM、eFuse、Secure SRAM、release register 或安全策略区。 |
| OOB MCU 边界 | OOB MCU/板级安全 MCU 纳入板级安全边界，允许通过 QSPI 对 NOR Flash 固定 FMC 分区进行授权刷写；自身必须安全启动，刷写包必须受授权，最终启动仍由 BootROM/eHSM 验证。 |
| FMC 防变砖模型 | 首版取消 BootROM 管理的 FMC A/B 自动 fallback。FMC 损坏、掉电中断或校验失败后，BootROM 记录失败状态并进入 OOB 恢复等待；OOB MCU 负责重新刷写固定 FMC 分区。 |
| 固件保护 | FMC、GSP 正式路径强制签名 + 加密；PM/RAS/Codec 等关键 Runtime 固件默认签名 + 加密，产品白名单允许的非敏感固件可采用 signature-only。 |
| 镜像格式 | 采用 eHSM native secure image package；NGU 项目级 metadata 放入被 eHSM 认证/解密保护的 NGU protected manifest。 |
| 算法策略 | 支持国密和国际算法栈；算法 authority 来自 eHSM control field，manifest/report 只记录 expected profile。 |
| 远程认证 | GSP 作为 SPDM Responder / attestation 控制面，eHSM 使用 Device Attestation Private Key 完成签名。 |
| Debug/RMA | USER/PROD 默认关闭 Debug；开启必须经过 challenge-response、scope、过期策略和审计。 |

## 1.3 安全基线架构

### 图 1-1 安全基线最小架构

```mermaid
graph TD
    subgraph SOC["SoC 安全域"]
        BR[BootROM]
        SEC[SEC Core / C908]
        EH[eHSM]
        eFuse[eFuse]
        FW[FMC / GSP / Runtime FW]
        STAT[Boot fail / recovery status]
    end

    subgraph BOARD["板级安全边界"]
        OOB[OOB MCU / Board Security MCU]
        NOR[NOR Flash<br/>Fixed FMC Partition<br/>GSP/Runtime storage optional]
    end

    Host[Host / Driver] -->|GSP/runtime package| SEC
    BMC[BMC / OOB Host] -->|FMC recovery package| OOB
    OOB -->|controlled request| SEC
    OOB -->|QSPI erase/write/readback| NOR
    BR -->|read fixed FMC package| NOR
    BR -->|verify/decrypt request| EH
    BR -->|write failure reason| STAT
    OOB -->|read recovery_required/status| STAT
    SEC --> EH
    EH --> eFuse
    SEC --> FW
    Host -. no trust .-> EH
    BMC -. no RoT .-> EH
    OOB -. no root key access .-> EH
```

### 图 1-2 安全启动与 OOB 恢复总时序

```mermaid
sequenceDiagram
    participant BMC as Host/BMC/Service Tool
    participant OOB as OOB MCU
    participant NOR as NOR Flash
    participant BR as BootROM
    participant EH as eHSM
    participant FMC as FMC/SEC1
    participant GSP as GSP/SEC2

    BR->>NOR: 读取固定 FMC package
    BR->>EH: VERIFY_FMC + decrypt mandatory
    alt FMC verify/decrypt PASS
        EH-->>BR: PASS + plaintext Code region
        BR->>BR: manifest / load / entry / measurement check
        BR->>FMC: jump
        FMC->>GSP: verify/release GSP and runtime FW
    else FMC verify/decrypt FAIL
        EH-->>BR: FAIL + error code
        BR->>BR: 记录 boot_fail_reason / recovery_required
        BMC->>OOB: 下发 FMC recovery package
        OOB->>OOB: 验证 update authorization / policy
        OOB->>NOR: QSPI 擦除并重写固定 FMC 分区
        OOB->>NOR: readback/hash 校验
        OOB->>BR: 触发 SoC reset 或等待下次上电
    end
```

# 2. 安全总体架构

## 2.1 逻辑架构

### 图 2-1 总体安全架构

```mermaid
graph TD
    subgraph SOC["SoC 安全域"]
        BR["BootROM"]
        EH["eHSM"]
        eFuse["eFuse / Root / Counter / Lifecycle"]
        FMC["FMC / SEC1"]
        GSP["GSP / SEC2"]
        FW["PM / RAS / OMP / RMP / Other FW"]
        ST["Secure status registers<br/>boot_fail_reason / recovery_required"]
    end

    subgraph BOARD["板级安全边界"]
        OOB["OOB MCU / 板级安全 MCU"]
        FLASH["NOR Flash / local Flash<br/>Fixed FMC Partition"]
    end

    HOST["Host"] -->|"deliver GSP/runtime protected packages"| FMC
    BMC["BMC / OOB Host"] -->|"management/update/recovery command"| OOB
    OOB -->|"controlled request / board flow"| GSP
    OOB -->|"QSPI flash recovery write<br/>fixed FMC partition"| FLASH

    BR -->|"locate FMC at fixed offset"| FLASH
    BR -->|"VERIFY_FMC: verify + decrypt mandatory"| EH
    EH --> eFuse
    BR -->|"load controlled result"| FMC
    BR -->|"failure visible to OOB"| ST
    OOB -->|"poll/read status"| ST
    FMC -->|"Host channel + VERIFY_IMAGE"| GSP
    GSP -->|"verify / measure / release"| FW
    GSP -->|"Mailbox security services"| EH

    HOST -.->|"no trust"| EH
    BMC -.->|"no RoT"| EH
    OOB -.->|"no root key access"| EH
```

## 2.2 角色职责

| 角色 | 软件职责 | 安全边界 |
| --- | --- | --- |
| BootROM | 最小平台初始化、读取 secure boot/lifecycle 配置、从固定 NOR Flash 地址读取 FMC package、调用 eHSM 验证和解密、做最小 manifest 检查、记录 measurement、跳转 FMC。 | 不持有私钥，不直接实现复杂验签/解密，不信任 Host 输入，不实现 A/B slot 选择和 fallback。 |
| FMC(SEC1) | 建立基础安全运行环境、初始化 Host 通道、接收 GSP 包、调用 eHSM 验证 GSP、向 GSP 转交运行期安全控制面。 | 只在 eHSM PASS 和 manifest policy PASS 后放行 GSP。 |
| GSP(SEC2) | 后续固件验证、release 控制、measurement 维护、SPDM Responder、Debug/RMA、Provisioning 编排。 | 是运行期安全控制面，外部请求必须经其收敛。 |
| eHSM | Root Secret 使用、签名验证、解密、KDF/key unwrap、counter、lifecycle、debug auth、attestation signing。 | 私钥和根材料不出 eHSM，不接受 Host 直连调用。 |
| Host/Driver | 传输 GSP/Runtime 固件包，发起 SPDM、状态查询、升级、Debug/RMA 请求；FMC 恢复时通过 BMC/OOB 通道重发 FMC recovery package。 | 不可信，不参与信任链，不拥有 release 权，不直接写 eHSM/eFuse/secure SRAM。 |
| BMC/OOB Host | 发起带外管理、运维、升级、FMC 恢复和电源控制请求。 | 不可信外部请求方，不直接访问 eHSM/eFuse；FMC 恢复包必须经 OOB MCU 授权检查。 |
| OOB MCU/板级安全 MCU | 纳入板级安全边界；承载带外代理、电源/复位、状态采集、JTAG MUX 控制和 QSPI 刷写 NOR Flash 固定 FMC 分区。 | 必须自身安全启动和受控升级；不进入 eHSM Root of Trust，不持有根材料，不直接判定 FMC 可执行。 |

## 2.3 信任边界

- eHSM、eFuse、Secure SRAM、SEC 执行区、证书/策略区属于 SoC 安全域。
- OOB MCU 属于板级安全边界内的受控执行体，安全级别高于 BMC/Host 链路，但不等于 SoC Root of Trust。
- Host PCIe inbound、BMC/OOB Host、SMBus/I2C/I3C、JTAG、UART、管理 DMA 都属于不可信输入面。
- Host 和 OOB 只能访问普通 staging buffer、普通状态寄存器、OOB recovery 状态和白名单接口。
- release、debug enable、lifecycle、flash secure write、rollback counter、key revoke 等高危控制面必须由 SEC/eHSM 控制。
- 固定 FMC 分区可由 OOB MCU 通过 QSPI 在授权维护流程下重写，但重写后的 FMC 必须经过 BootROM + eHSM 重新验证后才可执行。

## 2.4 顶层安全路径时序

### 图 2-2 BootROM / SEC / eHSM / Host / OOB 交互时序

```mermaid
sequenceDiagram
    participant BR as BootROM
    participant EH as eHSM
    participant FMC as FMC/SEC1
    participant GSP as GSP/SEC2
    participant H as Host
    participant OOB as OOB MCU

    BR->>EH: VERIFY_FMC(fixed_flash_addr, output_buf)
    EH-->>BR: PASS/FAIL
    alt PASS
        BR->>FMC: load + jump
        H->>FMC: deliver GSP protected package
        FMC->>EH: VERIFY_IMAGE(GSP)
        EH-->>FMC: PASS/FAIL
        FMC->>GSP: release GSP only if PASS
        H->>GSP: deliver runtime FW packages
        GSP->>EH: VERIFY_IMAGE(runtime FW)
        EH-->>GSP: PASS/FAIL
        GSP->>GSP: measure + release runtime cores
    else FAIL
        BR->>BR: set recovery_required + boot_fail_reason
        OOB->>BR: read/poll failure state through sideband/status
        OOB->>OOB: wait recovery package from Host/BMC
    end
```

# 3. 安全启动

## 3.1 安全启动架构

### 图 3-1 安全启动架构

```mermaid
graph TD
    PR[Power / Reset] --> BR[SoC BootROM]
    PR --> EH[eHSM ROM/BL/FW]
    PR --> OOB[OOB MCU secure boot]

    OOB -->|QSPI recovery write| FLASH[NOR Flash<br/>Fixed FMC Partition]
    BR --> CFG[secure_boot_enable / lifecycle / strap / control field]
    EH --> eFuse[eFuse / Key / Lifecycle]

    BR -->|read fixed FMC package| FLASH
    BR -->|VERIFY_FMC via eHSM| EH
    EH -->|verify + decrypt PASS/FAIL| BR

    BR -->|load+jump| SEC1[FMC / SEC1]
    BR -->|on FAIL: recovery_required| STATUS[Boot failure status]
    OOB -->|read status| STATUS
    SEC1 -->|PCIe init / host channel| HOST[Host]
    HOST -->|deliver GSP + PM/RAS/Codec| STAGE[Staging Buffer]
    SEC1 -->|VERIFY_IMAGE| EH
    EH -->|PASS/FAIL| SEC2[GSP / SEC2]
    SEC2 -->|verify + measure + release| OTHERS[PM/RAS/Codec Cores]
```

## 3.2 安全启动详细时序

### 图 3-2 安全启动详细时序

```mermaid
sequenceDiagram
    participant BR as BootROM
    participant EH as eHSM
    participant FL as NOR Flash
    participant S1 as FMC/SEC1
    participant H as Host
    participant S2 as GSP/SEC2
    participant MC as PM/RAS/Codec
    participant OOB as OOB MCU

    BR->>BR: 最小平台初始化
    BR->>BR: 读取 secure_boot_enable / lifecycle / strap
    BR->>EH: 拉起 eHSM / 等待 ready
    EH->>EH: ROM/BL/FW 自检、eFuse装载、生命周期恢复
    BR->>FL: 从固定 offset 读取 FMC package
    FL-->>BR: 返回 FMC package 地址/内容
    BR->>EH: VERIFY_FMC(addr,len,output_buf,mandatory_decrypt)
    EH->>EH: header / key_id / revoke / version / hash / signature / mandatory decrypt
    EH-->>BR: VERIFY_PASS / VERIFY_FAIL
    alt FMC 验证通过
        BR->>BR: 解析受保护 manifest，检查 load/entry/lifecycle/policy
        BR->>BR: 记录 FMC measurement 和 decrypt_applied 状态
        BR->>S1: 装载并跳转
        S1->>S1: 基础初始化
        S1->>S1: PCIe 初始化 / Host 通道建立
        H->>S1: 下发 GSP package
        S1->>EH: VERIFY_IMAGE(GSP)
        EH-->>S1: PASS / FAIL
        alt GSP 验证通过
            S1->>S2: 装载并跳转
            S2->>H: 请求后续镜像
            H->>S2: 下发 PM/RAS/Codec 固件
            S2->>EH: VERIFY_IMAGE(PM/RAS/Codec)
            EH-->>S2: PASS / FAIL
            S2->>MC: 对通过校验的微核 release 执行
        else GSP 验证失败
            S1->>S1: 记录错误并保持安全控制面不放行
        end
    else FMC 验证失败
        BR->>BR: 记录 boot_fail_reason / recovery_required
        OOB->>BR: 读取失败状态
        OOB->>FL: 等待授权后通过 QSPI 重刷 FMC
    end
```

## 3.3 启动链

```text
BootROM -> FMC(SEC1, fixed NOR Flash partition) -> GSP(SEC2, Host 下发) -> PM/RAS/Codec/Other Runtime FW
```

关键规则：

1. FMC 来自 NOR Flash 固定分区，不由 Host 通过 SoC 启动链路下发。
2. BootROM 不选择 slot，不解析复杂 slot metadata，不执行 boot confirm/fallback 状态机。
3. BootROM 必须等待 eHSM ready，并调用 eHSM 完成 FMC 的 verify + decrypt。
4. FMC 和 GSP 在正式路径中必须签名 + 加密。
5. GSP 及后续 Runtime 固件由 Host 传输到 staging buffer，由 FMC/GSP 调 eHSM 验证后 release。
6. 任一关键镜像验证失败，不允许降级为继续执行。
7. FMC 验证失败后进入 OOB 恢复等待，由 OOB MCU 通过 QSPI 重刷固定 FMC 分区。

## 3.4 启动阶段

| 阶段 | 执行体 | 输入 | 动作 | 输出 |
| --- | --- | --- | --- | --- |
| A. BootROM early | BootROM | strap、lifecycle、control field | 最小初始化，等待 eHSM ready，定位固定 FMC 分区。 | eHSM ready 或失败码。 |
| B. FMC 验证 | BootROM + eHSM | 固定 FMC package | eHSM native verify/decrypt、rollback、revoke、manifest policy。 | FMC plaintext code region 或失败码。 |
| C. FMC 装载 | BootROM | eHSM 输出 buffer、NGU manifest | load/entry 白名单、measurement、跳转。 | FMC 执行。 |
| D. GSP 验证 | FMC + eHSM | Host 下发 GSP package | verify/decrypt、rollback、manifest policy、measurement。 | GSP 执行。 |
| E. Runtime 验证 | GSP + eHSM | Host 下发 Runtime FW | verify/decrypt 或 signature-only 白名单检查、measurement。 | PM/RAS/Codec release。 |
| F. OOB 恢复 | OOB MCU | Host/BMC 下发 FMC recovery package | 授权检查、QSPI 擦写、写后读回校验、触发 SoC reset。 | 新 FMC 等待 BootROM/eHSM 验证。 |

## 3.5 镜像策略

| 镜像 | 来源 | 验证发起 | 验证执行 | 强制保护 | Release Owner |
| --- | --- | --- | --- | --- | --- |
| FMC(SEC1) | 本地 NOR Flash 固定分区 | BootROM | eHSM | 强制签名 + 加密 + rollback + revoke | BootROM |
| GSP(SEC2) | Host/PCIe staging buffer | FMC | eHSM | 强制签名 + 加密 + rollback + revoke | FMC/GSP |
| PM/RAS/Codec | Host/PCIe staging buffer | GSP | eHSM | USER/PROD 默认签名 + 加密；signature-only 需白名单 | GSP |
| FMC recovery package | Host/BMC -> OOB MCU | OOB MCU 预检查；最终 BootROM/eHSM 裁决 | OOB MCU 授权检查 + BootROM/eHSM verify/decrypt | 包授权 + FMC 自身签名加密 | OOB MCU 只负责刷写，BootROM 最终放行 |

## 3.6 失败处理

- FMC verify/decrypt 失败：BootROM 记录 `boot_fail_reason`、`ehsm_status`、`manifest_status` 和 `recovery_required`，停止安全启动，等待 OOB 恢复。
- FMC manifest policy 失败：按 FMC 验证失败处理，不跳转。
- GSP 验证失败：FMC 拒绝跳转 GSP，保持基础安全控制面，允许 Host 重新下发 GSP。
- Runtime 固件验证失败：GSP 拒绝对应微核 release。
- eHSM 未 ready 或 mailbox 超时：BootROM/FMC/GSP 进入受控失败路径，不允许旁路。
- USER/PROD 生命周期不允许自动降级到非安全启动。

### 图 3-3 BootROM 单 FMC 分区状态机

```mermaid
stateDiagram-v2
    [*] --> Reset
    Reset --> Init: Power-on / reset
    Init --> WaitEHSM: minimal init
    WaitEHSM --> ReadFMC: eHSM ready
    WaitEHSM --> HaltRecovery: eHSM timeout
    ReadFMC --> VerifyFMC: fixed NOR Flash offset
    VerifyFMC --> CheckManifest: eHSM verify+decrypt PASS
    VerifyFMC --> HaltRecovery: verify/decrypt FAIL
    CheckManifest --> LoadJump: manifest policy PASS
    CheckManifest --> HaltRecovery: manifest/load/entry FAIL
    LoadJump --> FMCRunning
    HaltRecovery --> WaitOOB: set recovery_required + boot_fail_reason
    WaitOOB --> Reset: OOB rewrites FMC and resets SoC
```

## 3.7 FMC 单分区 + OOB MCU 恢复刷写

### 3.7.1 设计定位

首版不引入 BootROM 管理的 FMC A/B 自动 fallback。FMC 防变砖由板级恢复机制承担：

```text
FMC 固定分区损坏 / 刷写中断 / 版本不一致 / eHSM 校验失败
    -> BootROM 设置 recovery_required
    -> OOB MCU 读取失败状态
    -> Host/BMC 通过 OOB 通道重发 FMC recovery package
    -> OOB MCU 授权检查后通过 QSPI 重写固定 FMC 分区
    -> OOB MCU 触发 SoC reset
    -> BootROM + eHSM 重新验证 FMC
```

该方案的设计边界如下：

| 项目 | 裁决 |
| --- | --- |
| 自动 fallback | 首版不支持，由 OOB 恢复刷写替代。 |
| 掉电刷写保护 | 不要求 BootROM 具备掉电恢复状态机；若刷写中断，允许 Host/BMC 通过 OOB 通道重新发包并重刷。 |
| BootROM 复杂度 | 删除 slot metadata、active/fallback、boot confirm、boot attempt counter、metadata 原子更新逻辑。 |
| 恢复依赖 | 恢复依赖 OOB MCU 独立上电、独立安全启动和 QSPI ownership。 |
| 最终可信裁决 | OOB MCU 只负责刷写和写后校验；FMC 是否可信仍由 BootROM + eHSM 决定。 |

### 图 3-4 OOB MCU QSPI 恢复刷写流程

```mermaid
sequenceDiagram
    autonumber
    participant Tool as Host/BMC/Service Tool
    participant OOB as OOB MCU
    participant NOR as NOR Flash Fixed FMC Partition
    participant BR as BootROM
    participant EH as eHSM

    BR->>EH: VERIFY_FMC(fixed partition)
    EH-->>BR: FAIL
    BR->>BR: 设置 recovery_required / boot_fail_reason
    OOB->>BR: 读取 boot failure status
    Tool->>OOB: 下发 FMC recovery package + authorization token
    OOB->>OOB: 校验 token / lifecycle / version / package hash
    OOB->>NOR: QSPI 擦除固定 FMC 分区
    OOB->>NOR: QSPI 写入新的 FMC package
    OOB->>NOR: readback + hash 校验
    alt 写入校验通过
        OOB->>BR: 触发 SoC reset
        BR->>EH: VERIFY_FMC(new fixed partition)
        alt eHSM PASS
            BR->>BR: 记录 recovery_success measurement/audit
            BR->>BR: 加载并跳转 FMC
        else eHSM FAIL
            BR->>BR: 保持 recovery_required，等待再次重刷
        end
    else 写入校验失败
        OOB->>Tool: 返回失败，要求重发或重试刷写
    end
```

### 图 3-5 OOB 恢复刷写状态机

```mermaid
stateDiagram-v2
    [*] --> NormalBoot
    NormalBoot --> RecoveryRequired: BootROM FMC verify/decrypt FAIL
    RecoveryRequired --> AwaitPackage: OOB detects recovery_required
    AwaitPackage --> AuthPackage: receive FMC recovery package
    AuthPackage --> FlashWrite: auth/version/policy PASS
    AuthPackage --> AwaitPackage: auth FAIL
    FlashWrite --> ReadbackCheck: QSPI write done
    ReadbackCheck --> ResetSoC: readback/hash PASS
    ReadbackCheck --> AwaitPackage: write/readback FAIL
    ResetSoC --> NormalBoot: BootROM + eHSM PASS
    ResetSoC --> RecoveryRequired: BootROM + eHSM FAIL
```

### 3.7.2 OOB MCU 刷写约束

| 项目 | 约束 |
| --- | --- |
| OOB MCU 自身可信 | OOB MCU 必须有独立 secure boot、升级鉴权、调试关闭、生命周期或板级授权状态。 |
| QSPI ownership | OOB MCU 执行刷写前必须获得 NOR Flash QSPI ownership，SoC 侧不得同时访问。 |
| 写入范围 | OOB MCU 只允许写固定 FMC 分区和授权允许的恢复状态区，不得写 eHSM/eFuse/SEC 执行区/证书私钥区。 |
| 包授权 | FMC recovery package 必须有 update authorization token 或等价签名授权。 |
| 写后校验 | OOB MCU 必须执行 readback/hash 校验；该校验只证明写入一致性，不证明 FMC 可执行。 |
| 最终验证 | BootROM/eHSM 必须在下一次启动时重新验证签名、解密、rollback、manifest policy 和 load/entry 白名单。 |
| 重发重刷 | 若掉电或写入失败，Host/BMC 可重新下发 package，OOB MCU 可重复擦写固定 FMC 分区。 |
| 审计 | recovery package ID、version、hash、授权方、刷写结果、BootROM 验证结果进入 audit/attestation 可见状态。 |

## 3.8 单分区模式下的密钥轮换策略

取消 FMC A/B 后，key rotation 不再绑定 inactive FMC 的启动确认。首版采用“保守轮换 + OOB 可恢复”策略：

1. 新 signer、FW_KEK 或 key epoch 必须由升级授权域签发 key rotation capsule。
2. 在旧 key revoke 前，必须确保 OOB MCU 可写入至少一个被新 key 策略接受的 FMC recovery package。
3. 过渡期允许双 key 或交叉签名策略，确保旧 FMC 与新 FMC 在窗口期内至少有一种可恢复路径。
4. revoke old key 不由普通 FMC 包自行触发，必须由升级授权 capsule、eHSM policy 和生命周期共同约束。
5. 若 key rotation 后启动失败，OOB MCU 按新授权策略重刷 FMC；如新策略不可用，必须依赖尚未 revoke 的旧策略恢复。

### 图 3-6 单分区 FMC 与 key rotation 绑定流程

```mermaid
flowchart TD
    A[升级授权方签发 key rotation capsule] --> B[SEC/GSP 或 OOB MCU 校验 capsule]
    B --> C[安装 new signer / FW_KEK policy 为 PENDING]
    C --> D[生成或接收使用 new policy 的 FMC recovery package]
    D --> E[OOB MCU 确认可通过 QSPI 重刷固定 FMC 分区]
    E --> F{新 FMC 启动验证通过?}
    F -- 否 --> G[保持 old key ACTIVE，不 revoke，等待重发/重刷]
    F -- 是 --> H[new key epoch ACTIVE，记录 recovery_success]
    H --> I[old key 进入 DEPRECATED]
    I --> J{确认 OOB 可恢复路径已切到 new policy?}
    J -- 否 --> K[禁止 revoke old key]
    J -- 是 --> L[按客户策略 revoke old key]
```

# 4. 固件包格式与验证解密流程

### 图 4-1 eHSM native package 与 NGU manifest 分层

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

### 图 4-2 SEC1 secure image file 组成

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

## 4.1 eHSM native package

FMC、GSP 和受保护 Runtime 固件采用 eHSM native secure image package。物理格式以 eHSM TRM 为准，NGU 不再定义第二套 physical verification header。

```text
[eHSM native header, plaintext, 1KB]
    Signature / Public_Key / IV / Valid_Flag / Image_Type / Plain_Flag
    Naked_Flag / Code_Size / Version_Counter / Public_Key_Ext / Reserved

[Code region, verified/decrypted by eHSM]
    NGU protected manifest
    firmware payload
    padding/alignment
```

## 4.2 eHSM header 字段使用规则

| 字段 | NGU 使用规则 |
| --- | --- |
| Signature | eHSM 原生签名或认证材料，NGU 不另行定义尾随 signature。 |
| Public_Key / Public_Key_Ext | eHSM 原生公钥或 key material 字段，不作为 NGU 证书区。 |
| Encrypt_IV | 加密 IV/nonce 元数据，语义跟随 eHSM profile。 |
| Valid_Flag | eHSM image valid marker，不扩展为 NGU release policy。 |
| Image_Type | 只表达 eHSM 原生类型，不承载 NGU 的 FMC/GSP/PM/RAS/Codec。 |
| Plain_Flag | 必须与 NGU manifest 的 decrypt_required 一致；FMC/GSP 正式路径不得为明文 profile。 |
| Naked_Flag | 仅用于 eHSM 允许的测试/开发 profile，USER 不依赖裸镜像。 |
| Code_Size | 覆盖 manifest、payload、padding 的完整 Code region。 |
| Version_Counter | eHSM physical anti-rollback 输入；NGU rollback_domain 只能映射到该机制。 |
| Reserved | 必须按 TRM 处理，不承载 NGU 自定义字段。 |

## 4.3 NGU protected manifest

NGU 项目级字段全部放入 eHSM 认证/解密后的 Code region。BootROM/FMC/GSP 只能在 eHSM PASS 后解析 manifest。

| 字段语义 | 用途 |
| --- | --- |
| magic / manifest_version / manifest_size | 标识 manifest ABI，拒绝未知或不兼容版本。 |
| ngu_image_type | 表达 FMC、GSP、PM、RAS、Codec 等项目级类型。 |
| security_policy_flags | 表达 sign_required、decrypt_required、rollback_required、measure_required、release_policy。 |
| payload_offset / payload_size | 定位 payload，必须完全落入 Code_Size。 |
| load_addr / entry_addr | 装载和跳转地址，必须命中白名单。 |
| rollback_domain / version | 项目级版本视图，映射到 eHSM version counter。 |
| measurement_slot | 指定进入 measurement table 的 slot。 |
| lifecycle_mask | 指定允许的 lifecycle 集合，最终以 eHSM/eFuse lifecycle 为准。 |
| product_sku_mask | 产品/SKU 策略。 |
| board_binding_policy | 板级绑定结果的 attestation/release 表达。 |
| expected_algorithm_profile | 与 eHSM control field 做一致性检查，不覆盖 eHSM 算法选择。 |
| payload_digest | payload digest 描述，hash profile 跟随产品策略。 |
| extension_offset / extension_size | TLV 或扩展区，必须边界检查。 |

## 4.4 设备侧验证解密流程

1. BootROM/FMC/GSP 定位 package，检查 `image_addr`、`image_len`、`output_addr` 是否处于白名单区域。
2. 调用 eHSM `bl_verify_image`、`soc_verify` 或项目 wrapper。
3. eHSM 解析 native header，检查 `Image_Type`、`Plain_Flag`、`Code_Size`、`Version_Counter`、key/signer/revoke/rollback/signature。
4. eHSM 对 Code region 进行认证和解密，输出 plaintext Code region 到受控 output buffer。
5. 调用方在 eHSM PASS 后解析 NGU manifest。
6. 调用方检查 `ngu_image_type`、policy flags、lifecycle、rollback、load/entry、measurement slot、algorithm profile。
7. 通过后记录 measurement，装载 payload，并执行 release 或 jump。

## 4.5 固件包制作与设备侧校验流程图

### 图 4-3 平台侧 SEC1 固件制作流程

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

### 图 4-4 通用固件包制作流程

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

### 图 4-5 设备侧 verify/decrypt 与 manifest policy 流程

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

### 图 4-6 实现级设备侧包校验流程

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

# 5. SEC 与 eHSM 接口

## 5.1 接口模型

SEC 与 eHSM 之间使用 Mailbox + shared memory。Mailbox 寄存器只传递包地址、doorbell 和状态，完整请求/响应包放在共享内存。Host、BMC/OOB Host 不得直接调用 eHSM Mailbox；OOB MCU 即使纳入板级安全边界，也只能通过 SEC/GSP 代理或受控板级接口协作，不能直连 eHSM 原生命令。
### 图 5-1 内外部接口架构

```mermaid
graph TD
    H[Host / Driver / Verifier] -->|PCIe / Doorbell / Data Buffer| SEC[SEC / C908]
    BMC[BMC/OOB Host] -->|request| OOB[OOB MCU]
    OOB -->|受控请求| SEC
    BR[BootROM] -->|早期编排| SEC
    SEC -->|Mailbox Req + Shared Memory Ptr| EH[eHSM]
    EH -->|Mailbox Resp + Result| SEC
    EH --> eFuse[eFuse]
    EH --> ALG[Verify / Key / Debug Auth / Counter / Attestation]
    SEC --> FW[SEC2 / PM / RAS / Codec / Recovery]
    H -. no direct access .-> EH
    H -. no direct access .-> eFuse
```

### 图 5-2 Host/BMC 经 SEC 调用 eHSM 的时序

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

### 图 5-3 Mailbox 实现级逻辑关系

```mermaid
graph TD
    H[Host / BMC / 外部软件] -->|PCIe / IRQ / Flag| SEC[SEC / C908]
    SEC -->|Mailbox Command + Shared Memory Ptr| EH[eHSM]
    EH -->|Mailbox Response + Shared Memory Result| SEC
    SEC -->|状态 / 结果| H
    EH --> eFuse[eFuse]
    EH --> ALG[HASH / PKE / SKE / TRNG / Counter / UTC]
```

### 图 5-4 Mailbox 请求路径时序

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


## 5.2 通用请求头

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

## 5.3 通用响应头

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

## 5.4 命令表

| Cmd ID | 命令 | 用途 | Caller |
| --- | --- | --- | --- |
| 0x0001 | VERIFY_SEC1 / VERIFY_FMC | FMC 验签、强制解密、rollback、measurement。 | BootROM early path / SEC |
| 0x0002 | VERIFY_IMAGE | GSP/Runtime package verify/decrypt 和 manifest policy。 | SEC |
| 0x0003 | VERIFY_MEASURE | 验签、按策略解密并更新 measurement。 | SEC |
| 0x0020 | GET_CHALLENGE | Debug/SPDM/RMA challenge。 | SEC |
| 0x0021 | DEBUG_AUTH | Debug/RMA 授权校验。 | SEC |
| 0x0022 | CLOSE_DEBUG | 关闭 Debug scope。 | SEC |
| 0x0023 | CHANGE_LIFECYCLE | 生命周期切换。 | SEC |
| 0x0040 | READ_COUNTER | 读取 rollback/version counter。 | SEC |
| 0x0041 | INCREASE_COUNTER | 提升 counter。 | SEC |
| 0x0060 | KEY_DERIVE | eHSM 控制的 KDF/key unwrap。 | SEC |
| 0x0080 | GEN_ATTEST_REPORT | 生成或签署 attestation report。 | SEC |
| 0x00A0 | PROVISION_ROOT | 制造阶段灌装根材料、anchor 和 policy。 | SEC / MANU |

## 5.5 Verify Image 请求/响应字段

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

关键约束：

- `ehsm_image_type_expected` 只使用 eHSM TRM 语义。
- `ngu_image_type_expected` 用于 NGU release policy、measurement 和 attestation 映射。
- `expected_algorithm_profile` 只做一致性检查，不覆盖 eHSM control field。
- `jump_on_pass` 不能让 Host 间接控制跳转。
- `dst_addr` 必须命中 SEC/BootROM 白名单。
- `decrypt_result` 必须区分未执行、成功、失败和 policy mismatch。

# 6. 密钥、证书与制造灌装

### 图 6-1 Root of Trust、密钥与证书体系架构

```mermaid
flowchart TD
    subgraph OFFLINE["Offline HSM / CA / KMS"]
        BOOT_SIGN["Secure Boot Signing Key"]
        UPDATE_AUTH["Update Authorization Signing Key"]
        DEBUG_AUTH["Debug Authorization Signing Key"]
        DEV_ATTEST_CA["Device Attestation CA Signing Key"]
    end

    subgraph ANCHOR["Device eFuse / Secure Storage"]
        BOOT_ANCHOR["Secure Boot Signer Anchor"]
        UPDATE_ANCHOR["Update Authorization Anchor"]
        DEBUG_ANCHOR["Debug Authorization Anchor"]
        CERT_STORE["Device Attestation Cert / Cert Block"]
        REVOKE_POLICY["Signer / Debug Revoke Bitmap"]
        KEY_EPOCH_POLICY["Key Epoch Policy"]
        CONTROL_POLICY["Counter / Lifecycle / Control Field"]
    end

    subgraph EHSM["Device eHSM / eFuse Key System"]
        CRK["Chip Root Key / Root Secret"]
        DRK["DRK / Attestation Seed<br/>device-local protection context"]
        ENC_KEY["SOC Encrypt Key"]
        UPG_ENC_KEY["SOC Upgrade Encrypt Key"]
        FW_VERIFY_HANDLE["SOC FW Verify Handle"]
        UPG_VERIFY_HANDLE["SOC Upgrade Verify Handle"]
        DBG_VERIFY_HANDLE["SOC Debug Verify Handle"]
        DEV_ID["Device Attestation Private Key"]
        CNT["Version Counter / Lifecycle Service"]
    end

    BOOT_SIGN -->|"sign boot firmware"| FWPKG["FMC / GSP / Runtime Package"]
    UPDATE_AUTH -->|"sign update capsule"| UPDATE["Update Capsule"]
    UPDATE_AUTH -->|"sign key rotation capsule"| KEY_ROT_CAP["Key Rotation Capsule"]
    DEBUG_AUTH -->|"sign challenge + UID + scope + expiry"| DBG_TOKEN["Debug / RMA Auth Token"]
    DEV_ATTEST_CA -->|"sign device CSR / public key"| CERT_STORE

    BOOT_ANCHOR --> FW_VERIFY_HANDLE
    UPDATE_ANCHOR --> UPG_VERIFY_HANDLE
    DEBUG_ANCHOR --> DBG_VERIFY_HANDLE
    REVOKE_POLICY --> EHSM
    KEY_EPOCH_POLICY --> EHSM
    CONTROL_POLICY --> EHSM

    CRK --> DRK
    DRK --> DEV_ID
    EHSM --> ENC_KEY
    EHSM --> UPG_ENC_KEY
    EHSM --> CNT

    FWPKG -->|"verify signature"| FW_VERIFY_HANDLE
    FWPKG -->|"decrypt / unwrap"| ENC_KEY
    UPDATE -->|"verify update authorization"| UPG_VERIFY_HANDLE
    KEY_ROT_CAP -->|"verify key rotation authorization"| UPG_VERIFY_HANDLE
    DBG_TOKEN -->|"verify debug authorization"| DBG_VERIFY_HANDLE
    DEV_ID -->|"sign attestation report"| REPORT["Attestation Report"]
```

### 图 6-2 密钥、固件验证、证明与 Debug 授权时序

```mermaid
sequenceDiagram
    participant HSM as Offline HSM/CA/KMS
    participant EF as eFuse
    participant EH as eHSM
    participant SEC as SEC/C908
    participant FW as Firmware Package
    participant V as Verifier
    participant DBG as Debug Client

    HSM->>FW: sign boot firmware / update capsule / key rotation capsule
    HSM->>DBG: issue debug or RMA scoped token
    EF->>EH: 提供 UDS / Root Secret / signer anchor / lifecycle / counter / key slot policy
    EH->>EH: 初始化 eHSM key domains
    EH->>EH: 准备 device-local DRK / Attestation Seed / Device Identity KeyPair
    EH->>EH: 绑定 FW verify / encrypt key slots 与 lifecycle / key slot policy
    EH->>EH: 绑定 Debug authorization anchor 与 scope policy

    SEC->>EH: VERIFY / DECRYPT_IMAGE(req)
    EH->>FW: 使用 Soc FW Verify / Upgrade Verify key 或 signer anchor 校验镜像
    FW-->>EH: PASS / FAIL
    EH->>FW: 按 Soc Encrypt / Upgrade Encrypt / FW_KEK policy 解密输出
    FW-->>EH: decrypt output / policy result
    EH-->>SEC: verify / decrypt result

    V->>SEC: challenge / nonce
    SEC->>EH: GEN_ATTEST_REPORT
    EH->>EH: 使用 Device Attestation KeyPair / Attestation Seed 组织并签署 report
    EH-->>SEC: signed report
    SEC-->>V: report + Device Attestation Cert / cert reference
    V->>V: verify report by device attestation issuer anchor

    DBG->>SEC: debug/RMA auth request + scoped token
    SEC->>EH: DEBUG_AUTH
    EH->>EH: 使用 Debug authorization anchor 校验 challenge / UID / scope / expiry / reason
    EH-->>SEC: granted / denied
```

## 6.1 密钥体系

NGU800 密钥体系分为三类，不把所有密钥描述为一棵设备内派生树。

| 体系 | 私钥位置 | 设备侧保存内容 | 主要用途 |
| --- | --- | --- | --- |
| 外部签名/授权体系 | 离线 HSM/CA/KMS/签名服务器 | signer anchor、public key hash、revoke bitmap、debug anchor、update anchor | 固件签名、升级授权、Debug/RMA token、设备证书签发。 |
| 设备内部 eHSM key 体系 | eHSM/eFuse/KMU 内部 | key handle、control field、counter、lifecycle、key slot policy | 固件解密、key unwrap、attestation private key 使用、counter/lifecycle 控制。 |
| 设备身份与证书体系 | 证明私钥在 eHSM；证书签发私钥在离线 CA/HSM | Device Attestation Cert、issuer anchor、cert serial/hash | 设备身份和 report 签名验证。 |

## 6.2 外部签名/授权密钥

| 对象 | 用途 | 设备侧对应 |
| --- | --- | --- |
| secure_boot_signing_key | 签 FMC/GSP/Runtime 启动固件包。 | secure_boot_signer_anchor / signer hash。 |
| firmware_packaging_encrypt_key | 制作加密固件包或包裹 per-image CEK。 | soc_encrypt_key / soc_upgrade_encrypt_key / FW_KEK policy。 |
| update_authorization_signing_key | 签 update capsule、key rotation capsule。 | update_authorization_anchor / key_epoch_policy。 |
| debug_authorization_signing_key | 签 Debug/RMA scoped token。 | debug_authorization_anchor / debug revoke bitmap。 |
| device_attestation_ca_signing_key | 签 Device Attestation Cert。 | device_attestation_issuer_anchor 或 Verifier trust anchor。 |

## 6.3 设备内部 eHSM 对象

| 对象 | 位置 | 是否导出 | 用途 |
| --- | --- | --- | --- |
| UDS / Root Secret | eFuse/eHSM 安全区 | 否 | 设备内部根材料上游。 |
| chip_root_key / DRK | eHSM 内部 | 否 | device-local protection / attestation seed 域。 |
| soc_encrypt_key | eHSM/KMU | 否 | FMC/GSP Code region 解密。 |
| soc_upgrade_encrypt_key | eHSM/KMU | 否 | 升级包和 key material 保护。 |
| soc_fw_verify_handle | eHSM key handle / anchor | 不导出私钥 | 固件验签。 |
| soc_upgrade_verify_handle | eHSM key handle / anchor | 不导出私钥 | 升级/轮换授权验签。 |
| soc_debug_verify_handle | eHSM key handle / anchor | 不导出私钥 | Debug/RMA token 验签。 |
| device_attestation_private_key | eHSM 内部 | 否 | 签署 attestation report。 |
| version counter / lifecycle state | eHSM/eFuse | Host 不可写 | rollback、lifecycle gating。 |

## 6.4 设备证明证书流程
### 图 6-3 Device Attestation Key / CSR / 证书签发流程

```mermaid
sequenceDiagram
    participant DEV as Device eHSM
    participant SEC as SEC/GSP or Provisioning FW
    participant CA as Offline CA/HSM
    participant STORE as Device Cert Store

    SEC->>DEV: Generate or derive Device Attestation KeyPair
    DEV-->>SEC: Public Key / CSR, private key remains inside eHSM
    SEC->>CA: Submit CSR + chip_id + product info + manufacturing record
    CA->>CA: Verify manufacturing authorization
    CA-->>SEC: Device Attestation Cert / Cert Block
    SEC->>STORE: Write cert block to protected storage
    SEC->>DEV: Lock identity / lifecycle policy if required
```


```text
Device eHSM 生成或派生 Device Attestation KeyPair
    -> 仅导出 Public Key / CSR
    -> Offline CA/HSM 校验制造记录、chip_id、product info
    -> CA 签发 Device Attestation Cert / Cert Block
    -> 设备写入受保护证书区或由 Verifier 侧预置
    -> eHSM 使用私钥签署 report，Verifier 使用证书和 issuer anchor 验证
```

边界规则：

- Device Attestation Private Key 不导出。
- Offline CA/HSM 只获得 CSR/公钥，不获得设备私钥。
- Attestation report 必须绑定 nonce、measurement、lifecycle、debug、secure boot、rollback、FMC 固定分区版本/key epoch/recovery state 等状态。
- 固件验签、Debug 授权和设备证明使用不同授权域，不用 Device Attestation 成功替代 Debug Auth。

## 6.5 制造灌装流程

### 图 6-4 制造灌装架构

```mermaid
graph TD
    HSM[Factory HSM / KMS] --> TOOL[Provisioning Tool / 工站]
    TOOL --> HOST[Host / BMC / 工装链路]
    HOST --> SEC[SEC / C908]
    SEC --> EH[eHSM]
    EH --> eFuse[eFuse]
    EH --> CFG[Control Bits / Lifecycle / Counter]
    EH --> KEY[Root / Anchor / Attestation / Debug Objects]

    SEC --> BOOT[MANU 验证启动]
    BOOT --> USER[USER 冻结]
    USER --> DEPLOY[量产部署]
    USER --> RMA[RMA / DEBUG 授权返修]
    RMA --> RESTORE[恢复量产安全状态]
```

### 图 6-5 制造灌装时序

```mermaid
sequenceDiagram
    participant TOOL as Provisioning Tool
    participant HOST as Host/BMC
    participant SEC as SEC/C908
    participant EH as eHSM
    participant eFuse as eFuse

    TOOL->>HOST: 下发 provisioning 计划与 blob
    HOST->>SEC: 受控转发 provisioning 请求
    SEC->>SEC: 参数白名单检查 / lifecycle 检查
    SEC->>EH: PROVISION_ROOT_MATERIAL / 写 signer / 写 debug anchor / 写 counter
    EH->>eFuse: 写入目标区
    EH-->>SEC: 写入结果
    SEC->>EH: 校验 / 锁定 / 读状态
    EH-->>SEC: verify / lock result
    SEC->>SEC: 执行 MANU 验证启动
    SEC->>EH: CHANGE_LIFECYCLE(USER)
    EH->>eFuse: 更新 lifecycle / lock 路径
    EH-->>SEC: USER freeze result
    SEC-->>HOST: 返回冻结结果与审计状态
    HOST-->>TOOL: 工站记录结果
```

| 阶段 | 主要动作 |
| --- | --- |
| MFG-0 封测初测 | 测试路径、基础 bring-up、健康检查。 |
| MFG-1 板级 bring-up | 电源、时钟、接口、Die 连通性。 |
| MFG-2 Provisioning 准备 | 工站认证、算法 profile、灌装 blob 准备。 |
| MFG-3 Root/Anchor Provisioning | 写入 UDS/Root、secure boot anchor、update anchor、debug anchor、attestation anchor/cert。 |
| MFG-4 Control Bit Provisioning | 写 secure boot、debug、attestation、rollback、algorithm control field。 |
| MFG-5 校验与锁定 | 写入后校验、锁位、状态确认、审计。 |
| MFG-6 MANU 验证启动 | 使用正式策略进行近量产安全启动检查。 |
| MFG-7 USER 冻结 | 清理测试 trust，关闭未授权 debug，推进 USER lifecycle。 |
| MFG-8 出厂验收 | 形成最终审计记录和出厂状态。 |

USER 冻结前必须完成：根材料写入与锁定、signer/debug/update/attestation anchor 写入、counter/control field 配置、测试 key 和测试白名单清理、FMC/GSP 安全启动验证、设备证明证书或 anchor 配置、审计记录归档。

# 7. 生命周期与安全 Debug

### 图 7-1 生命周期与 Debug 控制架构

```mermaid
graph TD
    HOST[Host / BMC / OOB] -->|受控请求| SEC2[SEC2]
    SEC2 -->|DEBUG_AUTH / CHANGE_LIFECYCLE| EH[eHSM]
    EH --> eFuse[eFuse Lifecycle / Control Bits]
    EH --> DBG[Debug Auth / Challenge / Scope Control]
    SEC2 --> STATE[Secure Boot / Debug / Update / Non-secure Boot Policy]
    STATE --> REPORT[Attestation Report State Block]
```

## 7.1 生命周期模型

| 状态 | 语义 | 启动策略 | Debug 策略 | Provisioning |
| --- | --- | --- | --- | --- |
| TEST | 裸片/封测/初测 | 可开放测试路径，不等价量产 | 实验室开放 | 最小测试 |
| DEV | 开发/EVB | 可安全/非安全启动 | 开发授权 | 开发升级 |
| MANUFACTURE | 正式制造 | 优先安全启动 | 有限受控 | 正式灌装 |
| PROD/USER | 量产交付 | 强制安全启动 | 默认关闭，仅授权临时开启 | 禁止根材料写入 |
| RMA/DEBUG | 受权返修 | 受控签名启动 | 授权后有限开放 | 维修流程 |
| DEST/DECOMMISSIONED | 销毁 | 不允许正常启动 | 关闭 | 禁止 |

生命周期主路径为：

```text
TEST -> DEV -> MANUFACTURE -> PROD
PROD -> RMA (authorized)
ANY -> DECOMMISSIONED (irreversible)
```

## 7.2 Debug 授权流程

### 图 7-2 Debug 授权与生命周期切换时序

```mermaid
sequenceDiagram
    participant H as Host/Service Tool
    participant SEC as SEC2
    participant EH as eHSM
    participant eFuse as eFuse

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
        EH->>eFuse: update lifecycle / control field / lock path
        EH-->>SEC: success / fail
        SEC-->>H: result
    end
```

1. Host/BMC/服务工具向 GSP 发起 debug request，携带目标、scope 和用途。
2. GSP 检查 lifecycle、目标白名单、当前 secure boot 状态和请求格式。
3. GSP 调用 eHSM `GET_CHALLENGE`。
4. 工具提交 Debug/RMA scoped token，对 challenge、UID、scope、expiry、reason 签名。
5. GSP 调用 eHSM `DEBUG_AUTH`。
6. eHSM 校验 token、证书/anchor、revoke、lifecycle、scope 和过期策略。
7. 通过后返回 granted scope、expire policy、audit id。
8. GSP 只打开授权范围内的 JTAG/trace/debug window，并设置自动关闭条件。
9. 超时、复位、生命周期切换、安全错误或显式 `CLOSE_DEBUG` 时关闭 scope 并记录审计。

## 7.3 Debug scope

| Scope | USER 默认 | 授权要求 |
| --- | --- | --- |
| CPU halt / single-step | 关闭 | Debug auth + time limit。 |
| Trace visibility | 关闭 | Debug auth + trace scope。 |
| Secure memory visibility | 关闭 | RMA 特批 + narrow scope + audit。 |
| GPU register / DRAM access | 关闭 | target whitelist + range whitelist。 |
| Flash access/update | 关闭 | signed image + update/debug auth。 |
| 安全子系统访问 | 关闭 | eHSM debug auth + minimal scope。 |
| Boundary scan | 关闭 | manufacturing lifecycle gating。 |

# 8. 设备远程认证

### 图 8-1 设备认证架构

```mermaid
graph TD
    V[Verifier / Host] -->|Challenge / Nonce / Request| SEC2[SEC2]
    SEC2 -->|Mailbox GEN_ATTEST_REPORT| EH[eHSM]
    EH --> eFuse[eFuse / Root / Lifecycle / Counter]
    SEC2 --> MT[measurement_table]
    SEC2 --> ST[secure boot / debug / version state]
    EH -->|sign / key use| REP[Signed Report]
    SEC2 -->|organize blocks| REP
    REP --> V
```

### 图 8-2 认证报告生成时序

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

## 8.1 角色

| 角色 | 位置 | 职责 |
| --- | --- | --- |
| Requester | Host driver / Host security agent | 发起 SPDM、获取证书、challenge、measurements、report。 |
| Responder | GSP(SEC2) | SPDM Responder 状态机、measurement 汇总、report 组织。 |
| Signing service | eHSM | 使用 Device Attestation Private Key 对 report 或 signed region 签名。 |
| Verifier | Host 本地策略模块或远端验证服务 | 校验证书链、签名、nonce、measurement、lifecycle、debug、rollback 和策略。 |

## 8.2 SPDM 流程

1. 版本、能力和算法协商：`GET_VERSION`、`GET_CAPABILITIES`、`NEGOTIATE_ALGORITHMS`。
2. 获取证书：`GET_DIGESTS`、`GET_CERTIFICATE`。
3. 挑战认证：Host 发起 `CHALLENGE`，设备使用证明私钥签署 challenge 上下文。
4. 会话建立：需要加密会话时执行 `KEY_EXCHANGE`、`FINISH`。
5. 获取测量：Host 发起 `GET_MEASUREMENTS`，GSP 从 measurement table 读取度量和状态，调用 eHSM 签名后返回 report。
6. 策略判定：Verifier 校验证书、签名、nonce、版本、状态、measurement 和 rollback 门限。

国密路径使用 SM3 作为 hash、SM2 作为 signature/cert key，安全会话可采用 SM4-GCM。国际路径保留 SHA-256/SHA-384、ECDSA-P256/P384、RSA3072 等 profile。

## 8.3 Measurement table

measurement table 至少覆盖：

- BootROM / immutable identity / ROM version。
- FMC(SEC1) hash、version、rollback counter、image protection policy、decrypt_applied。
- GSP(SEC2) hash、version、rollback counter、image protection policy、decrypt_applied。
- PM/RAS/Codec 等 Runtime 固件 hash、version、rollback counter。
- lifecycle state、debug state、secure boot enable、anti-rollback enable。
- chip_id / device_uuid / die info。
- FMC 固定分区版本、key epoch、OOB recovery state、board/die binding state。

## 8.4 Report 结构

```text
Report Header
+ Identity Block
+ Nonce / Session Binding Block
+ Measurement Block(s)
+ Lifecycle / Debug / Secure Boot Block
+ Firmware Version / Rollback Block
+ Optional Cert Chain Block
+ Signature Block
```

### Report Header

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

### Measurement Block

```c
typedef struct {
    uint16_t block_type;
    uint16_t block_version;
    uint32_t block_len;
    uint32_t flags;
    uint32_t reserved0;
} ngu_measurement_block_header_t;

typedef struct {
    uint32_t fw_type;
    uint8_t  hash[32];
    uint32_t fw_version;
    uint32_t rollback_counter;
    uint32_t reserved0;
} ngu_meas_fw_info_t;

typedef struct {
    uint32_t lifecycle_state;
    uint32_t debug_state;
    uint32_t secure_boot_enable;
    uint32_t anti_rollback_enable;
    uint8_t  chip_id[16];
    uint32_t reserved0;
} ngu_meas_state_t;
```

签名覆盖范围必须包含 header、identity、nonce/session、measurement、lifecycle/debug/secure boot/rollback、firmware version 和 cert chain metadata。Verifier 不能只校验签名，还必须校验状态和策略。


# 9. 安全引擎与软件模块

## 9.1 安全引擎使用边界

| 场景 | 使用位置 | 说明 |
| --- | --- | --- |
| 安全启动 / 固件解密 / 设备认证 | eHSM / 安全域 | Root key、派生 key、证明私钥不离开安全域。 |
| Runtime 数据保护 / 多租户隔离 | 设备侧受控加密引擎 | 由安全控制面配置 key 和策略，不接受 Host 直接下发密钥。 |
| Host SPDM / 证书验证 / 会话保护 | Host 软件库或主机加速引擎 | Host 可验证证书链和 report，不接触设备内部私钥。 |
| OOB FMC 恢复刷写 | OOB MCU / 板级安全域 | OOB MCU 只执行授权检查、QSPI 写入和写后校验；不执行 eHSM Root 裁决。 |

## 9.2 Host 侧模块

| 模块 | 职责 |
| --- | --- |
| `gpu_secure_transport` | 对接 PCIe BAR、doorbell、mailbox/shared buffer，处理请求投递、响应读取、超时、分片。 |
| `gpu_fw_loader` | 下发 GSP 和 Runtime 固件包到 staging buffer，管理 descriptor 和重试。 |
| `gpu_oob_recovery_client` | 通过 BMC/OOB 通道向 OOB MCU 下发 FMC recovery package，处理重发、重刷、状态查询。 |
| `gpu_spdm_requester` | SPDM Requester 状态机，发起版本/能力/算法协商、证书、challenge、measurement 和 session。 |
| `gpu_attest_verify` | 校验证书链、report 签名、nonce、measurement、lifecycle/debug/rollback/OOB recovery 状态策略。 |
| `gpu_policy_agent` | 将认证结果映射为驱动策略，决定是否继续初始化、开放业务队列、进入集群或降级。 |
| `gpu_debug_tool_proxy` | Debug/RMA 请求代理，组织 scope、challenge、token 和结果展示。 |

## 9.3 Device 侧模块

| 模块 | 运行位置 | 职责 |
| --- | --- | --- |
| `bootrom_secure_boot` | BootROM | eHSM ready、固定 FMC 分区读取、VERIFY_FMC、output buffer、manifest check、measurement、jump、recovery_required 状态设置。 |
| `fmc_boot_mgr` | FMC | Host channel、GSP package 接收、GSP verify/decrypt、基础错误上报。 |
| `gsp_image_mgr` | GSP | Runtime 固件接收、verify/decrypt、measurement、release。 |
| `ehsm_mailbox_client` | FMC/GSP | 封装 eHSM mailbox request/response、token、timeout、cache flush/invalidate。 |
| `measurement_mgr` | GSP | 维护 measurement table，提供启动阶段写入和运行态状态更新。 |
| `spdm_responder` | GSP | SPDM Responder 状态机、证书/挑战/测量/会话处理。 |
| `attest_store` | GSP/eHSM adapter | 管理 Device Attestation Cert block、cert digest、issuer anchor 引用。 |
| `debug_lifecycle_mgr` | GSP | Debug/RMA challenge、scope、expire、lifecycle gating、audit。 |
| `provisioning_mgr` | GSP/MANU FW | MANU 阶段灌装、校验、锁定、USER 冻结和审计。 |
| `crypto_hal` | SEC/eHSM adapter | 统一封装 SM2/SM3/SM4、SHA/ECDSA/RSA、KDF、eHSM command。 |
| `policy_mgr` | GSP | lifecycle、SKU、rollback、board binding、debug、attestation 输出策略。 |

## 9.4 OOB MCU 侧模块

| 模块 | 运行位置 | 职责 |
| --- | --- | --- |
| `oob_secure_boot` | OOB MCU ROM/FW | OOB MCU 自身安全启动、固件验签、生命周期/调试控制。 |
| `oob_recovery_agent` | OOB MCU | 接收 Host/BMC 下发的 FMC recovery package，执行授权检查、版本检查、重发控制和恢复状态机。 |
| `qspi_flash_writer` | OOB MCU | 获取 QSPI ownership，擦写固定 FMC 分区，执行 readback/hash 校验。 |
| `soc_status_reader` | OOB MCU | 读取 BootROM 暴露的 `boot_fail_reason`、`recovery_required`、`ehsm_status` 等状态。 |
| `oob_audit_log` | OOB MCU | 记录 package ID、hash、版本、授权方、刷写结果、复位原因和恢复次数。 |
| `board_debug_mux_mgr` | OOB MCU | 接收 SEC/eHSM 授权结果后配置 JTAG/CPLD/MUX scope。 |

# 10. 板级/OOB 安全边界

本章将 OOB MCU / 板级安全 MCU 纳入板级安全边界。BMC/OOB Host 仍按外部不可信请求方处理；OOB MCU 是板级安全控制点，可以承载带外管理、电源/复位、JTAG MUX 控制和 QSPI 刷写 FMC，但不替代 eHSM Root of Trust，不持有根密钥，不直接放行任何 SoC 固件。

## 10.1 板级/OOB 安全架构

### 图 10-1 板级/OOB 安全架构

```mermaid
graph TD
    BMC[BMC / OOB Host] -->|SMBus/I2C / I3C / PCIe sideband| MCU[OOB MCU / 板级安全 MCU]
    MCU -->|受控管理请求| SEC[SEC / C908]
    SEC -->|Mailbox + Shared Memory| EH[eHSM]
    EH --> eFuse[eFuse / Lifecycle / Counter]

    MCU -->|QSPI ownership + write| NOR[NOR Flash<br/>Fixed FMC Partition]
    BR[BootROM] -->|read fixed FMC package| NOR
    BR -->|verify/decrypt FMC| EH
    BR -->|boot_fail_reason / recovery_required| STAT[Secure Status Registers]
    MCU -->|read recovery status| STAT

    MCU -->|Power / Reset / PG / Fault| PWR[电源与复位控制]
    MCU -->|scope-gated control| MUX[JTAG / CPLD / MUX]
    MUX --> DBG[CPU / GPU / Flash / DRAM / 安全子系统]
    SEC -->|debug auth result / scope| MCU

    BMC -. no direct trust .-> EH
    MCU -. no root key access .-> eFuse
    MCU -. no firmware release decision .-> BR
```

## 10.2 OOB MCU 安全定位

| 对象 | 边界定位 | 允许 | 禁止 |
| --- | --- | --- | --- |
| BMC/OOB Host | 外部不可信请求方 | 发起管理、恢复、Debug、状态查询请求 | 直接写 eHSM/eFuse、直接打开 debug、直接 release SoC 核 |
| OOB MCU / 板级安全 MCU | 板级安全边界内的受控执行体 | 受控转发请求、执行电源/复位、控制 JTAG MUX、通过 QSPI 重刷固定 FMC 分区 | 读取根密钥、替代 eHSM 验签、直接 release SoC 核、直接判定 FMC 可信 |
| SEC/GSP | SoC 运行期安全控制面 | 收敛 Host/OOB 请求、调用 eHSM、维护 measurement 和 audit | 暴露私钥或根材料给 Host/OOB |
| eHSM | SoC Root of Trust | 验签、解密、counter、lifecycle、debug auth、attestation signing | 接受 Host/BMC/OOB 直连调用 |

OOB MCU 的权限扩大到 QSPI 写 NOR Flash，但该权限只用于授权维护和 FMC 恢复刷写，不能成为绕过 SoC secure boot 的后门。固定 FMC 分区被 OOB MCU 重写后，仍必须由 BootROM + eHSM 在下一次启动中完成验证。

## 10.3 OOB MCU 通过 QSPI 刷写 FMC

### 图 10-2 OOB MCU QSPI 刷写与 SoC 启动验证关系

```mermaid
sequenceDiagram
    autonumber
    participant Tool as BMC/OOB Host/Service Tool
    participant OOB as OOB MCU
    participant NOR as NOR Flash Fixed FMC Partition
    participant Stat as Boot Status Register
    participant BR as BootROM
    participant EH as eHSM

    BR->>EH: VERIFY_FMC(fixed FMC partition)
    EH-->>BR: PASS / FAIL
    alt FAIL
        BR->>Stat: 写 recovery_required + boot_fail_reason
        OOB->>Stat: 读取恢复状态
        Tool->>OOB: 下发 FMC recovery package + auth token
        OOB->>OOB: 校验 auth token / package hash / version policy
        OOB->>NOR: 获取 QSPI ownership
        OOB->>NOR: 擦除并写入固定 FMC 分区
        OOB->>NOR: readback/hash 校验
        alt 校验通过
            OOB->>BR: 触发 SoC reset
            BR->>EH: VERIFY_FMC(new package)
            EH-->>BR: PASS / FAIL
        else 校验失败
            OOB->>Tool: 请求重发或重刷
        end
    else PASS
        BR->>BR: 加载并跳转 FMC
    end
```

## 10.4 OOB 刷写安全规则

| 项目 | 规则 |
| --- | --- |
| 入口控制 | OOB MCU 只在 MANU/RMA/授权维护或 BootROM 设置 `recovery_required` 后执行 FMC 刷写。 |
| 包授权 | FMC recovery package 必须携带 update authorization token、签名或等价授权。 |
| 版本策略 | OOB MCU 可执行包版本预检查；rollback 的最终裁决仍由 eHSM counter/control field 完成。 |
| 写入范围 | OOB MCU 只写固定 FMC 分区和允许的恢复状态记录区；不得写 eFuse、eHSM 私有区、SEC/GSP 执行区、证书私钥区。 |
| QSPI 仲裁 | OOB MCU 写 Flash 前必须获得 QSPI ownership；SoC 访问必须被关闭或硬件仲裁。 |
| 掉电模型 | 不要求一次刷写具备掉电原子性；掉电或写入失败后允许 Host/BMC 重新发包并由 OOB MCU 重刷。 |
| 写后校验 | OOB MCU 必须 readback/hash 校验写入一致性。 |
| 最终放行 | BootROM/eHSM 是 FMC 可执行性的最终裁决者。 |
| 审计 | OOB MCU 和 SoC 分别记录恢复请求、授权、刷写结果、复位原因、eHSM 验证结果。 |

## 10.5 板级 Debug/OOB 请求时序

### 图 10-3 板级 Debug/OOB 请求时序

```mermaid
sequenceDiagram
    participant Tool as BMC/OOB/Service Tool
    participant MCU as OOB MCU
    participant SEC as SEC/GSP
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
            SEC->>MCU: 下发授权 scope / expire
            MCU->>MUX: 配置受限 JTAG scope
            MUX->>Target: 打开受限调试路径
            MCU->>MCU: 记录审计事件
        else 授权失败
            SEC-->>MCU: deny
        end
    else 普通管理请求
        SEC->>SEC: 地址/权限/状态检查
        SEC-->>MCU: result / status
    end
```

## 10.6 带外通道策略

| 通道 | 管理用途 | 安全要求 |
| --- | --- | --- |
| SMBus/I2C | 低速状态查询、传感器、电源管理、OOB 管理 | 命令白名单、状态只读优先，高权限命令必须经 OOB MCU/SEC/eHSM。 |
| I3C | 高性能带外业务、固件更新、高频状态采集 | 数据路径隔离，更新/调试/provisioning 必须鉴权。 |
| SPI/QSPI | NOR Flash FMC 恢复刷写 | OOB MCU 可通过 QSPI 重刷固定 FMC 分区；必须有 ownership、写保护、授权检查和 BootROM/eHSM 二次验证。 |
| PCIe VDM | 带外数据 over PCIe | 默认关闭；启用前纳入 Host 不可信模型。 |
| UART | 调试输入输出 | 默认关闭；启用必须按 debug 接口管控。 |
| JTAG | GPU/CPU/DRAM/Flash/安全子系统调试 | USER 默认关闭，需 challenge-response、scope bitmap、MUX gating、审计。 |
| 管理 DMA | OOB/管理子系统内部数据搬运 | 只能访问白名单 buffer，不得访问 eHSM/eFuse/Secure SRAM/SEC 执行区。 |

## 10.7 单 Die / 双 Die

- 单 Die / 双 Die 差异不应暴露为安全策略绕过路径。
- 跨 Die 访问必须经过地址映射、权限和 firewall 检查。
- OOB MCU 对多 Die 或多 Flash 形态执行带外刷写时，必须明确 `die_id`、`flash_id`、固定 FMC 分区偏移和版本策略，避免刷错目标或跨 Die 降级。
- board binding / die binding 默认进入 attestation measurement 或状态摘要；是否阻断 runtime release 由产品策略冻结。

# 11. 图表保留说明

本版已保留并融入详细设计中的主要框架图、流程图、时序图和状态机图，覆盖总体架构、启动链、固件包格式、包制作与设备侧校验、单 FMC 分区启动、OOB MCU QSPI 恢复刷写、key rotation、SEC/eHSM Mailbox、密钥/证书/制造、生命周期与 Debug、设备认证、板级/OOB 安全边界。后续如需导出 PDF，可直接使用支持 Mermaid 的 Markdown/Pandoc 流水线。

# 12. SoC/硬件依赖与冻结项

## 12.1 硬件依赖

| 依赖项 | 要求 |
| --- | --- |
| eHSM 集成 | 支持 secure boot verify/decrypt、key/counter/lifecycle/debug/attestation 服务。 |
| eFuse/control field | 固化 Root Secret、signer anchor、lifecycle、counter、算法 profile、revoke policy。 |
| BootROM 固定启动地址 | BootROM 能从 NOR Flash 固定偏移读取 FMC package，并调用 eHSM verify/decrypt。 |
| Boot failure status | BootROM 能输出 `boot_fail_reason`、`ehsm_status`、`manifest_status`、`recovery_required`，供 OOB MCU 读取。 |
| Firewall/UserID | 保护 eHSM、eFuse、Secure SRAM、SEC 执行区、证书/策略区、secure status register。 |
| Mailbox/SOC_MEM | 支持 SEC 与 eHSM 的请求/响应、镜像输出 buffer、attestation report buffer。 |
| Release gate | GSP、PM、RAS、Codec 等微核 reset/release/fetch_enable 只能由安全控制面控制。 |
| NOR Flash 分区 | 支持固定 FMC 分区，具备 SoC BootROM 读取和 OOB MCU QSPI 擦写路径。 |
| OOB MCU + QSPI 刷写 | OOB MCU 纳入板级安全边界，支持独立安全启动、授权更新、QSPI ownership/arbiter、NOR boot-critical 写保护、固定 FMC 分区重刷和审计。 |
| Debug MUX | JTAG/CPLD/MUX 受 SEC/eHSM 授权结果控制，复位和超时自动关闭。 |
| 状态可观测 | secure boot、FMC version、rollback、debug、lifecycle、attestation、OOB recovery、board binding 状态可记录、可导出、可审计。 |

## 12.2 冻结项

| 冻结项 | 影响范围 |
| --- | --- |
| 固定 FMC 分区地址、大小、擦写粒度、对齐和 BootROM 读取规则。 | BootROM、NOR Flash layout、OOB QSPI、制造烧录。 |
| BootROM 最小 manifest 检查字段：`ngu_image_type`、policy flags、payload offset/size、load/entry、lifecycle、measurement slot、algorithm profile。 | BootROM、FMC package、eHSM adapter。 |
| Boot failure status ABI：`boot_fail_reason`、`ehsm_status`、`manifest_status`、`recovery_required`、恢复计数。 | BootROM、OOB MCU、BMC/运维工具、attestation。 |
| OOB MCU QSPI 刷写策略：OOB MCU secure boot、授权格式、QSPI ownership、固定 FMC 分区写入范围、重发重刷流程、审计字段。 | 板级 MCU、NOR Flash、BootROM、RMA/工装工具。 |
| 掉电恢复模型：首版接受重发重刷，不在 BootROM 中实现自动 fallback。 | 产品可用性、运维流程、OOB recovery 工具。 |
| key rotation 策略：单分区模式下的双 key 过渡、old key revoke 条件、OOB recovery package 与新 key policy 的绑定。 | eHSM key policy、升级授权、OOB 恢复、制造。 |
| eHSM native header + NGU protected manifest ABI。 | 打包工具、BootROM、FMC、GSP、eHSM。 |
| Mailbox ABI：cmd_id、req/resp header、token、status、err_code、共享内存规则。 | SEC FW、eHSM FW、QEMU/stub、驱动。 |
| SPDM report ABI：block type、cert block、signature block、measurement slot、算法 ID。 | Host verifier、GSP SPDM、eHSM signing。 |
| Debug scope bitmap：CPU/GPU/Flash/DRAM/安全子系统/板级 MCU/JTAG boundary scan。 | Debug MUX、OOB MCU、SEC/eHSM、RMA 工具。 |
| Manufacturing provisioning：灌装对象、锁定顺序、测试 key 清理、USER freeze checklist。 | 工站、eFuse、eHSM、审计系统。 |

# 13. 附：最小软件落地清单

| 阶段 | 软件任务 |
| --- | --- |
| QEMU/stub 阶段 | 定义 eHSM adapter API、secure image parser、manifest parser、错误码、measurement table stub。 |
| BootROM 联调阶段 | eHSM ready 等待、固定 FMC 地址读取、VERIFY_FMC、output buffer、manifest check、measurement、jump、recovery_required 设置。 |
| FMC 阶段 | Host channel、GSP package 接收、VERIFY_IMAGE(GSP)、GSP release、基础错误上报。 |
| GSP 阶段 | Runtime FW verify/decrypt、measurement、SPDM responder、attestation report、debug/RMA、provisioning。 |
| OOB MCU/QSPI 阶段 | OOB MCU secure boot、FMC recovery package 鉴权、QSPI ownership、固定 FMC 分区重刷、readback/hash、重发重刷、BootROM/eHSM 二次验证、审计。 |
| Host 阶段 | GSP/runtime loader、SPDM requester、verifier、policy agent、OOB recovery client。 |
| 制造阶段 | Root/anchor/cert provisioning、test key 清理、USER lock、audit log。 |
| 测试阶段 | golden/tamper vector、verify fail、decrypt fail、rollback fail、OOB recovery 重刷、掉电中断后重发、debug auth fail/pass、attestation 校验。 |
