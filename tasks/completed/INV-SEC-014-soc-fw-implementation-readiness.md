# INV-SEC-014：BootROM/FMC/GSP编码前落实性审查

## 目标

把已批准的完整详设与`gsp-pmp-rmp-omp`当前BootROM/FMC/GSP代码事实逐项对照，回答：

1. 哪些合同在获得单独编码授权后可直接开始；
2. 哪些真实产品实现被平台资料阻断；
3. 哪些被Vendor交付阻断；
4. 是否存在需要负责人重新裁决的架构冲突。

## 约束

- 只读检查`gsp-pmp-rmp-omp`，只修改`security_-scheme`文档。
- 不修改Vendor、baremetal、测试工作簿或任何产品代码。
- 不执行Git、构建或测试。
- 当前live worktree已有未提交修改，不判断其作者、差异或归属。

## 检查范围

- BootROM/FMC/GSP入口和启动任务；
- 公共Manifest、Measurement、verify flow和镜像枚举；
- production/test source graph与stub/null provider；
- 三个stage linker；
- 真实Vendor Host/NGU800P port、loader、release和RAS绑定。

## 输出

- 代码证据：`evidence/code-investigations/CE-SEC-014-soc-fw-implementation-readiness.md`
- 统一审查：`docs/09-plans/SOC安全固件编码前落实性审查-v1.md`

## 验收

- [x] BootROM/FMC/GSP逐stage给出设计、代码事实、差距和阻断分类。
- [x] 公共逻辑、平台绑定、Vendor绑定和延期功能清晰分离。
- [x] 形成不代表编码授权的建议实施顺序。
- [x] 未发现需改变已批准原则的新冲突；所有实施缺口均回收到现有开放项/DoR。
- [x] 未修改三个代码来源、Vendor、测试工作簿，未执行Git。
