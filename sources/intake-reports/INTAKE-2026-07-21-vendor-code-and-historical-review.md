# 2026-07-21 Vendor 代码与历史 Review 入库报告

## Scope

- Vendor 代码混合快照：`source-vault/vendor-code/osr_eshm`。
- 总计 2747 个文件、383998843 bytes。
- 历史 Review 文档：11 份、276380 bytes。
- 本次只登记、分类和建立流程，不修改源码、不重新执行历史构建或测试。
- Vendor 范围仅为 eHSM 及其内部 Core，不包含 NGU800P SoC。
- 代码按交付快照整体管理，不逐文件建立 Source Card。

## Intake result

| Source ID | 名称 | 初始状态 | 文件数 | 树哈希 |
|---|---|---|---:|---|
| SRC-0018 | OSR eHSM 4019 软件交付快照 | VENDOR_IMPLEMENTATION | 2747 | `7fa5dfae174d67bea825f9bda621d87c73d1ae58d2632d67de10ff0e3281801c` |
| SRC-0019 | OSR eHSM 历史代码 Review 文档集 | PROPOSED | 11 | `b8bbd29d4f38f152a4f4bdb436e1016461f06d2f470d93ccfa4dba3432874e05` |

树哈希算法为：对全部文件按相对路径排序，将 `relative_path NUL file_sha256 NUL` 拼接后计算 SHA-256。该哈希固定当前目录快照，不等于 Vendor 原始压缩包哈希。

## Provenance boundary

当前目录是混合快照，至少包含三类内容：

1. Vendor 交付实现：BL、FW、Host API、External Driver API、Python tests 和 tools。
2. 构建及分析产物：`build*`、ELF、BIN、MAP、反汇编、`.codegraph/`、`.understand-anything/` 和 `reports/`。
3. 项目组历史分析：`docs/review/`、`docs/superpowers/` 和快照内的 `openspec/`。

因此 SRC-0018 只能作为当前实现快照；SRC-0019 单独表示历史 review，二者不能共同推导为已批准设计或已确认缺陷。

若 Vendor 建议被项目采纳，必须体现在 SRC-0016“芯片安全软件方案”或其后续有效版本中；SRC-0018/SRC-0019 不直接定义 SoC 方案。

## Component and document mapping

| Component | Snapshot version | Related registered document |
|---|---|---|
| Bootloader | `2.3.5-4019-72f8fdc` | SRC-0012 Bootloader TRM 4019 1.1 |
| Firmware | `2.3.2-4019-5a4a0a9` | SRC-0013 Firmware TRM 4019 1.1 |
| Host API | `2.3.1-4019-2ee044d` | SRC-0014 Host API TRM 4019 1.0 |
| External Driver API | `v1.1` | 尚无独立正式接口 Source |
| BL/FW Python tests | `1.1.2-4019-8fad478` | 测试说明分散在快照 README 和 SRC-0011 |

## Version discrepancy

`SW_changelist.md` 记录 Bootloader 为 `ehsm_bl-2.3.5-4019-eaec20c`，实际目录为 `ehsm_bl-2.3.5-4019-72f8fdc`。FW 和 Host 的后缀与 changelist 一致。当前无法判断 Bootloader 是后续补丁、重打包还是记录错误，因此：

- SRC-0018 保留实际目录版本；
- 在 `requirements/open-questions.yaml` 建立版本来源问题；
- 在差异确认前，不把历史 review 结论外推到其他 BL 版本。

## Historical Review result

- 11 份历史文档覆盖交付盘点、执行计划、Bootloader、M130、memory remap、错误处理、mailbox、Debug Auth、OTP/key 和 TRNG。
- 两份专题文档包含 17 条带编号发现：`FINDING-OTPKEY-01`～`08`、`FINDING-TRNG-01`～`09`。
- 其余专题含有核心结论、关注点和 Vendor 待确认问题，但尚未统一编号、严重度和验证状态。
- 完整候选项见 `sources/historical-review-index.yaml`。
- 后续独立核对已确认 FINDING-TRNG-05 对应的 BL/Host 自检位图冲突；见 `sources/conflict-reports/CONFLICT-SRC-0012-SRC-0014-TRNG-BITMAP.md`。其余 16 条仍待复核。

## Review promotion workflow

1. 历史 review 原文保留在 SRC-0019，不直接改写。
2. 候选发现初始状态统一为 `ASSUMPTION / pending_revalidation`。
3. 重新检查 SRC-0018 中的确切代码和版本，并对照 SRC-0012～SRC-0014、方案基线及 Vendor 回复。
4. 通过独立复核、可重复构建或测试形成 Evidence，再评估影响、严重度和处置方式。
5. 未确认行为写入 open questions；可复现缺陷写入 `issues/`；设计变化通过本仓库 OpenSpec change 管理。
6. 未通过上述门禁的候选发现不得进入正式安全需求、设计基线或 `CONFIRMED` 状态。
7. 需要采纳的 eHSM 改进建议必须进入芯片安全软件方案；涉及系统架构时还必须符合 SRC-0017。

## Open questions

1. 当前快照的正式交付批次、原始压缩包哈希和责任人是什么？
2. Bootloader 的 `eaec20c` 与 `72f8fdc` 是什么关系？
3. 历史 review 的作者、reviewer、批准状态、使用工具、构建环境和测试 Evidence 是什么？
4. 17 条候选发现中哪些已经向 Vendor 提交、回复、修复或关闭？
5. 快照中的 PDF 是否与 `source-vault/vendor-docs/` 中已登记版本完全相同？
6. 外部 Driver API v1.1 是否存在正式接口手册或适配要求？

## Next processing

1. 优先复核版本可直接对照的候选项，例如 Host/BL TRNG 位定义和 Host demo 返回码处理。
2. 为通过复核的发现建立独立 issue/Evidence，不在源码快照内继续维护 review 状态。
3. 对 SRC-0018 与 SRC-0016/SRC-0017 做“实现—方案基线”映射，但在方案审批前保持 PROPOSED/VENDOR_IMPLEMENTATION。
4. 获取附带 delivery/release note 的新交付后，为新快照分配新 Source ID，通过 `Supersedes` 关联 SRC-0018；不逐文件登记。
