---
title: "SEC_CFG寄存器与软件接口"
status: review_ready_with_open_bindings
evidence_state: PROPOSED
applicability:
  - NGU800P D0
source_ids:
  - SRC-0018
  - SRC-0022
  - SRC-0025
  - SRC-0026
owners:
  - GSP
  - RTL_Integration
last_reviewed: 2026-08-12
supersedes: []
superseded_by: []
---

# Interface objective

本接口把SEC_CFG寄存器模型转换为可审计的软件API合同。实现不得复制截图绝对地址；base必须来自关闭`OPEN-CONFLICT-015`后的RTL同步生成头。

# Data model

```c
struct sec_cfg_snapshot {
    uint64_t hsm_status;
    uint64_t hsm_err_hw;
    uint64_t hsm_err_fw;
    uint32_t lcs_raw;
    uint32_t uid_word[5];       /* word[0] = UID[31:0] */
    uint32_t dbg_cfg_raw;
    uint32_t dbg_out_word[4];   /* word[0] = output[31:0] */
};
```

建议将平台绑定和纯逻辑拆开：`sec_cfg_bind()`只接受权威生成头；`sec_cfg_read_snapshot()`只读并保留raw值；`sec_cfg_decode_status()`可解码已冻结status/hardware-error位；`sec_cfg_decode_fw_error()`在目标BL/FW定义缺失时返回`UNSUPPORTED_MAPPING`。

# Offset contract

| Group | Offsets | Access | Software rule |
|---|---:|---|---|
| HSM status | `0x000/0x004` | RO | low/high拼接；轮询先error后done |
| HW error | `0x008/0x00C` | RO | 保存raw并解码已定义位；不可清零 |
| FW error | `0x010/0x014` | RO | 定义冻结前仅raw |
| LCS | `0x018` | RO | raw值与status lifecycle交叉检查 |
| UID | `0x01C..0x02C` | RO | 五个word，显式定义输出端序 |
| Debug cfg | `0x030` | mixed | 只允许`[1:0]`，普通运行时禁写 |
| Debug output | `0x034..0x040` | RO | 四个word组成128-bit最终观察值 |

# Stable 64-bit read

APB一次只读32 bit且没有已证明的snapshot/latch。对于诊断一致性可采用：读high、读low、再次读high；两次high相同才返回。此方法只降低跨高字变化造成的撕裂，low在两次high之间独立变化仍需RTL一致性合同，调用方必须能够标记`BEST_EFFORT_SNAPSHOT`。

# Stage polling

每个启动阶段绑定一对`done/error`位。循环中先检查error；再检查done；再判断timeout。目标阶段由启动链显式选择，不能用统一`hw_boot_done`替代。错误或超时返回前必须采集三组raw 64-bit值并记录目标D0 RTL、eHSM BL/FW和软件build。

# UID serialization

寄存器word顺序固定为`uid0`低位至`uid4`高位。公共接口首选返回`uint32_t[5]`；若输出20-byte或文本，必须明确little-endian word内字节序和外部显示顺序。UID-valid时刻未冻结前，接口必须显式返回`VALIDITY_UNCONFIRMED`而不是默认成功。

# Debug write gate

只有受信任启动路径且产品策略冻结后才允许写`dbg_en_cfg[1:0]`。写入必须掩掉`[31:2]`、立即读回、读取128-bit最终输出并由独立Debug gate Evidence验收。普通运行期API、诊断CLI和Host接口不得暴露任意写。

# Error handling

- base未绑定：返回`SEC_CFG_ERR_MAP_UNAVAILABLE`，不得访问地址0。
- Firewall拒绝：保存Firewall错误快照，不把hide后的OK response当成功。
- LCS非法/不一致：fail-close并上报Security/RAS策略层。
- Firmware error映射未知：保存raw和build，不猜bit名。
- RO写行为未定义：生产软件不主动写探测，仅DV在受控环境验证。

# Related artifacts

- [架构边界](../03-architecture/sec-cfg-status-observation.md)
- [机读寄存器表](../../requirements/sec-cfg-register-map.yaml)
- [原子需求](../../requirements/sec-cfg-requirements.yaml)
- [验证设计](../06-verification/sec-cfg-test-design.md)

