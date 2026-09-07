---
title: "固件更新"
status: approved_with_platform_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0016
  - SRC-0017
owners:
  - GSP
last_reviewed: 2026-07-28
supersedes: []
superseded_by: []
---

# Purpose

维护正常固件更新和Flash原子性专题；规范性设计见[主详设第12章](../05-software-design/NGU800P安全软件详细设计.md)。

# Baseline

Host只写GSP受控staging；不得直接写active slot、降低`rollback_counter`或把“写成功”视为“安全启动成功”。

SoC stage镜像固定Vendor type 1，由GSP以`check_version=0`预验证和项目policy检查；type 2/3产品入口拒绝。成功后写inactive slot并读回hash，按A/B双metadata、单调sequence、完整性和commit-last规则提交后请求reset；下一次BootROM/FMC/GSP链作最终启动判定，预验证和Flash写入不得推进SoC counter。

eHSM Vendor FW单独处理：

- 初始启动时eHSM始终只运行BL；GSP被FMC release后建立受控ingress，等待Host下发完整Vendor type 0包，再请求BL验证/启动并等待`firmware_done && !firmware_err`。
- eHSM FW已经运行时，GSP不得复用BL type 0 verify/boot命令，也不得在线加载candidate；只把完整包写入inactive slot、读回并提交metadata后请求reset。
- reset后eHSM重新只运行BL，由BL从candidate slot执行最终type 0验证/启动；失败不得回退到未批准镜像或伪造FW ready。

# Approved boundary and inputs

ADR-0020已关闭`OPEN-DESIGN-016`并冻结更新/OOB/Recovery原则；partition/slot、transport、token和Flash Owner作为平台Profile输入。

# Verification impact

覆盖每个erase/write/readback/metadata/counter/reset掉电点、乱序chunk、低版本fallback、Host反复下发、初始Host下发后BL启动、运行中禁止复用BL命令，以及reset后BL最终验证失败。

# Change history

- 2026-07-28：区分初始eHSM FW Host下发→BL验证/启动与运行期只写inactive→reset后BL最终验证两条路径；SoC stage固定type 1且预验证固定`check_version=0`，更新metadata固定A/B双副本、单调sequence和commit-last。
- 2026-07-27：接受ADR-0020并关闭OPEN-DESIGN-016；冻结inactive更新、原子metadata和不降rollback counter恢复原则。
- 2026-07-24：同步主详设第12章。
