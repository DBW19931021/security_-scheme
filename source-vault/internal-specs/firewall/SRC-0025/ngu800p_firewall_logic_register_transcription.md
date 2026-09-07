# NGU800P Firewall 逻辑需求与寄存器截图转录

- Source ID：`SRC-0025`
- 输入日期：2026-08-12
- 适用对象：NGU800P D0 候选 SRAM、`sec_cfg`、`spifc` Firewall
- 事实状态：`DOCUMENTED`；与现有工程基线不一致的项目为 `CONFLICTING`
- 原始输入：本目录 7 张 PNG；截图是用户提供的内部设计输入，不替代 RTL 同步生成头 `SRC-0022`

## 1. 逻辑需求转录

### 1.1 共同传输规则

1. Firewall 位于 TNUI 与受保护 Slave IP 之间，时钟和复位与 Target NIU 一致。
2. Firewall 有总检查开关：打开时检查传输，关闭时透明旁路。
3. 失败请求由 TNUI 阻断，不应到达 Slave。
4. `hide_en=0` 时失败由 TNUI 返回 Error；`hide_en=1` 时 TNUI 返回 OK。Hide 只隐藏失败响应，不授权 Slave 访问。
5. 安全访问旁路到 Slave，由 Slave 返回正常响应。
6. 读写属性编码：`2'b00` 不可读写，`2'b01` 只写，`2'b10` 只读，`2'b11` 可读写。
7. 安全属性规则：配置为读安全/写安全时，只有 secure 标记的对应方向请求可通过；配置为非安全时，secure 和 non-secure 请求都可通过。

### 1.2 SRAM 特有规则

1. 同时执行地址区间检查和 Region 读写权限检查，任一失败即为非安全访问。
2. 最多支持 5 个独立 Region，每个 Region 独立配置并独立使能；未使能 Region 不参加安全属性检查。
3. Region 地址由 48-bit 起始地址和结束地址表示，截图文字称两者 4 KiB 对齐；寄存器默认结束地址为页末 `...FFFF`，因此端点语义仍待确认。
4. 地址检查仅在事务地址恰好落入一个已使能 Region 时通过；零命中或多命中均失败。
5. 4-bit UserId 把每个 die 的 master 分组；die0 与 die1 共用一套 32-bit authority，14 个非 reserved UserId 分别具有写、读权限位。

## 2. UserId 与 authority 完整编码

| UserId | Die | Master | 写位 | 读位 | 单 Master 读写掩码 |
|---:|---|---|---:|---:|---:|
| `0x0` | die0 | reserved | - | - | `0x00000000` |
| `0x1` | die0 | RAS Core | 0 | 1 | `0x00000003` |
| `0x2` | die0 | 通用/带内/带外管理 MCU Core | 4 | 5 | `0x00000030` |
| `0x3` | die0 | Codec MCU Core | 8 | 9 | `0x00000300` |
| `0x4` | die0 | 功耗管理 MCU Core | 12 | 13 | `0x00003000` |
| `0x5` | die0 | Host 经 PCIe 访问管理子系统 RAM | 16 | 17 | `0x00030000` |
| `0x6` | die0 | 管理子系统 DMA / TOP NOC DMA | 20 | 21 | `0x00300000` |
| `0x7` | die0 | eHSM | 24 | 25 | `0x03000000` |
| `0x8` | die1 | reserved | - | - | `0x00000000` |
| `0x9` | die1 | RAS Core | 2 | 3 | `0x0000000C` |
| `0xA` | die1 | 通用/带内/带外管理 MCU Core | 6 | 7 | `0x000000C0` |
| `0xB` | die1 | Codec MCU Core | 10 | 11 | `0x00000C00` |
| `0xC` | die1 | 功耗管理 MCU Core | 14 | 15 | `0x0000C000` |
| `0xD` | die1 | Host 经 PCIe 访问管理子系统 RAM | 18 | 19 | `0x000C0000` |
| `0xE` | die1 | 管理子系统 DMA / TOP NOC DMA | 22 | 23 | `0x00C00000` |
| `0xF` | die1 | eHSM | 26 | 27 | `0x0C000000` |

压缩位图公式仅适用于 `local_id = userid & 0x7` 非零的 UserId：

```text
die       = userid[3]
write_bit = 4 * (local_id - 1) + 2 * die
read_bit  = write_bit + 1
```

`0x0/0x8` 必须在减一前拒绝。bits[31:28] 未被截图定义，应保持 0。双 die 读写掩码依次为 RAS `0x0000000F`、管理 MCU `0x000000F0`、Codec `0x00000F00`、功耗 MCU `0x0000F000`、Host `0x000F0000`、DMA `0x00F00000`、eHSM `0x0F000000`；全部非 reserved Master 为 `0x0FFFFFFF`。

## 3. 寄存器表转录

- 截图页头：`Addr Width = 10`，`main_resetn = presetn`，Set/Clear Offset 为 `0x400/0x800`。
- 基址栏只显示占位值 `0xxx_0000`，不能用于编码。

| Offset | Register | 关键字段 | 截图 ResetValue |
|---:|---|---|---:|
| `0x000` | `F_SRAM_ENABLE` | bit1 `f_sram_hide_en`；bit0 `f_sram_check_en` | 两位均 `1'b1` |
| `0x004` | `F_SECCFG_ENABLE` | bit1 `f_seccfg_hide_en`；bit0 `f_seccfg_check_en` | 两位均 `1'b1` |
| `0x008` | `F_SPIFC_ENABLE` | bit1 `f_spifc_hide_en`；bit0 `f_spifc_check_en` | 两位均 `1'b1` |
| `0x00C` | `F_REGION1_EN` | bit0 `f_region1_en` | `1'b1` |
| `0x010` | `F_REGION1_AUTHORITY` | bits[31:0] authority | `0x000000F0` |
| `0x014` | `F_REGION1_STR_ADDR_LOW32` | Region1 start low32 | `0x05000000` |
| `0x018` | `F_REGION1_END_ADDR_LOW32` | Region1 end low32 | `0x057FFFFF` |
| `0x01C` | `F_REGION2_EN` | bit0 `f_region2_en` | `1'b1` |
| `0x020` | `F_REGION2_AUTHORITY` | bits[31:0] authority | `0x0000000F` |
| `0x024` | `F_REGION2_STR_ADDR_LOW32` | Region2 start low32 | `0x05080000` |
| `0x028` | `F_REGION2_END_ADDR_LOW32` | Region2 end low32 | `0x050BFFFF` |
| `0x02C` | `F_REGION3_EN` | bit0 `f_region3_en` | `1'b1` |
| `0x030` | `F_REGION3_AUTHORITY` | bits[31:0] authority | `0x0000F000` |
| `0x034` | `F_REGION3_STR_ADDR_LOW32` | Region3 start low32 | `0x050C0000` |
| `0x038` | `F_REGION3_END_ADDR_LOW32` | Region3 end low32 | `0x050FFFFF` |
| `0x03C` | `F_REGION4_EN` | bit0 `f_region4_en` | `1'b1` |
| `0x040` | `F_REGION4_AUTHORITY` | bits[31:0] authority | `0x00000F00` |
| `0x044` | `F_REGION4_STR_ADDR_LOW32` | Region4 start low32 | `0x05100000` |
| `0x048` | `F_REGION4_END_ADDR_LOW32` | Region4 end low32 | `0x0517FFFF` |
| `0x04C` | `F_REGION5_EN` | bit0 `f_region5_en` | `1'b1` |
| `0x050` | `F_REGION5_AUTHORITY` | bits[31:0] authority | `0x0FFFFFFF` |
| `0x054` | `F_SECCFG_AUTHORITY` | sec_cfg authority | 未显示 |
| `0x058` | `F_SPIFC_AUTHORITY` | spifc authority；字段名疑似误写为 `f_seccfg_authority` | 未显示 |
| `0x05C` | `F_REGION5_STR_ADDR_LOW32` | Region5 start low32 | `0x05180000` |
| `0x060` | `F_REGION5_END_ADDR_LOW32` | Region5 end low32 | `0x051FFFFF` |
| `0x064` | `F_SRAM_ERROR_CLEAR` | bit0 写 1 清 SRAM 错误状态 | `0` |
| `0x068` | `F_SRAM_ERR_INFO_ADDR_LOW32` | SRAM 非安全传输地址 low32 | `0` |
| `0x06C` | `F_SRAM_ERR_INFO_ADDR_HIGH16` | SRAM 非安全传输地址 high16 | `0` |
| `0x070` | `F_SRAM_ERR_INFO` | bit4 write，bits[3:0] UserId | `0` |
| `0x074` | `F_SRAM_ERR_STATUE` | bits[4:0] Region5..1 error status | `0` |
| `0x078` | `F_SECCFG_ERROR_CLEAR` | bit0 写 1 清 sec_cfg 错误状态 | `0` |
| `0x07C` | `F_SECCFG_ERR_INFO_ADDR_LOW32` | sec_cfg 错误地址 low32 | `0` |
| `0x080` | `F_SECCFG_ERR_INFO_ADDR_HIGH16` | sec_cfg 错误地址 high16 | `0` |
| `0x084` | `F_SECCFG_ERR_INFO` | bit4 write，bits[3:0] UserId | `0` |
| `0x088` | `F_SECCFG_ERR_STATUE` | bit0 Region1 error status | `0` |
| `0x08C` | `F_SPIFC_ERROR_CLEAR` | bit0 写 1 清 spifc 错误状态 | `0` |
| `0x090` | `F_SPIFC_ERR_INFO_ADDR_LOW32` | spifc 错误地址 low32 | `0` |
| `0x094` | `F_SPIFC_ERR_INFO_ADDR_HIGH16` | spifc 错误地址 high16 | `0` |
| `0x098` | `F_SPIFC_ERR_INFO` | bit4 write，bits[3:0] UserId | `0` |
| `0x09C` | `F_SPIFC_ERR_STATUE` | bit0 Region1 error status | `0` |

## 4. 截图默认值解码

| Region | low32 地址范围 | authority | 位图解码 |
|---|---|---:|---|
| R1 | `0x05000000..0x057FFFFF` | `0x000000F0` | 两个 die 的管理 MCU 读写 |
| R2 | `0x05080000..0x050BFFFF` | `0x0000000F` | 两个 die 的 RAS Core 读写 |
| R3 | `0x050C0000..0x050FFFFF` | `0x0000F000` | 两个 die 的功耗管理 MCU 读写 |
| R4 | `0x05100000..0x0517FFFF` | `0x00000F00` | 两个 die 的 Codec MCU 读写；逻辑需求文字称 MM Core |
| R5 | `0x05180000..0x051FFFFF` | `0x0FFFFFFF` | 全部 14 个非 reserved Master 读写 |

R1 包含 R2-R5 的全部 low32 窗口，与“多 Region 命中必须失败”同时成立时会导致 R2-R5 默认访问多命中。该组默认地址不能直接作为产品 policy。

## 5. 原始文件校验

| 文件 | SHA-256 |
|---|---|
| `01-logic-seccfg-spifc.png` | `8d8f3f3e3f3c3744a4f3f4fe629f6fb267ef664a89f04cc6288b71bb6ed23b85` |
| `02-logic-sram-part1.png` | `34db30c6e8e7170ff37cbcb6ead1dd3773a20f759643effd7eba7962e40be000` |
| `03-logic-sram-part2.png` | `176056c66bcbf3a2fc3ed4bca91ab5f69a2bc652b1cdddcfbf986f751ef3af81` |
| `04-registers-part1.png` | `dce259754b5c49fdd8966cb4b770fc1b627a0052ba1458b644c455cee4895d26` |
| `05-registers-part2.png` | `b2968a284162c18bcd4f5808b5bfda2c5163caae9820c898b5c5ab57b2956b29` |
| `06-registers-part3.png` | `7db7532623e4f55a2f027d114b02658fb268b16922f9d5b9655cce9f4bdb101b` |
| `07-registers-part4.png` | `68f471a239f37bf028bb62465e8a979af1316929e96cb16783fe3a5cf30df735` |

