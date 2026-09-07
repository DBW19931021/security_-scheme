# NGU800P D0 SEC_CFG 内网整理稿转录

## 1. 来源与证据边界

- Source ID：`SRC-0026`
- 输入：用户于2026-08-12提供的12张内网整理稿截图，其中第9、10张内容和SHA-256完全相同。
- 输入性质：内部二次整理资料，不是原始寄存器Excel、地址表、RTL、生成头或目标板Evidence。
- 可记录状态：截图明确呈现的文字、表格和示例为`DOCUMENTED`；推荐软件方法为`PROPOSED`；与SRC-0022冲突或缺失的内容为`CONFLICTING/BLOCKED`。
- 截图提到但本次未实际入库的`sec_cfg_apb_reg.xlsx`、`mgmt_addr_map.xlsx`、eHSM TRM和芯片安全方案，不得在本Source下伪装为原始附件。

## 2. 截图整理稿的核心结论

1. SEC_CFG被描述为SoC管理域的安全状态汇聚与只读观察窗口，镜像eHSM状态、硬件错误、固件错误、生命周期、UID，并输出SoC debug enable状态。
2. 它不是eHSM完整内部CSR、不是eFuse直接编程接口，也不是Firewall配置块。
3. 截图称System Address为`0x1010_07B0_0000`、窗口4 KiB、有效寄存器`0x000..0x040`、APB宽度32 bit、`pclk`、`porstn_async`。
4. 当前有效表共17个32-bit寄存器；除`dbg_en_cfg[1:0]`外均为RO。
5. `reset=0`只表示寄存器/字段复位值，释放复位后外部状态可立即更新，软件不得把运行期读值固定为0。
6. 截图称通用生成项存在Set/Clear offset `0x800/0x1000`，但SEC_CFG各字段没有Set/Clear标记，不能使用别名窗口。

## 3. 地址冲突

截图整理稿：

```text
SEC_CFG System Address = 0x1010_07B0_0000
Window size            = 0x0000_0000_1000 (4 KiB)
有效offset             = 0x000..0x040
```

工程当前`SRC-0022`生成头：

```text
SECURITY_NOC_S6_SEC_CFG_BASE = 0x1010_0820_0000
SECURITY_NOC_S6_SEC_CFG_SIZE = 0x4000 (16 KiB)
```

二者不一致。截图地址、窗口和下述绝对地址不得进入产品代码，等待`OPEN-CONFLICT-015`关闭。

## 4. 17个寄存器

下表中的绝对地址仅是“截图base + offset”的转录值，当前不是产品常量。

| Offset | 截图绝对地址 | Register | Field/拼接 | Access | Reset | 说明 |
|---|---|---|---|---|---|---|
| `0x000` | `0x1010_07B0_0000` | `hsm_status0` | `[31:0] hsm_status_0` | RO | `0` | `o_hsm_status[31:0]` |
| `0x004` | `0x1010_07B0_0004` | `hsm_status1` | `[31:0] hsm_status_1` | RO | `0` | `o_hsm_status[63:32]` |
| `0x008` | `0x1010_07B0_0008` | `hsm_err_hw0` | `[31:0] hsm_err_hw_0` | RO | `0` | `o_hsm_err_hw[31:0]` |
| `0x00C` | `0x1010_07B0_000C` | `hsm_err_hw1` | `[31:0] hsm_err_hw_1` | RO | `0` | `o_hsm_err_hw[63:32]` |
| `0x010` | `0x1010_07B0_0010` | `hsm_err_fw0` | `[31:0] hsm_err_fw_0` | RO | `0` | `o_hsm_err_fw[31:0]`；bit定义缺失 |
| `0x014` | `0x1010_07B0_0014` | `hsm_err_fw1` | `[31:0] hsm_err_fw_1` | RO | `0` | `o_hsm_err_fw[63:32]`；bit定义缺失 |
| `0x018` | `0x1010_07B0_0018` | `lcs` | `[31:0] lcs` | RO | `0` | Life Cycle State；合法编码缺失 |
| `0x01C` | `0x1010_07B0_001C` | `uid0` | `[31:0] uid_0` | RO | `0` | UID[31:0] |
| `0x020` | `0x1010_07B0_0020` | `uid1` | `[31:0] uid_1` | RO | `0` | UID[63:32] |
| `0x024` | `0x1010_07B0_0024` | `uid2` | `[31:0] uid_2` | RO | `0` | UID[95:64] |
| `0x028` | `0x1010_07B0_0028` | `uid3` | `[31:0] uid_3` | RO | `0` | UID[127:96] |
| `0x02C` | `0x1010_07B0_002C` | `uid4` | `[31:0] uid_4` | RO | `0` | UID[159:128] |
| `0x030` | `0x1010_07B0_0030` | `dbg_en_cfg` | `[1:0] dbg_en_cfg`；`[31:2] reserved` | RW/RO | `0` | Die1 pre-USER debug enable配置；scope/策略待确认 |
| `0x034` | `0x1010_07B0_0034` | `soc_dbg_en_out0` | `[31:0]` | RO | `0` | `o_soc_dbg_en_128b[31:0]` |
| `0x038` | `0x1010_07B0_0038` | `soc_dbg_en_out1` | `[31:0]` | RO | `0` | `o_soc_dbg_en_128b[63:32]` |
| `0x03C` | `0x1010_07B0_003C` | `soc_dbg_en_out2` | `[31:0]` | RO | `0` | `o_soc_dbg_en_128b[95:64]` |
| `0x040` | `0x1010_07B0_0040` | `soc_dbg_en_out3` | `[31:0]` | RO | `0` | `o_soc_dbg_en_128b[127:96]` |

## 5. `hsm_status[63:0]`

拼接顺序：`hsm_status0`为低32位，`hsm_status1`为高32位。

| Bit | Field | 截图含义/边界 |
|---|---|---|
| 63:48 | `hsm_fw_sta3` | `SYS_STA3[31:16]`；具体意义依目标Firmware文档 |
| 47:32 | `hsm_fw_sta2` | `SYS_STA2[31:16]`；具体意义依目标Firmware文档 |
| 31:28 | reserved | 保留 |
| 27 | `cpu_hart_halted` | eHSM CPU hart halted |
| 26 | reserved | 保留 |
| 25 | `cpu_wfi` | eHSM CPU WFI状态 |
| 24:21 | `hsm_fw_sta1` | Firmware状态片段 |
| 20 | `soc_cpu_release` | eHSM输出的SoC CPU release状态 |
| 19 | `soc_cpu_reset` | eHSM输出的SoC CPU reset状态 |
| 18 | `soc_reset` | eHSM输出的SoC reset状态 |
| 17 | `soc_dbg_en` | SoC debug enable状态 |
| 16 | `hsm_dbg_en` | eHSM debug enable状态 |
| 15 | `hsm_fw_sta0` | Firmware状态片段 |
| 14 | `hsm_lc_undef` | Undefined lifecycle |
| 13 | `hsm_lc_destroy` | Destroy lifecycle |
| 12 | `hsm_lc_debug` | Debug lifecycle |
| 11 | `hsm_lc_user` | User lifecycle |
| 10 | `hsm_lc_manu` | Manufacturing lifecycle |
| 9 | `hsm_lc_dev` | Development lifecycle |
| 8 | `hsm_lc_test` | Test lifecycle |
| 7 | `soc_verify_err` | SoC镜像验证错误 |
| 6 | `soc_verify_done` | SoC镜像验证结束 |
| 5 | `firmware_err` | eHSM Firmware加载/启动错误 |
| 4 | `firmware_done` | eHSM Firmware加载结束 |
| 3 | `bootloader_err` | eHSM Bootloader加载/启动错误 |
| 2 | `bootloader_done` | eHSM Bootloader加载结束 |
| 1 | `hw_boot_err` | eHSM hardware boot错误 |
| 0 | `hw_boot_done` | eHSM hardware boot结束 |

软件不得以`hw_boot_done`或“非零”单独判定所有阶段ready。每个阶段先检查error、再检查done；设置超时，超时或错误时同时采集status/hw error/fw error原始值。目标启动链究竟等待到`firmware_done`还是`soc_verify_done`由具体stage合同决定。

## 6. `hsm_err_hw[63:0]`

截图说明当前定义信号多为level状态。SEC_CFG只提供RO镜像，没有error clear/W1C寄存器；软件不得写SEC_CFG清错。

| Bit | Field | 含义 |
|---|---|---|
| 63:41 | reserved | 保留 |
| 40 | `wdt_timeout` | Watchdog timeout |
| 39:36 | reserved | 保留 |
| 35 | `otp_key_crc_err` | OTP key CRC error |
| 34 | `hw_trng_retry_warning` | 启动期间TRNG健康检测失败并进入retry |
| 33 | `hw_trng_retry_fail` | 启动期间TRNG健康检测retry超时 |
| 32 | `hw_trng_ht_fail` | 启动或功能阶段TRNG health test failure |
| 31:30 | reserved | 保留 |
| 29 | `soc_err_ahb_cfg` | CFG AHB response error |
| 28 | `soc_err_ahb_nvm` | NVM AHB response error |
| 27 | `soc_err_ahb_otp` | OTP AHB response error |
| 26 | `soc_err_ahb_mem` | SoC Memory AHB response error |
| 25 | `soc_err_axi_dma_rd` | DMA AXI read response error |
| 24 | `soc_err_axi_dma_wr` | DMA AXI write response error |
| 23:20 | reserved | 保留 |
| 19:16 | `mem_ecc_mb_pke3/2/1/0` | PKE3..PKE0 RAM uncorrectable ECC |
| 15 | `mem_ecc_mb_kmu` | KMU RAM uncorrectable ECC |
| 14 | `mem_ecc_mb_dram` | DRAM uncorrectable ECC |
| 13 | `mem_ecc_mb_iram` | IRAM uncorrectable ECC |
| 12 | `mem_ecc_mb_irom` | IROM uncorrectable ECC |
| 11:8 | reserved | 保留 |
| 7:4 | `mem_ecc_1b_pke3/2/1/0` | PKE3..PKE0 RAM correctable ECC |
| 3 | `mem_ecc_1b_kmu` | KMU RAM correctable ECC |
| 2 | `mem_ecc_1b_dram` | DRAM correctable ECC |
| 1 | `mem_ecc_1b_iram` | IRAM correctable ECC |
| 0 | `mem_ecc_1b_irom` | IROM correctable ECC |

不存在相应ECC功能时，截图称相关ECC bit应视为reserved。错误清除命令、锁存和重触发行为必须由匹配目标镜像的Firmware/RTL资料定义。

## 7. `hsm_err_fw[63:0]`

截图未提供具体bit定义，仅说明：信号由目标Firmware/Bootloader定义；通常为level；必须记录完整raw value和对应BL/FW build；定义缺失时不得对单bit命名或形成确定性Expected，也不得通过SEC_CFG写寄存器清错。

## 8. `lcs`

`SEC_CFG + 0x018`为32-bit RO生命周期镜像。截图没有给出合法编码、one-hot/枚举/eFuse raw语义，也没有证明其与`hsm_status[14:8]`采用相同编码。推荐同时读取两者并交叉验证；非法/未定义值fail-close。编码需Security/eFuse Owner冻结。

## 9. UID

`uid0..uid4`按低位到高位组成160-bit UID。寄存器只定义word拼接，不定义CPU字节序列化或外部文本格式；软件API宜返回`uint32_t word[5]`或明确端序的20-byte数组。截图未提供UID-valid位或稳定时刻，需RTL/平台Evidence。

## 10. Debug配置与输出

- `dbg_en_cfg[1:0]`是当前唯一RW字段；`[31:2]`保留。
- 截图文字称其为Die1 TEST/DEV/MANU、pre-USER debug enable配置，但bit0/bit1各自scope、写保护、lock、生命周期约束、与debug authentication的合成关系均缺失。
- `soc_dbg_en_out0..3`拼成最终128-bit观察输出，不意味着128个bit均由`dbg_en_cfg[1:0]`直接控制。
- 在scope和写保护闭环前，普通运行软件不得提供任意修改接口；若可信启动代码必须写，应只写`[1:0]`、立即读回，并验证最终输出/实际gate符合策略。

## 11. 与Firewall的关系

截图整理稿给出独立Firewall候选窗口`0x1010_0790_0000`，并引用`F_SECCFG_ENABLE@0x004`、`F_SECCFG_AUTHORITY@0x054`以及`0x078..0x088`错误寄存器。这些CSR属于Firewall而不属于SEC_CFG：Firewall决定“谁能访问SEC_CFG”，SEC_CFG决定“访问后能观察/配置什么”。Firewall base仍未由SRC-0022确认，相关内容继续受`OPEN-CONFLICT-014`约束。

## 12. 读取一致性与软件边界

- 对64-bit status/error，截图建议high-low-high retry作为软件防撕裂策略；这不等价于RTL原子快照，最终需CDC/锁存/一致性保证。
- RO offset不应由生产软件主动写探测；写响应和副作用需APB/集成规格验证。
- 推荐启动读取：确认eHSM reset释放与Firewall authority；轮询status并先error后done；错误/超时读取三组raw 64-bit；交叉读取lcs与status lifecycle；在UID有效阶段读取五个word；读取debug配置/输出；不通过SEC_CFG写操作清除状态或错误。

## 13. 截图建议的测试主题

地址/基本读、启动阶段、硬件错误注入、固件错误注入、生命周期切换、UID稳定性、Debug cfg/output/实际gate一致性、Firewall未授权访问、RO写入、reset释放后的动态镜像。所有测试当前均为`NOT_EXECUTED`。

## 14. 输入文件哈希

| 文件 | SHA-256 |
|---|---|
| `01-purpose-and-sources.png` | `6884d156f31c5c02aef8f86a7110904d475e34bc3765b63aea1d46245743eb5f` |
| `02-address-and-boundary.png` | `175a8ba9bfe2c258bb8db9786a5d84bdba5f1d7309064b6c24fac7af2fc79fc0` |
| `03-register-map-part1.png` | `aee79cd1ab21d658a0bd7749e5fcfa6e5add0d9ddd773729b0adbcf6e31fc609` |
| `04-register-map-and-status-part1.png` | `d630ae93d4c3479c42afefd372c7376bcb27001be400d1568ddbf9cdb0704fdb` |
| `05-status-part2.png` | `de5c6b55d03c7629d16be4924bb95134d6f925c4d7d502f33d9194b3ef52ddd4` |
| `06-hardware-error-part1.png` | `4e811f62079cedd3b3515bbc64dc629880c6e6bfa5d8aa89c3ccff8baf320152` |
| `07-hardware-error-and-fw-error.png` | `a49d32eda1c1703c03d8632b38bdf3bd2572ffdac72417847c428bb92c10601a` |
| `08-lcs-uid-debug-part1.png` | `477cdf0c504bc4c44c77ea4b2e3514f89e297962f6dd4b78f565e199d35ce55a` |
| `09-debug-firewall-part1.png` | `2d7fd33c87deb3e39994b1c0bcb6ee46caa19e2942a89d0722d47a49553c3693` |
| `10-debug-firewall-duplicate.png` | `2d7fd33c87deb3e39994b1c0bcb6ee46caa19e2942a89d0722d47a49553c3693` |
| `11-software-read-guidance.png` | `2c74b770193a9cdd20a14e2bc2011e116cee2d122b3514ee94050ffac2faac5b` |
| `12-dv-tests-and-gaps.png` | `595541605253ea5d60ac99ae070a41fcb78a89ec3738ea915d071fae8e3057c4` |

