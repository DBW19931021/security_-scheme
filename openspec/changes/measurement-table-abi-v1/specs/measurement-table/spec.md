# Measurement Table Requirements

## Requirement: Variable compact storage ABI

系统必须使用little-endian Measurement Table ABI v1.0：128字节Header、`fw_entry_count`个128字节Firmware Entry和唯一128字节SoC State Entry。

`total_len`必须满足：

`128 + fw_entry_count × 128 + 128`

物理Region固定为16 KiB、System Address `0x1010_050F_C000～0x1010_050F_FFFF`；产品实际`max_fw_entries`不得超过126，未使用尾部本版不得复用。

### Scenario: Variable runtime count

- Given 本次启动实际形成N个独立验证/加载/release固件实例
- When GSP完成Table
- Then `fw_entry_count`必须等于N
- And `total_len`必须等于`256 + N × 128`
- And Firmware Entry必须紧凑排列且无空洞

### Scenario: Length outside Region

- Given `total_len`公式错误、算术溢出或超过固定Measurement Region容量
- When 任一Consumer解析Header
- Then 必须拒绝整个Table
- And 不得release或产生SPDM证明

## Requirement: Single SoC State

系统必须在Firmware列表之后保存唯一SoC State Entry，不得保存`state_entry_count`。

### Scenario: State not final

- Given Firmware列表有效但SoC State不是COMMITTED
- When SPDM或Attestation读取Table
- Then 必须返回not ready
- And 不得输出部分SoC状态

## Requirement: BootROM is implicit measurement root

BootROM必须作为隐式可信Root of Trust for Measurement，不得创建普通BootROM Firmware Entry或以BootROM自Hash证明自身。

### Scenario: BootROM identity unavailable

- Given 平台没有芯片绑定的不可变BootROM identity/release digest
- When BootROM建立Measurement Table
- Then Firmware列表必须从FMC开始
- And 不得填充BootROM stub、伪Hash或PASS状态

## Requirement: Firmware instance identity

每个独立验证、加载或release/隔离实例必须形成独立Firmware Entry，并以`fw_type + die_id + instance_id`唯一标识。

### Scenario: Same image on two independent instances

- Given 两个微核使用相同固件Hash但分别加载或release
- When GSP提交Measurement
- Then 必须形成两个Entry
- And `instance_id`或`die_id`必须不同

### Scenario: Shared release object

- Given 多个微核共享同一代码实例且作为一个整体release
- When GSP提交Measurement
- Then 可以只形成一个Entry

## Requirement: Minimal fields

Measurement不得包含`table_flags`、`algorithm_profile`、`key_id`、`signer_id`、地址domain、eHSM状态或时间戳。Firmware Entry地址必须是64位baremetal System Address。

### Scenario: Manifest address conversion

- Given Manifest已经验证地址domain和canonical地址
- When Loader构造Firmware Entry
- Then 必须记录最终64位`SOC_PA`
- And 不得记录eHSM remote、local/remap或Host地址

### Scenario: Key rotation

- Given eHSM内部Bitmap选择当前Active Key并完成验证
- When Producer提交Firmware Entry
- Then 只记录验证结果和固件Hash
- And 不得复制内部Key slot或Signer ID

## Requirement: SoC state only

SoC State必须记录LCS、Debug、防回滚、16字节全局Counter、Firewall、OOB/Recovery、boot failure、Die数量和证书链状态，不得包含eHSM status/error/health或时间戳。

### Scenario: eHSM runtime error

- Given eHSM在Table完成后报告运行期错误
- When GSP处理错误
- Then 必须写入eHSM Adapter状态和RAS/audit
- And 不得覆盖SoC State Measurement

## Requirement: No generation field

ABI v1不得保存`generation`。BootROM必须在每次安全启动时先使Header失效并清零整个固定Measurement Region，再发布新Header。

### Scenario: Retained stale RAM

- Given RAM保留上次启动的COMMITTED对象
- When BootROM开始建立新Table
- Then 必须先把旧Header marker置EMPTY
- And 必须清零整个Region后才能发布新Header
- And 清零失败必须阻断启动链release

## Requirement: CRC and final commit

Header、Firmware Entry和SoC State Entry必须在offset 120保存CRC-32C，并以offset 124自然对齐的32位COMMITTED marker作为最后发布动作。

### Scenario: Torn entry

- Given 任一对象的payload、reserved、CRC或commit写入被截断
- When Consumer读取
- Then CRC、reserved或commit至少一项必须失败
- And 对象不得被接受

### Scenario: Append firmware entry

- Given Header有效且SoC State尚未COMMITTED
- When 当前Owner追加一个Firmware Entry
- Then 必须先把Header置WRITING
- And 必须先提交新Entry再更新count/length/CRC
- And Header COMMITTED必须是整个追加动作的最后发布点

### Scenario: Append after State

- Given SoC State已经COMMITTED
- When 任一Stage尝试追加Firmware Entry
- Then 必须拒绝并记录安全错误

## Requirement: Stable snapshot

SPDM和审计Reader必须比较前后Header取得稳定snapshot，不得输出Header WRITING、缺失State或混合版本内容。

### Scenario: Header changes during snapshot

- Given Producer在Reader复制期间更新Header
- When Header A和Header B不一致
- Then Reader必须丢弃第一次复制并重试一次
- And 第二次仍变化时返回`SNAPSHOT_BUSY`

## Requirement: Measurement hash and counter

SoC加载型固件必须记录loader目标readback的32字节Hash。eHSM Vendor FW必须记录实际通过Vendor认证的完整package blob Hash。`rollback_counter`必须为固定16字节事实快照。

### Scenario: Manifest hash copied without readback

- Given loader对源payload计算的摘要正确，但目标内存被篡改或拷贝错误
- When Builder准备SoC固件Entry
- Then loader readback Hash必须失败
- And Entry不得提交

### Scenario: Timestamp requested

- Given 启动早期只有可复位Timer且没有已批准可信RTC
- When Builder构造Measurement
- Then 不得增加时间戳字段
- And 诊断时间只能进入RAS/audit
