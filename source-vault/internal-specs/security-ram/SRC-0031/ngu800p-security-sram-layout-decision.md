# NGU800P 2 MiB安全SRAM初步精确划分裁决

- 日期：2026-08-19
- 提供者：项目负责人
- 事实状态：CONFIRMED（产品软件设计输入）
- 适用范围：NGU800P D0安全SRAM物理用途、BootROM/FMC/GSP生命周期、PMP/RMP常驻和MMP DDR边界

## 负责人裁决原意

1. 安全SRAM总容量为2 MiB。
2. 前1 MiB留给GSP域使用，包括GSP运行期缓存等；ROM代码使用的栈从该1 MiB低地址开始放置。
3. 前1 MiB末尾16 KiB本版只规划为Measurement固定区；即使当前逻辑Measurement没有用满，未使用空间也先保持保留，后续用途另行评审。
4. FMC固定占用前1 MiB中Measurement之前的128 KiB，即从`1 MiB - 128 KiB - 16 KiB`处开始；FMC退出后该128 KiB可由GSP清零并复用。
5. 中间512 KiB由功耗核和RAS核各使用256 KiB；保持既有PMP低地址、RMP高地址顺序。
6. MMP不驻留在该2 MiB安全SRAM，主要使用DDR；DDR精确地址、保护和加载绑定另行提供。
7. 最后512 KiB作为Host与Device之间的固件包缓冲区。

## 对应offset

以`MANAGEMENT_NOC_S9_SRAM_BASE`为`B`：

| 范围 | 大小 | 用途 |
|---|---:|---|
| `B + 0x000000 ～ B + 0x0DBFFF` | 880 KiB | GSP预加载/常驻静态区；低地址启动期叠加BootROM栈 |
| `B + 0x0DC000 ～ B + 0x0FBFFF` | 128 KiB | FMC运行区；FMC退出后由GSP受控回收 |
| `B + 0x0FC000 ～ B + 0x0FFFFF` | 16 KiB | Measurement固定Region；本版不复用 |
| `B + 0x100000 ～ B + 0x13FFFF` | 256 KiB | 功耗核/PMP最终运行区 |
| `B + 0x140000 ～ B + 0x17FFFF` | 256 KiB | RAS核/RMP最终运行区 |
| `B + 0x180000 ～ B + 0x1FFFFF` | 512 KiB | Host/Device固件包Ingress共享区 |

## 尚未由本输入冻结的内容

- 2 MiB SRAM的最终PMA属性、Firewall CSR、窗口数、最小粒度、lock/reset和readback行为。
- MMP DDR的System Address范围、eHSM可达性、DDR Firewall/IOMMU、加载与release原语。
- GSP、FMC、PMP和RMP最终release link map与运行期高水位；它们必须满足上述物理上限。
- 最后16 KiB Measurement未使用空间的后续用途；本版不得将其当作证书或通用scratch。

