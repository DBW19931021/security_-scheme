# ADR-0022：Measurement Table ABI v1、Commit与Snapshot

- 状态：accepted
- 日期：2026-07-27
- 决策人：项目负责人
- 相关Source ID：SRC-0016、SRC-0017、SRC-0022
- 关闭问题：OPEN-DESIGN-007逻辑ABI部分
- 相关合同：FW-C-007、SEC-FEAT-009
- 补充关系：补充ADR-0004、ADR-0005、ADR-0012、ADR-0019；不改变已批准的producer/consumer、16字节`rollback_counter`和release顺序

> 2026-08-21更新：ADR-0030已删除Manifest；Measurement身份由stage-owned registry产生，`load_addr/entry_addr`只保存typed-stage loader成功结果快照且`entry_addr=load_addr`。Measurement Table自身的128字节Header/Entry/State、可变长度、CRC/commit和Snapshot规则保持有效。当前完整合同见[《NGU800P安全软件详细设计》第9章](../docs/05-software-design/NGU800P安全软件详细设计.md#第9章-verificationloadercounter与measurement)。

> 2026-08-19更新：[ADR-0028《2 MiB安全SRAM精确划分与受控原地加载》](ADR-0028-security-sram-fixed-layout-and-in-place-loader.md)已固定Measurement物理Region为16 KiB、offset `0x0FC000`；本ADR逻辑ABI不变，实际产品`max_fw_entries`仍按启动拓扑生成且不得超过126项物理硬上限。

## 背景

BootROM提交FMC事实、FMC消费FMC事实并提交GSP事实、GSP继续提交eHSM FW和Runtime事实的流程已经批准。经过两轮评审，负责人要求Measurement回归SRC-0016核心字段，并明确：

- 只记录SoC安全状态，不记录eHSM状态；
- BootROM作为隐式可信测量根，不自证为普通Firmware Entry；
- 实际微核/固件实例数量可能变化，Table必须支持可变数量；
- 删除固定8槽、固定1280B、`state_entry_count`、`generation`、地址domain、Key/Signer和时间戳；
- 保留CRC和commit，结构只留一个reserved。

本ADR冻结逻辑ABI。物理基址、最大容量、PMA/cache、Firewall和SPDM wire映射仍由其他开放项管理，不阻止本ADR成为已批准设计。

## 决策

### 1. 可变长度布局

```text
Header[128]
Firmware Entry[fw_entry_count][128]
SoC State Entry[128]
```

`total_len = 128 + fw_entry_count × 128 + 128`。

- `fw_entry_count`是当前紧凑列表中COMMITTED Firmware Entry实际数量；
- Firmware列表禁止空洞；
- SoC State固定一项，因此删除`state_entry_count`；
- 物理Region按`max_fw_entries`固定预留，`max_fw_entries`不是Header字段。

### 2. Firmware实例语义

1. 一个Entry对应一个独立验证、加载或release/隔离实例。
2. `fw_type + instance_id + die_id`唯一；`instance_id`来自已认证Manifest。
3. 同一镜像被多个实例独立release时，允许Hash相同但必须分别建Entry。
4. 多个微核共享同一代码并作为一个整体release时，只建一项。
5. FMC必须为Entry 0，GSP必须为Entry 1；GSP追加项按`fw_type、die_id、instance_id`稳定排序。
6. Entry身份由producer按`fw_type + die_id + instance_id`生成；Manifest不提供物理slot或逻辑`measurement_slot`。

### 3. BootROM

BootROM是隐式可信Root of Trust for Measurement，不创建普通Firmware Entry，不记录“BootROM状态”，也不以自Hash证明自身。

若后续需要BootROM版本追溯，必须使用芯片绑定的不可变ROM identity/release digest并作为独立身份声明处理。

### 4. Header

Header固定128字节，只包含：

- `magic/version/header_len/total_len/fw_entry_count`；
- `device_uuid/chip_id`；
- 一个尾部`reserved[]`；
- CRC-32C和32位`commit_marker`。

删除：

- `state_entry_count`；
- `generation`；
- `table_flags`；
- Header中与State重复的LCS、Debug、Counter和Certificate。

由于暂不使用`generation`，BootROM每次安全启动必须先使旧Header失效并清零整个固定Region，再发布新Header；任何未完成该顺序的启动不得继续release。

### 5. Firmware Entry

Firmware Entry固定128字节，保留：

- `fw_type/instance_id/die_id/flags`；
- `hash_algo/hash_len/hash[32]`；
- `rollback_counter[16]`；
- `verify_result/release_state`；
- 64位`load_addr/entry_addr`；
- 一个尾部`reserved[]`；
- CRC和commit。

删除：

- `algorithm_profile`；
- `key_id/signer_id`；
- `load_addr_domain/entry_addr_domain`；
- raw status、counter before/after/relation/update state、timestamp和其他诊断字段。

Manifest地址固定为baremetal System Address并由loader/platform profile校验；Measurement中的地址统一为loader最终产生的同一64位System Address，不建立Local/System映射。

### 6. SoC State

SoC State固定一项、128字节，记录：

- LCS、Debug、防回滚、16字节全局Counter；
- Firewall、OOB/Recovery、boot failure、Die数量；
- SPDM certificate slot和证书链摘要；
- 一个尾部reserved、CRC和commit。

不记录：

- eHSM status/error/health；
- Key轮换Bitmap或Key/Signer内部slot；
- 普通Timer或时间戳；
- 运行期RAS事件。

### 7. CRC与Commit

Header、Firmware Entry和State均在offset 120保存CRC-32C，在offset 124保存32位commit marker：

- `EMPTY=0x00000000`；
- `WRITING=0x54495257`；
- `COMMITTED=0x54494D43`。

CRC只检测torn write和随机损坏，不提供Writer认证。真实可信性依赖安全启动、唯一Owner和安全RAM权限隔离。

每次追加Firmware Entry时，Writer先把Header置WRITING，再提交新Entry，最后更新`fw_entry_count/total_len/CRC`并重新发布Header COMMITTED。State COMMITTED后Table最终完成，不得继续追加。

### 8. Snapshot

SPDM/审计Reader：

1. 复制并验证Header A；
2. 按A中的count/length复制紧凑Firmware列表和唯一State；
3. 验证每项commit、CRC和字段；
4. 再复制Header B；
5. 只有A/B逐字节一致才接受；否则重试一次，仍变化返回`SNAPSHOT_BUSY`。

不再使用固定slot bitmap。

## 安全理由

- 可变列表直接表达实际启动实例，不需要为未启动微核填充空槽。
- 唯一State不需要`state_entry_count`。
- BootROM作为测量根而不是自证对象，避免把自Hash误当可信证明。
- Measurement只保存最终SoC事实；eHSM状态、Key内部slot和时间属于各自管理/审计域。
- 单一baremetal System Address语义消除domain字段；Manifest与loader按System Address范围和Region验证地址。
- Header WRITING/COMMITTED和每Entry commit允许BootROM、FMC、GSP顺序追加，同时阻止Reader接受半更新Table。
- SPDM请求Nonce和签名承担对外新鲜性，核心Measurement不依赖不可信启动时间戳。

## 影响

- OPEN-DESIGN-007逻辑ABI关闭。
- Manifest不再携带`measurement_slot`；公共registry、producer和Measurement Writer按实际实例元组生成Entry。
- FMC consumer改为按`fw_type/instance_id/die_id`查找，不依赖固定slot。
- 下一版测试规划需要覆盖可变count/length、紧凑前缀、重复实例、Header更新、State最终提交和Region上限。
- 当前不修改`gsp-pmp-rmp-omp`、`baremetal`、Vendor资料或测试工作簿。

## 剩余实现输入

- 产品最大独立Firmware/微核实例数与实际共享/独立release关系；物理Region已固定16 KiB且最多126项；
- Measurement Firewall，以及待SoC稳定后在`NON_CACHEABLE/HARDWARE_COHERENT`中唯一裁决的PMA属性；
- SPDM block编号、对外字段和证书/transport；
- 32位commit跨master可见性Evidence。
