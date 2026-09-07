# Proposal

## 背景

BootROM→FMC→GSP的Measurement producer/consumer、16字节`rollback_counter`和release门禁已经批准。负责人进一步确认实际启动的微核/固件实例数量可能变化，并批准Measurement只保存SoC安全事实。

## 变更目标

- 定义128字节Header、128字节Firmware Entry、唯一128字节SoC State Entry。
- 使用实际`fw_entry_count`和可变`total_len`，Firmware列表紧凑无空洞。
- BootROM作为隐式可信测量根，不形成普通Entry。
- 删除`state_entry_count`、`generation`、eHSM状态、Measurement地址domain、Key/Signer和时间戳。
- 保留CRC-32C和32位commit，定义跨stage append和稳定snapshot。
- 逻辑ABI保持独立；ADR-0028现已固定物理Region为16 KiB，实际最大实例数、PMA/Firewall和SPDM wire block仍为独立输入。

## 非目标

- 物理地址/容量已由后续ADR-0028固定；本change仍不冻结产品实际`max_fw_entries`或Firewall寄存器，PMA候选仅允许`NON_CACHEABLE/HARDWARE_COHERENT`并延期到SoC稳定后唯一裁决。
- 不定义eHSM BL rollback-counter命令wire packing。
- 不冻结SPDM block index、证书或secure-session参数。
- 不修改两个代码仓、Vendor资料、测试工作簿或Git状态。

## 推荐并批准的方案

```text
Header[128]
Firmware Entry[fw_entry_count][128]
SoC State Entry[128]
```

`total_len = 256 + fw_entry_count × 128`。

一个Firmware Entry对应一个独立验证、加载或release/隔离实例，以`fw_type + die_id + instance_id`唯一标识。State只保存SoC状态。BootROM每次启动先使旧Header失效并清零整个Region；Header/Entry使用CRC和commit检测半写。

## 风险

- 最大独立Firmware/微核实例数未到齐前，产品实际`max_fw_entries`不能冻结；物理16 KiB硬上限为126项。
- 若PMA/cache不能证明commit跨master可见性，真实共享RAM实现保持blocked。
- 可变列表不依赖Manifest slot；ADR-0024已删除`measurement_slot`，Entry由producer按实际`fw_type + die_id + instance_id`生成。

## 审批状态

2026-07-27项目负责人批准逻辑ABI方向和字段取舍。实现仍需单独授权并满足平台DoR。
