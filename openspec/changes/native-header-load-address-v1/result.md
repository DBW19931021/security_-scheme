# Result

- 设计结果：NGU Manifest删除，type 1 SoC包只使用1024字节Native Header和Code Region。
- 新字段：Header 1008的LE64 `load_addr`，1016的LE32 Header CRC32，1020的4字节零reserved。
- 长度/入口：`Code_Size`是唯一加载长度，entry固定等于load地址，CBC零对齐进入Measurement。
- 安全性：当前Vendor RSA-3072只消费384B扩展模数，OTP公钥Key ID不哈希原始400B扩展区；新字段不属于公钥输入但位于Vendor签名/CMAC范围。CRC-32/ISO-HDLC覆盖Header offset 256～1015并在preflight/PASS后双检，但不替代密码认证；load地址同时与stage固定目标exact-match。
- 兼容性：旧Manifest包和旧8字节零reserved包拒绝，不保留fallback；type 0 eHSM FW不变。
- 产品代码：未修改、未授权。
- 已同步：主详设、Package/Loader/公共ABI、RAM、Secure Boot、Anti-rollback、Measurement、BootROM/FMC/GSP、状态机、错误/OOB、基线、项目状态、Open Question、开发/验证计划和活动任务。
- 历史处理：`secure-package-manifest-v1`及早期评审包已标记`SUPERSEDED`或`historical_superseded_in_part`。
- 验证：`tools/scripts/project_check.py`通过frontmatter、Source ID、YAML、链接、追溯和任务结构检查；assumption检查保留16项既有warning。
