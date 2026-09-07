# SRC-0026 SEC_CFG入库报告

## 入库结果

- 原样保存12张PNG；第9/10张内容重复但均保留，避免改写用户输入。
- 建立完整文字转录、机读寄存器表、接口设计、测试设计和正式报告。
- 截图引用的四份上游资料未随本次输入提供，因此只登记为待补来源线索。

## 可进入设计的内容

- 17个寄存器的offset、拼接关系、RO/RW属性和截图reset。
- status与hardware error bit表、UID/debug输出拼接以及SEC_CFG/Firewall职责边界。
- error-first/done-second、timeout、raw采集、readback和fail-close软件原则。

## 编码停止项

- 地址常量、窗口大小和绝对地址：与SRC-0022冲突。
- Firmware error解码、LCS编码、debug写策略/lock、UID有效时刻和64-bit原子保证。
- 未获得目标D0 RTL/EMU/FPGA/硅Evidence前，不标记CONFIRMED或测试PASS。

