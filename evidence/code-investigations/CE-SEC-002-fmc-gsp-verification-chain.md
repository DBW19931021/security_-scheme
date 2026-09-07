# CE-SEC-002：FMC/GSP 验签、加载与跳转代码证据

## 调查身份

- 对应任务：`INV-SEC-002-fmc-gsp-verification-chain`。
- 公司仓库：`REPO-GSP-FIRMWARE`，`../gsp-pmp-rmp-omp`。
- 清单参考点：`master` / `08b29c7b7a29ee9478c0c01d4d708beb0b77b5d9`。
- 调查日期：2026-07-21。
- 调查范围：FMC/GSP solution 与链接脚本；image/header/manifest/policy/verify/measurement；image packager；Host/QEMU 测试入口。
- 执行约束：只读；未执行 Git、构建或测试；未修改 `gsp-pmp-rmp-omp`、`baremetal` 或 Vendor 源码。
- 基线限制：公司仓库盘点时已有未提交修改。本报告说明调查时工作区内容，不能在禁止 Git 操作的条件下区分其与清单 commit 的差异。

## 结论摘要

| 编号 | 分类 | 结论 | 设计影响 |
|---|---|---|---|
| FACT-01 | `CODE_FACT` | FMC 和 GSP 构建都包含 `security` 组件，但 FMC main 是裸机 hello-world，GSP main 是 FreeRTOS hello-world/可选 QEMU 测试。 | 当前没有 FMC→GSP→Runtime 生产验证或 release 链。 |
| FACT-02 | `CODE_FACT` | 唯一可见 verify 编排为 `verify_image -> ehsm_verify_decrypt_stub -> manifest_parse -> policy_check -> measurement_record`，且没有 FMC/GSP production caller。 | 验签、解密、回滚和加载均未接真实硬件。 |
| FACT-03 | `CODE_FACT` | Header/manifest parser 有固定小端布局和基本长度检查，但不完成签名、解密、counter、load/entry allowlist 或 jump。 | 解析骨架可复用，安全语义仍需补齐。 |
| FACT-04 | `CODE_FACT` | `pack_image.py` 只拼接零填充 header、manifest 和原始 payload，却标记为 ciphertext/sign+decrypt required；没有签名或加密。 | 该工具只能作为格式原型，不能用于发布包。 |
| FACT-05 | `CODE_FACT`/`CONFLICTING`（调查时）/`CONFIRMED`（后续裁决） | packager/tests的GSP默认地址为`0x1000_0808_0000`，GSP linker/README为`0x1010_0808_0000`；地址映射头显示两者是不同master view。 | ADR-0004采用NoC/system canonical `0x1010_0808_0000`，不再阻断规范选择。 |
| FACT-06 | `CODE_FACT` | rollback 仅有字段/状态位；stub 无条件标记 checked，未读取、比较或更新物理 counter。 | 防回滚当前是模拟状态，不可作为发布门禁。 |
| FACT-07 | `CODE_FACT` | measurement 是本镜像 BSS 中的 16-slot 全局数组；verify 只记录成功，digest 未由 stub 产生，失败/slot 错误状态不一致。 | 跨 stage 持久化、真实 digest、失败记录和并发契约缺失。 |
| FACT-08 | `CODE_FACT` | Host tests 显式编译 stub；QEMU/SPDM tests 注入 synthetic measurement。 | 现有测试覆盖结构/骨架，不证明 production eHSM、loader 或 release。 |

## 已确认代码事实

### FACT-01：FMC/GSP 目标链接安全组件但未调用启动验证

- `solutions/fmc/Makefile:13,39-49,67-70,97-100`：目标为 `fmc`，链接 `security`，使用 `chip_riscv_fmc/gcc_flash_sram.ld`。
- `solutions/fmc/sub.mk:19-24`：solution 只编译 `app/src/main.c`。
- `solutions/fmc/app/src/main.c:41-57`：仅 `board_init()` 和 hello-world 打印，最终返回；没有 package、verify、measurement、GSP release 或 jump。
- `components/chip_riscv_fmc/gcc_flash_sram.ld:19-33`：FMC 运行区为 `0x1000_080C_0000`，长度 `0x40000`，入口 `Reset_Handler`。
- `solutions/gsp/Makefile:13,39-49,67-72,99-102`：目标为 `gsp`，链接 FreeRTOS、security 和 pldm。
- `solutions/gsp/sub.mk:19-24`：编译 app 下全部 C 文件。
- `solutions/gsp/app/src/pre_main.c:27-51`：创建 application task 并启动 scheduler。
- `solutions/gsp/app/src/main.c:39-67`：只在显式 QEMU test 宏下运行测试；默认进入周期性 hello-world 循环，没有验证 Runtime image、release 下游或启动 production SPDM service。
- `components/chip_riscv_gsp/gcc_flash_sram.ld:19-33`：GSP 链接到 `0x1010_0808_0000`，长度 `0x40000`。

对 production 源码的调用搜索只定位到 `verify_image()` 自身定义，没有 FMC/GSP 调用点。

### FACT-02：当前 verify path 只通向 stub

- `components/security/sub.mk:34-40`：FMC/GSP 都编译 `ehsm_stub.c` 和 `verify_flow.c`。
- 同文件 `:49-65`：GSP 还编译 attestation/crypto/cert stub 和 SPDM/MCTP 框架。
- `components/security/src/verify_flow.c:6-28`：入口直接调用 `ehsm_verify_decrypt_stub()`，失败即返回。
- 同文件 `:36-51`：stub 成功后解析 manifest 并执行 policy。
- 同文件 `:53-84`：构造 measurement 并写入全局 store。
- `components/security/src/ehsm_stub.c:37-68`：解析少数字段，依据可控 fail flag 返回；成功动作是 `memcpy()`，随后无条件设置 sign/decrypt/rollback checked。

当前唯一完整数据流来自 BootROM demo 或 Host test，不来自 FMC/GSP production：

```text
synthetic package / host test
  -> verify_image
    -> ehsm_verify_decrypt_stub
      -> ehsm_parse_header
      -> memcpy(output)
    -> manifest_parse_after_ehsm_pass
    -> policy_check
    -> measurement_record
  -> return

FMC main / GSP main  -X-> 上述路径
上述路径             -X-> loader / release / jump
```

### FACT-03：parser 只实现基础格式门禁

- `components/security/include/security/ehsm_image.h:10-46`：定义 1024-byte header、稀疏字段 offset 和固定 valid flag。
- `components/security/src/ehsm_image.c:3-39`：按小端读取 valid flag/code size，只检查最小 header、valid flag、非零 code size 和 package 边界；不解析/验证 signature/public key/IV/version-counter 内容。
- `components/security/include/security/manifest.h:10-54`：manifest 基础长度 72，load/entry 为 64 位，扩展/digest offset 位于 72 之后。
- `components/security/src/manifest.c:4-24,26-77`：按小端解析，检查 magic/version、header/payload/extension 边界和 digest size 上限。
- `components/security/src/policy.c:3-47`：检查已知 image type、必须 sign、指定镜像必须 decrypt，以及 signature-only/decrypt 组合矛盾。

尚未检查：header reserved/flag 合法集合、unknown policy bit、rollback-required、image type 与调用 stage、load/entry allowlist/对齐/重叠、measurement slot 按镜像唯一性、algorithm profile、lifecycle、digest 与 plaintext 的绑定。

### FACT-04：当前制包工具不是签名/加密工具

- `components/security/tools/image_packager/pack_image.py:67-74`：创建零填充 1024-byte header，只写 valid/image/plain/naked/code-size。
- 同文件 `:77-104`：创建 72-byte manifest，设置 sign/decrypt required、地址、version、slot 等字段。
- 同文件 `:133-144`：直接读取 payload，并输出 `header + manifest + payload`；没有 key、hash、signature 或 encryption 操作。
- header 中 signature/public-key/IV 区保持全零，而 plain flag 被设置为 ciphertext。

因此 `pack_image.py` 是格式/测试 corpus 工具，不是可发布的 secure image builder。

### FACT-05：GSP load/entry 的地址域未定义

- `pack_image.py:52-64`：GSP/SEC2 默认 load/entry 为 `0x1000_0808_0000`。
- `tools/image_packager/tests/test_packager.py:57-63`：测试把该值固化为 expected。
- `components/security/tests/test_manifest_gate.c:19-51`、`test_verify_flow.c:20-45,74-87` 和 `test_policy.c:15-31` 也使用相同值。
- `components/chip_riscv_gsp/gcc_flash_sram.ld:19-33` 和仓库 `README.md:210-222`：GSP 运行/链接地址为 `0x1010_0808_0000`。
- `config_bus_address_mapping.h:607-616`：`0x1000_0808_0000` 的 security SRAM 映射到 `0x1010_0808_0000`。
- `subsys_address_mapping.h:812-822`：`0x1010_0808_0000` 是外部可见地址，并列出 die0 remap `0x1000_0808_0000`。

这可能是同一 SRAM 的不同 master address view，但 manifest 的 `load_addr`/`entry_addr` 应使用哪一视图、谁负责转换、eHSM DMA 和最终 PC 使用哪一视图均未定义。已登记 [OPEN-CONFLICT-003](../../sources/conflict-reports/CONFLICT-CODE-GSP-ADDRESS-DOMAIN.md)。

### FACT-06：anti-rollback 只有模拟结果

- `manifest.c:49,53` 读取 logical version 和 rollback domain。
- `policy.c` 不检查 `POLICY_ROLLBACK_REQUIRED`。
- `ehsm_stub.c:29,68` 初始化 rollback status 为 OK 并无条件设置 `MEASUREMENT_ROLLBACK_CHECKED`；没有 counter input。
- `verify_flow.c:68-70` 只把 version/domain 复制到 measurement。
- production 范围内未定位到物理 counter read/compare/update、耗尽、掉电恢复或 authorization 逻辑。

### FACT-07：measurement 是进程内成功记录骨架

- `components/security/src/measurement.c:4-32`：16-slot 静态全局数组；reset 清零；record 按 slot 覆盖并分配 sequence；没有锁、持久化或跨镜像 handoff。
- `verify_flow.c:19-21,53-84`：result 先清零；只有 eHSM、manifest 和 policy 全部成功后才构造 measurement；digest metadata 来自 manifest，digest bytes 没有来自 stub/eHSM 的赋值。
- 若 `measurement_record()` 因 slot 越界失败，`verify_image()` 返回该错误，但 `result.verify_status` 与 `measurement.error_code` 已保持 OK。
- `spdm_measurement_provider.c:4-24,28-46` 接受 `digest_size == 0` 的 valid record 并导出。
- `attest_report.c:59-86` 默认把 secure boot/anti-rollback 初始化为 enabled，再依据记录中的模拟 policy bit 下降状态。

因此当前 store 可用于结构和 provider 测试，但不能证明真实 measurement、失败可审计或跨 BootROM/FMC/GSP 的可信传递。

### FACT-08：测试覆盖边界

- `components/security/tests/host/run_core_host_tests.sh:26-59` 编译 header/manifest/policy/measurement/verify/demo 测试；verify/demo 明确链接 `ehsm_stub.c`。
- `test_ehsm_header.c` 覆盖字段 offset、正常 header 和短包。
- `test_manifest_gate.c` 覆盖 eHSM-pass gate 和 64 位地址读取，但未系统覆盖 malformed/overflow/extension/allowlist。
- `test_policy.c` 只覆盖 alias、GSP decrypt required 和 PM 默认策略。
- `test_measurement.c` 覆盖内存 round-trip 与非法 slot。
- `test_verify_flow.c` 覆盖 synthetic GSP 成功流，不覆盖真实密码、counter、loader/jump 和失败矩阵。
- QEMU SPDM tests 使用代码生成的 synthetic FMC/GSP measurements；这验证 provider/protocol glue，不验证启动产生的 measurement。
- 本次未执行任何上述测试，历史 docs/evidence 也未被当作当前工作区运行证明。

## 构建和平台影响

- FMC：bare-metal、`security` 组件、运行区 `0x1000_080C_0000/0x40000`。
- GSP：FreeRTOS、`security`/PLDM、运行区 `0x1010_0808_0000/0x40000`。
- `security` 基础源列表会把 `ehsm_stub.c` 同时编入 FMC/GSP 组件；GSP 还含证书/密码/签名 stub。
- Makefile/after-build 只生成 `.bin`；未发现自动调用 secure packager、签名服务或 release manifest 的步骤。
- QEMU security tests 由显式宏启用，不代表默认 production main 行为。

## 异常和错误路径

- parser/policy/stub 失败会从 `verify_image()` 提前返回；没有失败 measurement、stage ID、raw eHSM code、清零或统一终态。
- output buffer 太小由 stub 返回 address-range；没有 output/package overlap、明文 owner、cache、partial-write 或 zeroization 规则。
- measurement slot 错误在流程末尾出现，返回值与 result 内状态不一致。
- FMC/GSP main 没有调用 verify，因此也没有“验证失败时禁止 release”的实际 gate。

## 与目标设计的差异

| ID | 目标设计需求 | 代码现状 | 分类 | 影响 | 后续动作 |
|---|---|---|---|---|---|
| GAP-201 | FMC验证并release GSP | FMC hello-world，无调用 | `GAP` | 无FMC→GSP信任传递 | 冻结FMC状态机、source、Measurement/Manifest和release接口；当前不采用Handoff |
| GAP-202 | GSP 验证并 release Runtime | GSP hello-world/QEMU tests | `GAP` | 无运行态验证链 | 冻结 Runtime image 集和顺序 |
| GAP-203 | 真实签名/解密/eHSM command | stub memcpy + success bits | `GAP` | 核心安全结论为模拟 | 接 DD-02 production adapter |
| GAP-204 | 发布级 secure image builder | packager 不签名/不加密 | `GAP` | 产物不能发布 | 定义 build/sign/encrypt pipeline 与 key custody |
| GAP-205 | 地址域、range、load和jump契约 | GSP两套地址，loader不存在；canonical选择已裁决 | `GAP` | 规范已确定但实现/Evidence仍缺，可能写错区或跳错地址 | Manifest使用NoC/system `0x1010_0808_0000`；local/remap只在port/loader转换并验证 |
| GAP-206 | 真实 anti-rollback | 仅字段和模拟 checked | `GAP` | 旧镜像可被错误放行 | DD-03 冻结 counter 状态机 |
| GAP-207 | 可信 measurement | 内存 store、无真实 digest/失败记录 | `GAP` | SPDM/attest 可能输出空或模拟状态 | 定义跨 stage table 和 digest 来源 |
| GAP-208 | 全面负向 corpus | Host tests 以正向骨架为主 | `GAP` | parser/policy 边界未充分验证 | v0.3 和 baremetal 任务补齐 |

## 合理推断

- `INFERENCE-201`：parser/policy/Measurement的数据结构可以作为重构起点，但只有把来源、密码结果、counter、canonical地址和release门禁绑定后才可进入生产；当前不采用Handoff。
- `INFERENCE-202`：`0x1000...` 与 `0x1010...` 很可能是同一 SRAM 的 remapped master view；这不能替代 manifest address-domain 契约。
- `INFERENCE-203`：FMC 和 GSP 当前尚处于 SDK demo/框架阶段，不能按现有 main 函数增量补几行调用就认定启动链完成；需要先冻结 stage state machine 和共享 ABI。

## 无法确认或冲突

- `UNKNOWN-201`：FMC/GSP package 的真实存储位置、最大尺寸和可写主体。
- `UNKNOWN-202`：signature 覆盖区、加密模式/IV、key selection、algorithm profile 和 release signing 服务。
- `UNKNOWN-203`：plaintext output 是否直接落最终执行区，还是先落 staging 后 copy；cache/overlap/zeroization 规则。
- `UNKNOWN-204`：各 image 的 counter domain/ID/宽度、read/update API、原子性、耗尽和失败恢复。
- `UNKNOWN-205`：measurement table 的物理位置、跨 stage ownership、digest 口径和失败记录要求。
- `UNKNOWN-206`：FMC/GSP release/reset/privilege既有接口和失败终态；当前不新增Handoff ABI。
- `RESOLVED-003`：2026-07-28最终裁决为GSP Manifest、C908、loader、linker和eHSM共享descriptor统一使用baremetal System Address，不执行Local/System转换；旧080x输入拒绝。

## 设计与追溯回填

- 已回填：`docs/03-architecture/secure-boot.md`、`docs/03-architecture/anti-rollback.md`、`docs/05-software-design/fmc.md`、`docs/05-software-design/gsp.md`。
- Feature：SEC-FEAT-003～006、009、019 的代码观察更新为 CE-SEC-002；成熟度仍为 C1/C2，不提升为已实现。
- 冲突：调查时创建`OPEN-CONFLICT-003`；2026-07-22已按ADR-0004关闭，受影响设计可按canonical地址继续，物理访问仍需RTL/EMU Evidence。
- 测试：现有 Host/QEMU 测试只作为 L1/L2 骨架输入；v0.2 不修改，未生成测试通过结论。
- 下一步：对照 SRC-0016/SRC-0017 冻结 DD-01 启动时序和 DD-03 package/manifest/counter/measurement 契约；随后再拆分可编码 Feature。

## 评审结论

- 调查问题均已给出代码事实、冲突或 UNKNOWN。
- 本报告可以进入详设评审；不能作为代码已构建、测试已通过或硬件行为已验证的证明。

## 后续裁决记录

- 2026-07-22：负责人采用OPEN-CONFLICT-003推荐Option A；Manifest使用NoC/system canonical domain和`0x1010_0808_0000`，`0x1000_0808_0000`仅为local/remap view，转换收敛到NGU800P port/loader。
- 2026-07-28：最终地址裁决取代上述Option A转换方案；产品只使用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE` System Address且不建立Local/System映射。
- 2026-07-22：负责人确认当前不采用版本化Handoff；跨阶段度量、加载和错误分别由Measurement Table、Manifest/loader及统一错误/日志机制承担。
