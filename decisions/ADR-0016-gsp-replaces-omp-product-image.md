# ADR-0016：GSP替代OMP/Q&P产品固件

- 状态：accepted
- 日期：2026-07-24
- 决策人：项目负责人（本次工作流确认）
- 相关 Source ID：SRC-0016、SRC-0017、SRC-0018、SRC-0022
- 相关 Requirement/Open Question：OPEN-CONFLICT-006
- 补充关系：补充ADR-0007、ADR-0013、ADR-0015

## 背景

现有公司代码仓包含OMP命名、linker和Q&P CPU历史实现痕迹，而当前安全软件目标启动链只定义BootROM、FMC、GSP及由GSP加载的eHSM Vendor FW、PMP、RMP、MMP。若把GSP和OMP继续视为两个独立产品固件，会同时引入额外RAM footprint、package/image type、Measurement条目、release顺序和运行Owner，但当前软件方案没有定义这些内容。

## 决策

1. GSP是旧工程中OMP/Q&P固件的产品替代，是Q&P CPU在NGU800P D0安全软件方案中的唯一产品固件身份。
2. OMP不再作为独立产品镜像参与新启动链，不分配独立：
   - Secure Package或NGU Manifest image type；
   - 安全RAM常驻Region或linker目标；
   - Measurement Table条目；
   - rollback/security epoch对象；
   - loader、release、Firewall profile或测试Expected。
3. 现有OMP源码、linker、构建说明和历史测试只能作为GSP迁移/复用调查输入，不得直接作为新D0产品输出或目标设计依据。
4. 需要复用的OMP功能必须按功能逐项进入GSP详细设计、目标模块和测试追溯；不得通过保留第二个OMP镜像绕过GSP的唯一eHSM Owner、统一Measurement、统一错误或发布门禁。
5. 后续若产品重新要求OMP与GSP同时存在，必须通过新的方案amendment、ADR和OpenSpec change重新打开RAM容量、启动链、身份、Measurement、counter、Firewall和发布设计，不能局部恢复旧linker。

## 对OPEN-CONFLICT-006的影响

关闭“GSP/OMP产品角色”子项。OPEN-CONFLICT-006仍保持开放，仅继续追踪：

- 新D0各stage的reset/release PC地址视图及唯一转换Owner；
- BootROM/FMC/GSP/PMP/RMP/MMP release footprint、stack/heap和最大package/plaintext；
- 2 MiB RAM的PMA/cache/coherence属性；
- Firewall实例、窗口、粒度、master ID、默认权限、lock/reset及readback；
- 据此冻结P1精确offset、linker、Manifest地址和Expected。

## 软件影响

- 产品registry只登记FMC、GSP、PMP、RMP、MMP及其他经批准对象，不登记OMP独立产品类型。
- GSP构建计划需要识别OMP历史代码中可复用的模块，但最终输出、版本、入口和运行Owner都归属GSP。
- 现有OMP linker不得作为新D0地址模板；最终GSP linker由冻结的2 MiB布局统一生成。

## 测试影响

- 启动链和发布测试不得等待、加载或release独立OMP镜像。
- Measurement、SPDM和版本清单不得出现独立OMP产品条目。
- 应增加负向检查：未登记的OMP identity/package不能进入loader或release。
- OMP历史功能若迁入GSP，按其实际GSP API、Feature和case重新追溯，不复用旧“OMP启动成功”作为产品Expected。

## 风险

历史源码、目录名或构建脚本可能继续出现OMP命名，容易被误解为产品镜像仍保留。实现阶段必须通过构建产物清单、registry和发布manifest检查消除双重身份；本ADR不授权现在修改代码仓。

## 参考资料

- `docs/03-architecture/security-ram-layout.md`
- `docs/05-software-design/gsp.md`
- `sources/conflict-reports/CONFLICT-SECURITY-RAM-TARGET-AND-CURRENT-LINKERS.md`
- `evidence/code-investigations/CE-SEC-010-ngu800p-ehsm-mmio-memory-view-pma.md`
