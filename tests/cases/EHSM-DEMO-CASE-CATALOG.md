# eHSM Vendor Demo Case Catalog

- 状态：`draft_level_1_inventory`
- 来源：SRC-0018 `ehsm_host-2.3.1-4019-2ee044d/demo`
- 适用：NGU800P D0 eHSM能力验证规划
- Owner：GSP
- 目标实现：后续批准任务中的`../baremetal`
- 事实状态：入口和调用关系为`VENDOR_IMPLEMENTATION`；是否适用于NGU800P硬件及最终Expected仍需文档/EMU Evidence确认
- 执行声明：当前仅完成一级入口盘点，尚未执行任何case，也不表示相关功能已经通过

## 使用规则

1. 本清单记录Vendor Demo提供的完整能力面，不直接替代版本化测试工作簿。
2. `L1_mapped`表示顶层入口已登记；必须继续将入口函数内部的算法、模式、key type、输入长度、同步/异步、正负向和边界组合展开为二级case，达到`L2_complete`后才可声称源码case盘点完整。
3. BL态、FW态、OTP/LCS/Debug/升级等case按前置状态和破坏性等级分组执行，禁止把所有入口无条件串成一次回归。
4. Vendor命令拼装无特殊差异时优先复用；顶层case注册、选择、runner、timeout、结果格式和Evidence follow baremetal规则。
5. PASS/FAIL依据不得只是Vendor Demo最终打印；每个case保留raw eHSM返回和批准Expected。

## Root与分支入口

| Catalog ID | Vendor入口/行为 | Source位置 | 源码状态 | 展开状态 | 说明 |
|---|---|---|---|---|---|
| VND-EHSM-ROOT-001 | `ehsm_driver_init_library(WAIT_AND_POLL)` | `demo/test.c:22-27` | enabled_by_default | L1_mapped | 需增加init失败、重复init、资源清理case |
| VND-EHSM-ROOT-002 | Host driver version | `demo/test.c:29-32` | enabled_by_default | L1_mapped | driver版本与eHSM版本分开记录 |
| VND-EHSM-ROOT-003 | OTP默认值写入、reset、ready等待 | `demo/test.c:39-52` | enabled_by_default | L1_mapped | 破坏性/平台相关；不能原样作为默认回归前置 |
| VND-EHSM-ROOT-004 | eHSM版本识别并选择BL/FW分支 | `demo/test.c:54-60` | enabled_by_default | L1_mapped | BL和FW需分别形成执行profile |
| VND-EHSM-ROOT-005 | BL patch测试 | `demo/test.c:34-36` | compile_time_optional | L1_mapped | 需核实交付功能、宏和适用版本 |

## Bootloader态入口

| Catalog ID | 功能 | Vendor入口 | 源码状态 | 展开状态 | 风险/备注 |
|---|---|---|---|---|---|
| VND-EHSM-BL-001 | 生成OTP Key | `ehsm_bl_demo_gen_otp_key_entry` | enabled_by_default | L1_mapped | OTP/key资源受控 |
| VND-EHSM-BL-002 | OTP读写 | `ehsm_bl_demo_rd_wr_otp_entry` | enabled_by_default | L1_mapped | 破坏性、offset/权限待展开 |
| VND-EHSM-BL-003 | 寄存器读写 | `ehsm_bl_demo_rd_wr_reg_entry` | enabled_by_default | L1_mapped | 任意寄存器访问不得直接成为产品API |
| VND-EHSM-BL-004 | Debug Auth第一次调用 | `ehsm_bl_demo_debug_auth_entry` | enabled_by_default | L1_mapped | 与第二次调用的场景差异待核实 |
| VND-EHSM-BL-005 | Debug Auth第二次调用 | `ehsm_bl_demo_debug_auth_entry` | duplicate_pending_review | L1_mapped | 不静默去重；确认是正/负向组合还是源码重复 |
| VND-EHSM-BL-006 | 镜像验证 | `ehsm_bl_demo_image_verify_entry` | enabled_by_default | L1_mapped | 展开算法、image/profile、失败返回 |
| VND-EHSM-BL-007 | 镜像升级 | `ehsm_bl_demo_image_upgrade_entry` | enabled_by_default | L1_mapped | Flash/版本/掉电和授权需独立fixture |
| VND-EHSM-BL-008 | UART baud divisor | `ehsm_bl_demo_set_uart_buad_div_entry` | enabled_by_default | L1_mapped | 平台能力，需确认NGU800P适用性 |
| VND-EHSM-BL-009 | Self-test | `ehsm_bl_demo_self_test_entry` | enabled_by_default | L1_mapped | raw bitmap必须保留；bit18=TRNG，bit19未知/保留 |

## Firmware态入口

| Catalog ID | 功能 | Vendor入口 | 源码状态 | 展开状态 | 风险/备注 |
|---|---|---|---|---|---|
| VND-EHSM-FW-001 | 并行模式候选 | `test_parallel` | commented_reference_no_definition_found | L1_mapped | 当前快照仅见注释调用，未找到声明/定义；先核实是否漏交付、旧代码残留或不支持，不得作为已提供接口 |
| VND-EHSM-FW-002 | Hash | `ehsm_demo_hash_entry` | enabled_by_default | L1_mapped | 算法/driver mode/长度待展开 |
| VND-EHSM-FW-003 | HMAC | `ehsm_demo_hmac_entry` | enabled_by_default | L1_mapped | key type/生成与校验待展开 |
| VND-EHSM-FW-004 | 对称加解密 | `ehsm_demo_symm_cipher_entry` | enabled_by_default | L1_mapped | 算法/mode/key/长度待展开 |
| VND-EHSM-FW-005 | AEAD | `ehsm_demo_aead_entry` | enabled_by_default | L1_mapped | AAD/tag/错误tag待展开 |
| VND-EHSM-FW-006 | MAC | `ehsm_demo_mac_entry` | enabled_by_default | L1_mapped | 算法/key/校验失败待展开 |
| VND-EHSM-FW-007 | SM2 | `ehsm_demo_sm2_entry` | enabled_by_default | L1_mapped | 加解密/签名/验签待展开 |
| VND-EHSM-FW-008 | ECDSA | `ehsm_demo_ecdsa_entry` | enabled_by_default | L1_mapped | curve/hash/正负向待展开 |
| VND-EHSM-FW-009 | RSA | `ehsm_demo_rsa_entry` | enabled_by_default | L1_mapped | key size/padding/签名/加密待展开 |
| VND-EHSM-FW-010 | RNG | `ehsm_demo_rng_entry` | enabled_by_default | L1_mapped | 健康状态、长度、重复输出检查待展开 |
| VND-EHSM-FW-011 | OTP读写 | `ehsm_demo_rd_wr_otp_entry` | enabled_by_default | L1_mapped | destructive_controlled |
| VND-EHSM-FW-012 | 生命周期变更 | `ehsm_demo_chg_lifecycle_entry` | source_present_not_called | L1_mapped | irreversible/destructive_controlled，不进入默认回归 |
| VND-EHSM-FW-013 | OTP控制字段变更 | `ehsm_demo_chg_ctrl_field_entry` | enabled_by_default | L1_mapped | destructive_controlled |
| VND-EHSM-FW-014 | 安装OTP Key | `demo_install_otp_key_entry` | enabled_by_default | L1_mapped | destructive_controlled；slot/owner待确认 |
| VND-EHSM-FW-015 | Debug Auth | `ehsm_demo_debug_auth_entry` | enabled_by_default | L1_mapped | LCS/token/重放/失败锁定待展开 |
| VND-EHSM-FW-016 | 寄存器读写 | `ehsm_demo_rd_wr_reg_entry` | enabled_by_default | L1_mapped | 权限/allowlist待展开 |
| VND-EHSM-FW-017 | UART baud divisor | `ehsm_demo_set_uart_buad_div_entry` | enabled_by_default | L1_mapped | 平台适用性待确认 |
| VND-EHSM-FW-018 | 生成Key | `ehsm_demo_gen_key_entry` | enabled_by_default | L1_mapped | key type/slot/导出属性待展开 |
| VND-EHSM-FW-019 | 派生Key | `ehsm_demo_derive_key_entry` | enabled_by_default | L1_mapped | KDF/profile/context待展开 |
| VND-EHSM-FW-020 | 交换Key | `ehsm_demo_exchg_key_entry` | enabled_by_default | L1_mapped | 协议/peer/input validation待展开 |
| VND-EHSM-FW-021 | 导入Key | `ehsm_demo_import_key_entry` | enabled_by_default | L1_mapped | 包装/权限/非法key待展开 |
| VND-EHSM-FW-022 | 导出Key | `ehsm_demo_export_key_entry` | enabled_by_default | L1_mapped | 可导出属性/敏感清理待展开 |
| VND-EHSM-FW-023 | 删除Key | `ehsm_demo_remove_key_entry` | enabled_by_default | L1_mapped | 重复删除/使用后删除待展开 |
| VND-EHSM-FW-024 | 私钥导出公钥 | `ehsm_demo_get_pub_from_priv_key_entry` | compile_time_optional_marker | L1_mapped | 源码含生成器宏标记，需核实实际构建条件 |
| VND-EHSM-FW-025 | 镜像验证 | `ehsm_demo_image_verify_entry` | enabled_by_default | L1_mapped | 算法/profile/失败返回待展开 |
| VND-EHSM-FW-026 | 镜像升级 | `ehsm_demo_image_upgrade_entry` | enabled_by_default | L1_mapped | 版本/掉电/Flash/权限待展开 |
| VND-EHSM-FW-027 | digest输入的PKE签名验签 | `ehsm_demo_pke_sign_verify_by_msg_digest_test` | compile_time_optional_marker | L1_mapped | RSA/SM2/ECDSA二级组合待展开 |

## 完整性门禁

- [x] `ehsm_demo_test()`、`bl_demo`和`fw_demo`一级入口已登记。
- [ ] 每个入口内部所有command和case组合已展开，并与源码行、编译宏关联。
- [ ] 每个case已标注BL/FW版本、LCS、OTP/Key fixture、破坏性、可重复性和恢复要求。
- [ ] 每个case已绑定批准Expected、工作簿行和`baremetal`可执行入口。
- [ ] 正向、错误返回、边界、timeout/reset、并发和敏感材料清理覆盖已补齐。
- [ ] 在目标eHSM/EMU执行并形成Evidence；未执行前不得标为PASS。

## Change history

- 2026-07-22：根据负责人确认建立一级Vendor Demo完整能力清单；保留注释入口和BL重复Debug Auth待核实项。未生成或修改可执行测试。
