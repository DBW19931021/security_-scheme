# Proposal

## 背景

原2 MiB安全SRAM P1仅有容量审查样例。SRC-0031和ADR-0028现已冻结前1 MiB GSP/FMC/Measurement、中间PMP/RMP各256 KiB、末尾512 KiB Host ingress，并把MMP常驻移至DDR。2026-08-21按ADR-0030把原地加载源从旧Manifest后的`target+1152`更新为Native Header后的`target+1024`，长度唯一使用`Code_Size`。

## 变更目标

- 冻结六段物理Region的offset、size和生命周期。
- 删除独立`SEC_RAM_PLAINTEXT`，定义Host ingress独立、目标Region受控原地加载。
- 固定BootROM低地址栈、FMC 128 KiB及GSP回收合同。
- 固定Measurement 16 KiB且本版未使用空间不得复用。
- 固定PMP/RMP各256 KiB，MMP DDR参数继续开放。

## 非目标

- 不冻结PMA/Firewall CSR、窗口粒度、lock/reset或cache实现。
- 不冻结MMP DDR base/size、eHSM DDR可达性或release primitive。
- 不修改产品代码、linker或测试代码；实施需单独授权。

## 审批状态

2026-08-19项目负责人批准物理容量用途；ADR-0028为accepted。产品实现仍受PMA/Firewall、release link map和MMP DDR Profile门禁约束。
