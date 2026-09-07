# Design

## 系统角色

- 外部管理系统：生成新Key并完成SRC-0015规定的双层密文封装。
- GSP `security_service_task`：唯一SoC侧eHSM Owner，执行typed授权和Mailbox调用，不解密新Key。
- eHSM定制FW：校验授权、解析封装、选择内部轮换slot、写Key、提交Bitmap并destroy旧Key。
- RAS/reset Owner：执行GSP提出的平台批准eHSM reset请求。

SRC-0015中的`Host`映射为上述SoC侧GSP/eHSM Host角色，不产生外部Host通用Rotation API。

## 对象与状态

`key_type=0/1/2`分别表示SoC Verify/Encrypt/Debug Key；每类只有原始和轮换两个逻辑位置，只允许一次单向切换。1字节Bitmap由eHSM内部管理；真实bit和slot不进入SoC业务ABI。

## 封装

`ciphertext_A = Protect_DEVICE_ROOT_KEY(new_key[32])`

`payload48 = key_attributes[4] || ciphertext_A[32] || crc32(new_key)[4] || key_type[4] || padding[4]`

`ciphertext_B = Protect_RTL_SOC_KEK(payload48)`

外部命令是否再次携带`key_type`以及如何与加密副本做一致性校验，由Vendor定制wire ABI冻结；任何双副本不一致必须fail-close。

## 状态机

`AUTHORIZED -> ENVELOPE_ACCEPTED -> NEW_SLOT_PROGRAMMED -> NEW_SLOT_PROVED -> BITMAP_COMMITTED -> OLD_KEY_DESTROYED -> RESPONSE_SUCCESS -> RESET_REQUESTED -> REBOOT_LOADS_NEW_KEY -> OPERATIONAL_PROOF`

Vendor内部动作在`RESPONSE_SUCCESS`前完成。reset失败不回滚OTP状态；首次新Key运行证明失败进入受限恢复。

## 失败和掉电

- 新slot写入前失败：旧Key保持active。
- 新slot写入状态未知：旧Key仍为逻辑active，但新slot标记unknown/consumed，不自动重写。
- Bitmap提交状态未知：Key域和eHSM服务quarantine，不选择旧/新Key。
- Bitmap确认后失败：新Key保持active，禁止自动回旧Key。
- destroy后失败：旧Key不可恢复，只能fail-close。

精确原子粒度、状态查询、幂等性和各掉电点恢复证明必须由匹配Vendor交付和EMU/故障注入Evidence提供。

## 证书轮换

证书不使用OTP Bitmap机制，按ADR-0026使用Flash Cert0/Cert1、commit-last和扫描最大有效sequence，不设置active pointer。

## 未解决问题

ADR-0026已关闭`OPEN-DESIGN-014`的软件设计裁决。真实Vendor command/ABI/status/query、Bitmap/掉电原子性、密码封装参数、KMS密钥托管、一次性operation授权格式、旧Key destroy恢复、寿命和可执行制造recipe继续作为实施绑定；该授权不包含Debug scope，Debug不存在scope。
