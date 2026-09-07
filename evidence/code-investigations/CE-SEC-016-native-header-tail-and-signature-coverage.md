# CE-SEC-016：Native Header尾部使用与签名覆盖代码证据

## Evidence metadata

- Evidence ID：CE-SEC-016
- 日期：2026-08-21
- 调查任务：INV-SEC-016
- Vendor活参考：`../fsp`
- Remote：`http://192.168.70.8/gpnpu_system/firmware/fsp.git`
- Branch/commit：`master@a5193e31dfb93238595b822d2d94e07143ee903d`
- Worktree：存在大量既有modified文件；本次涉及的四个关键文件与HEAD内容一致，仅文件mode从100755显示为100644。
- 操作：只读搜索、函数体阅读及Git状态/差异查询；未修改FSP、产品代码、baremetal或工作簿。

## CODE_FACT-01：Native Header尾部16字节未被公钥解析或Key ID校验消费

- `ehsm_bootrom/src/component/fw_verify.h:13-27`和`ehsm_firmware/src/service/socvrfy_srv.h:14-27`固定：Header总长1024字节，`INFO_RESERVED/Public_Key_Ext`从偏移624开始，总长400字节。
- 两个实现都定义`RSA3072_PUBLIC_K_N_LEN=384`。
- `ehsm_bootrom/src/component/fw_verify.c:930-969`和`ehsm_firmware/src/service/socvrfy_srv.c:815-850`在RSA-3072路径只从`IMAGE_PUBLIC_K_EXT_OFFSET`复制384字节模数。
- 因此被消费范围为Header 624～1007，Header 1008～1023没有被当前公钥解析路径读取。RSA-2048、P-256和SM2路径只从主`Public_Key`区复制各自公钥，更不使用该尾部。
- BL的`fwverify_verify_pubkey()`（`fw_verify.c:148-166`）和FW的`socvrfy_verify_pubkey()`（`socvrfy_srv.c:100-125`）只对前一步按算法重组出的`pubkey[pubkey_len]`计算OTP Key ID哈希。RSA-3072的重组长度是64字节指数加384字节模数，未对原始400字节`Public_Key_Ext`整体做哈希。

结论：对当前commit及已批准算法集合，Header末尾16字节既不参与公钥材料重组，也不参与OTP公钥Key ID计算，可作为NGU800P项目overlay候选。

## CODE_FACT-02：末尾16字节在Vendor认证范围内

- BL定义`VALID_FLAG_OFFSET=592`、`EHSM_CODE_INFO_HASH_SIZE=1024-592`；`fwverify_sm2_check_code_sign()`、`fwverify_asym_check_code_sign()`和`fwverify_sym_check_code_sign()`分别在`fw_verify.c:450-675`把Header 592～1023送入SM3/SHA-256/CMAC，再继续认证整个Code Region。
- FW定义`EHSM_IMAGE_HEADER_HASH_SIZE=1024-592`；`socvrfy_sm2_verify_code_sign()`、`socvrfy_rsa_verify_code_sign()`和`socvrfy_sym_verify_code_sign()`在`socvrfy_srv.c:605-721`从`image_addr+592`认证432字节Header和`Code_Size`字节Code Region。
- 偏移1008～1023位于592～1023内。因此任何`load_addr`或reserved篡改都会改变签名/CMAC输入并导致验证失败。

结论：`load_addr`不是未认证的旁带地址，但必须在eHSM PASS后从稳定输出Header重读并执行stage策略检查。

## CODE_FACT-03：当前产品仓尚未实现目标overlay

- `../gsp-pmp-rmp-omp/components/security/include/security/ehsm_image.h`只把偏移624～1023整体定义为400字节`EHSM_HEADER_PUBLIC_KEY_EXT_SIZE`，没有1008/1016项目字段。
- 同仓当前仍存在旧`manifest.h/.c`和synthetic `pack_image.py`，且布局与此前批准的128字节Manifest也不一致。

结论：本轮是设计变更，不代表代码已经支持；实现时应删除产品Manifest依赖并增加固定offset解析、布局断言和负向测试，不能兼容保留两套解析路径。

## TARGET_DESIGN与限制

1. NGU800P type 1 SoC package把1008～1015解释为LE64 `load_addr`，1016～1023固定为0。
2. eHSM type 0 Vendor FW不使用该项目overlay。
3. 任何新增Vendor算法、Header修订、超过384字节的`Public_Key_Ext`消费，或把原始400字节字段整体纳入公钥Key ID计算的变化，都必须重新调查；未证明前禁止复用尾部。
4. 预验签阶段只能把地址用于“等于stage固定目标”的副作用限制，不能把它当作已认证事实；PASS后必须重读并再次exact-match。
5. 本证据没有证明现有制包工具已经正确写入、签名或加密该overlay，也没有授权产品代码修改。
