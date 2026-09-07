# NGU800P DICE风格一级动态证书输入

- Source ID：SRC-0032
- 日期：2026-08-19
- Owner：项目负责人
- 状态：用户确认的设计输入

## 用户确认内容

1. 在既有安全启动和证书方案中补充DICE风格动态证书能力。
2. BootROM阶段不生成证书，只保存实际加载FMC的Hash值。
3. FMC阶段不生成证书，只保存实际加载GSP的Hash值。
4. 最终由GSP阶段生成一级动态证书。
5. 功能形态参考用户提供截图：设备从UDS和启动测量形成KDF上下文，经eHSM派生证明密钥；GSP组装动态X.509证书；eHSM完成Hash、KDF、Key和Sign基础能力；Host验证证书、Measurement和签名报告。

## 截图功能转录

截图展示的功能元素包括：

- Flash中的`root_x509.crt`；
- eFuse中的`device_prv_key`和`UDS`；
- PCR/SRAM中的固件/Boot Hash、Lifecycle和Debug状态；
- `eHSM.hash`、`eHSM.kdf`、`eHSM.pke`和`eHSM.sign`；
- 动态生成的`fw_x509.crt`；
- X.509、Measurement Table、Measurement签名和状态组成的Report；
- Host侧证书和Report验证。

截图不作为准确Key槽、算法ID、证书OID/DN、地址、结构offset、eHSM command或SPDM wire编码的权威来源；这些内容必须由NGU800P受控设计定义。

## 与当前设计的边界

- 继续使用现有Measurement Table：BootROM生产FMC Entry，FMC生产GSP Entry；“只保存Hash”指DICE派生链的阶段贡献，不删除既有Entry中release/counter/load审计元数据。
- eHSM只承担不导出密钥的Hash/KDF/Key派生/签名基础能力；最终X.509 DER组装和Report组织由SoC侧GSP完成。
- 动态证书和证书工作缓冲不得使用固定16 KiB Measurement Region，应计入GSP运行峰值。
- 本方案目标是对外支持“DICE风格一级动态证明证书链”，不自动声明完整符合TCG DICE全部Profile。
