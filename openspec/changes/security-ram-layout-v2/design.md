# Design

## 物理布局

```text
+0x000000  GSP_STATIC       880 KiB
+0x0DC000  FMC_REUSE        128 KiB
+0x0FC000  MEASUREMENT       16 KiB
+0x100000  PMP              256 KiB
+0x140000  RMP              256 KiB
+0x180000  HOST_INGRESS     512 KiB
+0x200000  end
```

BootROM栈从低地址开始，BootROM退出后由FMC加载GSP时覆盖。FMC运行区在GSP entry后清零并加入GSP动态内存池；GSP静态段不得链接进FMC复用区。

## 原地加载

Host ingress与output不重叠。output等于尚未执行的目标Region起始地址。按ADR-0030，Vendor完整输出成功后，loader复验Header Overlay与typed-stage固定目标，在`target+1024`对完整`Code[Code_Size]`计算源摘要，以`memmove(target,target+1024,Code_Size)`搬移，清零旧包尾/Region剩余/BSS，再从最终地址计算目标摘要。CBC零对齐属于Code且不去除。完成Measurement、权限和指令同步前保持NX。

## 容量

- FMC package/runtime均不超过128 KiB。
- GSP package不超过512 KiB，预release静态占用不超过880 KiB，回收后GSP总可用不超过1008 KiB。
- PMP/RMP package和最终footprint各不超过256 KiB。
- Measurement固定16 KiB，逻辑`max_fw_entries <= 126`。

## MMP

MMP在DDR运行，不占本SRAM。DDR必须是Host不可写的受保护目标，且需另外证明eHSM/CPU可达性、缓存一致性、Firewall/IOMMU和release。
