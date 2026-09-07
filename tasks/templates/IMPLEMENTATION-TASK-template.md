# IMPLEMENTATION TASK

## TASK BRIEF

- Task ID：
- 状态：draft / approved_for_code / implementing / review / completed
- Owner：GSP
- 目标仓库：`gsp-pmp-rmp-omp` / `baremetal`（只能选择一个）
- Feature ID：
- Requirement ID：
- OpenSpec change：
- 相关详设：
- Source ID / 适用版本 / 事实状态：
- 冲突与裁决：
- 目标：
- 非目标：

## REPOSITORY BOUNDARY

- 清单中的默认分支/只读基线：
- Git 操作：禁止执行任何 Git 命令；不 add/commit/push/merge/rebase/tag；不修改 `.git`
- 工作方式：允许直接修改默认分支工作区，不要求功能分支
- 既有未提交修改及归属确认：
- 与本任务重叠文件的继续修改授权：
- 目标文件/符号：
- 禁止修改文件/目录：
- 无法区分已有修改时的停止点：

## DESIGN CONTRACT

- 前置条件：
- 输入/输出和 ABI：
- 正常状态机：
- 失败/恢复状态机：
- LCS/Role/Key/接口权限：
- timeout/busy/retry/reset/掉电/并发：
- 错误/中断/日志/审计：
- 敏感数据清零：
- 未决参数和隔离方式：

## IMPLEMENTATION PLAN

| 阶段 | 修改范围 | 验证 | Evidence | 停止条件 |
|---|---|---|---|---|
| 1 |  |  |  |  |
| 2 |  |  |  |  |
| 3 |  |  |  |  |

## TEST TRACE

- 实现侧 unit/contract test：
- `security_-scheme` 工作簿 case：
- `baremetal` 可执行测试入口：
- 正向 oracle：
- 权限/边界 oracle：
- 失败路径 oracle：
- EMU/RTL/样片要求：

## ACCEPTANCE

- [ ] 方案/详设/OpenSpec 已批准，影响本切片的冲突已裁决或隔离
- [ ] 仅修改批准范围，未覆盖无关工作区修改
- [ ] 生产 target 未链接或调用 test-only stub
- [ ] 指定编译返回 0，实际命令和输出已记录
- [ ] unit/contract/Host/QEMU 测试通过，未执行项明确标记
- [ ] 失败、权限、reset/掉电和清零路径满足验收
- [ ] Requirement→Design→Code→Case→Evidence 可追溯
- [ ] RESULT 记录实际文件、未提交 diff 摘要、命令、结果和剩余风险
- [ ] 未执行任何 Git 命令或提交工作，未修改 `.git`

## RESULT

- 实际修改文件：
- 未提交 diff 摘要（不得通过 Git 命令获取）：
- 执行命令与返回值：
- 测试结果：
- Evidence：
- 未完成项：
- 剩余风险：
- 文档/追溯回填：
