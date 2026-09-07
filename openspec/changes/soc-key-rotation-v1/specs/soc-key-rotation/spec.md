# SoC Key Rotation Requirements

## Requirement: Supported rotation objects

系统必须且只能按本change轮换SoC Verify、Encrypt和Debug三类Key；每类只允许一次从原始位置切换到轮换位置。

### Scenario: Repeated rotation

- Given 指定`key_type`的Bitmap已经表示轮换完成
- When 再次请求该类型轮换
- Then eHSM必须拒绝
- And 不得修改任何Key slot、Bitmap或destroy状态

## Requirement: HSM-owned mapping

1字节OTP Bitmap和物理slot选择必须由eHSM内部管理。SoC侧不得通过产品API指定物理slot或raw bitmap值。

### Scenario: Raw slot request

- Given 调用方试图指定物理slot或raw bitmap
- When GSP typed service校验请求
- Then 请求必须在提交Vendor命令前被拒绝

## Requirement: Protected 48-byte envelope

新Key必须按SRC-0015形成固定48字节双层保护封装；GSP不得获得或记录明文Key。

### Scenario: Malformed envelope

- Given 输入长度、属性、CRC、`key_type`副本或padding不符合匹配Vendor合同
- When eHSM处理轮换命令
- Then 必须在写OTP前拒绝

## Requirement: USER authorization

USER生命周期轮换必须先通过设备绑定的Challenge-Response并取得eHSM批准的轮换权限。外部Host不得获得GSP通用Key/Rotation API。

### Scenario: Authorization absent

- Given 设备处于USER且没有有效轮换授权
- When GSP收到轮换请求
- Then 不得提交eHSM轮换命令
- And 不得打开raw Debug或OTP写通道

## Requirement: One-way commit

eHSM必须按写新Key、证明新Key、提交Bitmap、destroy旧Key的顺序执行；成功响应后由平台批准的reset使新映射生效。

### Scenario: Bitmap status unknown

- Given 命令在Bitmap提交窗口timeout或掉电
- When 软件无法证明Bitmap状态
- Then Key域和eHSM服务必须quarantine
- And 不得自动重试或回退旧Key

### Scenario: Post-destroy failure

- Given 旧Key已经destroy
- When reset、首次新Key启动或结果回报失败
- Then 系统必须fail-close并进入受限恢复
- And 不得恢复旧Key

## Requirement: Vendor delivery gate

产品实现必须使用包含专用轮换command、Host API、Bitmap/slot映射和失败语义的匹配Vendor定制交付；当前通用安装接口不得替代。
