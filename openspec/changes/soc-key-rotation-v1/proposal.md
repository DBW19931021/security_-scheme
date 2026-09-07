# Proposal

## 背景

NGU800P已批准SoC密钥轮换策略。负责人进一步指定轮换机制参考SRC-0015《云天励飞26Q2 定制需求方案》，需要把Vendor eHSM内部机制转换为不向外部Host开放、可指导GSP集成且显式保留不可逆风险的产品合同。

## 当前问题

- SRC-0015只登记了元数据，尚未进行页面级机制抽取。
- 主详设只有通用inactive/active建议，与SRC-0015“Bitmap提交后立即destroy旧Key、再reset”的顺序不完全一致。
- 当前SRC-0018没有定制轮换命令，通用安装接口还明确拒绝USER/DEBUG生命周期。
- 物理slot/bit、wire ABI、掉电原子性和KMS托管未冻结。

## 变更目标

- 冻结三类SoC Key、每类一次、1字节HSM管理Bitmap和双层48字节密文模型。
- 冻结USER鉴权、GSP内部typed service、eHSM内部写新Key→Bitmap→destroy旧Key→reset生效顺序。
- 明确OTP状态未知时不重试、不回退、quarantine和fail-close。
- 将当前Vendor交付缺口和剩余产品绑定变为编码准入门禁。

## 非目标

- 不把PDF示例slot 9～14或示例bit位置写成产品物理配置。
- 不猜测Vendor定制command ID、结构体、密码mode/IV、CRC/属性编码或错误码。
- 不修改`gsp-pmp-rmp-omp`、`baremetal`或Vendor source-vault。
- 不授权产品编码、真实OTP操作或Git操作。

## 审批状态

2026-07-27，项目负责人指定密钥轮换机制参考SRC-0015。本change设计层接受；ADR-0026后来关闭`OPEN-DESIGN-014`的软件设计裁决，implementation仍因Vendor/KMS/Bitmap实施绑定未授权。
