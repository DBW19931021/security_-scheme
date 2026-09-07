---
title: "安全配置与注入"
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
owners:
  - GSP
  - Provisioning/KMS Owner待指定
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

定义设备Key/Certificate/Counter/LCS配置和SoC Key轮换的受控操作边界；规范性合同见[《NGU800P安全软件详细设计》第10章](../05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)。

# Scope

覆盖制造灌装、USER态SoC Key轮换、证书安装/切换、外部KMS输入、审计和不可逆失败处置。不定义真实密钥值，不在本仓库存放密钥、token或设备秘密。

# Confirmed facts

- 密钥轮换策略已批准；ADR-0021确认机制参考SRC-0015。
- ADR-0025/SRC-0024批准一机一密RTL Key、Table 34和16槽OTP对象唯一基线。
- ADR-0026批准Chip Root→其余Level1→Level2顺序、单设备Attestation Profile、Cert0/1和制造接口。
- ADR-0034批准DEV/MANU非安全制造子Profile、eHSM BL typed Key安装/状态/证明接口以及USER最终提交合同。
- GSP安全服务不向外部Host开放；GSP唯一`security_service_task`调用eHSM。

# Documented facts

- SRC-0015定义三类SoC Key、每类一次、1字节HSM Bitmap、48字节双层封装、USER鉴权、写Key→Bitmap→destroy→reset。
- 示例slot 9～14和bit位置不是产品配置。

# Vendor implementation observations

- 当前SRC-0018通用安装命令由caller指定slot，且FW拒绝USER/DEBUG生命周期安装。
- 当前交付未包含SRC-0015专用轮换command、Active Bitmap和USER轮换流程，见CE-SEC-013。

# Assumptions

无新增硬件行为假设。Vendor定制交付未到齐的字段保持开放。

# Approved design

Provisioning Matrix必须逐设备/Profile绑定Key对象、算法、usage、原始/轮换slot、Bitmap bit、LCS、一次性operation授权策略ID、KMS Key ID、recipe版本和可执行命令版本。该授权字段不属于Debug scope；Debug不存在scope。

BootROM保留安全/非安全两个顶层分支。制造接口只在`non_sec_boot=0`、LCS为DEV/MANU、Strap=0且制造Profile有效时，以`NON_SECURE_BOOT / MANUFACTURING_PROVISIONING`加载独立C908 Provisioning FW；`non_sec_boot=1`、LCS异常和其他非制造组合进入`RESTRICTED_NONSECURE`，不得调用制造接口。

Controller到Provisioning FW采用canonical CBOR/COSE signed recipe和typed command：`GET_CAPABILITIES/GET_IDENTITY/BEGIN/QUERY_OBJECT/INSTALL_WRAPPED_KEY/GENERATE_DEVICE_KEY/GET_DEVICE_PUBLIC_KEY/SIGN_PROOF/WRITE_CERT_SLOT/VERIFY_CERT_BINDING/TRANSITION_LCS/FINALIZE/ABORT`。Provisioning FW将其映射为Vendor eHSM BL typed query/install/generate/proof/finalize/LCS API。BL把C908视为不可信caller，必须验证LCS、硬件锁存制造条件、ticket、设备绑定、固定object→slot/Level/usage映射、依赖和顺序。禁止raw OTP、raw eHSM、caller自选slot/Attribute/`last_key`、任意消息签名和任意内存访问。

初始量产顺序固定为RTL个性化→DEV中的Chip Root→slot1～5其余Level1→目标Profile的Level2对象（slot6/7/12轮换位置保持blank）→slot14 Device Issuer Private及slot13 UDS operation proof→固定PoP/Enrollment Record→Host/CA离线签发并验证静态Issuer前缀→Cert0→lock→使用最终Key/证书/策略执行reset/reload及完整产品安全启动预演→DEV→MANU收口/readback→MANU→USER最后提交/readback→USER冷启动时由GSP生成一级动态Firmware Alias Leaf并完成外部证明。制造Device不构造PKCS#10、不解析通用X.509，只执行ticket/本地绑定/hash和Flash原子提交。

对象状态固定为`BLANK/PROGRAMMING_PARTIAL/PROGRAMMED_INVALID/PROGRAMMED_VALID/PROVED/LOCKED/UNKNOWN`。掉电或重启后必须先query；只有同一device、recipe、object、材料摘要、剩余bit单向兼容且CRC/ECC backend与BL明确支持续写时，才允许恢复`PROGRAMMING_PARTIAL`，否则隔离或报废。`LCS=USER last`是必要门禁，但不能替代半写恢复、LCS掉电语义和USER前完整产品证明。

若要求在USER安全链完全失效后再烧写`non_sec_boot`，必须提供独立于C908产品FMC/GSP和Provisioning FW成功启动的强授权Secure ATE/维修端口或不可变eHSM BL窄操作。`non_sec_boot=1`后的下一次启动仍是`RESTRICTED_NONSECURE`，不会重开制造灌装。

USER轮换流程：

1. 外部KMS确认设备身份、当前Key类型和未消耗轮换额度；
2. 生成32字节新Key并按SRC-0015形成48字节双层密文；
3. 取得设备绑定的一次性轮换授权；
4. GSP校验typed请求后调用Vendor专用命令；
5. eHSM完成写新Key、证明、Bitmap提交和旧Key destroy；
6. GSP只记录对象ID、Key类型、recipe版本、结果和审计ID，请求平台reset；
7. reset后执行新Key operational proof；失败进入受限恢复，不回旧Key。

任何unknown/timeout都不得自动重复同一OTP写操作。

# Open questions

- `OPEN-CONFLICT-012`：RTL逐die隐藏个性化绑定。
- `OPEN-CONFLICT-013`和`OPEN-DESIGN-014`的软件设计已经由ADR-0026关闭。
- Vendor BL typed制造API及状态/partial-write/接受点/LCS合同、RTL逐die合同、Key Attribute/CRC/ECC/backend、Flash base/erase粒度、KMS/CA/MES接口、制造wire数值和USER独立维修入口仍是实施绑定；Table 34 offset已冻结。

# Implementation impact

- 未来C908 Provisioning FW只实现Controller typed协议到eHSM BL typed制造API的Adapter；GSP只实现运行期内部typed服务，均不实现raw slot/bitmap写API。
- 实现前必须取得Vendor定制BL/FW、状态/掉电语义和release note；当前未授权编码。

# Verification impact

覆盖制造/受限子Profile矩阵与无fallback、C908不可信caller、错误设备/Key类型/授权/顺序、部分写同材料恢复与错材料拒绝、每个不可逆点掉电、DEV重入、MANU收口、USER最后提交、USER后制造不可达、reset失败和日志敏感信息扫描。

# References

- [ADR-0021《SoC Key轮换采用Vendor定制机制》](../../decisions/ADR-0021-soc-key-rotation-vendor-mechanism.md)
- [ADR-0025《一机一密RTL Key与16槽OTP Key基线》](../../decisions/ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md)
- [ADR-0026《量产灌装、设备证明、证书A/B与制造接口合同》](../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)
- [ADR-0034《非安全制造灌装、eHSM BL密钥安装与USER最终提交》](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)
- [《密钥管理》架构专题](../03-architecture/key-management.md)
- [《OTP/eFuse接口》专题](../04-interfaces/otp-efuse.md)
- [CE-SEC-013《SoC Key轮换Vendor交付差距》代码证据](../../evidence/code-investigations/CE-SEC-013-soc-key-rotation-delivery-gap.md)

# Change history

- 2026-09-02：按ADR-0034固定DEV/MANU非安全制造子Profile、eHSM BL typed制造接口、部分写恢复、产品安全启动预演和MANU→USER最后提交。
- 2026-08-04：按SRC-0024同步Table 34和16-slot唯一基线，纠正slot3～5 Level、slot8/13/14语义及baremetal派生关系。
- 2026-07-29：接受ADR-0026；批准灌装顺序、单Profile、Cert0/1和制造typed协议。
- 2026-07-29：补充一机一密、16槽、signed recipe、typed制造命令、Device Identity/证书和完整量产顺序。
- 2026-07-27：合入SRC-0015 SoC Key轮换Provisioning流程和Vendor交付门禁。
