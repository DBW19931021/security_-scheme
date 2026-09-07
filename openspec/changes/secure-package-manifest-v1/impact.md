# Impact

> **SUPERSEDED（2026-08-21）**：本变更中的NGU Manifest方案已由`native-header-load-address-v1`和ADR-0030替代，仅保留历史决策记录；不得作为当前实现或测试依据。

## 硬件依赖

不改变eHSM wire或SoC硬件。最终地址和counter命令依赖现有开放项。

## 软件影响

BootROM/FMC/GSP、公共security组件和release工具需使用同一生成registry与分层API。

## 接口影响

新增ADR-0024精简Manifest v1、公共状态/错误域和stage profile；eHSM BL按ADR-0019新增FMC专用staged-candidate exact-match commit API，其他Vendor公共Host API保持不变。

## 兼容性

eHSM FW继续使用Vendor type 0原生包；旧synthetic/私有Manifest包及ADR-0024之前的Manifest草案不兼容产品路径。

## 量产与运维

发布必须使用真实签名/加密工具并记录工具、key/cert标识、package/payload hash和Manifest摘要。

## 验证与 Evidence

需要跨语言golden vector、negative corpus、真实eHSM verify、EMU loader/release Evidence和no-stub source graph报告。

## 风险和回退

在真实工具和最终profile未ready前只允许parser/状态机验证，不得退回零签名或模拟成功路径。
