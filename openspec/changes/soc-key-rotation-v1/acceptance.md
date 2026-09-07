# Acceptance

- 三类SoC Key、每类一次、1字节HSM管理Bitmap和boot/upgrade复用关系明确。
- 48字节双层封装的已知字段和未冻结参数分开记录。
- USER鉴权、GSP内部typed service和“不向外部Host开放Rotation服务”一致。
- 写新Key→证明→Bitmap→destroy旧Key→reset的单向顺序明确。
- 所有不可逆点都有fail-close、quarantine、禁止自动retry/rollback规则。
- 示例slot/bit不进入产品配置。
- 当前Vendor通用安装接口不被误用为轮换接口。
- 物理slot/bit、wire ABI、掉电证明、KMS托管和Vendor定制版本保持显式门禁。
- 当前阶段不修改代码仓、Vendor快照，不执行Git操作。
