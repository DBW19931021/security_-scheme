# ADR-0025：一机一密RTL Key与16槽OTP Key基线

- Status: accepted
- Date: 2026-07-29
- Owner: 项目负责人
- Related: SRC-0015、SRC-0016、SRC-0017、SRC-0018、SRC-0024、ADR-0020、ADR-0021、ADR-0026、OPEN-CONFLICT-012、OPEN-CONFLICT-013、OPEN-DESIGN-014
- Last reviewed: 2026-08-04

## Context

项目负责人确认量产RTL Key采用一机一密；SoC Key轮换不再视为与Vendor方案存在概念冲突，密钥由KMS按设备管理，经RTL KEK保护后按[SRC-0015《云天励飞26Q2定制需求方案》来源卡](../sources/source-cards/SRC-0015.md)所述流程灌装和轮换。负责人提供的OTP Table 34和16-slot Key表已登记为[SRC-0024《NGU800P OTP内存映射与16-slot Key表》来源卡](../sources/source-cards/SRC-0024.md)，并明确其目标是`security_-scheme`方案基线；`baremetal`只派生实现和验证同一基线。

2026-08-04复核确认，本ADR早期版本把该表错误转录成另一套槽序，并错误地把slot 3～5记为Level 2。下表按SRC-0024纠正；旧转录不是独立基线，也不构成baremetal与方案之间的产品裁决冲突。

当前Vendor文档以RTL常量宏描述集成密钥，当前4019代码对Level1/Level2安装的生命周期限制、截图中的灌装阶段以及既有“双持久Attestation Key”设计之间仍存在实施绑定问题。这些问题不改变已经批准的产品目标，但必须在真实RTL、Vendor定制交付和量产recipe冻结前关闭。

## Decision

1. 量产设备采用一机一密。至少以下硬件秘密必须按die唯一，禁止以全产品共享的明文常量进入量产netlist、软件镜像、脚本、日志或仓库：
   - Chip RTL Root/Key；
   - RTL Install KEK eHSM；
   - RTL Install KEK SoC；
   - 量产仍启用时的Scan/测试授权秘密。
2. RTL Key只允许硬件消费，不提供C908、eHSM普通FW、Host或制造脚本的明文读取能力。其最终载体可以是隐藏OTP/eFuse、PUF派生、硬件Key Ladder或等价不可读实现，但必须由RTL/DFT/量产Owner提供逐die个性化、锁定、零化和readback/operation proof合同；在该合同到齐前不得把Vendor共享宏当作量产实现。
3. KMS为每个设备建立独立记录并托管对应的RTL Key句柄、Chip Root Key、Device Root Key及其版本。外部灌装材料必须先由对应设备的RTL Install KEK保护；eHSM内部再按层级使用Chip Root或Device Root保护OTP Key。任何recipe和审计记录只保存KMS句柄、hash和结果，不保存明文Key。
4. SoC Key轮换按[ADR-0021《SoC Key轮换采用Vendor定制机制》](ADR-0021-soc-key-rotation-vendor-mechanism.md)执行：KMS生成新Key，形成由Device Root保护的内层和RTL SoC KEK保护的外层，eHSM执行写新Key、证明、提交Bitmap、销毁旧Key并复位生效。该机制与一机一密一致，不构成架构冲突。
5. NGU800P OTP-KMU采用16个Key对象，物理Key ID、等级和截图Key类型固定如下。`未给出`不能由Vendor demo或baremetal实现反向补值：

   | 物理槽 | 对象 | Level | Key类型 | 截图已给权限 |
   |---:|---|---:|---|---|
   | 0 | Chip root key | 0 | symm | 未给出 |
   | 1 | Device root key | 1 | symm | 未给出 |
   | 2 | USER root key | 1 | 未给出 | 未给出 |
   | 3 | eHSM debug/verify key | 1 | asymm | 未给出 |
   | 4 | eHSM FW/update verify key | 1 | asymm | 未给出 |
   | 5 | eHSM FW/update encrypt key | 1 | symm | 未给出 |
   | 6 | SoC FW/update verify Rotation key | 2 | asymm | 未给出 |
   | 7 | SoC FW/update encrypt Rotation key | 2 | symm | 未给出 |
   | 8 | DICE root CA key | 2 | asymm | 七项权限 |
   | 9 | SoC debug verify key | 2 | asymm | 未给出 |
   | 10 | SoC FW/update verify key | 2 | asymm | 未给出 |
   | 11 | SoC FW/update encrypt key | 2 | symm | 未给出 |
   | 12 | SoC debug verify Rotation key | 2 | asymm | 未给出 |
   | 13 | UDS | 2 | asymm | 七项权限 |
   | 14 | Device private Key | 2 | asymm | 七项权限 |
   | 15 | User auth key | 2 | asymm | 未给出 |

   slot 8、13、14的七项权限为：签名或生成MAC、验签或验证MAC、加密、解密、派生或协商新密钥、删除、明文导入。该表记录OTP/KMU对象属性能力；产品软件仍必须通过受控typed service和Lifecycle限制调用者，不得据此向Host开放raw Key服务。

6. OTP内存映射同时按SRC-0024冻结：eHSM内部OTP基址为`0x33000000`，Key N Attribute/Key/CRC的offset分别为`0x070+0x28*N`、`0x074+0x28*N`和`0x094+0x28*N`，`N=0..15`。这是eHSM内部地址，不是GSP可直访的System Address；GSP仍只能经Mailbox访问。Key Attribute位编码、CRC、端序、ECC、锁位和backend物理编码继续作为实施输入。
7. [ADR-0026](ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)进一步冻结DEV阶段Chip Root→其余Level1→Level2灌装顺序、每设备单一Attestation Profile、Cert0/Cert1和制造接口；对应[冲突报告](../sources/conflict-reports/CONFLICT-16-SLOT-OTP-PROVISIONING-ORDER-AND-ATTESTATION.md)已经关闭。UDS必须按本表记录为slot 13/asymm/七项权限，不再写成slot 11的opaque KDF-only对象。
8. 当前Vendor 4019对Level1/Level2安装、RTL宏、OTP地址和定制轮换的差距属于实现交付/绑定问题，不推翻本ADR目标；对应范围保持`BLOCKED_BY_RTL_BINDING`或`BLOCKED_BY_VENDOR_DELIVERY`。

## Consequences

- [《NGU800P安全软件详细设计》第10章](../docs/05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)必须展开KMS、RTL个性化、16槽、制造阶段、证书和轮换流程。
- [ADR-0020《运行期、Provisioning、Attestation与发布原则》](ADR-0020-runtime-provisioning-attestation-update-and-release-principles.md)按ADR-0026更新为：软件保留P-256和SM2两套能力，但每台设备只灌装并使用一个受控Provisioning Profile。
- Cert0/Cert1 Flash布局和制造系统协议已由ADR-0026批准；精确Flash base、命令编号和外部系统schema仍是实施绑定。
- `baremetal`中的manifest、fixture说明和case绑定必须引用并匹配本方案表；允许同步派生记录，但不得形成第二份方案基线。

## Review history

- 2026-08-04：按项目负责人澄清和SRC-0024纠正早期转录：slot 3～5为Level 1，SoC轮换/主Key、DICE、UDS、Device Private和User Auth固定为当前0～15顺序；补入Table 34地址公式，并明确baremetal只派生同一基线。
