# Design

> **SUPERSEDED（2026-08-21）**：本变更中的NGU Manifest方案已由`native-header-load-address-v1`和ADR-0030替代，仅保留历史决策记录；不得作为当前实现或测试依据。

## 系统上下文

发布工具生成Vendor Header与受保护NGU Manifest；BootROM/FMC/GSP通过同一公共组件执行preflight、Vendor verify、认证Header复验、Manifest/policy、loader源摘要/copy/目标回读摘要比较、Measurement和release。

## 信任边界

- preflight Header不可信，只做规范化/容量检查。
- 仅eHSM PASS后的输出Header和Code Region可进入认证解析。
- Manifest受Vendor签名/加密保护，但仍需项目policy和range校验。

## 状态机

`NOT_SUBMITTED -> VENDOR_PASS -> HEADER_PASS -> MANIFEST_PASS -> POLICY_PASS -> LOADED_AND_READBACK_VERIFIED -> MEASUREMENT_COMMITTED -> RELEASED`。

确定失败进入`FAILED/ISOLATED`；接受状态未知进入`QUARANTINED`。

## 数据流

`Vendor Header[1024] + encrypted/authenticated {Manifest v1[128] + payload + CBC_zero_pad[0..15]}`。Manifest和Code Region中不存在独立expected digest对象。

## 密钥流

本change只引用ADR-0017批准的三套算法Profile，不存储私钥、不定义新密钥流。Profile 1/2/3分别为SHA-256+RSA-2048-PSS+AES-128-CBC、SHA-256+ECDSA-P256+AES-128-CBC、SM3+SM2+SM4-CBC；ADR-0020已批准具体设备/SKU和key/board/LCS绑定来自单一provisioning/release matrix，实际行缺失时制包和release默认拒绝。

## 生命周期影响

产品FMC/GSP/PMP/RMP/MMP必须签名、加密、rollback、measure、release gate；额外LCS/board规则由profile管理。

## 异常处理

双阶段`Code_Size == package_size - 1024`；非法magic/header size、reserved非0、rollback counter不等、固定offset错误、地址范围错误、出现expected digest对象或loader源/目标摘要不一致均拒绝。Manifest offset124的`uint32_t version`只供Host工具读取。

## 掉电恢复

不新增Handoff。`rollback_counter[16]`宽度、Owner和eHSM BL专用API方向已由ADR-0019冻结；counter寿命、耗尽和掉电恢复参数作为实现/EMU前DoR管理。

## 回滚策略

16字节global `rollback_counter`、FMC主动调用eHSM BL新增专用API更新，按ADR-0012/0019。

## 接口变化

新增公共ABI registry、package/Manifest/loader逻辑接口；Vendor公共接口不变。

GSP由第一个、最高优先级的`security_service_task`从bootstrap到runtime持续作为唯一eHSM Owner；其他task只能通过typed queue提交请求，bootstrap门禁完成后同一task转入runtime service loop，见ADR-0015。

## 软件影响

后续需替换私有Manifest、32位counter和synthetic packager；本change当前不修改代码仓。

## 测试影响

新增golden/negative package corpus、跨语言ABI vector、preflight未提交证明、PASS后阻断证明和no-stub构建检查。

## 兼容性

Vendor原生type 0～3保持兼容。Manifest v1不再提供major/minor或TLV兼容；`magic/header_size`不匹配直接拒绝，不兼容变化使用新magic。

## 安全分析

长度规范化消除尾随/截短多义；认证Manifest防止项目metadata被替换；generic verify不拥有release副作用。

## 未解决问题

OPEN-CONFLICT-006、OPEN-DESIGN-010/011继续按各自范围管理。ADR-0024已冻结精简Manifest布局并删除全部扩展机制。OPEN-CONFLICT-005/009已由ADR-0019关闭：唯一使用`rollback_counter[16]`并由FMC主动调用eHSM BL新增专用API；command/packing/LCS/交付版本是实现DoR。
