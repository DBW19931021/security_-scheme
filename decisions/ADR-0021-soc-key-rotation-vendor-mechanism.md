# ADR-0021：SoC Key轮换采用SRC-0015 Vendor机制

- Status: accepted
- Date: 2026-07-27
- Owner: 项目负责人
- Related: SRC-0015、SRC-0018、OPEN-BASELINE-002/004、OPEN-DESIGN-014、ADR-0001、ADR-0019、ADR-0020、ADR-0026
- Last reviewed: 2026-07-29

> 2026-07-29更新：ADR-0026已经关闭OPEN-DESIGN-014的软件设计裁决。下文关于Vendor ABI、Bitmap/掉电、KMS和制造recipe“开放”的表述，现统一解释为实施绑定，不再要求负责人重新裁决轮换方案；证书A/B选择也由ADR-0026更新为无active pointer。

## Context

负责人要求密钥轮换机制参考`source-vault/vendor-docs/osr_定制需求/`中的《云天励飞26Q2 定制需求方案》（SRC-0015）。该PDF已经明确eHSM内部的候选轮换模型，但历史登记只确认“策略已批准”，尚未把页面级机制、SoC软件边界和当前Vendor代码交付差距写入完整详设。

SRC-0015中的`Host`是调用eHSM Mailbox的SoC侧Host角色，不等于向外部Host开放GSP通用Key/Rotation服务。当前SRC-0018代码快照只有`INSTALL_RANDOM_KEY`和`INSTALL_ENCRYPT_KEY`通用安装接口；它们由调用者指定slot，且当前FW明确拒绝USER/DEBUG生命周期安装，没有SRC-0015要求的专用轮换命令、HSM内部Active Bitmap管理和USER鉴权后轮换逻辑。

## Decision

1. SoC Key轮换的产品目标机制采用SRC-0015：
   - 只覆盖`SoC Verify Key`、`SoC Encrypt Key`和`SoC Debug Key`三类对象；
   - Verify Key由启动验证和升级验证复用，Encrypt Key由启动解密和升级解密复用；
   - 每类Key具有原始/轮换两个逻辑位置且只允许轮换一次，整机最多完成三次此类切换；
   - eHSM内部管理1字节OTP Bitmap，SoC侧只提交受控`key_type`和密文封装，不直接选择物理slot或写bitmap；
   - PDF中的slot 9～14及Verify使用bit0只作为示例，不是NGU800P物理配置。
2. SRC-0015给出的48字节封装作为Vendor定制接口的设计输入：
   - 新SoC Key为32字节；
   - `ciphertext_A`由`DEVICE_ROOT_KEY`保护；
   - 48字节内层结构为`key_attributes[4] + ciphertext_A[32] + CRC32[4] + key_type[4] + padding[4]`；
   - 整个48字节再由`RTL SoC KEK`保护为`ciphertext_B`后送入eHSM。
   - 精确算法、mode/IV、字节序、属性编码、CRC参数、padding值和外层/内层`key_type`一致性检查，必须来自Vendor定制TRM、Host header和匹配BL/FW代码，当前不得猜测。
3. USER生命周期下，轮换必须经过现有SoC Challenge-Response鉴权并获得eHSM批准的轮换权限。GSP只通过内部typed service发起；不向外部Host开放GSP通用Key/Rotation API，不允许raw Mailbox透传。TEST/DEV/MANU是否由硬件默认满足该前置条件，按SRC-0015和匹配Vendor交付实现，不由SoC业务代码绕过。
4. eHSM内部目标顺序采用SRC-0015：解密和校验封装、写新Key、提交Bitmap、销毁旧Key、返回成功；SoC侧收到成功后请求平台批准的eHSM硬件复位，复位后eHSM读取Bitmap并装载新Key。业务代码只请求reset，不越过既有RAS/reset Owner边界。
5. 轮换是单向不可逆操作：
   - Bitmap提交前仍使用旧Key；OTP新slot若出现部分写或状态未知，不得自动清理、重写或重试；
   - Bitmap提交状态未知时，Key对象和eHSM服务进入quarantine，禁止猜测旧/新Key；
   - Bitmap已确认提交后不得自动回退旧Key；
   - 旧Key destroy已执行后，即使响应、reset或首次新Key启动验证失败，也只能fail-close并进入受限恢复，不能恢复旧Key。
6. SRC-0015要求“Bitmap始终指向有效Key”，但没有给出Bitmap写入原子粒度、掉电检测、状态查询、幂等性以及“提交Bitmap后、销毁旧Key前后”的恢复证明。ADR-0026关闭软件设计裁决后，以上项目连同专用command ABI、KMS密钥托管和可执行制造recipe作为实施绑定管理；在Vendor定制交付和这些绑定到齐前，不授权产品轮换编码。
7. 证书轮换不属于SRC-0015的OTP SoC Key轮换。证书按ADR-0026使用受保护Flash的Cert0/Cert1、commit-last和扫描最大有效sequence，不设置active pointer，私钥不随证书写入Flash。

## Consequences

- `OPEN-BASELINE-004`关闭：SRC-0016 v1.2原PDF保持不变，SRC-0015轮换机制通过本ADR和主详设第10章作为受控软件方案增量。
- 本ADR当时部分收敛`OPEN-DESIGN-014`；ADR-0026现已关闭其软件设计裁决。对象范围、每类一次、1字节HSM管理Bitmap、双层48字节封装、USER鉴权、内部写入顺序和复位生效已冻结；精确Vendor Bitmap/wire ABI、掉电原子性、旧Key销毁恢复、KMS托管和Vendor交付版本是实施绑定。
- 当前通用`INSTALL_RANDOM_KEY/INSTALL_ENCRYPT_KEY`不得替代专用轮换命令。
- 当前Vendor交付差距登记为`OPEN-CONFLICT-011`；在匹配定制Host/BL/FW、专用command和release note到齐前，implementation为`BLOCKED_BY_VENDOR_DELIVERY`。
- 本ADR只更新`security_-scheme`中的设计、OpenSpec和追溯；不修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor快照，不执行Git操作。
