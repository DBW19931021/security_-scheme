---
title: "OTP/eFuse 接口"
status: review_ready_with_open_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0015
  - SRC-0016
  - SRC-0017
  - SRC-0018
  - SRC-0024
  - SRC-0034
owners:
  - GSP
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

定义产品层OTP/eFuse的typed访问边界；主合同见[《NGU800P安全软件详细设计》第10章](../05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)。

# Interface rules

- 不提供任意offset raw API；使用object ID、固定长度、usage和LCS policy。
- 写前检查blank/monotonic、寿命、recipe、授权和白名单；写后用硬件/Vendor批准路径证明。
- timeout/掉电后的unknown对象隔离，禁止自动重写。
- `INSTALL_RANDOM_KEY/INSTALL_ENCRYPT_KEY`只映射批准的key slot。
- 日志不记录key material、完整OTP敏感值或token。
- SoC Key轮换不复用通用安装API；产品caller只提交`key_type`、固定48字节密文、授权引用和审计ID，物理slot/Bitmap由eHSM内部决定。
- 轮换专用Vendor命令必须在成功响应前完成新Key证明、Bitmap提交和旧Key destroy；Bitmap状态未知时返回不可自动重试的quarantine语义。
- 一机一密RTL Root/Install KEK不属于普通OTP raw API，只通过RTL/DFT专用个性化接口写入/派生并对CPU不可读。
- 16个物理Key槽及物理Key ID 0～15按ADR-0025/0026固定；业务caller只使用逻辑object ID，物理slot/level/usage/`last_key`由单一Provisioning Matrix生成。
- Device Root必须在DEV LCS中先于所有Level2对象完成安装和证明；当前Vendor 4019若不支持该顺序，按交付缺口管理。
- eHSM内部OTP基址为`0x33000000`；Table 34字段、Key N `0x28` stride和`N=0..15`按SRC-0024冻结。该地址不是GSP可直访的System Address。
- BootROM不获得任意offset eFuse接口；`non_sec_boot`只通过专用、复位稳定、只读且带valid/ECC状态的Boot Policy Fuse视图消费。
- C908只能在`non_sec_boot=0`、LCS为DEV/MANU、Strap=0的`MANUFACTURING_PROVISIONING`子Profile中调用eHSM BL typed制造接口；BootROM、`RESTRICTED_NONSECURE`、`non_sec_boot=1`和USER路径均不能写OTP/eFuse/KMU。

# eHSM BL typed制造接口

Vendor目标BL接口必须以逻辑object和受签名Recipe/Ticket为输入，并在BL内部校验LCS、硬件锁存的制造启动条件、设备绑定、固定slot/Level/usage映射、依赖和顺序。最低能力包括capability/identity query、object query、wrapped install、internal generate、public key、fixed PoP、operation proof、finalize和相邻LCS transition；禁止raw OTP、caller自选slot/Attribute/`last_key`、任意Key导入、任意消息签名和任意内存访问。

对象状态固定为：

```text
BLANK
PROGRAMMING_PARTIAL
PROGRAMMED_INVALID
PROGRAMMED_VALID
PROVED
LOCKED
UNKNOWN
```

部分写恢复只能由BL明确授权，并要求同一device/recipe/object/材料摘要、剩余bit满足单向编程且CRC/ECC backend支持续写；否则隔离或报废。推荐提交顺序为Key Data→CRC/ECC辅助字段→readback→Attribute/valid/lock/`last_key`最后提交→operation proof。timeout、掉电或无法无歧义解释的状态一律为`UNKNOWN`，不得自动retry。

# `non_sec_boot` Boot Policy Fuse合同

| 项目 | 最终逻辑合同 |
|---|---|
| 宽度 | 1 bit |
| 默认逻辑值 | 0；不覆盖既有LCS×`secure_boot`模式矩阵 |
| 烧写值 | 1；下一次BootROM重新进入并完成硬件锁存后，最高优先级强制现有受限非安全启动 |
| BootROM访问 | 只读一次逻辑快照，必须同时取得`valid/ecc_ok/mirror_match`；不暴露raw word/offset |
| 运行期访问 | Host、普通GSP服务、`RESTRICTED_NONSECURE`和产品非安全固件不能写入、清零或覆盖快照；DEV/MANU制造子Profile只能调用BL typed断言操作 |
| 异常处理 | 读取失败、ECC异常、来源无效、锁存未完成、镜像不一致或非0/1值进入`BOOT_POLICY_INPUT_ERROR`；安全和非安全FMC均不得release |
| 烧写接口 | DEV/MANU仅由`MANUFACTURING_PROVISIONING`执行typed 0→1；USER安全链失效后的事后烧写必须使用独立于C908产品启动的强授权Secure ATE/维修端口或不可变BL窄操作；均要求授权、当前值检查、权威readback、锁定和审计 |

BootROM平台逻辑接口为：

```c
typedef struct {
    uint8_t non_sec_boot;
    uint8_t valid;
    uint8_t ecc_ok;
    uint8_t mirror_match;
} ngu_bootrom_policy_fuse_snapshot_t;

ngu_sec_status_t ngu_bootrom_platform_read_policy_fuse(
    ngu_bootrom_policy_fuse_snapshot_t *snapshot,
    ngu_sec_error_t *error);
```

该接口不得泛化成`efuse_read(offset)`或`efuse_write(offset,value)`。目标硬件应只向BootROM暴露复位锁存的命名策略字段；物理word/bit/offset、blank/programmed编码、ECC/valid、镜像、复位时序和寄存器/API名称由目标D0 RTL/eFuse资料生成。资料缺失时产品绑定为`BLOCKED_BY_NON_SEC_BOOT_BINDING`。

# 16-slot object groups

| Slots | 逻辑对象组 | 关键约束 |
|---|---|---|
| 0 | Chip Root | Level0，先于Level1 |
| 1～5 | Device/USER Root、eHSM Debug/Verify/Encrypt | Level1；slot3～5不得误作Level2 |
| 6、7、12 | SoC Verify/Encrypt/Debug rotation | Level2；只由批准轮换流程消费 |
| 8 | DICE root CA key | Level2 asymm；七项权限 |
| 9～11 | SoC Debug/Verify/Encrypt primary | Level2产品对象 |
| 13、14 | UDS、Device private | Level2 asymm；七项权限 |
| 15 | User auth key | Level2 asymm |

# SoC Key rotation logical contract

| 字段 | 逻辑约束 | Caller可控 |
|---|---|---|
| API version | GSP内部typed ABI版本 | 是，必须精确匹配 |
| `key_type` | 0=Verify、1=Encrypt、2=Debug | 是，白名单 |
| `ciphertext_B` | 固定48字节双层密文 | 是，但GSP不解密 |
| authorization reference | 设备绑定、一次性、含rotation scope | 是，必须验证 |
| audit ID | 不含密钥材料的追溯标识 | 是，格式受控 |
| physical slot/Bitmap/destroy | eHSM内部资源和状态 | 否 |

精确C结构、command ID、地址字段和raw status必须从匹配Vendor定制Host/BL/FW生成；当前`0xff08/0xff09`通用接口不能占位。

# Open questions

`OPEN-DESIGN-024`等待`non_sec_boot`物理位、编码、ECC/valid、镜像、只读视图、复位锁存、DEV/MANU烧写/锁定Owner以及USER独立维修入口。`OPEN-CONFLICT-012`等待RTL个性化绑定；`OPEN-CONFLICT-013`和`OPEN-DESIGN-014`的软件设计已经关闭。Vendor BL typed制造command、对象状态/partial-write/接受点/LCS、Bitmap/destroy掉电、Key Attribute/CRC/ECC/backend及KMS/CA/MES是实施绑定；Table 34 offset不再开放。rollback counter由ADR-0019独立管理。

# Change history

- 2026-09-02：按ADR-0034增加eHSM BL typed制造接口、部分写对象状态与恢复合同，限定只有DEV/MANU制造子Profile可调用；`non_sec_boot=1`和USER仍无制造权限。
- 2026-08-25：按SRC-0034/ADR-0031增加`non_sec_boot`只读Boot Policy Fuse、0→1制造烧写和异常终态合同；物理绑定保持开放。
- 2026-08-04：按SRC-0024纠正16-slot、Level和权限，并补入Table 34地址公式。
- 2026-07-29：接受ADR-0026；冻结物理Key ID 0～15、DEV层级顺序和单Profile规则。
- 2026-07-29：接受ADR-0025；补充一机一密和16槽typed对象组，登记Device Root顺序及Vendor生命周期差距。
- 2026-07-27：接受ADR-0021；新增SoC Key轮换typed逻辑合同并禁止通用安装API替代。
- 2026-07-27：同步ADR-0019/0020状态；Key/Certificate物理Slot仍由OPEN-DESIGN-014裁决。
- 2026-07-24：同步主详设。
