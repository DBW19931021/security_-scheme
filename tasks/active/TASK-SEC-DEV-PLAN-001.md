# TASK-SEC-DEV-PLAN-001：NGU800P 安全软件开发主计划

## TASK BRIEF

- Task ID：TASK-SEC-DEV-PLAN-001
- 角色：`TASK-SEC-SOC-FW-001` 的开发波次和候选任务追溯记录，不是独立顶层任务，也不代表编码授权。
- 状态：active
- Owner：GSP
- 目标仓库：`security_-scheme`
- 目标：把 20 个安全 Feature 转换为有依赖、准入、代码落点、测试出口和 Evidence 的开发波次，并按门禁滚动创建实施任务。
- 非目标：本任务本身不修改 `gsp-pmp-rmp-omp` 或 `baremetal`，不运行构建/测试，不创建 Git 提交。
- 主计划：`docs/09-plans/NGU800P安全软件开发计划.md`。
- 输入：SRC-0016、SRC-0017、SRC-0018、SRC-0020、SRC-0021、CE-SEC-001～011、ADR-0003～0019、Feature矩阵、正式详设、已关闭OPEN-CONFLICT-001～005/007～009和开放OPEN-CONFLICT-006。

## TASK PLAN

| 工作项 | 内容 | 出口 | 状态 |
|---|---|---|---|
| DP-00 基线核对 | 核对两份 PDF 的启动、LCS/Debug/Key、轮换、SPDM、更新/OOB 和 Multi-Die 目标；结合 CE-SEC-001/002 | 目标—代码差距和关键依赖 | completed |
| DP-01 开发计划建立 | 建立 W0～W4、DEV-SEC-001～013、TST-SEC-001～005、DoR/DoD 和阻塞清单 | 开发主计划和实施任务模板 | completed |
| DP-02 W0 设计闭环 | 评审 CE、关闭 DD-02/DD-03/公共 ABI，隔离或裁决冲突 | 首批任务 `ready_for_openspec` | active；首轮评审包ready |
| DP-03 首批任务实例化 | 为近期 1～3 个任务创建 OpenSpec proposal 和单独实施任务 | `approved_for_code` 任务 | pending；等待 DP-02 和人工批准 |
| DP-04 滚动开发追踪 | 回填 Feature/Requirement/Code/Case/Evidence 和波次状态 | 每周可审查状态、风险和下一批任务 | pending |
| DP-05 EMU/发布闭环 | 接入 EMU Evidence，关闭 P0 风险或记录批准处置 | `emu_verified` / `release_ready` | pending |

## 当前执行

1. R1-05和D1～D3已完成裁决；将结果转化为Measurement、Header Overlay/typed-stage loader、错误和release详设。
2. Codex/GSP继续关闭DD-02双路径：展开`ehsm_demo_test()`二级case；只读形成`bl_demo`底层函数/command格式的A/B/C/D复用表和产品语义接口；同时完善DD-03 package/counter/measurement参数表。
3. Secure Package三套算法Profile已由ADR-0017确认；负责人后续确认设备/镜像/key/board/LCS provisioning绑定、SPDM profile、平台/Measurement Owner和重叠工作区修改归属；补录OPEN-CONFLICT-001原始Vendor回复材料。
4. 满足门禁后，产品侧优先实例化DEV-SEC-001/002/003最小组合；baremetal侧另建TST-SEC-002A全功能case任务，不使用一个任务跨仓修改。
5. 并行准备测试v0.3规划和Vendor Demo二级case映射，但每次生成新工作簿，不覆盖v0.2。
6. ADR-0016已关闭GSP/OMP关系；ADR-0028已冻结容量和Region，继续关闭OPEN-CONFLICT-006剩余PMA/Firewall/MMP DDR与footprint。在此之前不得发布production linker或把Header Overlay目标写成未经生成器约束的裸值。
7. ADR-0019已关闭OPEN-CONFLICT-005/009：开发规划新增eHSM BL专用16字节rollback-counter update API和FMC主动调用任务；command/packing/LCS/交付版本在编码前补齐。

## ACCEPTANCE

- [x] 主计划明确三仓边界和无 Git/无提交约束。
- [x] 20 个 Feature 已映射到候选开发波次和测试出口。
- [x] 建立实施任务模板、Definition of Ready 和 Definition of Done。
- [ ] 首批候选任务完成详设/冲突/Requirement/test oracle 门禁。
- [ ] 首批 OpenSpec proposal 获得人工批准并创建单独实施任务。
- [ ] 后续波次持续回填实际代码、测试和 Evidence 状态。

## 当前记录

- 2026-07-22：完成首版开发计划和任务模板，W0 启动。未修改 `gsp-pmp-rmp-omp` 或 `baremetal`，未执行 Git、构建或测试，尚无活动编码任务。
- 2026-07-22：形成W0首轮设计评审包，新增Boot Handoff ABI并补齐统一错误处理；DP-02继续active，等待W0-R1和D1～D3评审。
- 2026-07-22：登记W0-R1批准状态和“现有test/stub非目标依据”约束；D1按Bootloader位图关闭，R1-05仍待解释后评审，DP-02继续active。
- 2026-07-22：接受ADR-0004；D2/D3按推荐方案关闭，R1-05当前不采用；DP-02继续active但不再等待Handoff或三个冲突。
- 2026-07-22：DD-02按负责人确认拆成baremetal完整eHSM能力验证和gsp产品安全链移植；后续分别创建实施任务，当前只更新详设/case catalog。
- 2026-07-22：B0-R2接受ADR-0006/0007，安全RAM改为P1生命周期复用合同；OPEN-CONFLICT-006剩余项阻断linker相关实施任务，未修改代码仓。
- 2026-07-24：完成CE-SEC-011并登记OPEN-CONFLICT-009；当前Vendor BL只暂存候选、Vendor FW启动才提交OTP，与批准的FMC pre-release commit顺序不兼容，DEV-SEC-004/006相应范围继续阻断。未修改代码仓、Vendor快照，未执行Git、构建或测试。
- 2026-07-24：负责人决定Counter细节延期；DEV规划按16字节抽象接口继续，DEV-SEC-004/006不再因该项阻断设计，实际Vendor绑定仍是实现/EMU前门禁。
