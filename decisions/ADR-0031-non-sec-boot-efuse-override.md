# ADR-0031：non_sec_boot eFuse强制受限非安全启动

> 2026-09-02细化：[ADR-0034](ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)在`non_sec_boot=0`且LCS为DEV/MANU、Strap=0时定义独立制造Provisioning子Profile；`non_sec_boot=1`仍只能进入本ADR的`RESTRICTED_NONSECURE`，不获得OTP写权限。USER设备若需在产品安全链失效后再烧写该位，必须绑定独立于C908产品启动的强授权维修路径。

- 状态：accepted
- 日期：2026-08-25
- 决策人：项目负责人
- 来源：SRC-0034
- 关联：ADR-0018、ADR-0024
- 替代关系：仅在`non_sec_boot`可靠断言为1时覆盖ADR-0018的USER强制安全和Lifecycle×Strap模式选择；其余情况下既有模式矩阵继续有效

## Context

现有BootROM以SoC LCS和`boot_pin.secure_boot[3]`选择安全或受限非安全启动，USER固定进入安全启动。项目现增加一个永久逃生通路：在eFuse中增加1 bit `non_sec_boot`，默认0不生效，烧写为1后强制BootROM进入非安全启动。

该字段会主动降低设备启动安全等级，且BootROM此前无权任意读取SoC安全eFuse。设计必须同时冻结覆盖优先级、失败处理、最小权限路径和只读硬件绑定边界。

## Decision

### 1. 字段语义与优先级

- `non_sec_boot`是1 bit单向eFuse策略位，逻辑默认值为0。
- `VALID && ECC_OK && non_sec_boot==0`：不触发覆盖，继续执行既有SoC LCS与`secure_boot`模式矩阵。
- `VALID && ECC_OK && non_sec_boot==1`：最高优先级强制进入现有`RESTRICTED_NONSECURE`路径，包括`LCS=USER`、Strap=1和其他原本选择安全启动的组合。
- 读失败、ECC异常、来源无效、复位锁存未完成或镜像不一致：进入`BOOT_POLICY_INPUT_ERROR`终态，安全和非安全FMC均不得release；不得把错误值当成0或1。

等价策略为：

```text
if non_sec_boot is VALID_ASSERTED:
    mode = RESTRICTED_NONSECURE
elif non_sec_boot is not VALID_DEASSERTED:
    mode = BOOT_POLICY_INPUT_ERROR
else:
    mode = existing_lifecycle_and_secure_boot_policy()
```

### 2. BootROM读取边界

- BootROM在最小平台初始化后只读取一次复位稳定的不可变快照，本boot instance内不得重读后改变模式。
- 目标硬件接口是专用只读、带`valid/ecc_ok`的Boot Policy Fuse视图。它可以由eFuse控制器复位锁存到只读寄存器，但不得向BootROM开放任意offset raw eFuse访问或烧写能力。
- 该逃生通路不能依赖Host输入、普通GSP服务、eHSM FW或安全FMC启动成功。
- 物理word/bit/offset、blank/programmed编码、ECC/valid、镜像、复位时序和命名宏必须来自目标D0 RTL/eFuse资料；缺失时产品BootROM绑定保持`BLOCKED_BY_NON_SEC_BOOT_BINDING`。

### 3. 强制非安全路径

`non_sec_boot=1`只改变模式选择，不扩大非安全路径权限。BootROM必须复用现有独立`NONSECURE_FMC` Profile，并满足：

1. 不等待或调用eHSM，不验证/解密安全FMC，不比较或更新counter；
2. 不创建Measurement Firmware Entry、不提交SoC State、不生成安全启动审计声明；
3. 不开放安全RAM常驻区、OTP/eFuse/KMU写接口、生产密钥、raw eHSM、受保护Debug或GSP安全服务；
4. source、Region、最大长度、目标privilege、Firewall和release primitive均来自独立只读平台Profile；Profile缺失或非法时进入终态，不得回落安全路径；
5. 仍执行地址防溢出、range/overlap、W^X、目标readback、指令侧同步、Firewall readback和一次性release。

### 4. 烧写与生效

- `non_sec_boot`只能通过批准的制造/维修eFuse流程从0单向烧写为1，不能由BootROM、Host或普通运行期服务写入。
- 烧写操作必须具有授权、blank/当前值检查、写后权威readback、不可逆操作审计和锁定策略；准确Owner和Lifecycle权限由Provisioning/eFuse集成资料冻结。
- 新值在下一次重新进入BootROM且硬件重新锁存Boot Policy Fuse视图后生效；本boot instance内的软件状态不能覆盖硬件快照。

## Consequences

- `non_sec_boot=1`明确覆盖“USER总是安全启动”，因此这是永久安全降级能力，量产、RMA、合规和产品对外声明必须将其作为可审计配置管理。
- BootROM只增加一次只读快照、一个最高优先级策略分支及异常终态，不引入eFuse解析器、烧写驱动或eHSM依赖。
- 在RTL/eFuse绑定完成前可以实现并单测纯策略函数，但不得以临时地址、裸bit或常量假装完成产品集成。

## Rejected alternatives

- 让BootROM直接读写raw eFuse：扩大ROM攻击面且破坏现有eHSM独占安全OTP/eFuse边界，拒绝。
- 将读取失败当作`non_sec_boot=1`：故障注入可直接触发安全降级，拒绝。
- 将读取失败静默当作0：可能在已烧写逃生位时错误进入安全链，且掩盖硬件故障，拒绝。
- 新建“无限制非安全启动”路径：会绕过既有最小权限Profile，拒绝。
- 在FMC/GSP阶段再判断：无法保证BootROM跳过安全FMC路径，也不能满足逃生通路独立性，拒绝。
