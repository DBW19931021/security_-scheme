# Bootloader TRNG Self-Test Deep-Dive Document Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 形成一份可用于Bootloader代码Review的中文TRNG自检流程与统计原理专题文档。

**Architecture:** 以`selftest_trng_test()`为核心，向下解释`cpt_get_rand()`及当前硬件TRNG配置，向上连接三次重试、结果位图、启动自检和Host Mailbox命令。源码事实、数学推导和Review判断分别标识，并接入`03_bootloader_deep_review.md`主索引。

**Tech Stack:** Markdown、Mermaid、C源码引用、Poker/卡方统计公式、CodeGraph。

## Global Constraints

- 文档使用简体中文，代码符号、宏和错误码保留英文。
- 文档保存在`docs/review/`，编号使用`11`。
- 不把统计检验通过写成“证明随机数绝对安全”。
- 由阈值反推的卡方含义必须标记为分析结论，不冒充vendor注释。
- 新专题必须登记到`03_bootloader_deep_review.md`并提供返回主索引的链接。
- 当前目录不是Git工作树，不执行提交操作。

---

### Task 1: 编写TRNG自检专题文档

**Files:**
- Create: `docs/review/11_bootloader_trng_selftest_deep_dive.md`

**Interfaces:**
- Consumes: BL `selftest.c`、`crypto_lib_api.c`、TRNG配置/实现、`secure_boot.c`、Mailbox parser和Host demo/API。
- Produces: 自检调用链、采样过程、4-bit/8-bit Poker原理、阈值推导、结果状态和Review发现。

- [x] **Step 1: 固化源码调用链与数据流**

记录启动自动自检、Host触发自检、三次重试、1280字节采样、结果位图和启动失败处理。

- [x] **Step 2: 固化统计原理和判定公式**

分别给出`m=4`和`m=8`的样本数、分类数、期望频数、平方和、卡方等价公式及源码阈值。

- [x] **Step 3: 记录实现边界和Review发现**

明确`m=4 OR m=8`、三次任一通过、后处理输出、硬编码阈值、Host demo覆盖返回码等观察。

### Task 2: 接入Bootloader Review主索引

**Files:**
- Modify: `docs/review/03_bootloader_deep_review.md`

**Interfaces:**
- Consumes: Task 1生成的TRNG专题文档。
- Produces: 主Review到TRNG专题的稳定入口。

- [x] **Step 1: 添加编号11的专题索引行**

索引描述应覆盖Poker统计、重试、结果位图、启动/Host触发和安全边界。

### Task 3: 验证文档结构与链接

**Files:**
- Verify: `docs/review/11_bootloader_trng_selftest_deep_dive.md`
- Verify: `docs/review/03_bootloader_deep_review.md`

**Interfaces:**
- Consumes: Task 1和Task 2的Markdown结果。
- Produces: 无占位符、链接有效、公式参数与源码一致的最终文档。

- [x] **Step 1: 校验关键事实和章节**

Run: `rg -n 'POKER_ROUND|POKER_LENGTH|409600|414492|6400|7952|TRNG_RETRY_COUNT|m=4|m=8' docs/review/11_bootloader_trng_selftest_deep_dive.md`

Expected: 所有关键采样参数、阈值和重试逻辑均有匹配。

- [x] **Step 2: 校验相对链接和主索引**

Run: 提取本次Markdown文件中的相对链接逐一执行`test -e`，并确认`03`包含`11_bootloader_trng_selftest_deep_dive.md`。

Expected: 无失效链接且主索引匹配一次。
