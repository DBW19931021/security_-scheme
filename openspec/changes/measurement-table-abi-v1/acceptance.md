# Acceptance

- Header、Firmware Entry和唯一SoC State均有128字节逐字段offset表。
- `fw_entry_count`和`total_len`表达实际紧凑Firmware列表，并有容量/溢出检查。
- `state_entry_count`和`generation`已删除；reset依赖Header先失效和完整Region清零。
- BootROM作为隐式可信测量根，不形成普通Entry。
- State不包含eHSM状态或时间戳。
- Firmware不包含Key/Signer、算法Profile或地址domain；地址统一为64位`SOC_PA`。
- 每个结构只保留一个reserved。
- CRC-32C和32位commit具有确定Writer/Reader顺序。
- State提交后Table final，不允许继续append。
- SPDM前后Header snapshot不会返回半写内容。
- eHSM FW使用Vendor认证package Hash；SoC固件使用loader readback Hash。
- OpenSpec、接口详设、主详设、open question、Feature和任务状态一致。
- 当前不修改代码仓、Vendor快照、测试工作簿或Git状态。
