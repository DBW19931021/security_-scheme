# Acceptance

- `non_sec_boot=0`时既有Lifecycle×`secure_boot`全部Expected保持不变。
- `non_sec_boot=1`时所有LCS/Strap组合均进入现有受限非安全路径，包括USER。
- 强制分支不等待eHSM，不执行安全FMC验证/解密、counter、Measurement、SoC State或安全启动审计。
- `non_sec_boot`读取失败、ECC异常、来源无效、锁存未完成或镜像不一致时，两条FMC release路径均不可达。
- BootROM无raw eFuse offset读写接口；Host和运行期软件不能覆盖快照。
- 非安全Profile缺失/非法时终止，不回退安全启动。
- 产品代码不含临时eFuse地址、裸bit或猜测编码；未绑定时明确`BLOCKED_BY_NON_SEC_BOOT_BINDING`。
- 0→1烧写、写后readback、不可逆性和下次BootROM重锁存具有测试要求。
- 主详设、ADR、OpenSpec、接口专题、状态机、需求和项目状态一致。
