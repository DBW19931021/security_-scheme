# SRC-0015 SoC Key轮换目标与当前Vendor代码交付差距

## Identity

- Conflict ID: CONFLICT-SRC-0015-CURRENT-VENDOR-KEY-ROTATION
- Open Question: OPEN-CONFLICT-011
- Status: BLOCKED_BY_VENDOR_DELIVERY
- Evidence state: CONFLICTING
- Owner: Vendor接口Owner；eHSM交付Owner；GSP Key/Provisioning Owner
- Related: SRC-0015、SRC-0018、ADR-0021、CE-SEC-013、OPEN-DESIGN-014

## Conflict classification

- Type: accepted_customization_target_vs_current_vendor_delivery
- Affected scope: USER态SoC Verify/Encrypt/Debug Key轮换、Active Bitmap、专用Mailbox、旧Key destroy、reset后新Key加载
- Safe-to-continue scope: 逻辑详设、OpenSpec、KMS/制造输入清单、非破坏性Host测试设计
- Must-stop scope: GSP产品轮换实现、真实OTP写入、Bitmap/destroy测试和任何把通用安装命令当作轮换命令的适配

## Accepted target

ADR-0021依据SRC-0015冻结：

1. 三类SoC Key，每类只轮换一次；
2. 1字节Bitmap和物理slot由eHSM内部管理；
3. USER鉴权后调用专用Mailbox；
4. 固定48字节双层密文；
5. eHSM执行写新Key→证明→Bitmap→destroy旧Key；
6. 成功后请求eHSM硬件reset并加载新Key。

## Current Vendor delivery

CE-SEC-013确认SRC-0018：

1. Host/FW只有通用`INSTALL_RANDOM_KEY`和`INSTALL_ENCRYPT_KEY`；
2. 通用命令由caller显式指定物理`key_slot_id`；
3. 当前FW在USER和DEBUG生命周期明确返回`EHSM_ERR_EHSM_LIFECYCLE_LIMIT`；
4. 未发现专用轮换command、1字节Active Bitmap、每类一次门禁、USER鉴权后轮换或旧Key destroy流程；
5. BL/FW对`0xff08/0xff09`还有阶段相关含义，NGU800P不能自行猜测新command ID。

## Exact conflict

当前代码交付无法实现已接受的SRC-0015轮换目标。用通用安装API指定备用slot会同时破坏HSM内部资源Owner、USER生命周期权限和Bitmap/destroy原子流程，因此不是可接受的临时实现。

## Required resolution

Vendor需提供匹配的定制交付及release note，至少包含：

- eHSM BL/FW/Host API准确版本和互相兼容关系；
- 专用command ID、request/response布局、`key_type`语义和raw status/error；
- 1字节Bitmap真实bit、三类Key原始/轮换slot和上电映射；
- USER一次性operation鉴权、重复轮换拒绝和状态查询；
- 48字节封装精确算法/mode/IV、属性/CRC/padding/字节序；
- 新Key写入证明、Bitmap原子提交、destroy及各power-cut point恢复；
- reset后新Key加载和首次使用证明；
- 寿命/耗尽及制造/KMS recipe输入。

## Implementation gate

ADR-0021和OpenSpec `soc-key-rotation-v1`保持设计批准；implementation固定为`BLOCKED_BY_VENDOR_DELIVERY`。在匹配Vendor Host/BL/FW、专用command、release note到齐并通过只读diff、Host ABI测试和可恢复EMU故障注入前，不创建产品轮换编码任务，不使用现有通用安装接口模拟成功。

## Review history

- 2026-07-28：负责人确认云天定制需求为当前目标设计；后续Vendor Host/BL/FW按方案更新，当前实现统一标记`BLOCKED_BY_VENDOR_DELIVERY`。
- 2026-07-27：SRC-0015全文/逐页复核和SRC-0018只读搜索后建立；设计目标已由ADR-0021冻结，当前实现交付差距保持开放。
