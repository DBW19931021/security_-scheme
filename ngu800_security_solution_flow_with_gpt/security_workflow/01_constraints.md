# NGU800 安全约束定义（01_constraints.md）

版本：v1.4
状态：Draft（当前阶段约束收敛版本；`SRC-008 当前收敛安全软件方案 2.0` 已登记为当前方案基线；`SRC-009 OSR eHSM 软件代码包 4019` 与 `SRC-010 eHSM4.0 ROM Patch 方案` 已登记为新增实现输入）
适用范围：NGU800 / NGU800P 安全子系统 + 启动链路

---

# 1. 设计目标

本文件定义 NGU800 安全架构的强约束条件，用于：

- 统一架构设计边界
- 约束详细设计输出
- 指导后续代码实现（Codex）
- 支撑安全评审 / 流片冻结

---

# 2. Source-of-Truth 约束

<a id="c-src-01"></a>

## 【C-SRC-01】当前方案源必须以 `芯片安全软件方案_2.0.pdf` 为准

来源：
- `SRC-008 当前收敛安全软件方案 2.0`
- `CR-0014-current-plan-v2-source-of-truth-sync`
- 用户 2026-06-03 明确指示：`current_plan` 中更新的 `芯片安全软件方案_2.0.pdf` 为最新收敛版本，无特殊说明时均以该版为准

要求：
- `[CONFIRMED]` 除 accepted CR、`00_project/decision_log.md`、官方 eHSM/TRM、后续用户特殊说明或 `SRC-008` 内明确例外外，当前安全软件方案口径以 `SRC-008 当前收敛安全软件方案 2.0` 为准。
- `[CONFIRMED]` `SRC-001 当前安全方案基线` 降级为历史流程参考；若其与 `SRC-008` 冲突，必须采用 `SRC-008` 或更高优先级来源。
- `[CONFIRMED]` 后续更新 constraints、baseline、详细设计、实现级设计、code rules、traceability 和导出版方案时，必须先检查 `inputs_manifest.md` 中的 source precedence。
- `[TBD]` `SRC-008` 中未给出 bit-level ABI、exact key ID、exact OTP/control bit、OOB/QSPI register 或工具 CLI 的内容，仍保持 open questions，不得因 2.0 PDF 登记而自动升级为字段级冻结。

Evidence：
- `SRC-008` 覆盖安全启动、FMC/GSP 命名、单 FMC 固定分区、OOB/QSPI 恢复、eHSM native package、密钥/证书/制造、生命周期/Debug、SPDM/attestation、密钥轮换和板级/OOB 边界。
- 旧 `SRC-001` 已被 `SRC-008` supersede，仍只保留历史流程表达价值。

Decision Rationale：
- 当前方案源必须唯一明确，避免旧 current_plan、CR 分片和方案导出版之间形成并列事实源。
- 源优先级升级不等于发明新的 physical ABI；eHSM TRM 和 accepted CR 仍保持更高优先级。

Chapter Binding：
- all chapters / ch1 / ch3 / ch4 / ch5 / ch7 / ch8 / ch9 / ch10 / ch11 / ch12

Impl Binding：
- inputs_manifest / 10_full_design / efuse_key_fw_header_design / mailbox_if / spdm_report / manufacturing_provisioning / 05_code_rules / 06_traceability

---

<a id="c-src-02"></a>

## 【C-SRC-02】eHSM 已提供安全服务必须以 OSR 软件代码包为实现事实源

来源：
- `SRC-009 OSR eHSM 软件代码包 4019`
- `CR-0018-osr-ehsm-software-and-rom-patch-source-sync`
- 用户 2026-06-27 明确指示：后续 HSM 代码已经提供的安全服务原则上以该代码为准，安全方案需要适配该代码

要求：
- `[CONFIRMED]` eHSM 已提供的安全服务、mailbox command ID、request/response 结构、Host API 行为、secure boot verify/upgrade、OTP/key/counter、debug/lifecycle、外部 OTP/Flash driver API、image tool / OTP tool 对接，必须以 `SRC-009 OSR eHSM 软件代码包 4019` 作为实现级事实源。
- `[CONFIRMED]` 后续更新 `10_full_design.md`、`04_impl_design/*.md`、code rules、traceability、测试计划和代码时，必须先检查 `security_inputs/sw/` 中 OSR BL/FW/Host/API/tool 的当前代码与随包文档。
- `[CONFIRMED]` 若当前方案需要的服务、字段、命令或工具行为在 OSR 代码中不存在或语义不同，必须登记差异，并通过 wrapper、policy 限制、eHSM customization 或方案调整 CR 处理，不得在 NGU 文档中直接发明并列 eHSM ABI。
- `[CONFIRMED]` OSR 代码事实源不改变 Root of Trust = eHSM、Host 不可信、BootROM 不做复杂密码学、SEC1/SEC2 正式路径 sign + encrypt、eHSM first verifier 等已冻结架构原则。
- `[TBD]` OSR 代码中的 exact command field、error code、OTP offset、key ID、control bit、patch OTP layout、tool CLI 和 golden vector 需要后续逐项 source-conformance，同步到 `10_full_design.md` 与实现级分片后才能作为字段级冻结依据。

Evidence：
- `security_inputs/sw/SW_changelist.md` 登记 BL、FW、Host API、外部 OTP/Flash driver API、镜像工具、OTP 工具和 Patch 测试说明。
- `security_inputs/sw/ehsm_host-2.3.1-4019-2ee044d/src/mb.h` 与 `bl_mb.h` 已生成 mailbox 命令 ID 与 req/rsp 结构。
- `security_inputs/sw/ehsm_fw-2.3.2-4019-5a4a0a9/src/secboot.c` 已体现 OTP 默认 key map、SOC/eHSM version counter 更新、secure boot 初始化和 eHSM ready/fail 状态。
- OSR FW service 源码包含 `soc_verify`、`fw_upgrade`、OTP key install、debug auth、misc/OTP/control field、counter 等服务实现路径。

Decision Rationale：
- 方案如果只跟随旧 TRM 抽象或历史详设，可能设计出 OSR 代码当前没有提供的服务、命令字段或工具行为，导致后续代码落地反复返工。
- 以 OSR 代码作为实现事实源，并保留 accepted CR / decision log / 架构原则作为上层约束，可以同时避免“代码脱离方案”和“方案发明 eHSM ABI”。

Chapter Binding：
- all chapters / ch3 / ch5 / ch6 / ch8 / ch9 / ch10 / ch11 / ch12

Impl Binding：
- 10_full_design / efuse_key_fw_header_design / mailbox_if / ehsm_source_conformance_matrix / manufacturing_provisioning / spdm_report / tools/image_packager / tools/provisioning / 05_code_rules / 06_traceability

---

<a id="c-src-03"></a>

## 【C-SRC-03】Vendor eHSM 代码与 Wing 工具链变更必须先提醒、说明并记录

来源：
- `CR-0019-vendor-code-toolchain-change-control`
- 用户 2026-07-02 明确指示：eHSM BootROM / Firmware 代码和编译工具链均由 vendor 提供，后续改动必须谨慎；改动前需明确提醒，说明原因、改动点和影响，并记录到相关文档与 OpenSpec

要求：
- `[CONFIRMED]` 后续修改 `ehsm_bootrom/**`、`ehsm_firmware/**` 下的源码、头文件、链接脚本、CMake/toolchain 文件、构建工具或配置前，必须先明确提醒用户该操作触及 vendor 代码。
- `[CONFIRMED]` 后续修改 Wing 工具链安装目录、权限、软链接、动态库、`PATH`、`LD_LIBRARY_PATH`、`WING_TOOL_HOME`、`WING_TOOL_BIN` 或工具链相关脚本前，必须先明确提醒用户该操作触及 vendor 工具链或本地构建环境。
- `[CONFIRMED]` 受控变更前必须说明变更原因、具体改动点、影响范围、回退方式和验证计划；受控变更后必须在相关文档中记录原因、改动点、影响分析、验证结果和遗留风险。
- `[CONFIRMED]` 默认优先通过外层脚本、环境变量、wrapper、构建参数或文档说明解决问题；必须修改 vendor 内容时，采用最小作用域改动并保留记录。
- `[CONFIRMED]` 不得把本地临时 workaround、工具链软链接或兼容库替换描述为 vendor 官方行为。

Evidence：
- `SRC-009 OSR eHSM 软件代码包 4019` 已作为 eHSM 已提供安全服务的实现事实源。
- FSP OpenSpec 已新增 vendor 代码与工具链变更控制规则。
- Wing 工具链运行时依赖、权限和环境变量会影响构建可复现性和问题归因。

Decision Rationale：
- Vendor 代码和工具链是后续适配与联调的事实基础，静默修改会破坏 source-conformance、版本归因和团队评审。
- 将提醒、原因、影响、回退和验证作为强制前置项，可以降低 vendor 代码分叉、工具链漂移和不可复现构建风险。

Chapter Binding：
- all chapters / implementation workflow / source-conformance / build and toolchain documentation

Impl Binding：
- FSP OpenSpec / 05_code_rules / 06_traceability / review checklist / build scripts / toolchain environment docs

---

# 3. Root of Trust 约束

<a id="c-root-01"></a>

## 【C-ROOT-01】Root of Trust 必须在 eHSM

- Root Key 必须存储在 eFuse / OTP 安全区中，由 eHSM 使用
- 不允许 BootROM 持有 Root Private Key
- 不允许管理核持有 Root Key

Evidence：
- eHSM 作为芯片信任根，提供安全启动、生命周期、密钥管理、身份认证、固件升级等服务
- eHSM 通过 OTP/eFuse 接口访问 OTP/eFuse

Decision Rationale：
- Root Secret 一旦扩散到 BootROM 或管理核，会扩大攻击面
- 项目当前基线要求 eHSM 作为安全服务根和首个密码学验证主体

Chapter Binding：
- ch4 / ch5 / ch6 / ch12

Impl Binding：
- efuse_design / key_hierarchy / fw_header / mailbox_if

---

# 4. Secure Boot 约束

<a id="c-boot-01"></a>

## 【C-BOOT-01】所有镜像必须经过安全子系统验签

适用对象：
- SEC 核
- PMP / RMP / OMP / MMP
- 大核相关 FW

要求：
- 所有固件必须在执行前完成验签
- 验签必须由安全子系统执行（C908 + eHSM）
- 未验签固件不得 release 执行

---

<a id="c-boot-02"></a>

## 【C-BOOT-02】Boot 顺序必须由安全核控制

- 所有 MCU reset release 必须由 SEC 核控制
- 不允许 Host 直接拉起 MCU
- 不允许管理核自行启动

---

<a id="c-boot-03"></a>

## 【C-BOOT-03】BootROM 不实现复杂加解密

- BootROM 仅允许：
  - 基础加载
  - 跳转 / 编排
  - 调用 eHSM / 安全子系统受控接口完成 SEC1 验证与解密
- 禁止：
  - 复杂签名验证
  - 复杂镜像解密
  - 密钥管理

---

<a id="c-boot-04"></a>

## 【C-BOOT-04】SEC1 image confidentiality

来源：
- `CR-0001-sec1-encryption-fw-protection-master-sync`

要求：
- `[CONFIRMED]` SEC1 固件在正式安全启动路径中必须采用签名 + 加密保护。
- SEC1 解密、key unwrap、hash/signature 校验必须通过 eHSM 或安全子系统受控密码服务完成。
- BootROM 不得直接实现复杂解密逻辑，只能定位 SEC1 镜像、调用受控接口、根据结果装载或拒绝启动。
- Host 不参与 SEC1 投递；SEC1 来源保持为 NOR Flash / 本地 Flash。
- `[CONFIRMED]` SEC2 在正式安全启动路径中必须采用签名 + 加密保护。
- `[ASSUMED]` PM、RAS、Codec 等后续关键 runtime image 在 USER/PROD 产品形态中默认采用签名 + 加密保护。
- `[TBD]` 非敏感 runtime image 是否允许 signature-only，必须由产品安全策略和 image_type 白名单冻结；signature-only 不能作为默认路径。

Decision Rationale：
- SEC1 是 First Mutable Stage，若仅验签不加密，会暴露早期安全 bring-up 逻辑和后续安全控制面的关键入口。
- SEC1 来源为本地 Flash，攻击面不同于 Host 下发镜像，需要在存储态具备机密性保护。
- 将 SEC1 解密收敛到 eHSM / 安全子系统密码服务，可保持 BootROM 最小化和 Root of Trust 边界清晰。

Chapter Binding：
- ch3 / ch6 / ch8 / ch11 / ch12

Impl Binding：
- efuse_key_fw_header_design / mailbox_if / manufacturing_provisioning / spdm_report

---

<a id="c-boot-05"></a>

## 【C-BOOT-05】Runtime image protection policy

来源：
- `CR-0003-runtime-image-policy-board-binding-attestation-mfg-freeze`

要求：
- `[CONFIRMED]` SEC2 verify path 必须包含 signature verify、rollback check、revoke check、decrypt / unwrap、measurement 和 controlled release。
- `[CONFIRMED]` SEC2 decrypt failure 必须阻断安全控制面启动。
- `[ASSUMED]` PM / RAS / Codec 等关键 runtime image 在 USER/PROD 默认 sign + encrypt。
- `[TBD]` signature-only 只能作为显式白名单例外，准入条件至少绑定 image_type、lifecycle、product SKU、debug state、release policy、rollback policy、是否包含敏感逻辑/数据。
- `[CONFIRMED]` 首版不采用长期静态 recovery image；FMC 防变砖依赖 OOB MCU 受控重刷 NOR Flash 中的 FMC 主区域。
- `[TBD]` 若后续产品策略重新引入静态 rescue/recovery image，其 image_type、signer、trust anchor、rollback counter、decrypt policy 必须单独立项冻结。

Decision Rationale：
- SEC2 是后续安全控制面，必须同时保护完整性和机密性。
- PM / RAS / Codec 可能承载关键 runtime 控制逻辑，默认策略应偏保守。
- FMC 是本地 NOR Flash 中唯一需要片上启动保护的一级可变固件；OOB MCU 受控重刷提供恢复能力后，不再要求 SoC Flash 内部保留 FMC 备份分区。
- signature-only 作为产品策略例外时必须可被 measurement / attestation 或安全状态表体现。

Chapter Binding：
- ch3 / ch6 / ch8 / ch9 / ch12

Impl Binding：
- efuse_key_fw_header_design / mailbox_if / spdm_report / manufacturing_provisioning

---

<a id="c-boot-06"></a>

## 【C-BOOT-06】eHSM native header 与 NGU manifest 分层

来源：
- `CR-0004-ehsm-native-header-otp-layout-alignment`
- `SRC-006 eHSM Firmware TRM`
- `SRC-007 eHSM Bootloader TRM`

要求：
- `[CONFIRMED]` SEC1 / SEC2 等安全启动镜像的密码学 verify/decrypt container 必须采用 eHSM native secure boot image header；NGU 不得再定义与其并列的 physical verification header。
- `[CONFIRMED]` eHSM native header 中的 `Image_Type` 保持 eHSM TRM 定义，不承载 NGU `SEC1 / SEC2 / PM / RAS / Codec / Recovery` 项目级 image type。
- `[CONFIRMED]` NGU 项目级 metadata 必须放入 eHSM Code region 的 protected manifest 或等价 manifest extension，由 BootROM / SEC 在 eHSM verify/decrypt 成功后解析。
- `[TBD]` `ngu_image_manifest_t` 的 bit-level ABI、是否必须位于 Code region 起始位置、eHSM firmware / bootloader 是否直接解析 manifest 仍需后续冻结。

Decision Rationale：
- eHSM TRM 已定义 1KB plaintext image head、字段 offset/size 和 Code region，继续保留 NGU physical header 会形成两套事实源。
- 将 NGU metadata 放入 protected manifest，可保留项目级策略表达，同时不破坏 eHSM native header 和工具链语义。

Chapter Binding：
- ch6 / ch11 / ch12

Impl Binding：
- efuse_key_fw_header_design / ehsm_source_conformance_matrix / mailbox_if

---

<a id="c-boot-07"></a>

## 【C-BOOT-07】SEC1 / SEC2 加密镜像不得使用 NVM only verify

来源：
- `CR-0004-ehsm-native-header-otp-layout-alignment`
- `SRC-006 eHSM Firmware TRM`
- `SRC-007 eHSM Bootloader TRM`

要求：
- `[CONFIRMED]` eHSM NVM deploy / only verify 模式不适用于 SEC1 / SEC2 sign+encrypt 路径。
- `[CONFIRMED]` SEC1 / SEC2 必须走 eHSM verify+decrypt output path，将解密结果输出到 BootROM / SEC 认可的受控 RAM / staging / output buffer。
- `[CONFIRMED]` output buffer 必须受 firewall / address whitelist / DMA 默认拒绝策略保护。
- `[ASSUMED]` SEC1 early boot 优先映射到 eHSM Bootloader `bl_verify_image` 或等价 ROM path；SEC2 runtime/load 优先映射到 eHSM Firmware `soc_verify` 或 SEC wrapper。
- `[TBD]` 具体 BootROM 可调用命令、共享内存位置、输出 buffer 地址范围和错误隐藏策略由 RTL / eHSM 集成冻结。

Decision Rationale：
- eHSM TRM 明确 RAM deploy 支持 verify+decrypt，而 NVM deploy 只做 signature verify 且镜像不能加密。
- CR-0001 / CR-0003 已确认 SEC1 / SEC2 sign+encrypt，不允许因部署模式误选导致机密性保护失效。

Chapter Binding：
- ch6 / ch11

Impl Binding：
- mailbox_if / efuse_key_fw_header_design / ehsm_source_conformance_matrix

---

<a id="c-boot-08"></a>

## 【C-BOOT-08】固件制作流程与设备侧 verify/decrypt 流程必须共享同一 eHSM-native 契约

来源：
- `CR-0006-firmware-package-build-verify-flow`
- `SRC-008 当前收敛安全软件方案 2.0` 第 4 章 / 第 4.5 节
- `SRC-001 当前安全方案基线` 历史流程参考
- `CR-0004-ehsm-native-header-otp-layout-alignment`
- `SRC-006 eHSM Firmware TRM`
- `SRC-007 eHSM Bootloader TRM`

要求：
- `[CONFIRMED]` 平台侧固件制作工具和设备侧 BootROM/SEC/eHSM verify-decrypt 路径必须使用同一套 eHSM native secure boot image header + NGU protected manifest 契约。
- `[CONFIRMED]` 平台侧工具不得再生成与 eHSM native header 并列的 NGU physical verification header；旧 `SRC-001` 中的 `header + Signed Region + signature + wrapped_cek + enc_payload` 仅保留为历史流程意图参考，当前包格式以 `SRC-008` 的 eHSM native package 口径为准。
- `[CONFIRMED]` 固件制作流程必须明确 payload、NGU protected manifest、eHSM native header、Code region、版本计数、算法 profile、签名/加密 profile 和发布验收检查之间的关系。
- `[CONFIRMED]` 设备侧 verify/decrypt 流程必须先由 eHSM 完成 native header 检查、签名校验、rollback / version counter 检查、decrypt output，再由 BootROM / SEC 解析 NGU manifest 并执行项目级 release policy。
- `[TBD]` `ngu_image_manifest_t` bit-level ABI、eHSM 是否解析 manifest、exact key ID、per-image CEK / wrapped CEK 和工具 CLI / golden vector 仍需 owner 后续冻结。

Decision Rationale：
- 只描述“需要验签和解密”不足以指导代码落地，工具侧产物和设备侧解析/验证流程必须共享同一契约。
- CR-0004 已废弃自定义 physical FW header；CR-0006 只补齐制作/验证流程，不恢复旧 physical ABI。
- 用图形和步骤固化制作链路，可减少 image packager、BootROM、SEC verify flow、eHSM adapter 之间的解释偏差。

Chapter Binding：
- ch3 / ch6 / ch10 / ch11 / ch12

Impl Binding：
- efuse_key_fw_header_design / mailbox_if / ehsm_source_conformance_matrix / tools/image_packager / 05_code_rules / 06_traceability

---

<a id="c-ehsm-01"></a>

## 【C-EHSM-01】OTP / key / counter source-conformance gate

来源：
- `CR-0004-ehsm-native-header-otp-layout-alignment`
- `SRC-006 eHSM Firmware TRM`
- `SRC-007 eHSM Bootloader TRM`

要求：
- `[CONFIRMED]` eHSM TRM 已定义的 OTP/control field、Version Counter、OTP key ID / level / purpose 是实现级 physical field 的优先事实源。
- `[CONFIRMED]` `OTP-0..OTP-7` 仅可作为 NGU logical view / documentation alias，不表达 physical OTP/eFuse offset。
- `[CONFIRMED]` `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 仅可作为 NGU logical rollback domain，不得写成 eHSM physical OTP 32-bit counter。
- `[CONFIRMED]` NGU key name 仅是 logical alias；必须映射到 eHSM key ID / level / purpose 或标记为 `eHSM-customization-TBD`。
- `[TBD]` exact key ID mapping、exact OTP/control bit mapping、per-image rollback counter、per-image CEK / wrapped CEK 均未冻结。

Decision Rationale：
- 当前实现级文档的“建议字段”容易被误读为 physical ABI；CR-0004 要求所有 header / OTP / key / counter 字段建立 source-conformance matrix。

Chapter Binding：
- ch5 / ch6 / ch9 / ch11

Impl Binding：
- efuse_key_fw_header_design / ehsm_source_conformance_matrix / manufacturing_provisioning / 05_code_rules / 06_traceability

---

<a id="c-ehsm-02"></a>

## 【C-EHSM-02】OSR eHSM 代码必须进入 eHSM source-conformance gate

来源：
- `SRC-009 OSR eHSM 软件代码包 4019`
- `CR-0018-osr-ehsm-software-and-rom-patch-source-sync`

要求：
- `[CONFIRMED]` eHSM 适配层、BootROM/SEC verify flow、mailbox driver、image packager、provisioning tool、debug/lifecycle tool 和测试向量不得只按 NGU 自定义抽象开发，必须逐项对齐 `SRC-009` 中 OSR BL/FW/Host API 的真实接口。
- `[CONFIRMED]` `bl_verify_image`、`bl_fw_upgrade`、`soc_verify`、`fw_upgrade`、OTP read/write、debug auth、close debug、counter、key install / import / derive / exchange 等服务的存在性、命令 ID、req/rsp 字段和错误模型，应以 OSR 代码和随包 TRM 4019 为实现级对照来源。
- `[CONFIRMED]` OSR 代码已经提供的 OTP 默认 key map、SOC/eHSM version counter 更新、eHSM ready/fail 状态、fault-injection delay / check 等实现行为，需要纳入后续 `ehsm_source_conformance_matrix.md`、manufacturing、secure boot 和测试设计。
- `[TBD]` OSR 代码与当前 `10_full_design.md` / `04_impl_design` 的差异清单尚未完成；在完成差异清单前，不得声称 full design 已经完全适配 OSR 代码。

Decision Rationale：
- CR-0004 建立了 eHSM TRM source-conformance；CR-0018 将 source-conformance 从“文档字段”扩展到“OSR 代码实际提供的服务和 ABI”。
- 代码事实源进入 gate 后，后续开发可以直接从 OSR API / mailbox / tool 行为出发，减少文档和实现偏差。

Chapter Binding：
- ch3 / ch5 / ch6 / ch7 / ch9 / ch11 / ch12

Impl Binding：
- ehsm_source_conformance_matrix / mailbox_if / efuse_key_fw_header_design / manufacturing_provisioning / 10_full_design / 05_code_rules / 06_traceability

---

<a id="c-ehsm-03"></a>

## 【C-EHSM-03】eHSM4.0 ROM Patch 必须作为 OTP + 硬件 BOOT 机制纳入安全约束

来源：
- `SRC-010 eHSM4.0 ROM Patch 方案`
- `CR-0018-osr-ehsm-software-and-rom-patch-source-sync`

要求：
- `[CONFIRMED]` eHSM ROM patch 是硬件 BOOT 从 OTP patch 配置表加载 patch 信息，并在 CPU 访问 IROM 时由 patch 模块对命中地址返回替换指令的机制；它不是 Host、SEC 或普通软件在运行期任意修改 ROM 的热补丁入口。
- `[CONFIRMED]` Patch 配置表固化在 OTP 中，在 eHSM 初始化时由硬件加载到 eHSM 内部；CPU 对 patch hit 无感，不应产生异常或软件可见的替换流程。
- `[CONFIRMED]` Patch 地址按 word 对齐；当前输入资料给出 32 行 patch、每行 1 个 word 数据替换的设计方向。
- `[CONFIRMED]` Patch data 在 OTP 中按明文保存；这属于 patch 模块与 IROM/cipher 位置关系的硬件设计输入，不得被 Host 运行期覆盖。
- `[CONFIRMED]` `SRC-010` 结论包括：不需要 APB CFG IF；patch hit 时屏蔽 IROM 访问；增加 patch enable 用于整体使能。
- `[TBD]` NGU 项目的最终 Patch_en / Patch_addr / Patch_data OTP offset、patch enable 编码、patch 表烧录权限、MANU/USER 锁定策略、patch 验收脚本、attestation/audit 是否报告 patch 状态，必须由 RTL/eHSM/security owner 后续冻结。

Evidence：
- `SRC-010 eHSM4.0 ROM Patch 方案` 描述 patch 流程：eHSM 全局复位释放后，Boot 模块读取 Patch 信息到 ipatch 模块；hardware boot OK 后释放 CPU 复位；CPU 访问 IROM 时 patch hit 则返回替换值，否则正常访问 IROM。
- `SRC-010` 给出 OTP 增量字段：Patch_en、Patch_addr、Patch_data，并说明相对标准版增加约 0.2KB OTP 空间。
- `SRC-010` 给出 patch 配置表字段方向：2-bit Patch_en、16-bit word address、32-bit Patch_data。

Decision Rationale：
- ROM patch 能改变 eHSM ROM 实际执行指令，必须进入 OTP、制造、生命周期、审计和 attestation 影响分析。
- 若 patch 机制被误解为运行期可写入口，会形成绕过 ROM 固化和安全启动边界的高风险通道。

Chapter Binding：
- ch3 / ch6 / ch7 / ch9 / ch11 / ch12

Impl Binding：
- efuse_key_fw_header_design / manufacturing_provisioning / ehsm_source_conformance_matrix / mailbox_if / spdm_report / 05_code_rules / 06_traceability

---

# 5. Crypto 约束

<a id="c-if-01"></a>

## 【C-IF-01】所有密码操作必须走 eHSM

必须：
- HASH
- 签名验证
- 加解密
- Key 派生

禁止：
- 软件实现 SM2/SM3/SM4（或其他正式安全路径算法）
- 普通核直接调用 crypto 引擎

---

# 6. Key Management 约束

<a id="c-key-01"></a>

## 【C-KEY-01】私钥不可导出

- Private Key 不允许：
  - 被 Host 读取
  - 被普通核访问
- Key 使用必须通过 eHSM 内部机制

---

<a id="c-key-02"></a>

## 【C-KEY-02】Key 必须绑定生命周期

- Key 使用权限与 lifecycle 强绑定
- 不同生命周期：
  - TEST / DEVE / MANU / USER / DEBUG / DEST
  - Key 权限必须不同

---

# 7. Debug 约束

<a id="c-debug-01"></a>

## 【C-DEBUG-01】USER 态关闭调试能力

USER 生命周期：
- 禁止 JTAG
- 禁止内部总线访问
- 禁止 debug boot

---

<a id="c-debug-02"></a>

## 【C-DEBUG-02】DEBUG / RMA 必须认证

- Debug 开启必须：
  - 鉴权
  - 生命周期允许
  - 可审计

---

# 8. Host 约束

<a id="c-host-01"></a>

## 【C-HOST-01】Host 不可信

Host 只能：
- 传输 SEC2 及后续 firmware / 受保护镜像包
- 发起请求（mailbox / PCIe）

Host 不允许：
- 参与签名
- 参与 Root of Trust
- 访问密钥
- 直接 release 执行
- 下发或替换 SEC1

---

# 9. 访问控制约束

<a id="c-access-01"></a>

## 【C-ACCESS-01】安全子系统必须隔离

禁止直接访问：
- eHSM
- Secure SRAM
- OTP

访问方式：
- mailbox
- interrupt
- 受控共享内存

---

<a id="c-access-02"></a>

## 【C-ACCESS-02】必须使用 UserID + Firewall

- 所有 master 必须带 UserID
- 所有访问必须经过 firewall
- 权限必须可配置、可隔离

---

# 10. Board / Management 约束

<a id="c-board-01"></a>

## 【C-BOARD-01】管理子系统总体架构和流程可遵循，但安全边界必须由安全方案裁决

来源：
- `SRC-005 管理子系统方案`

要求：
- 管理子系统文档中的总体架构、模块职责、带外管理链路、电源/复位流程、单/双 Die 约束原则上作为系统级流程输入。
- 涉及 Root、debug、JTAG、secure boot、lifecycle、provisioning、firmware update、secure memory、OTP/eFuse、security subsystem 访问的内容，必须以安全基线为准。
- 若管理子系统文档中存在未鉴权调试、越权访问、绕过 SEC/eHSM、绕过 lifecycle gating 或直接访问安全资产的设计，不能直接继承，必须在详设中列为风险并给出替代设计。

Evidence：
- `SRC-005 管理子系统方案` 描述了 BMC、OAM 模组、板级 MCU/GPU 之间的 SMBus/I2C、I3C、SPI、PCIe、JTAG、UART、电源/复位管理和管理子系统整体逻辑。
- `SRC-005 管理子系统方案` 对 JTAG 的描述包括可接入 GPU 芯片 JTAGBUS、寄存器空间、DRAM、Flash、安全子系统和 CPU 调试单元。

Decision Rationale：
- 管理子系统属于系统流程和板级集成的重要输入，但其带外通道和调试能力具备高权限，不能天然视为安全可信通道。
- JTAG、DMA、Flash 更新、电源复位和 OOB 链路若不受 lifecycle/debug auth/firewall 约束，会绕过现有安全启动和密钥边界。

Chapter Binding：
- ch4 / ch6 / ch10 / ch11 / ch13 / ch14

Impl Binding：
- mailbox_if / spdm_report / manufacturing_provisioning / firewall_access_rules

---

<a id="c-board-02"></a>

## 【C-BOARD-02】带外管理通道不得成为安全策略绕过路径

来源：
- `SRC-005 管理子系统方案`

要求：
- SMBus/I2C、I3C、PCIe VDM、SPI、UART、BMC/OOB/板级 MCU 链路只能作为受控管理或转发通道。
- 带外管理通道不得直接修改 lifecycle、secure boot、debug enable、Root/anchor、rollback counter、provisioning 状态。
- 带外管理通道若承载 firmware update、状态查询、power/reset、debug request 或 provisioning proxy，必须经 SEC/C908 收敛，并受地址白名单、命令白名单、lifecycle gating 和审计约束。

Evidence：
- `SRC-005 管理子系统方案` 明确带外管理通道支持 SMBus/I2C、I3C、JTAG，且存在 BMC、OAM 模组、模组 MCU、GPU 和板级 MCU/GPU 之间的多条链路。

Decision Rationale：
- OOB 链路物理上独立、权限高、部署复杂，若作为安全服务直接入口，会破坏 Host 不可信和 SEC 统一控制面的基线。

Chapter Binding：
- ch10 / ch11 / ch12 / ch14

Impl Binding：
- mailbox_if / firewall_access_rules / audit_log

---

<a id="c-board-03"></a>

## 【C-BOARD-03】JTAG 必须受 lifecycle、debug auth、scope bitmap 和板级 MUX 联合控制

来源：
- `SRC-005 管理子系统方案`

要求：
- USER/PROD 生命周期默认关闭 JTAG 和等价调试访问。
- 任何 JTAG 接入 GPU、CPU、DRAM、Flash、安全子系统或板级 MCU 的能力，必须先通过 challenge-response / debug auth。
- JTAG MUX / CPLD / 板级控制单元不得提供绕过 eHSM debug authorization 的直通路径。
- 授权结果必须包含 scope、目标、时限和审计记录。
- `[ASSUMED]` 板级 MUX 控制权应由 SEC/eHSM 授权信号约束，OOB/BMC 不得单独打开。
- `[TBD]` JTAG scope bitmap bit-level mapping、eHSM debug bitmap 与板级 MUX 寄存器归属需在实现阶段冻结。

Evidence：
- `SRC-005 管理子系统方案` 描述 JTAG 可接入 BMC、UBB、OAM、板级 MCU/GPU，并可访问 GPU 芯片 JTAGBUS、所有寄存器空间和 DRAM，也可接入安全子系统、CPU 调试单元、GPU Flash 和板级 MCU。

Decision Rationale：
- JTAG 是最高风险板级入口之一，若在量产态未被强制关断或受控授权，会直接绕过 secure boot、内存隔离、密钥和生命周期保护。

Chapter Binding：
- ch9 / ch10 / ch11 / ch14

Impl Binding：
- lifecycle_control / debug_auth / firewall_access_rules / audit_log

---

<a id="c-board-04"></a>

## 【C-BOARD-04】管理子系统 DMA、mailbox、中断、互斥访问和复位控制必须被隔离和审计

来源：
- `SRC-005 管理子系统方案`

要求：
- `[CONFIRMED]` 管理子系统 DMA、Host DMA、OOB DMA 对安全资源默认拒绝。
- 管理子系统 DMA 只能访问被 firewall 白名单允许的普通 staging / data buffer，不得访问 eHSM、OTP/eFuse、Secure SRAM、SEC1/SEC2 执行区、证书/策略区、measurement_table 安全写区域、debug/lifecycle/rollback 控制寄存器和 recovery 区。
- 管理子系统 mailbox、中断、互斥寄存器只能用于普通协作或经 SEC 收敛后的安全服务请求，不得作为直接安全服务入口。
- 电源、上下电、复位、PowerBrake 等板级控制信号若影响安全启动或故障恢复，必须进入安全状态机和审计模型。
- `[TBD]` 具体 UserID、firewall region、地址范围、错误隐藏策略、审计字段由 RTL/实现设计冻结。

Evidence：
- `SRC-005 管理子系统方案` 描述 CPU 子系统通用 DMA、mailbox 中断、互斥访问机制、电源管理接口、上下电和复位管理。

Decision Rationale：
- DMA、复位和中断可改变系统状态或数据路径，若缺少隔离与审计，会破坏安全启动、证明状态和运行态可信边界。

Chapter Binding：
- ch6 / ch10 / ch11 / ch12 / ch13

Impl Binding：
- mailbox_if / firewall_access_rules / spdm_report / audit_log

---

# 11. Firmware 更新约束

<a id="c-update-01"></a>

## 【C-UPDATE-01】必须支持防回滚

- 固件版本必须受控
- 必须具备 anti-rollback 机制
- 反回滚计数必须落到 eHSM Version Counter / monotonic counter / owner 确认的等价机制，而不是仅软件字段
- `SEC1_MIN_VER / SEC2_MIN_VER / *_MIN_VER` 只能表达 NGU logical rollback domain，不能作为 eHSM physical OTP 32-bit counter

---

<a id="c-update-02"></a>

## 【C-UPDATE-02】必须支持受控升级 / 恢复

来源：
- `SRC-008 当前收敛安全软件方案 2.0`
- `CR-0013-oob-mcu-secure-boundary-single-fmc-recovery`

- 必须定义升级路径
- 必须定义失败恢复策略
- `[CONFIRMED]` 首版 FMC 恢复采用单 FMC 主区域 + OOB MCU 受控重刷，不采用 SoC Flash 内部 FMC 备份分区。
- OOB MCU 写入 NOR Flash 成功不等于固件可信；BootROM/eHSM 下次启动必须重新执行 verify/decrypt/rollback/revoke/manifest policy。
- 恢复机制必须冻结 OOB MCU secure boot、QSPI ownership/arbiter、NOR 写保护、恢复授权 capsule、状态/审计记录和掉电保护。

---

# 12. Attestation 约束

<a id="c-att-01"></a>

## 【C-ATT-01】必须支持设备认证

- 支持设备身份
- 支持测量值输出
- 支持远程证明（SPDM）
- 私钥不得离开 eHSM

---

<a id="c-att-02"></a>

## 【C-ATT-02】Attestation report 必须覆盖安全状态和策略状态

来源：
- `CR-0003-runtime-image-policy-board-binding-attestation-mfg-freeze`

要求：
- `[CONFIRMED]` Attestation report 必须包含 measurement、lifecycle、debug state、secure boot state、rollback state。
- `[ASSUMED]` Attestation report 增加 image protection policy、decrypt_applied、image_type policy 字段。
- `[ASSUMED]` board binding 默认进入 attestation；若 board binding 参与证明或验证，board_bind_result 必须进入 report。
- `[TBD]` PowerBrake / PG / FAULT / reset event 是否进入主 report，还是进入扩展 event log。

Decision Rationale：
- report 不得只返回签名，必须返回签名覆盖的状态。
- RMA/debug state 必须在 report 中可见，防止把 RMA/debug 态伪装成 USER/PROD 态。

Chapter Binding：
- ch8 / ch10 / ch11 / ch12

Impl Binding：
- spdm_report / mailbox_if / manufacturing_provisioning

---

# 13. Manufacturing / Provisioning 约束

<a id="c-mfg-01"></a>

## 【C-MFG-01】必须定义 Root Key 灌装与锁定流程

- 必须定义制造 / 灌装 / 锁定 / 审计流程
- 必须定义 MANU → USER 的冻结动作
- 不得只写“后续补充”
- `[CONFIRMED]` USER freeze 必须锁定 secure boot、debug、anti-rollback、SEC1/SEC2 decrypt key / FW_KEK、test trust cleanup。
- `[CONFIRMED]` RMA 不允许 long-open debug，不允许绕过 challenge/auth，不允许长期保留 SEC1/SEC2 decrypt bypass。
- `[TBD]` Root injection mode、OTP/eFuse readback 验收方式、RMA re-acceptance 流程。

---

# 14. 待补充（后续自动收敛）

- PCIe 安全模型细化
- SPDM report 字段级定义
- mailbox command ID 最终分配
- board binding 是否参与 SEC2/runtime release decision
- JTAG scope bitmap、CPLD/MUX 控制权和板级调试授权闭环
- 管理子系统 DMA / mailbox / 复位控制的 firewall 和审计字段
- OOB MCU FMC 重刷恢复 ABI：OOB secure boot、QSPI ownership、NOR 写保护、恢复授权 capsule、状态/审计记录、掉电保护
- 若未来重新引入静态 rescue/recovery image，则单独冻结 image_type、signer、trust anchor、rollback counter、decrypt policy
- OSR eHSM 软件代码与 `10_full_design.md` / `04_impl_design` 的逐项差异清单：mailbox command、req/rsp 字段、错误码、OTP/key/counter、debug/lifecycle、tool CLI、golden vector
- eHSM4.0 ROM Patch 字段级集成：Patch_en / Patch_addr / Patch_data OTP offset、烧录权限、USER 锁定、验收脚本、证明/审计表达
