# 16槽OTP层级、灌装阶段与Attestation容量冲突

## Identity

- Conflict ID: CONFLICT-16-SLOT-OTP-PROVISIONING-ORDER-AND-ATTESTATION
- Open Question: OPEN-CONFLICT-013
- Status: RESOLVED
- Evidence state: CONFIRMED
- Owner: 项目负责人；eHSM/Vendor Owner；Provisioning/KMS Owner；SPDM/Certificate Owner
- Related: SRC-0017、SRC-0018、SRC-0024、ADR-0020、ADR-0025、ADR-0026、OPEN-DESIGN-014、OPEN-DESIGN-015

## Classification

- Type: approved_physical_map_vs_hierarchy_sequence_and_existing_attestation_requirement
- Resolution scope: 灌装先后顺序、Attestation单Profile、物理Key ID、Cert0/Cert1和制造接口的软件设计已经冻结；SRC-0024纠正早期16-slot转录。
- Remaining implementation bindings: Vendor定制交付、RTL逐die实现、Key Attribute/CRC/ECC/backend、Flash base/擦除粒度和KMS/CA/MES接口。

## 2026-08-04基线纠正

项目负责人明确：所提供Key表和全部SoC信息以`security_-scheme`为目标，baremetal记录必须与方案一致。本报告及ADR-0025/0026早期把截图转录成另一套槽序属于方案记录错误，不是“baremetal基准”和“security_-scheme基准”之间的裁决冲突。当前唯一槽序见SRC-0024和修订后的ADR-0025。

## Conflict A：Device Root必须先于Level2对象

截图把Device Root Key定义为Level1、MANU灌装，同时把eHSM Debug/FW、DICE、Device Private和UDS等Level2对象标为TEST/DEV灌装。Level2对象在OTP中由Device Root Key保护，因此写入这些对象之前Device Root必须已经存在且可用。

当前4019实现进一步表现出交付差距：

- Host API声明Level1仅支持TEST/DEV，Level2支持TEST/DEV/MANU；
- FW注释称MANU只允许Device Root和Level2，但实际`otpkinstl_check_life_cycle()`在MANU只接受Level2；
- BL具有Device Root特殊level和RTL SoC KEK重包能力，但在MANU又限制Chip Root解密路径；
- 因此当前版本不能直接作为最终16槽量产recipe的实现依据。

### 已批准裁决

“MANU”是制造流程阶段，不要求灌装前切换到MANU LCS。设备保持DEV，依次安装Chip Root、Device Root和USER Root，再安装全部Level2对象，全部证明完成后才相邻切换MANU/USER。当前Vendor实现若不支持该顺序，作为交付缺口处理，不重新打开目标设计。

## Conflict B：一个Device Private槽不能同时承载两把曲线私钥

最终16槽表只分配一个`Device Private Key`槽。ADR-0020此前要求同一设备同时持久化不可导出的P-256和SM2两把Attestation私钥及两套证书。一个32字节私钥槽不能安全地同时作为两条曲线的独立长期身份，也不应复用同一标量跨曲线。

### 已批准裁决

软件镜像保留P-256和SM2两套能力，但每台设备由受控Provisioning Profile选择其中一种Device Attestation算法。槽14只生成并持久化所选算法的一把私钥，Cert0/Cert1是同一算法、同一设备身份和同一私钥的A/B证书链；不支持同机同时使用两套长期身份。槽14的OTP属性同时保留SRC-0024列出的七项权限，产品接口仍按typed policy收敛。

## Conflict C：UDS类型列

截图把UDS固定为slot 13、Level 2、`asymm`并列出七项权限。早期把它转录为slot 11并改写成opaque KDF-only语义没有依据，现已由SRC-0024和ADR-0025/0026纠正。具体算法、材料形态和产品内部usage仍需Key Attribute/Provisioning Profile绑定；属性能力不得自动转换为Host通用服务。

## Conflict D：表格顺序与物理Key ID

截图表格自上而下固定为物理Key ID 0～15。SRC-0024同时冻结Table 34：eHSM内部OTP基址`0x33000000`，Key N Attribute/Key/CRC offset为`0x070/0x074/0x094 + 0x28*N`。未冻结的是属性位、CRC、ECC、锁位和backend编码，不再把offset列为未知。

## Resolution

项目负责人于2026-07-29批准全部推荐方向，并通过[ADR-0026](../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)冻结：

1. DEV LCS完成Chip Root→其余Level1→Level2→证书，再切MANU/USER；
2. 每设备只选择一个Attestation Profile，槽14只保存一把设备私钥；
3. UDS固定为slot 13/Level 2/asymm/七项权限，具体算法和产品内部usage待实施合同；
4. 表格顺序和Table 34地址公式固定；
5. Cert0/Cert1布局和signed-recipe/typed-command制造接口一并批准。

## Review history

- 2026-07-29：依据负责人指定的最终16槽截图与当前4019代码建立。推荐方案已写入主详设但保持`PROPOSED/CONFLICTING`，等待一次性裁决。
- 2026-07-29：项目负责人批准全部推荐方案；建立ADR-0026并关闭OPEN-CONFLICT-013。剩余Vendor/RTL/Flash/KMS/CA/MES事项改按实施绑定管理。
- 2026-08-04：负责人明确Key表目标为security_-scheme、baremetal必须同步同一基线；纠正早期slot 6～15顺序、slot 3～5 Level、UDS/DICE/Device Private语义及“精确offset未知”记录。
