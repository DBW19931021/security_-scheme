# TASK-SEC-EMU-PREP-001：EMU 前安全方案与测试就绪

## TASK BRIEF

- Task ID：TASK-SEC-EMU-PREP-001
- 角色：两个顶层任务共用的 EMU 准备与阶段门禁记录，不是独立顶层任务。
- 状态：active
- 背景：预计约 4 周后具备 C908 + eHSM 的 EMU 环境，需要在此之前完成软件方案细节和测试执行准备。
- 当前问题：方案 Feature 尚未全部转化为可实现 ABI/状态机/权限/错误契约；公司代码仍有 demo/stub；100 条用例缺少可排程的优先级和执行元数据。
- 目标：由 GSP 在 `security_-scheme` 形成可完全指导开发的正式安全软件详设、任务规划、测试规划/工作簿和开发追溯，并在 `gsp-pmp-rmp-omp` 落实 `SEC-FEAT-001`～`SEC-FEAT-020` 的正式代码，在 `baremetal` 落实可执行测试，使 EMU 到位后可直接执行。
- 非目标：本任务当前计划修订阶段不修改两个公司仓库、不运行安全测试、不证明硬件已通过、不生成 Git 提交。
- 已确认事实：SRC-0017 是系统/架构上位方案；SRC-0016 是软件落地下位方案；Vendor 仅指 eHSM/Core；密钥轮换策略已批准但细节以软件方案为准。
- 假设：EMU 在约 4 周后首次可用；具体日期、版本和能力仍待 EMU 团队确认。
- 输入资料：SRC-0001～SRC-0022、ADR-0001～0016、CE-SEC-001～011、已关闭OPEN-CONFLICT-001～004/007/008、开放OPEN-CONFLICT-005/006/009、公司软件仓库安全组件。
- 相关设计：`docs/05-software-design/NGU800P安全软件详细设计.md`、`docs/09-plans/SECURITY-FEATURE-REALIZATION-MATRIX.md`、`docs/09-plans/EMU前安全方案落实工作计划.md`、`docs/06-verification/EMU-TEST-READINESS-PLAN.md`。
- 目标仓库：正式详设、全部任务、测试策略/计划/工作簿、开发追溯和 Evidence 索引为 `security_-scheme`；正式软件实现为 `../gsp-pmp-rmp-omp`；可执行测试/EMU runner/tools 为 `../baremetal`；三个部分均由 GSP 负责。
- 相关代码：`gsp-pmp-rmp-omp/components/security` 和必要 BootROM/FMC/GSP glue；`baremetal/components/ngu_security/tests`、`tools` 和必要 EMU solution glue。
- 风险：Owner/版本不明、算法/SPDM profile和物理参数待定、100条全部P0、EMU能力未知、公司仓库已有未提交修改。
- 待确认事项：Feature Owner、三套已批准算法到设备/镜像/key/board/LCS的provisioning绑定、EMU baseline/capability、不可逆测试政策、允许修改的代码文件范围。

## TASK PLAN

- 基线仓库/当前分支/只读 commit：
  - `security_-scheme` / `master` / `acaf0527e735caf795f831e8341148b3c78bc11d`（来自 `manifests/repositories.yaml`，本任务未执行 Git 命令复核）。
  - `gsp-pmp-rmp-omp` / `master` / `08b29c7b7a29ee9478c0c01d4d708beb0b77b5d9`（来自清单；工作区已知存在既有修改）。
  - `baremetal` / `UNKNOWN` / `UNKNOWN`（遵守本轮无 Git 操作约束，尚未查询；写入任务前必须登记）。
- 工作方式：允许默认分支工作区直接修改；不创建功能分支；不执行 `git add`、`git commit`、`git push`、merge、rebase 或 tag。
- 当前修改范围：仅 `security_-scheme` 中的仓库分工、计划、Feature 矩阵、测试 readiness、任务、状态、测试索引和变更记录。
- 当前禁止修改范围：`../gsp-pmp-rmp-omp`、`../baremetal`、`source-vault/vendor-code/osr_eshm`、现有 v0.2 工作簿、任何 `.git` 内容。
- 后续修改范围：正式详设、项目任务和测试规划继续在 `security_-scheme` 修改；只有在单独批准的 OpenSpec/实施任务中，才可由 GSP 在 `gsp-pmp-rmp-omp/components/security/` 修改正式实现，或在 `baremetal/components/ngu_security/` 编写可执行 case/runner/tools。每个子任务单独列文件和既有修改保护方式，两个代码仓不新增项目级主计划。
- 分阶段实现计划：
  1. Feature/Owner/Source/code/case gap 盘点。
  2. 在 `security_-scheme/docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 建立主详设/专题详设，完成冲突、算法 profile、接口/状态机/权限/错误规则裁决。
  3. 详设、requirements/OpenSpec/amendment 与测试 oracle 冻结。
  4. 在 `gsp-pmp-rmp-omp` 实现 production provider/adapter 和必要测试钩子。
  5. 在 `security_-scheme/tests/cases/` 生成 v0.3/v0.4，在 `baremetal` 实现由最新工作簿派生的可执行case、EMU runner、工具和输入包；Host测试向量不得模拟真实eHSM成功或进入EMU/产品路径；在 `security_-scheme` 维护测试追踪和 Evidence 索引。
  6. EMU readiness review 和 Wave 0 接入。
- 每阶段验证方法：文档检查、Source/Requirement/Feature/case 追踪、Host/unit/QEMU/Vendor sim（适用时）、EMU dry-run/执行（环境到位后）。
- 预期文件：见三份主计划；每个后续行为变化另建 OpenSpec change/task/result/evidence。
- 失败停止条件：发现方案/代码/Vendor/实测冲突；无法保护既有未提交修改；EMU/产品路径存在stub、simulated success、test provider、未批准hardcode或silent fallback；不可逆测试无批准；环境版本不可追踪。
- 回退方案：文档变更保留为未提交工作区 diff；实现任务通过小范围可审查修改回退，不执行 `git reset`/`git clean`；硬件冲突范围保持 `CONFLICTING` 并停止确定性验收。

## 里程碑

- [ ] M1 / T-4：20 个 Feature 分配方案、软件、RTL/eHSM、测试 Owner。
- [x] M2 / T-4：OPEN-CONFLICT-001/002/003已关闭；Secure Package三套算法Profile已由ADR-0017冻结，具体provisioning绑定和SPDM profile继续在专题详设收敛。
- [ ] M3 / T-3：`security_-scheme/docs/03-architecture/`、`docs/04-interfaces/`、`docs/05-software-design/` 的正式详设覆盖 20 个 Feature，ABI/状态机/权限/错误/oracle 完成评审，可完全指导开发。
- [ ] M4 / T-3：形成 accepted amendment/requirements/OpenSpec，并由 GSP 在 `security_-scheme/tests/cases/` 形成 v0.3 测试工作簿，在 `baremetal` 形成对应可执行 case。
- [ ] M5 / T-2：关键 Feature 达到 C2；Host/QEMU/Vendor sim 前置回归和测试资产就绪。
- [ ] M6 / T-1：v0.4、EMU runner、软件/输入包、环境清单、Evidence 模板完成 dry-run。
- [ ] M7 / T0：Wave 0 冒烟完成并形成 Go/No-Go 结论。

## TASK ACCEPTANCE

- [ ] 20 个 Feature 均进入 `security_-scheme` 正式详设，并有 Source、Owner、`gsp-pmp-rmp-omp` 模块/文件/API、工作簿 case、`baremetal` 可执行入口和完成标准；开发不需要依赖聊天记录补全行为。
- [x] OPEN-CONFLICT-001/002/003均已裁决并登记ADR-0003/0004。
- [ ] OPEN-CONFLICT-006剩余项已裁决：C908 PC/linker地址视图、各Region冻结容量、PMA、最终linker和Firewall参数均已冻结；GSP/OMP关系已由ADR-0016关闭。
- [ ] 所有 P0-Blocker 具有可机器/人工复核的 oracle、输入、命令、环境和 Evidence 路径。
- [ ] EMU/产品路径不存在demo/stub/simulated success、test-only eHSM/crypto/cert/key/provider、未批准hardcode或silent fallback；现有test/stub流程不作为目标或最终oracle。
- [ ] SRC-0018 `ehsm_demo_test()`全部BL/FW/可选功能已展开为受控case list并映射到baremetal可执行入口；Vendor组包复用不替代baremetal顶层结果规则，也不替代产品安全链测试。
- [ ] Host/unit/QEMU/Vendor sim 的可前移测试已执行并保存 Evidence。
- [ ] EMU Wave 0/1 的 runner、镜像、配置和恢复步骤通过 dry-run。
- [ ] 实际执行绑定 RTL/eHSM/BootROM/FMC/GSP/Host 版本。
- [ ] RESULT 记录实际修改文件、未提交 diff 摘要、命令、测试结果、Evidence 和剩余风险。
- [ ] 未执行 `git add`、`git commit`、`git push`、merge、rebase 或 tag。

## 当前阶段记录

- 2026-07-21：完成资料、公司软件安全组件和 v0.2 测试用例的只读盘点；创建 Feature 矩阵、4 周工作计划和测试就绪计划。
- 2026-07-21：按负责人补充信息修正交付：GSP 在 `gsp-pmp-rmp-omp` 完成正式详设/实现，在 `baremetal` 完成测试 case/EMU 自动化。
- 2026-07-21：按负责人进一步确认，正式详设、全部任务、测试规划/工作簿和开发追溯统一在 `security_-scheme` 管理；`gsp-pmp-rmp-omp` 只承载软件实现，`baremetal` 只承载可执行测试/EMU 自动化。
- 2026-07-21：启动子任务 `TASK-SEC-DD-001`，建立正式详设主入口和按Feature滚动闭环；当前不进入生产代码修改。
- 2026-07-22：登记DD-02双路径和Vendor Demo完整case覆盖要求；baremetal全功能验证与gsp产品移植后续分别立项，当前未修改代码仓。
- 2026-07-22：按ADR-0007采用P1生命周期布局；旧080x被替代，PMP/RMP/MMP常驻，BootROM/FMC尾部回收，GSP/Measurement/Mailbox连续且SPDM并入GSP；剩余地址视图/容量/OMP/Firewall问题阻断linker实施。
- 当前未执行构建或测试，未修改两个公司仓库，未修改 v0.2 工作簿。
