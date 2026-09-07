# NGU800P 安全测试用例目录

本目录是飞书 `Security` 工作表的本地开发索引。最终业务字段以[飞书安全测试 Case 表](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)为准；本文用于后续 Codex 建立代码 Case、保持编号和追踪 ID 一致，不替代在线表。

核对日期：2026-08-20。最终表共 63 条，编号连续为 `NGU800P-D0-SECURITY-001～063`；`001～061`为软件/BSP主导的实现或协同范围，`062～063`由 RTL-DV/EDA 构造真实故障，软件只提供观察和清除能力。工程内 `NGU800P_Security_Case_Table_Final_v5.xlsx` 的 76 条口径仅作为历史快照保留。

## Mailbox BASIC：47条

### BL阶段：001～015

| 最终编号 | 原追踪ID | 最终用例描述 |
|---:|---|---|
| 001 | `MB-BL-001-BASIC` | eHSM BL版本读取 |
| 002 | `MB-BL-002-BASIC` | eHSM BL OTP读取 |
| 003 | `MB-BL-003-BASIC` | eHSM BL OTP写入与读回 |
| 004 | `MB-BL-007-BASIC` | eHSM BL镜像验签 |
| 005 | `MB-BL-008-BASIC` | eHSM BL随机根密钥生成 |
| 006 | `MB-BL-009-BASIC` | eHSM BL密钥加密 |
| 007 | `MB-BL-010-BASIC` | eHSM BL调试鉴权挑战值获取 |
| 008 | `MB-BL-011-BASIC` | eHSM BL调试鉴权 |
| 009 | `MB-BL-012-BASIC` | eHSM BL关闭调试 |
| 010 | `MB-BL-013-BASIC` | eHSM BL串口波特率设置 |
| 011 | `MB-BL-014-BASIC` | eHSM BL自检启动 |
| 012 | `MB-BL-015-BASIC` | eHSM BL自检结果读取 |
| 013 | `MB-BL-04-BASIC` | eHSM随机OTP密钥安装 |
| 014 | `MB-BL-05-BASIC` | eHSM加密OTP密钥安装 |
| 015 | `MB-BL-06-BASIC` | eHSM SoC密钥轮换命令综合验证 |

### FW阶段：016～047

| 最终编号 | 原追踪ID | 最终用例描述 |
|---:|---|---|
| 016 | `MB-FW-016-BASIC` | eHSM对称加解密 |
| 017 | `MB-FW-017-BASIC` | eHSM AES-GCM认证加解密 |
| 018 | `MB-FW-018-BASIC` | eHSM SM4-CCM认证加解密 |
| 019 | `MB-FW-019-BASIC` | eHSM SHA-256摘要 |
| 020 | `MB-FW-020-BASIC` | eHSM HMAC生成与验证 |
| 021 | `MB-FW-021-BASIC` | eHSM MAC生成与验证 |
| 022 | `MB-FW-022-BASIC` | eHSM SM2加解密 |
| 023 | `MB-FW-023-BASIC` | eHSM SM2签名与验签 |
| 024 | `MB-FW-024-BASIC` | eHSM RSA加解密 |
| 025 | `MB-FW-025-BASIC` | eHSM RSA签名与验签 |
| 026 | `MB-FW-026-BASIC` | eHSM ECDSA P-256签名与验签 |
| 027 | `MB-FW-027-BASIC` | eHSM随机数生成 |
| 028 | `MB-FW-028-BASIC` | eHSM FW OTP写入与读回 |
| 029 | `MB-FW-029-BASIC` | eHSM FW OTP读取 |
| 030 | `MB-FW-030-BASIC` | eHSM FW调试鉴权挑战值获取 |
| 031 | `MB-FW-031-BASIC` | eHSM FW调试鉴权 |
| 032 | `MB-FW-032-BASIC` | eHSM FW关闭调试 |
| 033 | `MB-FW-034-BASIC` | eHSM SoC镜像验签 |
| 034 | `MB-FW-035-BASIC` | eHSM随机OTP密钥安装 |
| 035 | `MB-FW-036-BASIC` | eHSM加密OTP密钥安装 |
| 036 | `MB-FW-038-BASIC` | eHSM生命周期切换 |
| 037 | `MB-FW-039-BASIC` | eHSM控制字段切换 |
| 038 | `MB-FW-040-BASIC` | eHSM FW串口波特率设置 |
| 039 | `MB-FW-043-BASIC` | eHSM FW版本读取 |
| 040 | `MB-FW-044-BASIC` | eHSM进入WFI并恢复通信 |
| 041 | `MB-FW-045-BASIC` | eHSM密钥生成与删除 |
| 042 | `MB-FW-046-BASIC` | eHSM密钥派生 |
| 043 | `MB-FW-047-BASIC` | eHSM密钥交换 |
| 044 | `MB-FW-049-BASIC` | eHSM密钥导出 |
| 045 | `MB-FW-050-BASIC` | eHSM密钥删除 |
| 046 | `MB-FW-051-BASIC` | eHSM私钥派生公钥 |
| 047 | `MB-FW-052-BASIC` | eHSM测试OTP写入命令 |

## eHSM负向安全功能：7条

| 最终编号 | 原追踪ID | 最终用例描述 | 代码判定重点 |
|---:|---|---|---|
| 048 | `EHSM-NEG-VERIFY-001` | eHSM镜像验签拒绝：签名、载荷和摘要篡改 | 请求真实到达eHSM；逐项篡改均拒绝；不复制、不放行、不更新状态；合法材料复测成功 |
| 049 | `EHSM-NEG-VERIFY-002` | eHSM SoC验签与防回滚拒绝 | 错误签名、载荷或低回滚计数均拒绝；Measurement、release和计数器不变化 |
| 050 | `EHSM-NEG-AUTH-001` | eHSM BL调试鉴权失败关闭调试 | 错误响应被eHSM拒绝；对应Debug状态和真实通路保持关闭 |
| 051 | `EHSM-NEG-AUTH-002` | eHSM FW调试鉴权失败关闭调试 | 错误响应被eHSM拒绝；对应Debug状态和真实通路保持关闭 |
| 052 | `EHSM-NEG-KEY-001` | eHSM拒绝篡改的密钥安装和导入材料 | 篡改受完整性保护的材料后拒绝；Key/OTP对象不产生部分更新；正确材料复测成功 |
| 053 | `EHSM-NEG-OTP-001` | eHSM拒绝越界、跨对象和受保护OTP访问 | 非法访问必须到达eHSM；不泄露受保护值、不产生部分写入；若只被Host拦截则为`INCONCLUSIVE` |
| 054 | `EHSM-NEG-STATE-001` | eHSM拒绝非法生命周期和控制字段转换 | 回退、越级、重复或保留值转换被拒绝；OTP状态保持基线且设备仍可查询 |

## SoC软件/协同：7条

| 最终编号 | 原追踪ID | 最终用例描述 | 代码/协同边界 |
|---:|---|---|---|
| 055 | `SOC-SECCFG-001` | SEC_CFG全部寄存器查询与值比较 | 单Case遍历17个寄存器；保存raw值并与已冻结场景期望比较 |
| 056 | `SOC-SECCFG-DBG-001` | `dbg_en_cfg`从Die Debug开关功能 | 验证默认关闭、主Die写开、真实从Die Debug联通、写回关闭；需要Debug模块协同 |
| 057 | `SOC-FW-SECCFG-001` | SEC_CFG Firewall默认权限 | 默认仅启动核可访问；eHSM和其他核拒绝；不修改Firewall配置 |
| 058 | `SOC-FW-SPIFC-001` | SPIFC Firewall默认权限 | 默认仅启动核可访问；eHSM和其他核拒绝；不修改Firewall配置 |
| 059 | `SOC-FW-SRAM-001` | SRAM Firewall五Region默认权限 | 单Case遍历Region0～4；启动核和eHSM允许，其他核拒绝 |
| 060 | `SOC-FW-SRAM-RECFG-001` | SRAM Firewall五Region重新配置 | 仅由启动核逐Region修改地址范围和允许Master ID，验证新范围后恢复；在线Case未包含非启动核配置写负向矩阵 |
| 061 | `SOC-MAILBOX-IRQ-001` | eHSM Mailbox中断路由与清除 | 以真实、无副作用命令触发响应；检查IRQ来源、计数、清除、重触发和屏蔽态轮询 |

## EDA/硬件协同：2条

| 最终编号 | 原追踪ID | 最终用例描述 | 责任边界 |
|---:|---|---|---|
| 062 | `EDA-ERR-CRITICAL-IRQ-001` | 严重错误组合中断与`o_hsm_err_hw`映射 | RTL-DV/EDA逐源和组合注入；软件可观察寄存器、IRQ和清除，不负责伪造错误源 |
| 063 | `EDA-ERR-ECC1B-IRQ-001` | ECC 1-bit独立脉冲、外部锁存/计数和可清除中断 | RTL-DV/EDA逐RAM注入；软件观察独立IRQ、锁存/计数、清除和重触发 |

## 后续实现共同规则

1. 用例编号和原追踪ID都写入测试注册、日志和Evidence，禁止重新顺序编号。
2. `001～061`可以建立软件Case；`062～063`只能建立软件观察器/协同入口，不能以普通软件模拟故障冒充EDA覆盖。
3. Host本地拒绝不能作为eHSM拒绝PASS；负向Case必须保存真实Mailbox提交和eHSM原始响应。
4. 缺少地址、Master ID、IRQ、寄存器期望、Debug挑战类型或恢复条件时，相关子项记为`INCONCLUSIVE`，不得猜测。
5. 不修改或定制eHSM BL/FW，不增加测试命令、测试钩子和特殊返回。
6. `dbg_en_cfg`、`soc_dbg_en_out`和`hsm_dbg_en/o_hsm_status[16]`属于不同观察链；实现前必须按挑战类型选择正确状态位和真实Debug通路。
7. 三个Firewall在产品方案中的配置Owner均为启动核，但在线Case 060只直接覆盖“启动核成功重配SRAM五Region”。非启动核/eHSM配置写拒绝仍是待补验证范围，不能从Case 060的PASS自动推导。
