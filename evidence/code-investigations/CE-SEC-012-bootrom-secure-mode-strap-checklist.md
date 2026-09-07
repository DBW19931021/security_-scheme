# CE-SEC-012：BootROM Secure Boot Checklist与Strap绑定复核

> 2026-07-29设计回填：项目负责人指定以SRC-0023截图为准，产品合同固定为`boot_pin.secure_boot[3]`、default 0、`0=非安全启动/1=安全启动`。下文保留2026-07-24发现的代码事实与当时调查结论；其中“等待二选一”的产品决策已经结束，但当前SRC-0022生成头仍未同步，真实寄存器绑定保持`BLOCKED_BY_RTL_SYNC`。不得直接读取当前名为`DIE_ID`的bit3，也不得沿用当前bit0的旧Expected。

## Evidence metadata

- Evidence ID：CE-SEC-012
- 日期：2026-07-24
- 父任务：TASK-SEC-SOC-FW-001 / 第6章BootROM收敛
- 资料基线：SRC-0023、SRC-0022、SRC-0016、SRC-0017、SRC-0018；`baremetal`和`gsp-pmp-rmp-omp`只读工作区
- 适用范围：NGU800P D0 BootROM启动模式、eHSM/FMC安全启动和失败闭锁
- 操作：复核负责人截图、RTL同步生成头、当前BootROM/安全组件文本；未修改两个代码仓，未构建/测试，未执行Git命令

## FACT-01：负责人提供七项BootROM重点Checklist

SRC-0023逐项列出：

1. `secureboot.001`：读取`secure_boot` Strap与Lifecycle；USER强制安全启动，LCS异常不得进入非安全启动。
2. `secureboot.002`：安全模式下有界等待eHSM BL、自检、eFuse Autoload和Lifecycle加载，失败不得启动FMC。
3. `secureboot.003`：使用真实eHSM `VERIFY_IMAGE`，FMC必须签名并加密，禁止stub。
4. `secureboot.004`：FMC 16字节Version Counter低于可信global counter、读取失败或结果未知时拒绝。
5. `secureboot.005`：只有eHSM PASS后才解析Protected Manifest，任一字段失败不得跳转。
6. `secureboot.006`：记录FMC真实Measurement，包括真实digest和load/entry。
7. `secureboot.007`：受控加载、确认Measurement、一次性跳转；任一失败记录原因/raw状态并保持FMC不可达。

`secureboot.002～007`与ADR-0003/0010～0014和主详设第3～5章的主体不变量一致，可继续展开；下面列出的顺序/职责解释必须服从已批准合同。

## FACT-02：截图Strap位号与SRC-0022软件寄存器不同

SRC-0023截图记录：

- source：`boot_pin.secure_boot`
- bit：`[3]`
- default：0
- 0=非安全启动，1=安全启动

SRC-0022当前RTL同步生成头（auto-generated 2026-07-15，register version `0.85r_0708`）记录：

| 文件/寄存器 | `SEC_BOOT` | bit3 |
|---|---|---|
| `regs/mgmt_sys_apb_reg.h` / `MGMT_SYS_APB_REG_STRAP_PIN` | `POS=0`、`MASK=0x1` | `DIE_ID` |
| `regs/sec_sys_apb_reg.h` / `SEC_SYS_APB_REG_STRAP_PIN` | `POS=0`、`MASK=0x1` | `DIE_ID` |

文件SHA-256：

- `mgmt_sys_apb_reg.h`：`f074ba6132432e761ddb03dc5e6fb37b2e224837df8b5d12e003e9b2bcca3eee`
- `sec_sys_apb_reg.h`：`be97a542fa1149ab46051de65cecb7ee06a1d1ec24814c7bdbba1b89898bf829`

当前两个代码仓没有找到上述Strap寄存器宏的BootROM consumer。截图可能描述上游`boot_pin`向量而生成头描述软件寄存器重排，也可能存在资料版本/位号错误；现有资料不能证明两者关系。

2026-07-24调查结论：登记OPEN-CONFLICT-010。2026-07-29负责人已裁决产品定义以截图为准；真实实现前仍必须由RTL/启动Owner更新生成头，或确认并交付`boot_pin.secure_boot[3]`到软件可见命名字段的权威映射，同时明确读取MGMT还是SEC镜像。不能仅在BootROM写私有常量。

## FACT-03：LCS×Strap完整策略未给出

SRC-0023已经给出两个安全不变量：

- `LCS=USER`时无视Strap，强制安全启动；
- LCS读取失败、非法或UNDEFINED时fail-close，不得进入非安全启动。

“其他Lifecycle按批准的Strap策略”没有附带Lifecycle枚举、允许非安全启动的状态、Debug/OTP/镜像权限或strap=0/1矩阵。现有SRC-0016/0017和accepted ADR也没有形成可编码的完整表。

结论：登记OPEN-DESIGN-012。USER安全路径和LCS错误门禁可以继续；任何非安全启动分支必须在矩阵批准后才能实现。未批准组合不得silent fallback到非安全启动。

## FACT-04：Checklist与现有合同的顺序解释

1. `secureboot.004`的rollback证明由真实eHSM/Counter合同产生；BootROM只比较16字节值、不写counter。低值和unknown拒绝，相同允许，高值作为FMC候选交FMC/GSP epoch链。
2. `secureboot.006`不能在loader前提交“实际load/entry”。正确展开是：验证/Manifest/digest通过后prepare Measurement；loader返回实际地址后finalize并commit。
3. `secureboot.007`的Halt/Wait Reset表示FMC不可达的fail-stop终态。security只记录、撤销、清零和上报；reset由RAS决定。RAS未ready时按ADR-0010关闭普通中断并进入无限WFI fail-stop循环，仅允许平台批准的Reset/NMI退出。OOB重刷必须服从后续Recovery方案，不能由BootROM错误路径自行宣称可用。
4. eHSM ready前置状态和LCS读取的准确顺序取决于SoC LCS接口是否依赖eFuse Autoload完成；当前没有平台Evidence证明。状态机可冻结逻辑前置条件，但真实寄存器顺序仍需平台输入。

## FACT-05：当前代码不满足Checklist

- `gsp-pmp-rmp-omp` BootROM默认仍为hello-world；可选路径使用静态package和`ehsm_stub`，没有真实Strap/LCS/eHSM/FMC jump。
- `baremetal`只提供当前RTL同步寄存器头，没有发现BootROM产品模式选择consumer。
- 因此SRC-0023是目标设计输入，不是当前实现已具备或已测试的证明。

## 设计回填

- 新增SRC-0023 Source Card和转录。
- 新增OPEN-CONFLICT-010、OPEN-DESIGN-012。
- 更新BootROM专题、函数级状态机、OpenSpec项目约束和主详设第6章。
- 第6章建立`secureboot.001～007`覆盖矩阵；Strap位号和非USER矩阵关闭前，不生成BootROM寄存器常量或非安全分支。
- 2026-07-29：产品位号、default和极性已按截图关闭；非USER矩阵为0受限非安全/1安全，USER仍强制安全。当前只保留RTL/生成头映射阻断，不生成真实寄存器常量或EMU确定性Expected。
