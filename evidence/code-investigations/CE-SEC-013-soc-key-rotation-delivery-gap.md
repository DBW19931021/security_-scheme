# CE-SEC-013：SRC-0015 SoC Key轮换机制与当前Vendor交付差距

## Evidence metadata

- Evidence ID：CE-SEC-013
- 日期：2026-07-27
- 父任务：TASK-SEC-SOC-FW-001 / DD-04 Key、Certificate、Rotation
- 资料基线：SRC-0015 PDF；SRC-0018 `ehsm_bl-2.3.5-4019-72f8fdc`、`ehsm_fw-2.3.2-4019-5a4a0a9`、`ehsm_host-2.3.1-4019-2ee044d`
- 适用范围：NGU800P SoC Verify/Encrypt/Debug Key轮换
- 操作：PDF全文抽取与逐页视觉核对；Vendor代码只读搜索和函数/结构体复核；未修改Vendor、两个代码仓，未构建/测试，未执行Git命令

## FACT-01：SRC-0015的目标轮换模型

SRC-0015第1～5页明确：

1. 轮换对象为SoC Verify、Encrypt、Debug三类Key，每类仅轮换一次。
2. eHSM内部管理1字节OTP Bitmap，每bit控制一类Key的原始/轮换状态。
3. USER下通过SoC Challenge-Response鉴权并打开批准权限；SoC调用新增专用Mailbox命令。
4. 外部管理系统形成48字节双层密文；eHSM执行解密/校验、写新Key、更新Bitmap、destroy旧Key。
5. SoC收到成功后硬件复位eHSM；eHSM上电读Bitmap并使用新Key。
6. 文档中的slot 9～14和具体映射均为示例，真实排布由客户决定。

## FACT-02：当前Host/FW只有通用安装接口

当前Host `src/mb.h`定义：

- `MB_CMD_ID_INSTALL_RANDOM_KEY = 0xff08`；
- `MB_CMD_ID_INSTALL_ENCRYPT_KEY = 0xff09`；
- 命令由调用方显式传入`key_slot_id`；
- encrypted install输入为固定48字节，描述为“32字节Key或公钥Hash + 4字节CRC + padding，经RTL KEK加密”。

当前Host `src/api.c`的`ehsm_install_random_key()`和`ehsm_install_encrypted_key()`只是组装以上通用命令；没有轮换对象枚举、Active Bitmap或HSM内部slot选择接口。

## FACT-03：当前FW通用安装接口不能用于USER轮换

当前FW `src/service/otpkinstl_srv.c`：

- `otpkinstl_check_life_cycle()`明确在`USER`和`DEBUG`生命周期返回`EHSM_ERR_EHSM_LIFECYCLE_LIMIT`；
- service handler只分派`INSTALL_RANDOM_KEY`和`INSTALL_ENCRYPT_KEY`；
- 未发现SRC-0015所述专用轮换命令、1字节Active Bitmap、每类一次轮换、USER鉴权后写入或旧Key destroy流程。

因此，当前通用安装接口不是SRC-0015轮换机制的兼容实现，不能由GSP通过指定“备用slot”模拟轮换。

## FACT-04：BL/FW命令ID命名空间不能猜测复用

当前eHSM BL中`0xff08/0xff09`分别表示`BL_GET_RANDOM_KEY`和`BL_ENCRYPT_KEY`，Vendor FW/Host中同值用于安装接口。新增轮换命令的执行阶段、command ID、结构体和状态码必须由Vendor定制交付明确，项目不能自行占用或复用现有ID。

## 设计回填

- 接受ADR-0021，把SRC-0015的轮换对象、资源模型、封装、鉴权、单向顺序和reset生效写入主详设。
- ADR-0026已关闭`OPEN-DESIGN-014`的软件设计裁决；Vendor定制command/Host API/BL/FW版本、Bitmap/掉电原子性、旧Key destroy恢复和KMS托管继续作为实现准入门禁。
- 当前SRC-0018仍可作为通用OTP安装、48字节输入和生命周期限制的`VENDOR_IMPLEMENTATION`证据，但不得作为产品轮换实现Evidence。
