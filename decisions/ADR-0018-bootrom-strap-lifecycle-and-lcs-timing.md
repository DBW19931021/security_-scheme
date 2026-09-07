# ADR-0018：BootROM Strap、Lifecycle与LCS读取时序

> 2026-08-25后续裁决：[ADR-0031《non_sec_boot eFuse强制受限非安全启动》](ADR-0031-non-sec-boot-efuse-override.md)在本ADR前增加最高优先级覆盖。只有`non_sec_boot=0`时执行本文Lifecycle×Strap矩阵；可靠值1时包括USER在内均强制进入现有受限非安全路径。

- 状态：accepted
- 日期：2026-07-24
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0022、SRC-0023
- 相关 Requirement/Open Question：OPEN-CONFLICT-010、OPEN-DESIGN-012
- 补充关系：补充ADR-0009、ADR-0012

> 2026-07-29更新：项目负责人指定SRC-0023截图为本项产品权威，`boot_pin.secure_boot[3]`固定为default 0、`0=非安全启动`、`1=安全启动`。当前SRC-0022生成头仍把软件寄存器bit0命名为`SEC_BOOT`、bit3命名为`DIE_ID`，因此产品策略已经关闭，但真实寄存器绑定保持`BLOCKED_BY_RTL_SYNC`：RTL/生成头或权威映射修正前，不得把当前bit0或当前bit3用于产品模式判定。2026-07-28关于LCS异常进入独立受限非安全启动且不写Measurement、SoC State或启动审计的[ADR-0024](ADR-0024-manifest-simplification-boot-mode-and-ehsm-self-test.md)继续有效。当前流程见[《NGU800P安全软件详细设计》第6章](../docs/05-software-design/NGU800P安全软件详细设计.md#第6章-bootrom安全启动详细设计)。

## 背景

SRC-0023截图把顶层`boot_pin.secure_boot`标为bit `[3]`，并给出default 0、`0=非安全、1=安全`的说明；SRC-0022登记的当前RTL同步生成头则把MGMT/SEC `STRAP_PIN.SEC_BOOT`定义为软件可见bit 0，bit 3定义为`DIE_ID`。项目负责人已明确要求以截图为准，因此前者是产品合同，后者是尚待RTL/生成头同步的实现冲突。原Checklist只说明USER生命周期强制安全启动，没有给出非USER的完整二值策略，也没有确认SoC LCS是否等待eHSM eFuse Autoload。

这些差异阻断BootROM真实模式判定和确定性测试Expected，因此由项目负责人作最终产品裁决。

## 决策

1. `secure_boot`产品字段以SRC-0023截图为准：
   - 唯一字段身份为`boot_pin.secure_boot[3]`；
   - reset/default值为0；
   - 逻辑语义为`0=非安全启动`、`1=安全启动`；
   - 这里的`[3]`是产品pin字段定义，不授权软件直接读取当前APB `STRAP_PIN`寄存器bit3。
2. 当前SRC-0022生成头与该产品合同冲突：
   - 当前bit0虽命名为`SEC_BOOT`，其位号和0/1语义均不得作为本产品模式Expected；
   - 当前bit3命名为`DIE_ID`，不得被BootROM强行解释为`secure_boot`；
   - RTL/启动Owner必须提供与截图一致的新生成命名宏，或提供经批准且可验证的`boot_pin.secure_boot[3]`到软件可见字段映射；
   - 在此之前只允许实现消费归一化逻辑值的纯策略函数和fake/unit测试，禁止真实寄存器绑定与EMU确定性Expected。
3. Lifecycle与Strap模式矩阵冻结为：

| SoC LCS | `secure_boot`归一化值 | 启动模式 |
|---|---:|---|
| `USER` | 0 | 安全启动 |
| `USER` | 1 | 安全启动 |
| 任一已识别非`USER`状态 | 0 | 受限非安全启动 |
| 任一已识别非`USER`状态 | 1 | 安全启动 |
| LCS读取失败、非法、`UNDEFINED`或未识别 | 任意 | 受限非安全启动 |
| Strap读取失败、值非法或RTL绑定未同步 | 已识别非USER | fail-close，不选择任一模式 |

4. USER生命周期无条件忽略Strap并强制安全启动，Strap不能把USER降级为非安全。
5. 非USER生命周期按截图语义选择：`0=受限非安全启动`、`1=安全启动`。
6. SoC LCS由BootROM直接读取，其有效性不依赖eHSM Autoload。BootROM可在等待eHSM BL Ready之前完成Strap/LCS采样和模式判定。
7. 安全启动路径继续执行第6章完整eHSM/FMC门禁。非安全启动是独立模式入口，不得复用安全路径后再跳过若干检查；其允许镜像、权限和接口按本ADR最低约束执行，并在详设中显式定义。
8. 模式判定先按LCS分类：USER直接强制安全，LCS异常按ADR-0024直接进入受限非安全；只有已识别非USER需要消费Strap。此时Strap读取失败、字段未绑定、镜像不一致或平台初始化失败不得选择任一模式。

## 非安全启动最低安全边界

为避免把“非安全启动”误解为“无边界启动”，固定以下最低规则：

1. 已识别非USER且`secure_boot=0`可达；LCS异常也按ADR-0024进入该路径；
2. 不把非安全FMC记录为“已通过安全验证”，Measurement中的验证状态固定为`NOT_VERIFIED_BY_POLICY`；
3. 不允许访问USER专属OTP、生产密钥、受保护Debug、counter更新或安全服务；
4. 不得改变eHSM、Firewall、LCS或不可逆资源的安全默认状态；
5. 非安全FMC的source、load/entry Region、最大尺寸、执行privilege和允许接口仍必须受allowlist约束；
6. 进入USER后，任何历史非安全配置或缓存状态不得使非安全入口继续可达。

非安全镜像的最终格式、独立Region和具体接口仍可作为后续实现参数登记，但不得改变上述模式矩阵和权限边界。

## 软件影响

- BootROM状态机在最小平台初始化后先读取SoC LCS；仅在LCS为已识别非USER时读取Strap，再选择安全或非安全独立入口。
- 平台适配只可使用后续与SRC-0023一致的RTL生成命名宏或批准映射，不维护私有bit定义；当前SRC-0022字段不得直接绑定。
- `ngu_bootrom_select_mode()`使用本ADR的固定矩阵；LCS异常按ADR-0024进入受限非安全，已识别非USER的Strap异常才fail-close。
- 安全路径不再等待eHSM Autoload后才读取LCS。
- 新增非安全路径的stage profile和测试，但产品编码仍需其package source、load/entry、privilege和Firewall参数具备平台Evidence。

## 测试影响

1. 覆盖USER下Strap 0/1均强制安全启动。
2. 覆盖所有已识别非USER状态下`0=受限非安全、1=安全`。
3. 分别覆盖：LCS异常进入受限非安全且不获得有效LCS权限；USER在Strap无效时仍强制安全；已识别非USER在Strap读取失败、非法或映射未同步时不进入任一启动模式。
4. 在RTL/生成头同步前，不生成真实寄存器Expected；同步后验证命名字段确实对应`boot_pin.secure_boot[3]`且保持`0=非安全、1=安全`。当前APB bit3仍名为`DIE_ID`，不得被测试绕过命名冲突直接驱动。
5. 验证LCS读取和模式判定在eHSM Autoload未完成时仍使用SoC独立有效值。
6. 非安全启动用例必须证明无法访问USER专属密钥、OTP、Debug和counter更新能力。

## Review history

- 2026-07-29：项目负责人指定以SRC-0023截图为准；产品字段更新为`boot_pin.secure_boot[3]`、default 0、`0=非安全/1=安全`。当前SRC-0022生成头降为本字段的冲突实现证据，OPEN-CONFLICT-010重开为`BLOCKED_BY_RTL_SYNC`。
- 2026-07-28：ADR-0024替代LCS异常fail-close规则，改为受限非安全且无Measurement/SoC State/启动审计。
- 2026-07-24：首次接受本ADR；当时采用SRC-0022 bit0和相反极性，该部分已被2026-07-29裁决替代。

## 参考资料

- `sources/source-cards/SRC-0022.md`
- `sources/source-cards/SRC-0023.md`
- `sources/conflict-reports/CONFLICT-BOOTROM-SECURE-BOOT-STRAP-BIT-AND-POLICY.md`
- `docs/05-software-design/NGU800P安全软件详细设计.md`
- `evidence/code-investigations/CE-SEC-012-bootrom-secure-mode-strap-checklist.md`
