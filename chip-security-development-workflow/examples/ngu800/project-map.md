# NGU800 Workflow Example Map

NGU800 是通用工作流的首个示例，不是所有芯片必须照抄的安全规则。

| Layer | Location |
|---|---|
| Security project | [`ngu800_security_solution_flow_with_gpt`](../../../ngu800_security_solution_flow_with_gpt/) |
| Code repository | [`gsp-pmp-rmp-omp`](../../../../gsp-pmp-rmp-omp/) |
| Security component | [`components/security`](../../../../gsp-pmp-rmp-omp/components/security/) |
| Component OpenSpec | [`components/security/docs/openspec`](../../../../gsp-pmp-rmp-omp/components/security/docs/openspec/) |
| Pilot change | [`add-mctp-mailbox-spdm-endpoint`](../../../../gsp-pmp-rmp-omp/components/security/docs/openspec/changes/add-mctp-mailbox-spdm-endpoint/) |

事实源分工：

- 芯片约束、威胁、基线、决策和 CR 位于 Security project。
- MCTP/SPDM Endpoint 的需求、设计、任务、问题和验证位于 pilot OpenSpec。
- 通用流程、模板、检查器和候选知识位于本仓库。
