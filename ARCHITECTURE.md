# 工程协作架构

本文描述工程协作和事实管理架构，不代表芯片最终安全架构。

```text
原始资料与已验证事实
        ↓
Source Index / Source Card / 事实分级
        ↓
Work：需求、威胁、候选方案和影响分析
        ↓
OpenSpec change / ADR
        ↓
      人工批准
        ↓
远端 Codex 在 gsp-pmp-rmp-omp 当前工作区实现（默认分支允许）
        ↓
构建、测试、CI、仿真或硬件验证
        ↓
RESULT + 未提交 diff 摘要 + Evidence 索引
        ↓
Work 一致性和安全错误路径评审
        ↓
正式文档、追踪矩阵、ADR 和项目状态更新
```

## 事实源

`security_-scheme` 是方案、任务、决策和 Evidence 索引的唯一事实源。`source-vault` 保存原始资料；大型或敏感文件是否进入 Git 需按公司规则和 Git LFS 策略单独确认。`source-vault/vendor_rtl`已登记为SRC-0035，是eHSM内部硬件实现细节、RTL实现基线和问题查询第一入口，但保持只读和`VENDOR_IMPLEMENTATION`状态。

## 方案权威链

`SRC-0017 芯片系统安全方案`是系统/架构上位方案；`SRC-0016 芯片安全软件方案`在其硬件基础、架构边界和原则约束下负责软件工程落地。Vendor 仅覆盖 eHSM 及其内部 Core，其文档、代码和建议只能作为软件方案输入；项目采纳的内容必须进入 SRC-0016 或后续有效版本。

密钥轮换策略已经批准；SRC-0015的三类Key、每类一次、HSM Bitmap、48字节封装、USER鉴权和单向切换已由ADR-0021进入软件方案增量。ADR-0026已经关闭OPEN-DESIGN-014的软件设计裁决；Vendor ABI、Bitmap/掉电和KMS/制造参数继续作为实施绑定。

PDF 作为受控原始基线，`docs/09-plans/BASELINE-CONTROL.md` 维护当前有效版本和增量裁决。内部方案是规范目标，Vendor 文档/代码和测试是当前 eHSM/Core 的证据；两者冲突时保留实际差距，按 ADR-0002 停止受影响范围并提交负责人裁决。

eHSM内部RTL调查先锁定SRC-0035树哈希并证明top/filelist/define/parameter/generate/wrapper和实例链；详细规则见`docs/09-plans/RTL-INVESTIGATION-WORKFLOW.md`。SRC-0035不能覆盖SRC-0022的SoC地址/寄存器/IRQ数值，也不能定义SoC wrapper、PMA/Firewall、Lifecycle、Key、Boot或软件策略。静态RTL、动态Evidence和产品方案分别保留事实层级。

## 代码实现

`../gsp-pmp-rmp-omp` 是独立多人软件仓库。Codex 可以直接在默认分支工作区修改，但不得执行 `git add`、`git commit`、`git push`、merge、rebase 或 tag。本仓库只保存只读基线、实际修改文件、diff 摘要和验证 Evidence。

Vendor 代码不逐文件登记；一个交付版本作为一个 Source 快照管理。后续更新应提供 delivery/release note，并以新 Source ID 和 `Supersedes` 关系登记。

Vendor RTL也按整个交付快照管理并记录树哈希、顶层、filelist、外部define/parameter、wrapper/库及匹配验证版本。当前SRC-0035缺正式filelist/release note，所有实际生效结论都必须显式限定。目录中的Key/KEK字面量不得复制、外发或作为量产Key。

## 正式详设阶段的代码调查

正式详设不能根据方案文字猜测代码现状，也不能把现有代码直接提升为目标设计。需要确认代码事实时，在 `security_-scheme/tasks/active/` 创建小粒度 `INV-SEC`任务，Codex只读调查 `gsp-pmp-rmp-omp`或Vendor快照，并把带路径、符号、调用链和构建条件的`CE-SEC`报告写入`evidence/code-investigations/`。若问题属于eHSM内部硬件，调查先使用SRC-0035并额外记录树哈希、elaboration、实例链和逻辑锥。

代码证据经抽查后回填`docs/03-architecture`、`docs/04-interfaces`、`docs/05-software-design`和现有追溯矩阵。冲突、产品取舍和编码准入保留人工裁决。完整规则见`docs/09-plans/CODE-INVESTIGATION-WORKFLOW.md`。

## 防止事实漂移

文档头部、Source ID、Requirement ID、ADR、OpenSpec、任务结果和自动检查共同防止方案、实现与证据逐步失去一致性。
