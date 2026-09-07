# Proposal

> **SUPERSEDED（2026-08-21）**：本变更中的NGU Manifest方案已由`native-header-load-address-v1`和ADR-0030替代，仅保留历史决策记录；不得作为当前实现或测试依据。

## 背景

NGU800P需要在保持Vendor eHSM Header和公共代码不变的前提下，表达FMC/GSP/PMP/RMP/MMP的项目语义，并消除Header `Code_Size`与实际包长度的多义解释。

## 当前问题

- SRC-0016对Vendor `Image_Type`的简化说明与SRC-0012/0018实现不一致。
- Vendor verify调用长度和Header `Code_Size`之间缺少项目侧强制相等门禁。
- 现有代码资产存在私有Manifest、32位counter和零签名/未加密制包骨架，不能作为产品ABI。

## 变更目标

- 保持Vendor wire `Image_Type=0/1/2/3`和Vendor公共源码不变。
- 在提交前和eHSM PASS后执行精确长度校验。
- 冻结ADR-0024精简后的128字节NGU Manifest v1、32位管理`version`、16字节`rollback_counter`、固定baremetal 64位System Address、`payload_offset=128`、无expected digest及公共registry。
- 冻结verify→loader→Measurement→release的分层状态机。
- 冻结三套产品算法Profile及其Manifest/policy/制包/测试门禁，三套全部实现并通过端到端测试。

## 非目标

- 不冻结精确Region offset/容量、PMA、Firewall、物理counter命令，以及具体设备/SKU、key域、boot/upgrade key、board/LCS provisioning绑定；地址视图已冻结为baremetal System Address。
- 不修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor source-vault。
- 不授权产品编码或Git操作。

## 适用范围

BootROM验证FMC、FMC验证GSP、GSP验证PMP/RMP/MMP，以及发布制包/解析工具。eHSM Vendor FW仅使用Vendor原生type 0专用流程。

## 已知事实

- Vendor Header为1024字节，`Image_Type`原生值为0～3。
- Vendor `Version_Counter`为16字节。
- Vendor公共verify输出包含Header和解密后的Code Region。
- eHSM Vendor FW由GSP加载；SoC Runtime各自独立隔离。

## 假设

无新增硬件行为假设。算法集合和System Address视图已经冻结；精确Region offset/容量/PMA/Firewall、具体provisioning/release绑定和counter操作继续由现有开放项管理。

## 候选方案

1. 修改Vendor Header/源码理解NGU项目类型。
2. 保持Vendor层，在认证Code Region内使用独立NGU Manifest，并由项目边界规范化长度。

## 推荐方案

采用方案2，详见ADR-0014、公共ABI和secure-package规格。

## 风险

若不同工具或代码仓手工复制registry，仍可能漂移；实现阶段必须改为单一机器可读源生成。

## 待确认问题

- PMP/RMP/MMP实际依赖图、package入口和release primitive；项目负责人确认资料后续补充。

## 审批状态

2026-07-23，项目负责人已批准方案2、双阶段长度校验和Manifest v1基础合同。2026-07-24进一步批准ADR-0015的GSP唯一eHSM Owner，以及ADR-0017的三套产品算法Profile。2026-07-28通过ADR-0024精简Manifest，删除ABI版本、地址domain、board/Measurement/component绑定、expected digest和TLV，并冻结payload offset128、baremetal System Address及loader源/目标双摘要。本change保持设计批准、实现未授权状态。
