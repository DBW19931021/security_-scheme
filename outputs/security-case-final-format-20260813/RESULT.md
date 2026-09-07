# 安全测试最终格式整理结果

## 目标

参考mailbox和spinlock测试的最终表格格式及过程说明粒度，在`security_-scheme`内完成安全测试用例设计终版、SoC架构/功能说明、Case说明和全过程记录，不执行baremetal移植。

## 输入与范围

- 内容来源：v0.3.8 SoC安全功能收敛版、SRC-0016/0017/0018/0022/0025/0026/0027；
- 格式与过程粒度来源：SRC-0028；
- 被测对象：eHSM固件、eHSM硬件和NGU800P SoC安全集成；
- 排除项：Host本地软件健壮性、Firewall IP完整功能回归、eHSM固件定制和本轮baremetal移植。

## 交付物

1. `tests/cases/outputs/20260813-security-final/NGU800P_Security_Case_Table_Final_v3.xlsx`
2. `docs/06-verification/NGU800P安全架构功能与测试用例说明.md`
3. `docs/06-verification/NGU800P安全测试用例设计与Codex移植指导.md`
4. `tests/cases/security/README.md`
5. `tests/cases/security/CASE_TABLE.md`
6. `docs/06-verification/NGU800P-security-test-case-workflow-record.md`

## 数量

- Mailbox BASIC：52；
- eHSM负向：14；
- SoC软件/协同：7；
- EDA/硬件：3；
- 最终表：76；
- 后续Codex移植：73。

## 验证

- 固定13列，单一`Security`工作表；
- 连续编号`NGU800P-D0-SECURITY-001～076`；
- 52条Mailbox BASIC和14条eHSM负向的“输入”已逐条重写为实际API/参数/顺序/矩阵/恢复动作；66条输入互不重复，旧通用模板关键句命中0项；
- 52条Mailbox BASIC、14条eHSM负向、7条SoC和3条EDA的“通过准则”均已逐条收敛为命令/功能直接判定条件；76条均非空，旧通用模板关键句命中0项，最长84个字符；
- 66条Mailbox前置条件均明确eHSM正常启动和BL/FW阶段，并把服务通道1、轮询等待、单次命令100 ms超时配置展开写明；
- 76条“输出”和“测试目的”均已逐Case重写，旧家族输出模板、“合同有效”和“允许副作用”命中0项；
- 分5段渲染并目视检查全部数据行；
- 重新导入后确认有效范围`Security!A1:M77`；
- 公式错误扫描匹配0项；
- 最终XLSX大小28959字节，SHA-256为`e91961aa068bcbc0a6379293aed84938f808fc9a086c9be9c458e978e6773087`；
- 工程链接、来源索引和项目一致性由项目自检统一确认。

## 状态与边界

结果为`PROPOSED_DESIGN_COMPLETE / NOT_EXECUTED`。本任务未修改baremetal、eHSM BL/FW或产品代码，未执行EMU/FPGA/EDA/硅上测试，也不声明硬件PASS。
