# ADR-0028：2 MiB安全SRAM精确划分与受控原地加载

> 2026-08-21后续裁决：[ADR-0030《删除NGU Manifest并复用Native Header尾部承载load_addr》](ADR-0030-remove-manifest-and-use-native-header-tail.md)把原地加载源偏移从`target+1152`更新为`target+1024`，长度改为Native Header `Code_Size`；本ADR的六段SRAM布局、Owner/权限、无独立plaintext和源/目标摘要原则继续有效。

- 状态：accepted
- 日期：2026-08-19
- 决策人：项目负责人
- 相关Source ID：SRC-0016、SRC-0017、SRC-0022、SRC-0031
- 相关问题：OPEN-CONFLICT-006、OPEN-DESIGN-010、OPEN-DESIGN-021
- 替代关系：替代ADR-0007中PMP/RMP/MMP全部驻留、独立`SEC_RAM_PLAINTEXT`和BootROM/FMC复用最高Runtime池的容量拓扑；ADR-0007的System Address、生命周期隔离、W^X和清零原则继续有效

## 背景

原P1只给出`448/64/256/256/256/256/512 KiB`容量审查样例，尚未冻结offset。项目负责人现已冻结2 MiB内部用途：前1 MiB为GSP域并包含FMC启动复用和16 KiB Measurement，中间512 KiB分别给功耗核/PMP与RAS核/RMP，最后512 KiB为Host/Device固件包Ingress；MMP不驻留该SRAM而主要使用DDR。

新布局没有独立plaintext Region。为了保持Host密文与已认证明文的隔离，同时满足固定容量，FMC/GSP/PMP/RMP采用“Host ingress独立、解密输出与尚未运行的最终目标Region精确别名”的受控原地加载模式；任何正在执行的Region仍禁止作为输出。

## 决策

### 1. 固定物理布局

以`B = MANAGEMENT_NOC_S9_SRAM_BASE = 0x1010_0500_0000`：

| Region ID | offset | System Address | 大小 | 生命周期 |
|---|---:|---:|---:|---|
| `SEC_RAM_GSP_STATIC` | `0x000000` | `0x1010_0500_0000～0x1010_050D_BFFF` | 880 KiB | FMC release GSP前承载完整GSP静态加载结果；GSP运行期常驻 |
| `SEC_RAM_FMC_REUSE` | `0x0DC000` | `0x1010_050D_C000～0x1010_050F_BFFF` | 128 KiB | FMC运行时独占；GSP接管、清零和权限回读后加入GSP动态内存池 |
| `SEC_RAM_MEASUREMENT` | `0x0FC000` | `0x1010_050F_C000～0x1010_050F_FFFF` | 16 KiB | 全启动链固定；本版未使用尾部仍保留，不作证书或scratch复用 |
| `SEC_RAM_PMP` | `0x100000` | `0x1010_0510_0000～0x1010_0513_FFFF` | 256 KiB | 功耗核/PMP最终运行区 |
| `SEC_RAM_RMP` | `0x140000` | `0x1010_0514_0000～0x1010_0517_FFFF` | 256 KiB | RAS核/RMP最终运行区 |
| `SEC_RAM_HOST_INGRESS` | `0x180000` | `0x1010_0518_0000～0x1010_051F_FFFF` | 512 KiB | Host唯一可写固件包窗口；submit后seal并撤销Host写 |

总和固定为`1024 + 256 + 256 + 512 = 2048 KiB`。

### 2. BootROM与FMC/GSP生命周期

- BootROM代码位于ROM；其启动栈存储从`SEC_RAM_GSP_STATIC`低地址开始，当前主栈8 KiB、异常栈8 KiB，SP指向各自栈顶。BootROM data/BSS/context的最终边界由BootROM link map门禁，但必须完全低于`0x0DC000`。
- BootROM退出后，FMC使用固定`SEC_RAM_FMC_REUSE`运行；FMC加载GSP前可以清零并覆盖已退出的BootROM栈/工作区。
- GSP在FMC仍运行时的静态加载结果、启动栈和启动所需BSS必须完全位于`SEC_RAM_GSP_STATIC`，不得触及FMC或Measurement。
- GSP entry后使用`SEC_RAM_GSP_STATIC`内的栈撤销FMC权限、清零并回读`SEC_RAM_FMC_REUSE`，随后才可把该128 KiB加入GSP Heap/动态buffer池。静态段不得链接到该复用区。

### 3. 受控原地加载

FMC/GSP/PMP/RMP SoC包固定使用`Native Header[1024] || Code[Code_Size]`；CBC零对齐属于Code和`Code_Size`本身，不再存在Manifest、独立payload长度或设备端去padding。Host ingress与eHSM output必须完全不重叠。对尚未运行且处于`LOADER_RW_NX`的目标Region：

1. eHSM将包含Header的完整解密包写到目标Region起始地址；`package_size`必须不超过目标Region容量。
2. Vendor完成且确认不再访问output后，loader从稳定输出Header复验`Image_Type/Plain/Naked/Code_Size/Version_Counter`、offset1008的LE64 `load_addr`、offset1016的LE32 Header CRC、offset1020的零reserved和typed-stage policy；CRC按ADR-0033覆盖offset256～1015且只做格式筛查，`load_addr`必须精确等于该stage固定`target_base`，再把后续仍需元数据复制到stage私有小结构。
3. 对`target_base + 1024`处的源Code Region计算Profile摘要。
4. 使用经过审计的重叠安全`memmove(target_base, target_base + 1024, Code_Size)`；禁止使用对重叠未定义的`memcpy`。
5. 清零旧包尾及目标Region剩余/BSS；从`target_base`回读`Code_Size`字节重算摘要并常量时间比较。CBC零对齐随Code一起搬移和度量，不由设备剥离。
6. 完成write/release、`fence.i`、Measurement commit和权限回读后，才把code切换为RX并release目标。

失败时保持目标NX并清零确定不再被eHSM访问的完整已写范围；timeout/acceptance unknown时目标Region与context一起quarantine，不得清零、执行或复用。

### 4. 容量门禁

- `FMC package_size <= 128 KiB`，且FMC image/data/BSS/stack总峰值不超过128 KiB。
- `GSP package_size <= 512 KiB`；FMC release前GSP静态加载、启动BSS和栈必须不超过880 KiB；GSP接管后可用上限为1008 KiB，但其中最后128 KiB只能用于回收后动态分配。
- `PMP package_size <= 256 KiB`且最终运行footprint不超过256 KiB。
- `RMP package_size <= 256 KiB`且最终运行footprint不超过256 KiB。
- 16 KiB Measurement Region的逻辑长度必须满足`256 + fw_entry_count × 128 <= 16384`；产品`max_fw_entries`仍按实际实例数生成，且不得超过126。
- Host ingress最大完整package为512 KiB；大于该值的包在提交Vendor前拒绝。

### 5. MMP DDR边界

MMP不占本2 MiB的常驻Region。其安全包、counter、Measurement和独立release身份保留；DDR load/entry allowlist、受保护carveout、eHSM可达性、Firewall/IOMMU、缓存一致性和release primitive由OPEN-DESIGN-010继续阻断。证据到齐前不得把任意Host DDR或普通共享DDR当作明文执行区。

## 安全理由

- Host只能写独立Ingress，不能直接修改解密输出或执行区。
- output只与当前尚未执行的目标Region别名；FMC、GSP、PMP、RMP各自的运行期Region不被下一次请求覆盖。
- 源摘要在重叠搬移前完成，目标摘要在搬移后从最终地址回读，保留原方案的双摘要检查语义。
- 固定16 KiB Measurement隔离了跨stage事实；未用空间保持保留避免证书/临时buffer破坏commit对象。
- MMP移至DDR释放SRAM，但不弱化DDR必须具备的明文、执行和Owner保护。

## 影响与开放项

- 主详设第5章、RAM专题、Measurement接口、Firewall专题、OpenSpec和OPEN-CONFLICT-006必须同步。
- `SEC_RAM_PLAINTEXT`从产品布局删除；“plaintext”成为目标Region在`LOADER_RW_NX`阶段的生命周期状态，不再是独立物理Region。
- ADR-0006/0007和依赖文档中的旧P1容量拓扑由本ADR替代；eHSM full-aperture、PMA候选、Owner转换、W^X、清零和RAS原则不变。
- 精确offset已经冻结；产品编码仍等待PMA/Firewall参数、最终link map/watermark、MMP DDR Profile和实现OpenSpec授权。
