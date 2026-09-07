# 安全测试用例版本管理

## 目录职责

- `source-vault/internal-specs/test-inputs/SRC-xxxx/` 保存用户提供的原始测试表，按 Source ID 不可变保存。
- `tests/cases/` 保存 v0.2 和后续经审视、补全、评审的版本化测试工作簿，是测试 case 规划的唯一主维护位置。
- `../baremetal/components/ngu_security/tests/` 和 `tools/` 由 GSP 保存可执行 case、EMU runner、测试工具和必要测试数据；不在该仓库保存测试规划或工作簿。首次写入时按 baremetal 仓库约束创建目录。
- `tests/matrices/security-test-matrix.yaml` 登记当前工作簿版本、来源、数量、阻塞项及其对应可执行测试入口。
- `EHSM-DEMO-CASE-CATALOG.md`登记SRC-0018 `ehsm_demo_test()`的完整能力覆盖盘点；它是后续工作簿和baremetal可执行case的输入清单，不是执行Evidence或独立发布oracle。
- `evidence/` 保存实际执行日志、报告、签核和发布 Evidence；测试用例文件本身不是 Evidence。

## 版本与高亮规则

1. 每次更新在 `security_-scheme/tests/cases/` 生成一个新的 `.xlsx` 文件，不覆盖既有版本；该目录是唯一可编辑主版本位置。
2. 黄色表示修改已有内容；绿色表示新增测试行；橙色表示方案、接口、版本、Owner 或预期仍需确认/裁决。
3. 新版本必须在“版本说明”工作表记录原始来源、版本、修改数量、冲突和未执行声明。
4. 原始模板列保持不变；辅助说明可放在独立工作表，不把 Source 依据塞入未定义的新模板列。
5. 发现方案—Vendor—代码—测试冲突时，受影响预期标为 `CONFLICTING`，建立 conflict report，并在裁决前停止确定性验收。
6. 现有 `gsp-pmp-rmp-omp`/`baremetal` 中的test、stub、demo、synthetic flow和历史Expected只用于代码事实/差距分析，不是目标测试流程、最终oracle、验收或发布依据；可执行case必须从最新有效工作簿和批准方案/Requirement/ADR/OpenSpec派生。
7. EMU/产品验收不得以stub、simulated success、test key/cert/provider、未批准hardcode或silent fallback形成PASS；能力缺失应得到FAIL/INCONCLUSIVE或明确阻塞，不能伪造完成。

## 流片前发布门禁

- 每个 P0 安全用例必须有 Owner、RTL/软件版本、载体、前置条件、输入、输出、通过准则和 Evidence 路径。
- 覆盖正向、异常、边界、生命周期、权限、掉电/复位、回滚、故障注入和敏感材料清理。
- Vendor 回归只能作为 eHSM 快照实现证据；SoC 发布必须补充 BootROM/FMC/GSP/Runtime、Firewall、多 Die、OOB 和板级路径 Evidence。
- 所有橙色阻塞项必须裁决或由发布负责人书面接受风险。

## 当前版本

- 当前在线最终基线：[飞书安全测试Case表（Security工作表）](https://mx4lbik1jc.feishu.cn/wiki/HsC6wEjSqivStckreWqc6gIVnXG?sheet=o4uhR0)。2026-08-20逐行核对为63条：47条Mailbox BASIC、7条eHSM负向、7条SoC软件/协同和2条EDA/硬件协同；连续编号`NGU800P-D0-SECURITY-001～063`，软件/BSP主导范围为`001～061`。企业策略禁止下载/导出，当前不生成新的本地XLSX镜像。
- 当前本地开发索引：[security/CASE_TABLE.md](security/CASE_TABLE.md)；当前开发契约：[NGU800P安全测试用例设计与Codex移植指导](../../docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md)。
- 原始输入：SRC-0020 `test_case-v0.1.xlsx`。
- 审视版：`NGU800P-security-test-cases-v0.2.xlsx`，100 条已编号用例。
- 历史设计版：`NGU800P-security-test-cases-v0.3.xlsx`，按每条命令固定派生BASIC/BOUNDARY/POLICY，共156条Mailbox命令用例和67条SoC集成用例；状态为`SUPERSEDED_DESIGN / NOT_EXECUTED`，文件保留且不覆盖。
- 历史风险裁剪版：`NGU800P-security-test-cases-v0.3.1.xlsx`，52条BASIC、30条BOUNDARY/POLICY和67条细分SoC用例；状态为`SUPERSEDED_DESIGN / NOT_EXECUTED`，文件保留且不覆盖。
- 历史BASIC-only版：`NGU800P-security-test-cases-v0.3.2.xlsx`，52条Mailbox BASIC + 3条SoC软件用例；因误将eHSM负向与Host本地负向一并删除，状态改为`SUPERSEDED_DESIGN / NOT_EXECUTED`，文件保留且不覆盖。
- 历史eHSM DUT修订版：`NGU800P-security-test-cases-v0.3.3.xlsx`，52条BASIC + 14条eHSM负向 + 3条SoC软件，共69条；因把可软件配置的Mailbox/HSM错误/reset/IRQ功能整体放入硬件表，已由v0.3.4替代。
- 历史软件可构造功能复审版：`NGU800P-security-test-cases-v0.3.4.xlsx`，52条Mailbox BASIC + 14条真实到达eHSM的负向用例 + 8条SoC软件用例，共74条Codex可移植软件用例；用例范围有效，已由v0.3.5补充中文术语说明。
- 历史设计交付版：`NGU800P-security-test-cases-v0.3.5.xlsx`，用例数量、判据和移植边界与v0.3.4一致；说明性英文术语已改为中文解释，新增“术语说明”工作表；命令、寄存器、用例ID、错误码、状态码和门禁码保留原标识；状态为`SUPERSEDED_DESIGN / NOT_EXECUTED`。
- 实现状态回填版：`NGU800P-security-test-cases-v0.3.6.xlsx`，由既有工作流于2026-08-10生成并保留，不在本次Firewall输入中覆盖。
- Firewall评审补充版：`NGU800P-security-test-cases-v0.3.6-firewall-review.xlsx`，基于v0.3.6新增“Firewall用例”和“Firewall UserId”页，完整登记16个UserId与49条细化测试（40 READY、9 BLOCKED）；现作为v0.3.8输入和专项参考，不是当前顶层Case清单。
- 设计来源版：`NGU800P-security-test-cases-v0.3.8-soc-converged.xlsx`，保留52条Mailbox BASIC和14条eHSM负向；SoC收敛为7条软件/协同Case，分别为SEC_CFG全寄存器、`dbg_en_cfg`从Die Debug、SEC_CFG/SPIFC默认权限、SRAM默认权限、SRAM重新配置和Mailbox IRQ；Codex可移植合计73条。另列3项EDA/硬件需求，不进入移植计数。状态为`PROPOSED_DESIGN_COMPLETE / NOT_EXECUTED`。
- 历史最终格式修订版：`outputs/20260813-security-final/NGU800P_Security_Case_Table_Final_v5.xlsx`，曾按SRC-0028～0030形成76条本地快照。它仍用于追溯此前字段复审和方案变更，但已被在线63条最终基线取代，不再指导后续代码编号和范围。v4及更早版本同样保留为历史。
- [安全架构、功能与测试用例说明](../../docs/06-verification/NGU800P安全架构功能与测试用例说明.md)介绍SoC总体架构、eHSM/SEC_CFG/Debug/Firewall/IRQ/错误上报功能和当前63条Case分组；`security/README.md`与`security/CASE_TABLE.md`提供最终交付入口和编号目录。
- [NGU800P安全测试用例设计与Codex移植指导](../../docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md)规定后续Codex的实施契约；目标仓库为`../baremetal`，但本版本不预设其目录、模块、Runner或文件名。
- [SoC安全方案EDA必测与硬件协同验证项](../../docs/06-verification/NGU800P软件不便覆盖的硬件安全验证项.md)保留历史硬件分析；当前最终表只计入严重错误和ECC 1-bit两项EDA Case，原条件性`EDA-FW-EHSM-MASTER-001`不再计入最终用例。
- v0.3.8中`dbg_en_cfg`只验证主Die控制从DieDebug；`soc_dbg_en_out`加入Mailbox Debug Auth成功/失败及`CLOSE_DEBUG`判据。
- 三个Firewall只有启动核可配置，eHSM和其他核配置写均拒绝；SEC_CFG/SPIFC不执行成功重新配置，数据访问默认仅启动核允许；SRAM默认权限和重新配置保持两条用例，且每条用例内部都遍历Region0～Region4，SRAM数据默认仍允许启动核和eHSM访问。
- 49条Firewall细化条目只作为专项模型/DV参考，不派生49个顶层SoC Case。
- v0.3.5不规划只在Host本地完成的NULL/buffer/local-enum/timeout/log等健壮性用例，但保留eHSM负向，并包含16路Mailbox、INFO/NOTE、HSM错误映射、软件可控复位和IRQ软件功能case。映射未冻结时为INCONCLUSIVE，不因暂缺绑定删除case。
- Firewall配置权限测试不依赖鉴权/LCS：只有承担启动核角色的C908可配置，eHSM及其他核不可配置；精确Master ID和寄存器由RTL/SRC-0022冻结。
- 所有用例和硬件验证需求均禁止修改/定制eHSM BL/FW，禁止新增测试命令、测试钩子或特殊返回。
- v0.3.5使用独立case ID，尚未与v0.2逐项完成ID、Owner和历史内容对账；正式评审前必须完成该项。它也不替代`EHSM-DEMO-CASE-CATALOG.md`的Vendor Demo二级组合展开门禁。
- 后续可执行case只在批准的实现任务中进入`../baremetal`；具体落点服从该仓库现状及其`AGENTS.md`。
- 当前已登记的OPEN-CONFLICT-001/002/003均已关闭；新的方案、参数或环境冲突仍按冲突流程管理。
- eHSM Vendor Demo一级case catalog已建立；下一步展开每个entry内部的command/算法/模式/正负向组合，并在新工作簿版本中建立映射，v0.2保持不变。
- Mailbox命令与SoC安全集成设计包已形成；Vendor Demo每个entry内部的算法/模式/长度/宏/正负向组合仍需继续展开，不能因v0.3.5存在而标记为`L2_complete`。
- 已关闭项：OPEN-CONFLICT-001（自检位图）采用Bootloader定义，Host定义错误；v0.2保持不变，v0.3中将用例093改为bit18=`TRNG`、bit19=unknown/reserved并以黄色高亮。
- 已关闭项：OPEN-CONFLICT-002（Die1 Measurement）采用内部独立实例记录；v0.3中将用例107改为`NGU_FW_TYPE_DIE1_FW`/`die_id=1`并高亮。
- 已关闭项：OPEN-CONFLICT-003（GSP地址）最终采用baremetal `MANAGEMENT_NOC_S9_SRAM_BASE/SIZE` System Address，C908/Manifest/loader/linker/eHSM descriptor不建立Local/System映射；v0.3高亮修改相关Expected并补充旧080x/local-remap输入拒绝场景。
