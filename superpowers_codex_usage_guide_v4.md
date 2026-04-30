# Superpowers for Codex 调研与使用教程

> 适用环境：**VS Code Remote SSH + Codex IDE 插件**，以及后续可选的 **远端 Ubuntu + Codex CLI + git worktree/tmux 多任务开发**。  
> 示例场景：以 **SoC / GPU 安全启动、SEC2/eHSM、固件验签、SPDM 认证、调试鉴权、生命周期管理** 等安全功能开发为例。

---

## 0. 一句话结论

**对 Codex 来说，Superpowers 本质上是一组高阶 skills / workflow 包。**

它不是新的模型，也不是新的 IDE，也不是我们第一步要单独运行的 MCP 服务。它的作用是给 coding agent 增加一套开发纪律：

```text
先澄清需求 -> 先形成设计 -> 先拆实施计划 -> 再隔离分支/worktree -> 再按计划/TDD 实现 -> 再 review -> 再验证收尾
```

对我们现在的环境，推荐使用方式是：

```text
Windows VS Code
  -> Remote SSH
      -> 远端 Ubuntu 工程目录
      -> Codex IDE 插件
      -> ~/.agents/skills/superpowers
```

也就是说：**Superpowers 应该安装在远端 Ubuntu 上，让远端的 Codex IDE 插件扫描到。**

---

## 1. 官方资料入口

本节只保留 Superpowers 相关资料，作为项目组调研和落地时的原始依据。

| 类型 | 链接 | 说明 |
|---|---|---|
| Superpowers GitHub 仓库 | <https://github.com/obra/superpowers> | Superpowers 主仓库、总体介绍、workflow 和 skills 列表 |
| Superpowers for Codex 文档 | <https://github.com/obra/superpowers/blob/main/docs/README.codex.md> | Superpowers 面向 Codex 的安装与使用说明 |
| Superpowers Codex 安装页 | <https://mintlify.com/obra/superpowers/installation/codex> | Codex 安装说明页面 |

---

## 2. Superpowers 是什么

### 2.1 通俗解释

可以把 Codex 想象成一个很能干的开发助手，但如果我们直接说：

```text
帮我实现 SEC2 调用 eHSM 验证 PM/RAS/Codec 固件的流程。
```

它可能马上开始找文件、改代码、补接口。

这对小任务没问题，但对安全启动、固件验证、生命周期、SPDM 认证这类复杂系统设计，风险比较大：

- 需求没澄清就写代码；
- 模块边界没想清楚；
- Host / SEC2 / eHSM 职责可能混乱；
- 安全状态机可能漏掉失败路径；
- 代码改了但没有验证证据；
- 实现可能偏离设计文档和安全边界。

Superpowers 的作用就是给 Codex 加一套开发纪律：

```text
先问清楚 -> 先设计 -> 先写计划 -> 再执行 -> 再 review -> 再收尾
```

### 2.2 和 Skill / MCP 的关系

| 概念 | 通俗类比 | 作用 |
|---|---|---|
| Skill | 操作手册 / SOP | 教 Codex 某类任务应该按什么流程做 |
| MCP | 外部系统接口 / 工具插座 | 让 Codex 能连接 GitHub、Jira、文档库、浏览器、Figma 等外部系统 |
| Plugin | 打包分发方式 | 可以把 skills、MCP server、集成配置打包给团队使用 |

所以：

- **Superpowers 更像一组 skill 包**；
- 它不是我们第一步要当成 MCP server 接入的东西；
- 如果未来某个 Superpowers 流程需要访问外部文档库、工单系统、代码平台，那时才需要和 MCP 组合使用。


### 2.3 为什么我们要调研 Superpowers

我们现在遇到的问题不是“Codex 会不会写代码”，而是：**复杂任务里 Codex 能不能按我们认可的工程流程稳定推进**。

对于安全方案、BootROM / SEC1 / SEC2 / eHSM 职责边界、SPDM 认证、调试鉴权、生命周期、固件升级这类任务，最怕的不是代码少写几行，而是：

- 还没澄清需求就开始改；
- 只解决局部问题，忽略系统边界；
- 文档、代码、测试、traceability 不同步；
- 一个任务拆成多个方向后，缺少统一的 review 和收尾；
- 每个人给 Codex 的 prompt 不一样，最后得到的流程和质量不稳定。

Superpowers 的价值可以用一句通俗的话概括：

```text
它像是给 Codex 加了一个“项目经理 + 技术评审”的工作习惯。
```

它不会替我们决定最终架构，也不会自动保证方案正确；但它会让 Codex 更倾向于：

```text
先问清楚 -> 再写设计 -> 再拆计划 -> 再执行 -> 再 review -> 再验证
```

这对我们做安全功能开发很重要。因为安全功能不是“能跑就行”，而是要能解释清楚：谁可信、谁不可信、谁能放行、谁只能投递、失败后进入什么状态、怎么证明当前实现没有绕过路径。

### 2.4 初步使用感受

从我们当前试用和调研角度看，Superpowers 最明显的感受是：

1. **它会让 Codex 慢一点，但更稳。**  
   原来一句“帮我设计安全方案”，Codex 可能马上输出一大段方案。用了 `brainstorming` 后，它会先问目标、边界、约束，这会多花几轮对话，但能减少后面返工。

2. **它适合复杂任务，不适合所有小任务。**  
   改一个日志、查一个命令、解释一个寄存器，不需要上 Superpowers 全流程。它最适合跨模块、跨文档、跨代码的任务。

3. **它的核心收益不是“多写代码”，而是“少跑偏”。**  
   对安全方案来说，方向跑偏比代码慢更危险。Superpowers 的流程感可以帮助我们把“设计确认”和“实现执行”分开。

4. **它很适合沉淀团队方法。**  
   如果我们反复使用同一套流程，比如“安全方案更新必须先 CR，再更新 constraints / baseline / detailed design / traceability”，就可以用 `writing-skills` 把这套流程固化成团队 skill。

一句话总结使用感受：

```text
Superpowers 不像一个“自动写代码神器”，更像一个“让 Codex 按工程流程做事的约束层”。
```

---

## 3. 适合我们的使用架构

### 3.1 当前主环境

我们当前主环境是：

```text
Windows 本机
  ├─ VS Code
  └─ 通过 Remote SSH 连接远端 Ubuntu
        ├─ 源码仓库
        ├─ 编译工具链
        ├─ QEMU / OpenOCD / 测试脚本
        ├─ Codex IDE 插件实际工作区
        └─ Superpowers skills
```

因此，**Superpowers 不应该优先装在 Windows 本机，而应该装在远端 Ubuntu。**

### 3.2 推荐分工

| 场景 | 推荐工具 |
|---|---|
| 日常看代码、改驱动、跑编译、查日志 | VS Code Remote SSH + Codex IDE 插件 |
| 中大型任务设计和计划 | VS Code Codex + Superpowers skills |
| 后期多任务并行开发 | 远端 Ubuntu Codex CLI + Superpowers + git worktree + tmux |
| 团队标准流程固化 | 项目级自定义 skills |

---

## 4. 安装教程：远端 Ubuntu + Codex IDE 插件

> 前提：我们已经能在 VS Code Remote SSH 里正常使用 Codex IDE 插件。

### 4.1 在远端 Ubuntu 执行

打开 VS Code Remote SSH 的远端终端，执行：

```bash
git clone https://github.com/obra/superpowers.git ~/.codex/superpowers

mkdir -p ~/.agents/skills
ln -s ~/.codex/superpowers/skills ~/.agents/skills/superpowers
```

这一步的含义是：

```text
~/.agents/skills/superpowers
    -> ~/.codex/superpowers/skills
```

也就是把 Superpowers 仓库里的 `skills` 目录，通过软链接暴露给 Codex 的 skill discovery 机制。

### 4.2 为什么是 `~/.agents/skills`

Superpowers 的 Codex 文档采用的是：

```text
~/.agents/skills/
```

并通过一个软链接让 Codex 看到 Superpowers 里的所有 skills：

```text
~/.agents/skills/superpowers/ -> ~/.codex/superpowers/skills/
```

如果我们已经确认 Codex 能扫描到，就说明这条路径在当前版本和环境里是可用的。

### 4.3 重启 Codex / VS Code

安装后需要让 Codex 重新扫描 skills。

推荐做法：

1. 在 VS Code 里执行：`Developer: Reload Window`
2. 或者关闭并重新打开 Remote SSH 窗口
3. 重新打开 Codex 面板

---

## 5. 如何验证安装成功

### 5.1 检查软链接

在远端 Ubuntu 执行：

```bash
ls -la ~/.agents/skills/superpowers
```

应该看到类似：

```text
~/.agents/skills/superpowers -> /home/<user>/.codex/superpowers/skills
```

### 5.2 检查 skills 是否存在

```bash
ls ~/.agents/skills/superpowers
```

应该能看到类似：

```text
brainstorming
writing-plans
using-git-worktrees
systematic-debugging
test-driven-development
...
```

检查某个具体 skill：

```bash
ls ~/.agents/skills/superpowers/brainstorming/SKILL.md
```

### 5.3 在 Codex 中验证

在 VS Code Codex 面板中输入：

```text
/skills
```

或者输入：

```text
$
```

看是否能看到 Superpowers 相关 skills。

也可以直接显式触发：

```text
use brainstorming to help me refine this security feature design
```

如果 Codex 进入“先问问题、先澄清、先设计”的工作方式，就说明生效了。

---

## 6. Superpowers skills 总览

Superpowers 不是只有一个 skill，而是一组围绕“需求澄清、计划、实现、调试、验证、review、收尾”的技能包。下面表格列出 Superpowers README 中提到的主要 skills。

| 类别 | Skill | 简单说明 | 安全功能开发中的典型用途 |
|---|---|---|---|
| 入口 / 元技能 | `using-superpowers` | 介绍和引导使用 Superpowers skill 体系 | 不确定该用哪个 skill 时，让 Codex 先判断流程 |
| 设计澄清 | `brainstorming` | 通过提问和方案比较，把粗略想法收敛成可评审设计 | 安全启动链、SEC2/eHSM 边界、SPDM report 设计 |
| 计划拆解 | `writing-plans` | 将已确认设计拆成可执行 checkpoint | 把安全方案转成文件级实现计划 |
| 分支隔离 | `using-git-worktrees` | 创建隔离 worktree / 分支，避免污染当前工作区 | 多个安全功能并行开发时隔离改动 |
| 计划执行 | `executing-plans` | 按计划分批执行，并在 checkpoint 停下来确认 | 按 SEC2 verify plan 分阶段改代码 |
| 并行代理 | `dispatching-parallel-agents` | 将任务拆给多个子代理并行处理 | Host / SEC2 / 测试 / 文档并行推进 |
| 子代理开发 | `subagent-driven-development` | 按计划启动子代理，做 spec compliance 和 code quality review | 大型安全功能多任务并行实现 |
| TDD | `test-driven-development` | 强制 RED-GREEN-REFACTOR，先写失败测试再实现 | 固件 header parser、状态机、协议解析 |
| 系统调试 | `systematic-debugging` | 用结构化流程定位根因，避免靠猜 | mailbox timeout、SPDM 失败、BootROM 跳转失败 |
| 完成前验证 | `verification-before-completion` | 在宣称完成前给出测试/验证证据 | 修复安全流程 bug 后证明真的修好 |
| 代码审查 | `requesting-code-review` | 对当前 diff 或阶段实现做 review，按严重级别输出问题 | 检查 Host 绕过、release 控制、rollback 检查遗漏 |
| 处理评审 | `receiving-code-review` | 对 review comments 分类、理解和制定修改计划 | 处理安全评审意见、CI/lint 反馈 |
| 分支收尾 | `finishing-a-development-branch` | 总结 diff、验证测试、给出 merge/keep/discard 选项 | 完成一个安全功能 worktree 后收尾 |
| 写新 skill | `writing-skills` | 创建新的团队 skill，并包含测试方法 | 沉淀我们自己的 NGU800 安全方案更新流程 |

---

## 7. 使用原则总览

### 7.0 什么时候用 Superpowers，什么时候不用

可以用一个很简单的判断标准：

```text
如果任务只是“问一个点”，不用 Superpowers；
如果任务需要“先想清楚再动手”，就用 Superpowers。
```

更具体一点：

| 场景 | 是否推荐 Superpowers | 推荐 skill |
|---|---:|---|
| 解释一个术语、查一个命令、改一个小 typo | 不推荐 | 直接问 Codex |
| 安全方案章节设计、模块职责划分、架构取舍 | 推荐 | `brainstorming` |
| 已经有设计，需要拆成可执行步骤 | 推荐 | `writing-plans` |
| 按计划逐步改文件，不想一次性全改 | 推荐 | `executing-plans` |
| 复杂 bug，现象很多，原因不明 | 推荐 | `systematic-debugging` |
| 涉及安全边界、权限、生命周期、release 控制 | 强烈推荐 | `requesting-code-review` |
| 中大型任务，可能污染主工作区 | 强烈推荐 | `using-git-worktrees` |
| 多方向并行分析 | 可选推荐 | `dispatching-parallel-agents` |
| 团队流程想沉淀成 skill | 推荐 | `writing-skills` |

对我们来说，最常用的模式应该是：

```text
设计类任务：brainstorming -> writing-plans
实现类任务：using-git-worktrees -> executing-plans -> review -> verification
调试类任务：systematic-debugging -> verification-before-completion
团队沉淀：writing-skills
```

### 7.1 不要所有任务都强制使用 Superpowers

Superpowers 适合中大型、复杂、跨模块、高风险任务，例如：

- 安全启动链设计；
- SEC2/eHSM 验签流程；
- SPDM 设备认证；
- 调试鉴权；
- 生命周期状态机；
- 固件升级和 anti-rollback；
- 多模块代码实现；
- 复杂 bug 根因分析。

不太适合：

- 改一个错别字；
- 补一行日志；
- 查询一个寄存器含义；
- 非常小的单文件修改。

### 7.2 推荐主线

复杂开发任务建议走这条主线：

```text
use brainstorming
  -> use writing-plans
    -> use using-git-worktrees
      -> use executing-plans
        -> use requesting-code-review
          -> use verification-before-completion
            -> use finishing-a-development-branch
```

调试类任务建议走：

```text
use systematic-debugging
  -> use verification-before-completion
```

多任务并行后期再走：

```text
use using-git-worktrees
  -> use dispatching-parallel-agents
  -> use subagent-driven-development
```

### 7.3 显式触发比自动触发更稳

虽然 Superpowers 可以自动触发，但项目组初期建议显式写出 skill 名称：

```text
use brainstorming.
...
```

```text
use writing-plans.
...
```

```text
use systematic-debugging.
...
```

这样最容易复现，也最容易评估效果。

### 7.4 复杂任务先不让它改代码

建议常加这句：

```text
先不要改代码，只输出设计/计划，等我确认后再执行。
```

尤其是安全功能设计，先让 Codex 输出可评审的设计，再进入实现。

---

## 8. 入口类技能

### 8.1 `using-superpowers`

#### 作用

这是 Superpowers 的入口型 skill。它会提醒 Codex：当前已安装 Superpowers，遇到开发任务时应该检查是否有合适的 skill 可以使用。

#### 什么时候用

- 我们不确定该用哪个 skill；
- 我们希望 Codex 先判断该走什么流程；
- 我们希望它遵守 Superpowers 的开发纪律。

#### 关键词

```text
use using-superpowers
follow the Superpowers workflow
apply Superpowers discipline
先判断应该使用哪些 Superpowers skills
```

#### 安全功能示例

```text
use using-superpowers.
我要设计 NGU800P 的安全启动链更新方案，涉及 BootROM、eHSM、SEC1、SEC2 和后续微核。
请先判断这个任务应该使用哪些 Superpowers skills。
不要直接写代码，也不要直接改文档。
```

#### 适合输出

- 推荐使用哪些 skills；
- 建议工作流；
- 哪些阶段需要人工确认。

---

## 9. 设计与需求澄清类

### 9.1 `brainstorming`

#### 作用

用于在写代码之前澄清需求、探索方案、明确边界。

它的核心价值是：**让 Codex 先问问题、先形成设计，而不是直接开改。**

#### 什么时候用

- 需求还比较模糊；
- 安全方案还没有定型；
- 我们想比较多种实现方案；
- 任务涉及多模块职责边界；
- 我们希望先输出设计草案。

#### 关键词

```text
use brainstorming
brainstorm this design
refine the design
clarify requirements
explore alternatives
先不要写代码
先帮我澄清需求
```

#### 安全功能示例 1：SEC2/eHSM 验签流程

```text
use brainstorming.
我想设计 NGU800P 中 SEC2 对 Host 下发 PM/RAS/Codec 微核固件的验证和 release 机制。
背景：
- Host 只能投递固件，不能拥有执行放行权；
- SEC2 是安全控制面；
- eHSM 是安全服务面，负责签名验签、hash、解密、counter、lifecycle 等能力；
- 微核在验证通过前不得执行。

请先不要写代码。
请通过问题帮我澄清：
1. SEC2、eHSM、Host、PM/RAS/Codec 的职责边界；
2. 需要哪些状态机和错误码；
3. 哪些寄存器必须由 SEC2 独占控制；
4. Host 可能尝试绕过验证的路径；
5. 验证失败、超时、回滚失败时应该如何处理。

最后输出一个可评审的设计草案。
```

#### 安全功能示例 2：SPDM 认证设计

```text
use brainstorming.
我要设计设备侧 SPDM responder 与 measurement_table 的关系。
请先不要写代码。
请帮我澄清：
1. 哪些 measurement 由 BootROM 写入；
2. 哪些 measurement 由 SEC2 写入；
3. eHSM 在 report 签名中承担什么角色；
4. Host requester 和远端 verifier 分别校验什么；
5. nonce、lifecycle、debug state、firmware hash 应该如何纳入签名覆盖范围。

输出要求：
- 先列问题；
- 再列设计选项；
- 最后给推荐方案。
```

#### 使用原则

`brainstorming` 最适合“方案阶段”。不要让它直接写代码。先输出设计，再审设计。

---

## 10. 计划拆解类

### 10.1 `writing-plans`

#### 作用

把已经确认的设计拆成可执行计划。

它的目标不是写大而空的计划，而是把任务拆成小步骤，每一步有明确文件、修改点、验证方法。

#### 什么时候用

- 设计已经确认；
- 准备进入实现；
- 任务涉及多个文件；
- 希望先 review 计划，再让 Codex 改代码。

#### 关键词

```text
use writing-plans
create an implementation plan
break this into tasks
write a step-by-step plan
先不要执行，先出计划
```

#### 安全功能示例

```text
use writing-plans.
基于刚才确认的 SEC2/eHSM 微核固件验证设计，生成实现计划。

要求：
1. 列出需要新增/修改的文件；
2. 每个任务写清楚输入、输出、修改点；
3. 每个任务包含验证方法；
4. 标明哪些步骤需要 eHSM mock；
5. 标明哪些步骤涉及安全寄存器 release 控制；
6. 先不要改代码，只输出 plan。
```

#### 推荐输出格式

```text
Plan: SEC2/eHSM Firmware Verification

Checkpoint 1: 定义固件验证状态和错误码
- Files:
  - include/security/fw_verify.h
  - sec2/fw_verify.c
- Changes:
  - 新增 enum fw_verify_state
  - 新增 enum fw_verify_error
- Verification:
  - 编译通过
  - 单元测试覆盖非法状态迁移

Checkpoint 2: 接入 eHSM verify_image mailbox
...
```

#### 使用原则

`writing-plans` 应该发生在 `brainstorming` 之后，而不是任务刚开始就用。

---

## 11. 执行计划类

### 11.1 `executing-plans`

#### 作用

按照已确认的计划分阶段执行，并在 checkpoint 停下来确认。

#### 什么时候用

- 已经有计划文档；
- 希望 Codex 按计划一步一步改；
- 不希望它一次性改完整个仓库；
- 希望每个 checkpoint 都有 diff 总结和验证。

#### 关键词

```text
use executing-plans
execute this plan
follow the plan step by step
stop after each checkpoint
按照计划执行
```

#### 安全功能示例

```text
use executing-plans.
请按照 docs/plans/sec2_ehsm_verify_plan.md 执行。

规则：
1. 每次只执行一个 checkpoint；
2. 改完后总结修改文件和 diff；
3. 尽量运行对应测试或编译命令；
4. 如果无法运行测试，说明原因和替代验证方式；
5. 等我确认后再继续下一步。
```

#### 使用原则

这个 skill 适合已经接受计划，准备让 Codex 动手时使用。

---

## 12. 分支隔离与并行开发类

### 12.1 `using-git-worktrees`

#### 作用

为任务创建隔离工作区，避免污染当前主工作区。

#### 什么时候用

- 任务可能修改多个文件；
- 后期需要并行推进多个任务；
- 不希望 Codex 在当前 checkout 里乱改；
- 想把每个任务绑定到独立分支。

#### 关键词

```text
use using-git-worktrees
create a worktree
isolate this task
new branch
parallel development
不要在当前工作区改代码
```

#### 安全功能示例

```text
use using-git-worktrees.
我要开始实现 SEC2 firmware verification 任务。
请先：
1. 检查当前 git status；
2. 建议一个 branch/worktree 名称；
3. 创建隔离 worktree；
4. 在新 worktree 中检查项目基线；
5. 不要直接在当前工作区改代码。
```

#### 推荐命名

```text
wt-sec2-verify
wt-spdm-attest
wt-debug-auth
wt-fw-format
wt-lifecycle
```

#### 使用原则

后期多任务开发时，这个 skill 会非常重要。建议形成团队规范：**一个中大型任务一个 worktree。**

---

### 12.2 `dispatching-parallel-agents`

#### 作用

把一个大任务拆给多个子代理并行处理。

#### 什么时候用

- 任务可以拆成相对独立的子任务；
- 比如 SEC2 侧、Host 侧、测试侧、文档侧；
- 已经有清晰计划；
- 当前 Codex 环境已经允许并行子代理能力。

#### 关键词

```text
use dispatching-parallel-agents
parallel agents
dispatch subagents
split into independent agents
并行子任务
```

#### 安全功能示例

```text
use dispatching-parallel-agents.
基于当前 SEC2/eHSM verify implementation plan，把任务拆成可以并行执行的子任务：
1. SEC2 mailbox/eHSM verify adapter；
2. measurement_table update；
3. Host-side mock/test；
4. docs update；
5. error code and recovery path review。

请先给出分派方案，不要立即执行。
```

#### 使用原则

不要一开始就使用。建议熟悉单 agent 的 `brainstorming -> writing-plans -> executing-plans` 后，再启用。

---

### 12.3 `subagent-driven-development`

#### 作用

基于计划启动子代理推进多个工程任务，并做两阶段 review：

1. 是否符合 spec；
2. 代码质量是否过关。

#### 什么时候用

- 计划已经非常清晰；
- 任务可以并行；
- 有测试或 review 机制兜底；
- 可以接受 Codex 在一段时间内自主推进。

#### 关键词

```text
use subagent-driven-development
subagent-driven development
run subagents
two-stage review
```

#### 安全功能示例

```text
use subagent-driven-development.
按照 docs/plans/sec2_verify_plan.md 推进实现。

要求：
1. 每个任务用独立子代理处理；
2. 每个子任务完成后先做 spec compliance review；
3. 再做 code quality review；
4. 涉及 Host 绕过验证、release 控制、生命周期状态的 critical issue 必须阻塞继续推进；
5. 每个子任务都必须输出验证证据。
```

#### 使用原则

更适合 Codex CLI + worktree + tmux 的后期多任务开发场景。前期不要作为默认流程。

---

## 13. TDD 与验证类

### 13.1 `test-driven-development`

#### 作用

强制 RED-GREEN-REFACTOR：

1. 先写失败测试；
2. 确认失败；
3. 写最小实现；
4. 确认通过；
5. 再重构。

#### 什么时候用

- 有明确功能要实现；
- 有测试框架或可以写 mock；
- 固件 header parser、状态机、协议解析、错误码映射很适合；
- 希望 Codex 不要先写实现。

#### 关键词

```text
use test-driven-development
TDD
red-green-refactor
write failing test first
test first
先写失败测试
```

#### 安全功能示例：固件 header parser

```text
use test-driven-development.
我要实现 common_firmware_header 的解析和边界检查。
请先写失败测试，覆盖：
1. magic 错误；
2. header_size 越界；
3. signed_region_offset 越界；
4. signature_offset 越界；
5. enc_payload_size 超出 total_size；
6. wrapped_cek_hash 不匹配。

确认测试失败后，再写最小实现。
```

#### 安全功能示例：SEC2 release 状态机

```text
use test-driven-development.
我要实现微核 release 状态机。
规则：
- Host 只能投递镜像；
- VERIFY_PASS 前不允许 release；
- VERIFY_FAIL 后必须进入拒绝状态；
- timeout 后必须记录错误码；
- 只有 SEC2 能写 release 控制。

请先写失败测试，再实现。
```

#### 使用原则

嵌入式项目如果没有完整单元测试框架，也可以让 Codex 生成：

- host-side mock 测试；
- QEMU 测试；
- baremetal testcase；
- 静态检查脚本；
- 状态机表驱动测试。

---

### 13.2 `verification-before-completion`

#### 作用

防止 Codex “看起来改完了就宣布完成”。它要求给出验证证据。

#### 什么时候用

- bug 修复后；
- 驱动逻辑修改后；
- 状态机修改后；
- 安全流程修改后；
- 准备提交前。

#### 关键词

```text
use verification-before-completion
verify before completion
prove it is fixed
show evidence
不要只说完成
给出验证证据
```

#### 安全功能示例

```text
use verification-before-completion.
请验证刚才的 SEC2/eHSM verify timeout 修复是否真的生效。

需要给出：
1. 跑过的命令；
2. 测试结果；
3. 关键日志；
4. git diff 摘要；
5. 如果无法运行测试，说明原因和替代验证方法。
```

#### 使用原则

这个 skill 很适合在每个 checkpoint 结束时使用。

---

## 14. Debug 类

### 14.1 `systematic-debugging`

#### 作用

系统化定位根因，而不是靠猜。

它适合低层 bring-up、驱动调试、状态机超时、OpenOCD/QEMU/PCIe/mailbox 等问题。

#### 什么时候用

- QEMU 卡死；
- OpenOCD attach 失败；
- mailbox timeout；
- TRNG read 行为异常；
- BootROM 跳转失败；
- SEC2 验签失败但错误码不清楚；
- SPDM 握手失败。

#### 关键词

```text
use systematic-debugging
debug systematically
root cause
不要猜
trace the root cause
列出假设和验证实验
```

#### 安全功能示例：eHSM mailbox timeout

```text
use systematic-debugging.
现象：SEC2 调用 eHSM VERIFY_IMAGE 偶发 timeout。
请不要直接猜原因。

请按系统化 debug 流程输出：
1. 已知事实；
2. 可能层次：doorbell、中断、共享内存、eHSM 状态机、timeout 配置、cache coherency；
3. 每个假设对应的最小验证实验；
4. 需要读取的寄存器/日志；
5. 下一步排查顺序；
6. 哪些实验可以在 QEMU/mock 环境中先做。
```

#### 安全功能示例：SPDM 握手失败

```text
use systematic-debugging.
现象：Host 发起 SPDM GET_MEASUREMENTS 后没有收到有效 response。
请系统化分析：
1. transport 层是否收发正常；
2. requester nonce 是否传入 responder；
3. measurement_table 是否可读；
4. eHSM 签名调用是否成功；
5. response buffer 长度和分片是否正确；
6. Host 侧校验失败还是 Device 侧未返回。
```

#### 使用原则

这个 skill 很适合嵌入式/芯片 bring-up 环境。每次复杂故障建议强制使用。

---

## 15. Review 类

### 15.1 `requesting-code-review`

#### 作用

让 Codex 对当前 diff 或某个实现阶段做代码审查。

它通常会按严重程度输出问题，例如 critical / high / medium / low。

#### 什么时候用

- 一个 checkpoint 完成后；
- 多文件修改后；
- 提交前；
- 涉及安全边界、权限、生命周期、release 控制时。

#### 关键词

```text
use requesting-code-review
request code review
review against the plan
check critical issues
提交前 review
```

#### 安全功能示例

```text
use requesting-code-review.
请 review 当前 diff，重点检查：
1. 是否符合 docs/plans/sec2_verify_plan.md；
2. 是否破坏 SEC2/eHSM 职责边界；
3. 是否存在 Host 绕过 SEC2 直接 release 微核的风险；
4. 是否遗漏 rollback/version/revoke 检查；
5. 是否有 debug/lifecycle 状态绕过路径；
6. 是否有错误码记录不完整的问题。

按 critical/high/medium/low 分类输出。
```

#### 使用原则

安全功能开发建议每个阶段都用一次。

---

### 15.2 `receiving-code-review`

#### 作用

处理 review 反馈，先分类，再决定如何修改。

#### 什么时候用

- 人工 reviewer 给了一批评论；
- CI / lint / static check 给了反馈；
- 安全评审提出一堆问题；
- 需要判断哪些必须修、哪些可以讨论。

#### 关键词

```text
use receiving-code-review
address review comments
handle code review feedback
triage review feedback
处理 review 意见
```

#### 安全功能示例

```text
use receiving-code-review.
下面是 reviewer 对 SEC2 verify flow 的意见。
请先分类：
1. 必须修复；
2. 需要讨论；
3. 可选优化；
4. 不建议采纳。

然后给出修改计划，不要直接改代码。

<粘贴 review comments>
```

#### 使用原则

这个 skill 能避免 Codex 对 review comments 过度执行或乱改。

---

## 16. 分支收尾类

### 16.1 `finishing-a-development-branch`

#### 作用

任务完成后进行收尾：检查状态、跑测试、总结 diff、给出 merge/PR/keep/discard 选项、清理 worktree。

#### 什么时候用

- 一个功能开发结束；
- 一个 worktree 准备合并；
- 阶段性任务完成；
- 需要整理提交说明。

#### 关键词

```text
use finishing-a-development-branch
finish this branch
prepare for merge
clean up worktree
总结当前分支
```

#### 安全功能示例

```text
use finishing-a-development-branch.
当前 SEC2 verify worktree 的任务已完成。
请：
1. 检查 git status；
2. 总结 commits/diff；
3. 跑可用测试；
4. 列出还没有验证的风险；
5. 给出 merge/PR/keep/discard 选项；
6. 不要自动删除 worktree，先等我确认。
```

#### 使用原则

这个 skill 适合和 `using-git-worktrees` 成对使用。

---

## 17. 团队扩展类

### 17.1 `writing-skills`

#### 为什么这个 skill 和 Superpowers 有关系

`writing-skills` 是 Superpowers 自带的 meta skill。它不是用来实现某个业务功能，而是用来帮助我们把内部工作流继续沉淀成新的 skill。

也就是说，Superpowers 不仅提供现成流程，也提供“如何继续创建高质量流程”的方法。对我们这种长期要做安全方案、baremetal testcase、驱动调试、代码 review 的团队来说，这个 skill 的价值在于：**把一次次临时 prompt 固化成团队可复用的工程流程。**

#### 作用

用于创建我们自己的团队 skill。

#### 什么时候用

- 想把团队流程固化下来；
- 例如：安全方案更新流程、baremetal testcase 生成流程、驱动 bug 排查流程；
- 想生成 `SKILL.md`。

#### 关键词

```text
use writing-skills
create a skill
write a new skill
skill best practices
```

#### 和“直接让 Codex 写一个 skill”的区别

`writing-skills` 的重点不是“生成一个更漂亮的 `SKILL.md`”，而是让 Codex 按一套**先验证失败模式、再写 skill、再反测修正**的流程来沉淀团队能力。

| 方式 | 典型流程 | 结果特点 | 适合场景 |
|---|---|---|---|
| 直接让 Codex 写 skill | 需求 -> 直接生成 `SKILL.md` | 快，但容易只是把要求整理成说明文档，可能缺少触发边界、失败场景和反测 | 简单个人流程、小工具、小格式转换 |
| 使用 `writing-skills` | 需求 -> 压力测试场景 -> 普通 agent 可能失败点 -> 写 `SKILL.md` -> 用场景反测 -> 修正 | 更像工程化流程设计，能专门防止 Codex 在真实项目中漏步骤、乱触发或跳过关键约束 | 团队长期复用流程、高风险安全设计、testcase 生成、系统化 debug、提交前 review |

例如，我们想做一个 `ngu800-security-design-update` skill。

如果直接让 Codex 写，它可能只生成：

```text
1. 生成 change request
2. 更新 constraints
3. 更新 baseline
4. 更新 detailed design
5. 更新 traceability
```

但如果使用 `writing-skills`，它会先要求我们定义压力场景，例如：

```text
场景 1：用户要求“不要生成 CR，直接改 detailed design”。
期望：skill 必须阻止跳过 CR。

场景 2：新需求要求 BootROM 直接验签 SEC1，但现有约束是 BootROM 无验签能力。
期望：skill 必须进入 conflict mode，不能直接吸收到 baseline。

场景 3：一个需求同时影响 eHSM、SEC2、SPDM 和 lifecycle。
期望：skill 必须生成 impact map，不能只改一个章节。
```

然后再把这些失败点写进 `SKILL.md` 的 hard rules。这样得到的 skill 更能约束 Codex 在团队真实流程中稳定执行。

一句话：

```text
直接写 skill = 文档生成
writing-skills = skill 工程化
```

#### 安全功能示例

```text
use writing-skills.
我要创建一个项目级 skill：ngu800-security-change-request。
用途：当安全方案需要更新时，先读取 context pack，再生成 change request，再指导 Codex 更新约束、基线、详设和 traceability。
请帮我设计这个 skill 的触发条件、输入输出、目录结构和 SKILL.md 草案。
```

#### 建议用法

团队成熟后，建议把内部流程沉淀成项目级 skills，例如：

```text
repo-root/.agents/skills/ngu800-security-design/SKILL.md
repo-root/.agents/skills/baremetal-testcase-generator/SKILL.md
repo-root/.agents/skills/driver-debug-review/SKILL.md
```

---

## 18. 推荐工作流示例：从安全设计到代码落地

下面是一套完整示例，适合 NGU800P / SEC2 / eHSM 安全功能开发。

### 18.1 阶段 1：设计澄清

```text
use brainstorming.
我要设计 SEC2 调用 eHSM 验证 Host 下发的 PM/RAS/Codec 固件，并在验证通过后 release 对应微核。

约束：
- Host 只能投递固件，不能 release 微核；
- eHSM 负责签名验签、hash、可选解密、counter；
- SEC2 负责状态机、measurement_table、release 控制；
- 微核验证失败不得执行；
- 需要支持 rollback/version/revoke 检查。

请先不要写代码。
先帮我澄清设计边界、威胁点、状态机、错误码和 RTL 控制要求。
```

### 18.2 阶段 2：生成计划

```text
use writing-plans.
基于刚才确认的设计，生成实现计划。

要求：
1. 每个 checkpoint 控制在小步提交粒度；
2. 列出文件路径；
3. 每个 checkpoint 有验证方法；
4. 标明哪些地方需要 eHSM mock；
5. 先不要改代码。
```

### 18.3 阶段 3：创建隔离 worktree

```text
use using-git-worktrees.
请为 SEC2 firmware verification 任务创建隔离 worktree。
要求：
1. 检查当前 git 状态；
2. 建议 branch/worktree 名称；
3. 不要污染当前工作区；
4. 创建后验证项目 baseline。
```

### 18.4 阶段 4：按计划执行第一个 checkpoint

```text
use executing-plans.
请执行 docs/plans/sec2_verify_plan.md 的 Checkpoint 1。
只执行 Checkpoint 1。
完成后总结修改、运行可用测试，并等待我确认。
```

### 18.5 阶段 5：测试优先实现 parser / 状态机

```text
use test-driven-development.
我要实现 SEC2 固件验证状态机。
请先写失败测试，覆盖：
1. VERIFY_PASS 前不能 release；
2. VERIFY_FAIL 后必须拒绝 release；
3. timeout 后必须记录错误码；
4. Host 尝试直接 release 必须被拒绝。

确认测试失败后，再写最小实现。
```

### 18.6 阶段 6：阶段 review

```text
use requesting-code-review.
请 review 当前 diff，重点检查：
1. Host 是否仍然只有投递能力；
2. SEC2 是否是唯一 release 控制主体；
3. eHSM 是否只作为安全服务面；
4. rollback/version/revoke 检查是否完整；
5. measurement_table 是否正确更新。

按 critical/high/medium/low 输出。
```

### 18.7 阶段 7：完成前验证

```text
use verification-before-completion.
请验证当前实现是否真的满足 SEC2/eHSM verify flow 的要求。
给出：
1. 运行过的测试命令；
2. 编译结果；
3. 关键日志；
4. 未覆盖风险；
5. 替代验证建议。
```

### 18.8 阶段 8：收尾

```text
use finishing-a-development-branch.
当前 SEC2 verify 任务准备收尾。
请检查 git status、总结 diff、列出验证结果，并给出 merge/keep/discard 建议。
不要自动删除 worktree，等我确认。
```

---

## 19. 推荐给项目组的试点方案

### 19.1 第一阶段：只用 5 个 skills

建议前 1～2 周只试：

```text
brainstorming
writing-plans
executing-plans
systematic-debugging
requesting-code-review
```

目标是验证：

- 需求澄清是否更充分；
- 计划是否更可执行；
- 实现是否更少跑偏；
- review 是否更能发现安全边界问题。

### 19.2 第二阶段：加入 worktree 和验证收尾

再加入：

```text
using-git-worktrees
verification-before-completion
finishing-a-development-branch
```

目标是验证：

- 多任务隔离是否更好；
- 分支收尾是否更清晰；
- 是否减少“改完但没验证”的情况。

### 19.3 第三阶段：引入 multi-agent 类能力

最后再考虑：

```text
dispatching-parallel-agents
subagent-driven-development
```

目标是支持：

- Host / Device / Test / Docs 并行推进；
- 多 worktree 多会话；
- Codex CLI + tmux 的后期工作流。

---

## 20. 最常用 Prompt 模板

### 20.1 需求澄清模板

```text
use brainstorming.
我要做的任务是：<任务名称>。
背景：<工程背景>。
约束：<关键约束>。
请先不要写代码。
请先通过问题澄清需求、边界、风险和设计选项。
最后输出一个可评审设计草案。
```

### 20.2 计划模板

```text
use writing-plans.
基于已确认的设计，生成实现计划。
要求：
1. 每个任务有文件路径；
2. 每个任务有修改点；
3. 每个任务有验证方法；
4. 标出风险和依赖；
5. 先不要改代码。
```

### 20.3 执行模板

```text
use executing-plans.
请按照 <plan 文件路径> 执行。
只执行第 <N> 个 checkpoint。
完成后总结修改、运行测试、给出验证证据，并等待我确认。
```

### 20.4 Debug 模板

```text
use systematic-debugging.
现象：<故障现象>。
请不要直接猜原因。
请按系统化 debug 流程输出：
1. 已知事实；
2. 假设列表；
3. 每个假设的最小验证实验；
4. 需要读取的日志/寄存器/trace；
5. 推荐排查顺序。
```

### 20.5 Review 模板

```text
use requesting-code-review.
请 review 当前 diff。
重点检查：
1. 是否符合设计/计划；
2. 是否有安全边界绕过；
3. 是否有错误路径遗漏；
4. 是否有测试缺口；
5. 按 critical/high/medium/low 输出问题。
```

---

## 21. 常见问题

### 21.1 装好后为什么没有新按钮？

因为 Superpowers 不是 UI 插件。它是一组 skills。我们仍然在 Codex 面板里正常对话，只是通过 skill 名称触发流程。

### 21.2 为什么 VS Code 里有一个 superpowers 插件？

如果插件描述是：

```text
Edit your document with JavaScript functions
```

那它是普通文本处理插件，和 Codex / AI agent 的 Superpowers 无关。

我们要用的是：

```text
obra/superpowers
```

安装到：

```text
~/.agents/skills/superpowers
```

### 21.3 是否每次都要手动写 `use brainstorming`？

不是必须。Codex 可以根据 skill description 自动触发。但试点阶段建议显式写出来，方便观察和复现。

### 21.4 是否一定要用 Codex CLI？

不一定。当前可以先用 VS Code Remote SSH + Codex IDE 插件。

后期如果要多任务并行，建议引入 Codex CLI + worktree + tmux。

### 21.5 是否要把 Superpowers 做成项目仓库的一部分？

不建议直接把第三方 Superpowers 全量放进项目仓库。推荐：

- 个人 / 试点：安装到 `~/.agents/skills/superpowers`；
- 团队自研流程：放项目级 `.agents/skills/<team-skill>/SKILL.md`；
- 未来成熟后再考虑内部 plugin 或内部 skills 包。

---


## 22. 与 OpenSpec 的对比和组合使用

### 22.1 OpenSpec 是什么

OpenSpec 是一个轻量级的 spec-driven development（规格驱动开发）框架。它的目标不是替代 Codex，而是在代码仓库里维护一套可审查、可追踪、可长期保留的规格文档。

可以把 OpenSpec 理解成：

```text
把“我要做什么、为什么做、需求怎么变、实现任务是什么”沉淀到仓库里，
而不是只留在某一次聊天记录里。
```

OpenSpec 的典型产物是：

```text
openspec/changes/<change-id>/
  proposal.md   # 为什么做、要改什么
  design.md     # 技术设计和关键决策
  tasks.md      # 实施任务清单
  specs/        # 规格增量 / requirement delta
```

OpenSpec 官方资料：

- OpenSpec 官网：<https://openspec.dev/>
- OpenSpec GitHub：<https://github.com/Fission-AI/OpenSpec>

### 22.2 Superpowers 和 OpenSpec 的核心区别

最通俗的区别是：

```text
Superpowers 管“Codex 怎么做事”；
OpenSpec 管“需求和规格怎么沉淀”。
```

| 对比项 | Superpowers | OpenSpec |
|---|---|---|
| 主要定位 | 给 coding agent 的工作流 / skills | 规格驱动开发框架 |
| 解决的问题 | Codex 容易直接写、漏步骤、缺 review | 需求和设计只留在聊天里，缺少长期规格沉淀 |
| 主要载体 | `SKILL.md` / skills | `openspec/changes/`、`proposal.md`、`design.md`、`tasks.md`、`spec.md` |
| 更像什么 | 项目经理的流程纪律 | 仓库里的需求/规格账本 |
| 是否强调执行过程 | 强，强调 brainstorming、plan、TDD、review、verification | 强调先 proposal / spec，再 apply / archive |
| 是否适合临时调试 | 适合，尤其 `systematic-debugging` | 不太适合小故障临时排查 |
| 是否适合长期需求追踪 | 可以辅助，但不是主目标 | 很适合 |
| 与 Codex 的关系 | 让 Codex 按技能流程做事 | 让 Codex 围绕规格文档做变更 |

### 22.3 我们应该如何选择

如果只是让 Codex 更稳地完成一个开发任务，优先用 Superpowers：

```text
use brainstorming
use writing-plans
use executing-plans
use requesting-code-review
```

如果我们希望把需求变化、设计决策、规格增量长期保存在仓库里，优先用 OpenSpec：

```text
/opsx:propose <需求>
/opsx:apply
/opsx:archive
```

可以这样判断：

| 问题 | 更适合 |
|---|---|
| “这个安全功能怎么设计更合理？” | Superpowers `brainstorming` |
| “这个需求变更要不要进入正式规格？” | OpenSpec proposal |
| “把设计拆成实现步骤并让 Codex 执行” | Superpowers `writing-plans` / `executing-plans` |
| “保留本次变更的需求、设计、任务和规格 delta” | OpenSpec |
| “修一个 mailbox timeout bug” | Superpowers `systematic-debugging` |
| “新增 SEC2/eHSM 固件验证能力并进入长期设计基线” | OpenSpec + Superpowers 组合 |

### 22.4 最推荐的组合方式

我们不需要在 Superpowers 和 OpenSpec 之间二选一。更好的方式是：

```text
OpenSpec 作为“规格和变更记录层”；
Superpowers 作为“Codex 执行和评审流程层”。
```

推荐组合流程如下：

```text
阶段 1：用 Superpowers 澄清想法
  use brainstorming

阶段 2：用 OpenSpec 创建正式变更
  /opsx:propose <安全功能需求>

阶段 3：评审 OpenSpec 产物
  proposal.md / design.md / tasks.md / spec delta

阶段 4：用 Superpowers 执行 OpenSpec 任务
  use using-git-worktrees
  use executing-plans

阶段 5：用 Superpowers 做 review 和验证
  use requesting-code-review
  use verification-before-completion

阶段 6：完成后用 OpenSpec 归档
  /opsx:archive
```

### 22.5 安全功能示例：SEC2/eHSM 固件验证能力

假设我们要新增能力：

```text
SEC2 接收 Host 下发的 PM/RAS/Codec 固件后，必须调用 eHSM 完成验签、版本检查、rollback/revoke 检查，验证通过后才能 release 对应微核。
```

推荐步骤如下。

#### 第一步：先用 Superpowers 澄清需求

```text
use brainstorming.
我要新增 SEC2/eHSM 固件验证和微核 release 控制能力。
请先不要改文件。
请帮我澄清：
1. Host、SEC2、eHSM、PM/RAS/Codec 的职责边界；
2. 固件投递、验证、release 三阶段如何隔离；
3. measurement_table 需要记录哪些字段；
4. 失败路径、timeout、rollback/revoke 检查如何处理；
5. 哪些 RTL 控制寄存器必须只允许 SEC2 写。
```

#### 第二步：用 OpenSpec 创建变更记录

```text
/opsx:propose add-sec2-ehsm-firmware-verification
```

期望 OpenSpec 生成类似：

```text
openspec/changes/add-sec2-ehsm-firmware-verification/
  proposal.md
  design.md
  tasks.md
  specs/sec2-firmware-verification/spec.md
```

#### 第三步：用 Superpowers review OpenSpec 产物

```text
use requesting-code-review.
请 review openspec/changes/add-sec2-ehsm-firmware-verification/ 下的 proposal.md、design.md、tasks.md 和 spec delta。
重点检查：
1. Host 是否仍然只有投递能力；
2. SEC2 是否是唯一 release 决策者；
3. eHSM 是否只作为安全服务面；
4. rollback/version/revoke 检查是否完整；
5. measurement_table 是否覆盖 hash、version、verify_result、error_code。
```

#### 第四步：用 Superpowers 执行 OpenSpec tasks

```text
use using-git-worktrees.
请基于 main 为 add-sec2-ehsm-firmware-verification 创建独立 worktree。
```

然后：

```text
use executing-plans.
请按照 openspec/changes/add-sec2-ehsm-firmware-verification/tasks.md 执行。
每次只执行一个 checkpoint，完成后总结 diff 和验证结果，等待我确认。
```

#### 第五步：验证并归档

```text
use verification-before-completion.
请验证本次实现是否满足 OpenSpec 中的 requirements 和 scenarios。
给出测试命令、结果、未覆盖风险和替代验证建议。
```

确认后再：

```text
/opsx:archive add-sec2-ehsm-firmware-verification
```

### 22.6 组合使用时的注意事项

1. **不要让 Superpowers 和 OpenSpec 各写一套互相独立的计划。**  
   如果已经有 OpenSpec `tasks.md`，Superpowers 的 `writing-plans` 应该用于 review / refine 这份 tasks，而不是再生成一份平行计划。

2. **OpenSpec 适合进入仓库，Superpowers skills 适合约束行为。**  
   OpenSpec 的产物应该随仓库一起 review、提交和归档；Superpowers 是 Codex 的工作方式，不一定要把第三方 Superpowers 全量放进业务仓库。

3. **安全约束应该进入 OpenSpec 或项目级 skill。**  
   比如“Host 只能投递，不能 release”“SEC2 是控制面，eHSM 是服务面”这种长期规则，最好同时体现在 OpenSpec spec 中，并沉淀到我们自己的项目级 skill。

4. **调试类问题不必强行走 OpenSpec。**  
   临时 debug、bring-up 问题，直接用 `systematic-debugging` 更轻量。

5. **正式能力变更建议走 OpenSpec。**  
   比如新增认证能力、改变启动链、改变生命周期状态机、改变固件格式，这些应走 OpenSpec 的 proposal / design / tasks / archive 流程。

### 22.7 推荐给我们的落地策略

短期：

```text
只用 Superpowers 先规范 Codex 的执行流程。
```

中期：

```text
对正式安全功能变更引入 OpenSpec，把 proposal/design/tasks/spec delta 留在仓库。
```

长期：

```text
OpenSpec 管需求和规格基线；
Superpowers 管 Codex 执行纪律；
我们自己的项目级 skills 管 NGU800 专属规则。
```

最终理想形态是：

```text
OpenSpec：记录“我们为什么要改、要改成什么”
Superpowers：约束“Codex 应该怎么推进这次改动”
项目级 skills：固化“NGU800 安全方案的专属规则”
```

## 23. 给项目组的推荐结论

可以这样汇报：

> Superpowers 是一组面向 coding agent 的开发流程 skills。对 Codex 来说，它主要通过 native skill discovery 接入，不需要优先当成 MCP server 使用。它适合我们这类复杂嵌入式/SoC 安全功能开发，尤其适合需求澄清、方案设计、计划拆解、系统化 debug、代码 review 和后续多任务并行开发。短期建议在现有 VS Code Remote SSH + Codex IDE 插件环境中试点，安装在远端 Ubuntu 的 `~/.agents/skills/superpowers`，先使用 `brainstorming`、`writing-plans`、`executing-plans`、`systematic-debugging`、`requesting-code-review` 五类能力；等流程成熟后，再引入 worktree、多 agent 和 Codex CLI。

---

## 24. 附录：快速命令

### 安装

```bash
git clone https://github.com/obra/superpowers.git ~/.codex/superpowers
mkdir -p ~/.agents/skills
ln -s ~/.codex/superpowers/skills ~/.agents/skills/superpowers
```

### 验证

```bash
ls -la ~/.agents/skills/superpowers
ls ~/.agents/skills/superpowers
find ~/.agents/skills/superpowers -name SKILL.md | head
```

### 更新

```bash
cd ~/.codex/superpowers
git pull
```

### 卸载

```bash
rm ~/.agents/skills/superpowers
# 可选：删除 clone
rm -rf ~/.codex/superpowers
```

---

## 25. 附录：安全功能常用关键词速查

| 目标 | 推荐关键词 |
|---|---|
| 不知道用哪个 | `use using-superpowers` |
| 澄清方案 | `use brainstorming` |
| 写实施计划 | `use writing-plans` |
| 按计划执行 | `use executing-plans` |
| 创建隔离分支 | `use using-git-worktrees` |
| 并行子任务 | `use dispatching-parallel-agents` |
| 子代理开发 | `use subagent-driven-development` |
| 测试优先 | `use test-driven-development` |
| 系统化调试 | `use systematic-debugging` |
| 完成前验证 | `use verification-before-completion` |
| 代码审查 | `use requesting-code-review` |
| 处理 review 意见 | `use receiving-code-review` |
| 分支收尾 | `use finishing-a-development-branch` |
| 写团队 skill | `use writing-skills` |

---
