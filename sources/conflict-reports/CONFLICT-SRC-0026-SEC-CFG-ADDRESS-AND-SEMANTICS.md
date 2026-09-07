# SEC_CFG地址、窗口与未冻结语义冲突

- Open Question: OPEN-CONFLICT-015
- Sources: SRC-0026, SRC-0022, SRC-0018
- State: CONFLICTING / BLOCKED_BY_RTL_SYNC

## 冲突事实

1. SRC-0026截图称SEC_CFG为`0x1010_07B0_0000`、4 KiB；SRC-0022生成头定义`SECURITY_NOC_S6_SEC_CFG_BASE=0x1010_0820_0000`、16 KiB。
2. 当前baremetal把`SECURITY_EHSM_HSM_STATUS0_MAP_VALID=0`、地址为0，证明实现侧尚未接受任一status地址。
3. 截图未给`hsm_err_fw[63:0]`bit定义、LCS编码、debug scope/lock/合成、UID-valid和64-bit快照合同。
4. 截图提到的原始Excel/TRM/方案文件未随本次Source入库，无法验证其版本与目标D0匹配关系。

## 影响

- 阻断SEC_CFG MMIO base/size、生成头绑定、真实driver访问和确定性EMU/FPGA/硅Expected。
- 阻断Firmware error逐bit解码、生命周期编码API、普通软件debug写接口和UID稳定性PASS。
- 不阻断offset/bit的参数化数据模型、fake-MMIO单元测试、读取顺序和DV计划。

## 关闭条件

1. RTL/Integration Owner确认目标D0实例、decode和窗口，并同步SRC-0022生成头。
2. 提供匹配build的SEC_CFG寄存器生成源和readback Evidence。
3. eHSM BL/FW提供匹配build的`hsm_err_fw`定义；Security/eFuse提供LCS唯一编码。
4. Security/RTL冻结`dbg_en_cfg[1:0]`scope、写Owner、生命周期/lock和128-bit consumer矩阵。
5. RTL/DV提供跨32-bit读一致性、UID-valid、RO写行为及reset动态镜像Evidence。

## 当前决策

- 产品代码不得使用截图base，也不得继续使用地址0占位执行真实MMIO。
- 最终地址只从更新后的SRC-0022/后继权威生成源导入。
- `dbg_en_cfg`普通运行时写API禁止；Firmware error未定义时仅保存raw 64-bit。

