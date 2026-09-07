---
title: "生产制造"
status: review_ready_with_open_bindings
evidence_state: CONFIRMED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0015
  - SRC-0016
  - SRC-0017
  - SRC-0024
owners:
  - Manufacturing Owner待指定
  - Provisioning/KMS Owner待指定
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

定义量产阶段安全对象灌装、锁定、USER转换和为后续SoC Key轮换预留资源的recipe约束。

# Scope

覆盖逐die RTL个性化、设备身份、Root/SoC Key、证书、Counter初值、Bitmap/备用slot、Debug policy、LCS和Evidence；不记录真实密钥。

# Confirmed facts

- 进入USER前不得存在可达test key/cert/provider、stub或bypass。
- RTL Root/Install KEK按die唯一，CPU不可读；量产netlist不得使用共享Vendor默认宏。
- OTP物理对象、Level和Table 34采用SRC-0024/ADR-0025的唯一基线；baremetal只派生测试清单。
- ADR-0026批准DEV中的Chip Root→其余Level1→Level2顺序、每设备单Attestation Profile、Cert0/1和typed制造接口。
- ADR-0034批准在DEV/MANU的非安全制造子Profile中运行Provisioning FW，由eHSM BL执行typed Key安装/状态/证明，并把MANU→USER作为最终提交。
- SoC Key轮换使用每类原始/轮换两个逻辑位置，制造时必须预留未消耗的轮换位置和初始Bitmap状态。

# Documented facts

- SRC-0015的三类Key为Verify、Encrypt、Debug，每类只轮换一次。
- SRC-0015要求外部体系具备`RTL SoC KEK`、`CHIP_ROOT_KEY`、`DEVICE_ROOT_KEY`相关密钥生成/托管能力。

# Vendor implementation observations

- 当前Vendor通用安装代码可提供制造期OTP安装证据，但不能证明USER轮换能力。

# Assumptions

真实Vendor Bitmap bit、RTL个性化接口、Flash base/erase粒度和写入原子性仍待输入，不在recipe中填入Vendor示例值。

# Approved design

制造recipe至少按以下阶段执行：

1. 绑定设备ID、SKU、RTL/eHSM/BL/FW版本和recipe版本；
2. 保持`non_sec_boot=0`和DEV，通过Strap=0进入`MANUFACTURING_PROVISIONING`；BootROM加载独立Provisioning FW但不写OTP；
3. 通过Secure ATE逐die写入/派生RTL Root/Install KEK，证明不可读和锁定；
4. Provisioning FW等待eHSM BL ready，提交signed recipe/ticket；BL验证LCS、硬件制造条件、设备绑定、固定对象映射和顺序；
5. 安装Chip Root，再按recipe安装slot1～5的Level1对象，其中Device Root必须在所有Level2之前可用；DEV→MANU前全部Level1必须`PROVED/LOCKED`；
6. 安装目标Profile的Level2对象；slot6/7/12是三类轮换位置并保持blank，主Key为slot10/11/9；
7. 在slot14内部生成Device Private，导出公钥/PoP并由离线CA签发；按批准Profile处理slot8 DICE root CA和slot13 UDS的七项属性权限；
8. 写Cert0并完成DER/chain/device/public-key/Profile校验，Cert1保持inactive；
9. 证明1字节Bitmap处于Vendor规定的原始映射状态；安装Counter初值和Debug policy，验证Boot/Update Key复用映射；
10. 扫描并关闭test/bypass，锁定临时写权限并提交审计；
11. Strap切安全，使用最终Key、证书和策略执行reset/reload，完成真实FMC/GSP/Measurement、动态证书、SPDM/签名和授权负例；
12. P11全部通过后执行DEV→MANU并readback；MANU只执行批准的收口白名单，不补写需要DEV权限的对象；
13. MANU→USER作为最后一个不可逆提交并readback；随后执行USER冷启动产品证明，USER后制造Profile不可达。

对象状态必须使用`BLANK/PROGRAMMING_PARTIAL/PROGRAMMED_INVALID/PROGRAMMED_VALID/PROVED/LOCKED/UNKNOWN`。中断后只允许在DEV/MANU重新由pin进入制造Profile并先query；`PROGRAMMING_PARTIAL`仅在device/recipe/object/材料完全一致、OTP剩余bit单向兼容且BL/backend明确支持时恢复，否则隔离或报废。`LCS=USER last`不能替代该恢复合同。

任何slot/Bitmap状态UNKNOWN、不允许恢复的部分写、轮换位置已被写、KMS记录与设备不一致或审计无法提交时，设备必须隔离，不得通过重烧或软件覆盖继续量产。`non_sec_boot=1`只进入`RESTRICTED_NONSECURE`，不是制造重入手段；USER安全链失效后的事后断言必须依赖独立强授权维修硬件路径，未绑定时不得声明可救回。

# Open questions

- `OPEN-CONFLICT-012`：RTL个性化载体、ATE、lock/proof。
- `OPEN-CONFLICT-013`和`OPEN-DESIGN-014`的软件设计已经由ADR-0026关闭。
- RTL个性化、Vendor BL typed制造命令及状态/partial-write/LCS原子性、Flash base、KMS/CA/MES接口、USER独立维修入口、寿命/耗尽和报废执行细则仍是实施绑定。

# Implementation impact

Provisioning/release matrix和制造recipe必须来自同一受控配置；没有设备/SKU精确匹配行时默认拒绝。

# Verification impact

量产验证覆盖制造/受限启动Profile矩阵、空白/partial/unknown检查、同材料恢复、错误slot/顺序/KMS设备绑定、Bitmap非初始值、test资产残留、最终配置安全启动预演、锁定失败、MANU白名单、USER最后提交和USER转换后所有制造写入拒绝。

# References

- [ADR-0021《SoC Key轮换采用Vendor定制机制》](../../decisions/ADR-0021-soc-key-rotation-vendor-mechanism.md)
- [ADR-0025《一机一密RTL Key与16槽OTP Key基线》](../../decisions/ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md)
- [ADR-0026《量产灌装、设备证明、证书A/B与制造接口合同》](../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)
- [ADR-0034《非安全制造灌装、eHSM BL密钥安装与USER最终提交》](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)
- [《安全配置与注入》操作专题](provisioning.md)
- [《NGU800P安全软件详细设计》第10章](../05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)

# Change history

- 2026-09-02：按ADR-0034固定非安全制造子Profile、eHSM BL不信任C908的typed安装边界、部分写恢复、MANU收口和USER最终提交。
- 2026-07-29：接受ADR-0026；批准Root顺序、单Profile、UDS、Cert0/1和制造接口，待补实施绑定。
- 2026-08-04：按SRC-0024纠正Level1/Level2顺序、轮换槽、Device Private slot14、DICE slot8和UDS slot13。
- 2026-07-29：补充一机一密RTL个性化、16槽、Device Private/CA/Cert0、完整锁定和产品证明顺序。
- 2026-07-27：新增SoC Key原始/轮换资源预留、Bitmap初始状态和USER前门禁。
