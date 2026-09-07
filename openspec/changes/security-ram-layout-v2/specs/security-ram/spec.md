# Security RAM Layout v2 Requirements

## Requirement: Fixed 2 MiB partition

系统必须按ADR-0028固定GSP static 880 KiB、FMC reuse 128 KiB、Measurement 16 KiB、PMP 256 KiB、RMP 256 KiB和Host ingress 512 KiB。

### Scenario: Layout overflow or drift

- Given 任一生成Region offset/size与受控布局不一致
- When 生成linker、typed-stage Region registry/Header Overlay目标或测试Expected
- Then 构建必须失败
- And 不得使用旧P1样例或旧080x地址回退

## Requirement: FMC/GSP lifecycle overlay

BootROM栈必须从低地址启动区分配；FMC必须固定在128 KiB复用区；GSP静态加载不得覆盖运行中的FMC，GSP接管后才可回收该区。

### Scenario: Reclaim before handoff

- Given FMC仍在运行或其eHSM事务未闭环
- When GSP或loader请求复用FMC Region
- Then 必须拒绝或quarantine
- And 不得清零或覆盖活跃FMC状态

## Requirement: Measurement reservation

Measurement物理Region必须固定16 KiB，本版未使用部分也不得由GSP、证书或普通scratch分配。

### Scenario: Certificate workspace request

- Given 证书生成或SPDM请求需要工作缓冲
- When allocator选择内存
- Then 不得返回Measurement Region的任何范围

## Requirement: In-place authenticated load

Host ingress必须与output完全分离。对尚未执行的FMC/GSP/PMP/RMP目标，loader必须在原地搬移前计算源摘要、使用重叠安全搬移、从最终地址计算目标摘要，并在Measurement和权限门禁后release。

### Scenario: Payload move corruption

- Given Vendor认证成功但原地搬移或目标内存发生错误
- When loader计算目标readback摘要
- Then 摘要比较必须失败
- And 目标保持NX且不得release

## Requirement: MMP protected DDR

MMP不得占用本2 MiB常驻Region；只有经批准的DDR carveout、eHSM/CPU可达性、Firewall/IOMMU和release Profile到齐后才可加载执行。

### Scenario: Ordinary host DDR supplied

- Given Host提供未受保护的普通DDR地址作为MMP load地址
- When GSP复验MMP Header Overlay并与typed-stage DDR目标匹配
- Then 必须拒绝
- And 不得解密明文到该地址
