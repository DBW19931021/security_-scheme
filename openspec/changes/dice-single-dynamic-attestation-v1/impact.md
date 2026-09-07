# Impact

- 主详设第8/9/10/11章增加DICE一级动态证明。
- 证书管理从“三张静态Leaf链”改为“三张静态Issuer前缀+一张动态Leaf”。
- SPDM Slot0运行期拼接动态Leaf，签名Key改为当前Alias Key。
- GSP增加固定Profile X.509 writer、TCB Context、eHSM typed KDF-KeyGen和Report Provider。
- eHSM只增加/绑定基础算法typed接口，不负责X.509 DER组装。
- BootROM/FMC不新增证书栈；现有Measurement ABI不变。
- GSP SRAM峰值增加不超过12 KiB，Measurement Region无变化。
