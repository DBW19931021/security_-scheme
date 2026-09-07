# INV-SEC-011：Vendor Version Counter与通用Counter合同

> 2026-07-27后续裁决：本文件保留2026-07-24调查时使用的`security_epoch/global epoch`历史术语和“绑定延期”结论。ADR-0019已将产品字段统一为`rollback_counter[16]`，关闭OPEN-CONFLICT-005/009，并要求FMC主动调用eHSM BL新增专用update/readback API；Vendor FW启动副作用和Host通用64位Counter均不得作为产品实现路径。

## 调查背景

- 对应Feature：SEC-FEAT-003、006、011、019。
- 对应正式详设：`docs/03-architecture/anti-rollback.md`、`docs/04-interfaces/measurement-table.md`、`docs/05-software-design/NGU800P安全软件详细设计.md`第9章。
- 触发问题：OPEN-CONFLICT-005已经冻结单一16字节global `security_epoch`、GSP package锚点、FMC唯一更新和其他镜像匹配语义，但Vendor的字节序、物理资源、compare/update/readback命令、未烧写值、寿命和timeout状态未知恢复仍未收敛。
- 目标资料/代码：SRC-0018中的eHSM BL、eHSM FW、Host、counter service/driver、OTP map及匹配测试；必要时对照Vendor文档。
- 只读基线：Vendor快照整体只读；按项目硬约束不执行任何Git命令。

## 调查目标

区分并分别建立：

1. Vendor安全启动`version counter`的16字节Header/OTP/DRAM传递、比较和持久化合同；
2. Vendor运行期`create/read/increase counter`服务的命令、索引、宽度、访问控制和寿命合同；
3. 两者是否是同一物理资源和同一Host API，能否支持NGU800P由FMC负责的global `security_epoch` compare/update/readback流程。

## 需要回答的问题

1. 16字节version counter在Header、BL内存和OTP中的octet顺序是什么；Vendor比较函数按何种大数顺序解释？
2. BL验证eHSM FW与SoC FW时分别读取哪个物理counter、何时只比较、何时把候选值暂存在DRAM？
3. eHSM FW启动时何时把暂存值写入OTP，是否有写后回读、重复写、掉电恢复和错误状态？
4. Host通用counter service的command ID、request/response结构、counter数量、index/ID、位宽和increment规则是什么？
5. 通用counter与安全启动version counter是否共享物理资源；若不共享，FMC如何合法触发global epoch更新？
6. 未烧写值、最大值/耗尽、寿命、权限/LCS、并发、timeout/unknown completion分别如何处理？
7. 哪些内容可由当前Vendor代码冻结，哪些仍需Vendor或项目负责人裁决？

## 建议检查范围

- `ehsm_bl-*/src/component/fw_verify.c`、`otp_data.*`、`driver/otp.*`及对应Header。
- `ehsm_fw-*/src/secboot.c`、`src/service/counter_srv.*`、`src/driver/counter_driver.*`、OTP map/driver。
- `ehsm_host-*/src/mb.h`、公共API和`fw_demo` counter case。
- 匹配的Python firmware/bootloader tests、配置和OTP layout；历史review只作候选线索。
- 必须记录文件、函数/宏、紧凑行范围和调用链，区分version counter与通用counter。

## 输出

- 代码证据：`evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md`。
- 回填OPEN-CONFLICT-005、防回滚、Measurement/FMC状态机、完整主详设第9章和测试方向。
- 如发现“已批准global epoch流程无法由当前Vendor接口实现”或Vendor资料内部冲突，建立独立conflict report并在对话中提交负责人裁决。

## 限制和停止条件

- 只修改`security_-scheme`调查和设计资料。
- 不修改Vendor快照、`gsp-pmp-rmp-omp`或`baremetal`，不执行Git、构建或测试。
- 不把通用counter服务自动等同于安全启动version counter。
- 不从模拟OTP、Demo Expected或历史review冻结产品物理资源和寿命。
- 不因Vendor FW内部能自动更新version counter，就覆盖已批准的“FMC唯一更新global security_epoch”产品Owner语义；若不兼容必须升级冲突。

## 验收

- [x] 16字节version counter的字段顺序、比较、暂存和持久化调用链可复核。
- [x] 通用counter命令和物理/逻辑资源边界可复核。
- [x] NGU800P global epoch流程的可复用项、缺口和冲突明确。

## 调查结果

- 已形成`evidence/code-investigations/CE-SEC-011-vendor-version-counter-contract.md`。
- 已确认BL成功verify只暂存候选值，eHSM FW启动才提交OTP；`check_version=OFF`不能关闭BL候选暂存。
- 已确认Vendor写函数内部执行写后回读，但产品LCS缺少Host可用的确切version-counter readback合同。
- 已确认Host 64位通用counter API不等同于16字节version counter，且匹配FW快照无实现/dispatch。
- 已建立OPEN-CONFLICT-009；该项在调查完成时曾按“默认存在FMC接口、具体绑定延期”继续详设，后续已由ADR-0019关闭并改为eHSM BL新增FMC专用API。
- [x] OPEN-CONFLICT-005和完整主详设同步。
- [x] 未修改两个代码仓或Vendor快照，未执行Git命令。
