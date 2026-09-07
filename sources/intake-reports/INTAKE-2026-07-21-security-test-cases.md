# 2026-07-21 NGU800P 安全测试用例入库与审视报告

## Identity and hash

- Source ID：SRC-0020。
- 原始文件：`source-vault/internal-specs/test-inputs/SRC-0020/test_case-v0.1.xlsx`。
- SHA-256：`153d55f6797e4b97230d4358d84bf388b0b8ecee146e84d358c0b52ecf4d428a`。
- 大小：24991 bytes。
- 原始工作簿：3 个可见工作表；Sheet1 使用范围 A1:W77，Sheet2/Sheet3 为空。

## Version and applicability

- 原始文件未内嵌版本号，本次按用户“初版”说明登记为 v0.1。
- 适用于 NGU800P D0 安全测试规划；逐项执行仍需填写 RTL、BootROM/FMC/GSP/eHSM/Host 版本、载体和配置。
- 审视版生成到 `tests/cases/NGU800P-security-test-cases-v0.2.xlsx`，不覆盖原始输入。

## Authority and evidence state

- 原始工作簿状态为 `PROPOSED`，不是已批准测试基线，也不代表执行结果。
- 项目预期按 SRC-0017 → SRC-0016 → 已批准要求/裁决派生。
- SRC-0018 的 Vendor Python tests、源码和 ROM Patch 测试只作为 eHSM 实现/回归参考，不替代 SoC 级验收。

## Original workbook findings

- 76 条数据行，75 条已编号，1 条 ROM Patch 占位。
- 75 个编号全部唯一；编号不连续但无重复。
- `前置条件` 仅 38/76 非空，`输入/输出` 各 50/76，`测试目的` 49/76，`通过准则` 12/76。
- `软件版本`、`自动化状态`、`关联缺陷`、`开始时间`、`结束时间` 和 `备注` 基本未填写。
- MD5 用例的测试目的误写为 SHA-256；多处“秘钥”术语和 AES/SM4 模式描述存在可修正问题。

## v0.2 processing result

- 保留统一模板 23 列和原有样式；增加“版本说明”“覆盖审视”工作表。
- 黄色表示修改已有单元格，绿色表示新增测试行，橙色表示待确认/阻塞。
- 补全算法占位项和系统级用例的输入、输出、目的或通过准则。
- 完成原 ROM Patch 占位项，并依据 SRC-0011/SRC-0018 加入使能、禁用、单行/多行和生命周期边界。
- 新增 24 行，用例总数变为 100，覆盖密钥轮换异常/掉电、更新原子性、TRNG 健康、memory/DMA 边界、错误传播、安全存储、多 Die/Firewall、故障注入、敏感材料清零、摘要/PKE矩阵、Vendor 回归、SPDM 异常和安全审计。
- 最终工作簿 SHA-256：`4b1da21cd6cb4037a5deb1ca22931891b52f4a99d39f8b9a1a67df0a8c0154b8`，大小 36689 bytes。

## Conflicts

1. 入库时已知 `OPEN-CONFLICT-001`：eHSM BL/Host 对自检 bit18/bit19 的 SHA256/TRNG 定义不一致。v0.2 只保留整体验证；该冲突于2026-07-22按Bootloader定义关闭，v0.2保持不变，v0.3再高亮更新用例093。
2. 入库时新增 `OPEN-CONFLICT-002`：SRC-0016 §4.3 的 `NGU_FW_TYPE_DIE1_FW`/`die_id` 与 §9.4“不单独记录 Die1 固件状态”对测试预期不一致。相关用例在 v0.2 中标橙色；该冲突于2026-07-22采用独立Die1实例记录关闭，v0.3再更新用例107。

## Open questions

1. SRC-0020 的正式 Owner、批准人、版本规则和保密分类是什么？
2. MD5、SHA-512/256、AES-192-XTS、DES/TDES 分别是 Vendor 兼容回归、硬件能力验证还是产品发布门禁？
3. 各新增用例的实际执行团队/负责人、RTL/软件版本、测试载体和自动化状态是什么？
4. TRNG 采用的标准、统计阈值、在线健康检测窗口和故障恢复要求是什么？
5. Die1 measurement 采用独立条目、复用 Die0 条目还是按 profile 条件化？

## Affected requirements, designs, decisions and tests

- 工作流程写入 `docs/06-verification/security-test-strategy.md` 和 `tests/cases/README.md`。
- 工作簿登记写入 `tests/matrices/security-test-matrix.yaml`。
- 冲突写入 `sources/conflict-reports/CONFLICT-SRC-0016-DIE1-MEASUREMENT.md` 和 `requirements/open-questions.yaml`。
- 本次未执行测试，未创建 Evidence，也未修改 Vendor 源码或公司代码仓库。

## Review history

- 2026-07-22：补记后续Die1 Measurement裁决；采用推荐Option A，OPEN-CONFLICT-002不再阻塞v0.3用例107更新。
- 2026-07-22：补记后续裁决：采用Bootloader位图，Host定义错误；OPEN-CONFLICT-001不再阻塞v0.3用例093更新。
- 2026-07-21：接收重新上传文件，完成入库、审视、v0.2 生成、可视化检查和流程登记。
