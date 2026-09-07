# Acceptance

## 设计验收

- 六段Region无空洞、无越界，总和严格等于2 MiB。
- 所有绝对地址均由`MANAGEMENT_NOC_S9_SRAM_BASE + offset`生成。
- GSP静态段不进入FMC复用区；Measurement无任何普通GSP/证书分配。
- Host写权限只覆盖Ingress接收期。
- 原地加载源/目标摘要、重叠搬移、NX、清零、Measurement和release顺序完整。
- MMP DDR参数缺失时保持blocked，不回退到普通Host DDR。

## 后续实现验收场景

1. 构建期拒绝FMC、GSP、PMP、RMP package或runtime footprint越界。
2. FMC运行时尝试覆盖FMC复用区的GSP静态段必须构建失败。
3. Host seal后写、Host访问目标Region和Measurement必须被Firewall阻断。
4. 原地搬移前后摘要不一致、Header offset1008地址不等于typed-stage固定目标、offset1016 Header CRC不匹配、offset1020 reserved非零、`Code_Size`异常或设备端尝试去除CBC零对齐时，必须保持NX并禁止release。
5. eHSM timeout时output和context必须quarantine，不得清零或执行。
6. GSP回收FMC区前必须完成撤权、清零、barrier和readback。
7. Measurement逻辑长度超过16 KiB或Entry数超过产品上限必须拒绝。
