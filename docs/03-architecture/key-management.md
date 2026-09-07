---
title: "密钥管理"
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
last_reviewed: 2026-09-02
supersedes: []
superseded_by: []
---

# Purpose

维护Key/Provisioning/Rotation专题；规范性合同见[《NGU800P安全软件详细设计》第10章](../05-software-design/NGU800P安全软件详细设计.md#第10章-lifecycledebugrmakeyotpefuse证书与轮换)。

# Baseline

- [ADR-0025《一机一密RTL Key与16槽OTP Key基线》](../../decisions/ADR-0025-device-unique-rtl-key-and-16-slot-otp-baseline.md)冻结量产RTL Root/Install KEK按die唯一，只供硬件消费；Vendor共享宏不得作为量产值。
- [ADR-0026《量产灌装、设备证明、证书A/B与制造接口合同》](../../decisions/ADR-0026-provisioning-attestation-certificate-and-manufacturing-contract.md)冻结Chip Root→Level1→Level2顺序、Attestation单Profile、物理Key ID、Cert0/1和制造接口；SRC-0024冻结Table 34和16-slot表。
- [ADR-0034《非安全制造灌装、eHSM BL密钥安装与USER最终提交》](../../decisions/ADR-0034-nonsecure-manufacturing-provisioning-and-user-final-commit.md)冻结DEV/MANU制造子Profile、BL不信任C908的typed安装/查询/证明、部分写恢复、最终配置产品安全启动预演和USER最后提交。
- 外部证书签发CA私钥不进入设备；Chip/Device Root、UDS和Device Private Key不经普通产品接口导出。slot8 DICE root CA对象的具体材料形态由Key Attribute/证书Profile补齐。
- key对象按boot/upgrade、firmware encryption、attestation、debug authorization和session用途分域。
- Vendor `INSTALL_RANDOM_KEY/INSTALL_ENCRYPT_KEY`只用于批准的持久OTP key slot。
- 最终物理对象固定为16槽：slot0 Chip Root；slot1～5 Level1（Device/USER Root和eHSM Debug/FW Key）；slot6/7为SoC Verify/Encrypt轮换；slot8 DICE Root CA；slot9～11为SoC Debug/Verify/Encrypt主Key；slot12 SoC Debug轮换；slot13 UDS；slot14 Device Private；slot15 User Auth。准确表见主详设10.5。
- 当前只有slot14一个Device Private对象。软件保留P-256和SM2两套能力，每设备Profile只选择一种持久私钥；同机不同时使用两套长期身份。禁止同一scalar跨曲线复用，禁止RAM态`ehsm_km_gen_key`代替持久身份。
- slot8/13/14为Level2 asymm对象并具有SRC-0024列出的七项OTP/KMU权限。具体算法、材料形态和Key Attribute编码仍待Profile；产品接口不得因属性能力扩大为Host raw Key服务。
- ADR-0021冻结SoC Verify/Encrypt/Debug三类Key，每类只有原始/轮换两个逻辑位置且只允许一次切换；1字节Bitmap和物理slot由eHSM内部管理。
- SoC Key轮换按48字节双层密文、写新Key并证明、提交Bitmap、destroy旧Key、reset生效；Bitmap未知时quarantine，提交后不回旧Key。
- 证书轮换使用Flash Cert0/Cert1和commit-last，不复用OTP Key Bitmap；禁止增加可撕裂active pointer。
- USER轮换需要本operation一次性鉴权，提交后无论终态都消费并关闭；不得复用Debug授权或Vendor全局user-auth布尔值。test key或bypass存在时禁止进入USER。
- SoC Key轮换以云天定制需求为目标设计；当前通用Vendor接口不得模拟，真实实现状态为`BLOCKED_BY_VENDOR_DELIVERY`。
- 初始Key灌装也不得由当前通用安装接口或C908 raw slot调用模拟；Vendor BL必须验证硬件制造条件、LCS、ticket、device和固定对象顺序，并以`BLANK/PROGRAMMING_PARTIAL/PROGRAMMED_INVALID/PROGRAMMED_VALID/PROVED/LOCKED/UNKNOWN`返回权威状态。

# Open questions

- ADR-0020已关闭`OPEN-DESIGN-011`并批准单一provisioning/release matrix；实际设备/SKU行仍由Product/Provisioning/KMS/Release Owner补齐，缺失行默认拒绝。
- `OPEN-CONFLICT-012`：一机一密目标已批准，RTL隐藏个性化、ATE、不可读/lock/proof合同等待硬件绑定。
- `OPEN-CONFLICT-013`已由ADR-0026关闭。
- `OPEN-DESIGN-014`的软件设计已关闭；制造BL接口由`OPEN-DESIGN-025`跟踪，真实Vendor command/Bitmap掉电、RTL、Flash和KMS/CA/MES接口是实施绑定。
- 当前SRC-0018只有通用安装接口且拒绝USER/DEBUG安装，不能替代轮换接口；见CE-SEC-013。

# Verification impact

覆盖制造/受限Profile隔离、C908伪造调用、usage/LCS/slot/顺序错误、partial同材料恢复与错材料拒绝、私钥导出、轮换每个掉电点、USER前安全启动预演、USER最后提交、权限未关闭和test key扫描。

# Change history

- 2026-09-02：按ADR-0034增加eHSM BL typed制造、partial状态恢复和USER最后提交合同。
- 2026-08-04：按SRC-0024纠正16-slot顺序、slot3～5 Level、slot8/13/14权限和Table 34；baremetal只派生同一方案基线。
- 2026-07-29：接受ADR-0026；冻结Root顺序、每设备单Attestation Profile、物理Key ID 0～15、Cert0/1和typed制造接口。
- 2026-07-29：接受ADR-0025；同步一机一密、16槽、单Device Private槽冲突、UDS语义和Cert0/1 commit-last候选。
- 2026-07-27：接受ADR-0021；合入SRC-0015三类SoC Key、每类一次、HSM Bitmap、48字节封装、单向切换/reset和Vendor交付门禁。
- 2026-07-27：接受ADR-0020；关闭OPEN-DESIGN-011，保留OPEN-DESIGN-014等待物理Slot/Bitmap/Recipe裁决。
- 2026-07-24：同步主详设第10章。
- 2026-07-28：Key Rotation按云天定制需求冻结并标记Vendor交付阻塞；补充持久Attestation key和一次性operation授权合同。
