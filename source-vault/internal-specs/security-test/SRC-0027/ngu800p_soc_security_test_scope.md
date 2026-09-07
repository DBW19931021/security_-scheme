# NGU800P SoC 安全测试范围与错误中断补充输入

## 1. 输入范围

- 日期：2026-08-13
- 提供方：项目负责人
- 适用对象：NGU800P D0 SoC 安全方案相关的 SEC_CFG、Firewall、eHSM Mailbox IRQ 和 eHSM 错误上报测试设计。
- 边界：只验证安全方案使用到的功能，不扩展为 Firewall、Debug 或中断 IP 的完整功能验证。

## 2. SEC_CFG 与 Debug 测试口径

1. `SOC-SECCFG-001`作为单一软件用例遍历并查询全部17个32-bit SEC_CFG寄存器，按场景期望表比较；不按寄存器拆分用例或参数向量。
2. `dbg_en_cfg`和`soc_dbg_en_out`承担不同职责：
   - `dbg_en_cfg`可读写，由主Die控制从Die Debug开关。默认必须关闭；主Die写入打开前从Die Debug通道不可联通，打开后应联通，恢复关闭后再次不可联通。
   - `soc_dbg_en_out`是eHSM Mailbox Debug鉴权后的输出状态。鉴权成功时必须同步检查其值和实际Debug访问；鉴权失败时保持关闭；`CLOSE_DEBUG`后实际访问恢复拒绝且状态和最终输出同步关闭。
3. `dbg_en_cfg`由`SOC-SECCFG-DBG-001`覆盖；`soc_dbg_en_out`加入BL/FW Debug Auth、负向鉴权和`CLOSE_DEBUG`用例判据，不将两个寄存器合并成同一功能语义。

## 3. Firewall 测试口径

1. SEC_CFG和SPIFC Firewall默认只允许启动核和eHSM读写，其他核均无权限；这两个Firewall只验证默认权限，不修改Firewall配置。
2. SRAM Firewall保留两个软件用例：
   - 默认权限用例：在同一用例内遍历Region0～Region4，逐个验证启动核/eHSM可读写、其他核不可读写。
   - 重新配置用例：在同一用例内遍历Region0～Region4，逐个修改地址范围和允许通过的Master ID，验证新增授权核可访问、未授权核和范围外地址仍被阻断，并恢复默认配置。
3. 五个Region只作为同一用例中的循环步骤和记录项，不拆成五条独立用例。

## 4. eHSM Mailbox IRQ

- 建立一条SoC软件用例，在用例内遍历适用Mailbox通道，验证真实eHSM响应产生的中断路由、屏蔽、清除和再次触发。
- 不使用APLIC软件置位、C908自写Mailbox或固件测试钩子伪造eHSM中断。

## 5. 错误上报与EDA边界

### 5.1 严重错误组合中断

截图中以下`o_hsm_err_hw`错误源组合为一根送往SoC的严重错误中断线：

- `wdt_timeout`；
- `otp_key_crc_err`；
- `hw_trng_retry_warning`、`hw_trng_retry_fail`、`hw_trng_ht_fail`；
- `mem_ecc_mb_pke3/2/1/0`、`mem_ecc_mb_kmu`、`mem_ecc_mb_dram`、`mem_ecc_mb_iram`、`mem_ecc_mb_irom`不可纠正错误。

正常软件接口不能稳定、无副作用地逐源构造上述硬件故障，因此激励必须由RTL/DV/EDA故障注入提供。软件可以作为观察者读取错误寄存器、记录中断和执行批准的清除流程，但不得把软件业务错误或伪中断当作硬件故障覆盖。

### 5.2 ECC 1-bit独立中断

- `mem_ecc_1b_*`可纠正错误从电平改为脉冲信号，单独形成另一根送往SoC的中断线。
- SoC外部集成逻辑需要把脉冲转换为可供软件查询和清除的状态，并由外部计数逻辑统计发生次数。
- EDA必须逐RAM注入、检查脉冲、查询状态、中断、清除、再次触发和计数；软件只负责读取/清除和证据采集。
- 计数器宽度、饱和或回绕、读清/写清关系和复位语义仍需RTL/CSR冻结，未冻结前相关子判据为`INCONCLUSIVE`。

## 6. 原始图片

- `01-hsm-err-hw-bits.jpg`：`o_hsm_err_hw`字段及严重错误/ECC分类截图。
- `02-ecc1b-pulse-and-external-count.png`：ECC 1-bit脉冲化、外部电平化和计数要求截图。

