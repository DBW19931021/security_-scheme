# eHSM Review Deep-Dive Index Links Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让Bootloader Review主文档完整索引现有专题文档，并确保每篇专题文档可返回主Review入口。

**Architecture:** `03_bootloader_deep_review.md`作为唯一的专题文档主索引，集中登记`04`到`10`。每篇专题文档只增加一条返回主索引的引用，避免形成难以维护的全互链；OpenSpec负责约束后续新增专题同步更新索引。

**Tech Stack:** Markdown、OpenSpec、Shell链接校验。

## Global Constraints

- Review正文使用简体中文，源码符号和文件名保留英文。
- 不修改专题文档既有技术结论。
- 所有相对链接必须可从所在Markdown文件解析到真实文件。
- 当前目录不是Git工作树，不执行提交操作。

---

### Task 1: 补齐Bootloader Review主索引

**Files:**
- Modify: `docs/review/03_bootloader_deep_review.md`

**Interfaces:**
- Consumes: `docs/review/04_*.md`到`docs/review/10_*.md`。
- Produces: 覆盖全部现有专题文档的稳定入口。

- [x] **Step 1: 在“专题文档索引”表中登记04到10**

每个文件只登记一次，并概括其实际覆盖范围。

- [x] **Step 2: 校验索引覆盖**

Run: `for n in 04 05 06 07 08 09 10; do rg -q "${n}_" docs/review/03_bootloader_deep_review.md; done`

Expected: 命令退出码为0。

### Task 2: 增加专题文档返回链接

**Files:**
- Modify: `docs/review/04_bootloader_crt0_m130_startup_deep_dive.md`
- Modify: `docs/review/05_bootloader_m130_interrupt_deep_dive.md`
- Modify: `docs/review/06_bootloader_m130_soc_memory_remap_deep_dive.md`
- Modify: `docs/review/07_bootloader_secboot_report_error_deep_dive.md`
- Modify: `docs/review/08_ehsm_mailbox_host_fw_otp_read_flow.md`
- Modify: `docs/review/09_ehsm_debug_auth_challenge_response_deep_dive.md`
- Modify: `docs/review/10_bootloader_otp_key_generation_host_device_flow.md`

**Interfaces:**
- Consumes: `03_bootloader_deep_review.md`主入口。
- Produces: 每篇专题到主入口的统一回链。

- [x] **Step 1: 在每篇H1后增加统一引用**

使用“主Review索引”文本链接到`03_bootloader_deep_review.md`，不改变正文编号。

- [x] **Step 2: 校验回链数量**

Run: `rg -l '03_bootloader_deep_review.md' docs/review/0{4,5,6,7,8,9}_*.md docs/review/10_*.md | wc -l`

Expected: 输出`7`。

### Task 3: 固化OpenSpec维护规则并验证链接

**Files:**
- Modify: `openspec/specs/vendor-ehsm-review/spec.md`

**Interfaces:**
- Consumes: 主索引和专题文档引用约定。
- Produces: 后续新增专题文档时必须同步登记和回链的规范。

- [x] **Step 1: 新增专题索引维护Requirement和Scenario**

规则要求新建或重命名专题文档时同步更新`03_bootloader_deep_review.md`，并在专题文档中保留返回主入口的链接。

- [x] **Step 2: 校验全部Markdown相对链接**

Run: 对本次修改的Markdown文件提取相对链接，并逐一执行`test -e`。

Expected: 无失效链接，命令退出码为0。
