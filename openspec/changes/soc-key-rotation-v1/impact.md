# Impact

## 硬件依赖

依赖eHSM OTP双位置、1字节Bitmap、destroy能力、USER鉴权状态、硬件reset和掉电可观测性；真实绑定待Vendor定制交付。

## 软件影响

未来GSP增加内部typed rotation service；当前不编码。BootROM/FMC只消费eHSM上电后的有效映射，不直接写Key/Bitmap。

## 接口影响

需要Vendor新增专用Mailbox command和Host API。现有`INSTALL_RANDOM_KEY/INSTALL_ENCRYPT_KEY`不兼容。

## 兼容性

每类只轮换一次；完成后不能回到原Key。证书轮换继续走Flash机制，不受OTP Bitmap协议替代。

## 量产与运维

外部KMS必须具备设备级密钥托管、双层封装、授权审计和恢复能力；具体custody/recipe仍开放。

## 验证与 Evidence

需要TEST/DEV/MANU/USER授权矩阵、错误封装、重复轮换、每个OTP/Bitmap/destroy掉电点、reset失败、首次新Key验证失败和敏感日志扫描。

## 风险和回退

OTP和destroy不可逆，不存在软件回退。输入或状态未知时只能停止、隔离并等待权威查询/受限恢复。
