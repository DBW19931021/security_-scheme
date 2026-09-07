# Impact

## 硬件依赖

不改变硬件。真实实现依赖C908对齐32位store可见性、SoC稳定后从`NON_CACHEABLE/HARDWARE_COHERENT`中选定的唯一PMA属性、固定Measurement Region和Firewall/PMP权限Evidence；裁决前保持`BLOCKED_BY_PMA_INPUT`。

## 软件影响

- BootROM/FMC/GSP共享可变长度ABI和顺序append协议。
- BootROM不生成自身普通Entry。
- FMC按类型/实例查找自身Entry，不使用旧固定slot。
- GSP按实际独立实例追加Runtime并以唯一SoC State结束Table。
- 旧BSS私有表、固定8/16 slot、32位counter、地址domain和可覆盖Entry不能进入产品路径。

## 接口影响

ADR-0024已删除Manifest `measurement_slot`。Entry身份由producer按`fw_type + die_id + instance_id`生成；新增Header/Entry builder、append/commit、consumer和snapshot接口。

## 兼容性

旧4224B及固定1280B候选均未授权编码，无产品兼容负担。v1字段或大小的破坏性变化必须提升版本。

## 量产与运维

无新密钥、OTP或时间源。Key轮换Bitmap和eHSM状态留在各自管理/审计域。

## 风险和回退

平台可见性、最大实例容量或物理Region未证明时保持实现blocked；不得退回stub、无CRC、delay-based提交或伪时间戳。
