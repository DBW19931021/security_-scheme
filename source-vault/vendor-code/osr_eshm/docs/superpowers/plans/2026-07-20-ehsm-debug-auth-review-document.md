# eHSM Debug Auth Review Document Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 形成一份可用于项目组代码评审和方案介绍的中文 eHSM Debug Auth 深度文档。

**Architecture:** 以 Host demo/API 发起的 challenge-response 为主线，向下追踪 mailbox、BL/FW服务、OTP信任锚、算法分支和调试端口控制。BL实现作为主要证据，FW实现用于说明运行期差异，并单列代码观察和量产建议。

**Tech Stack:** Markdown、Mermaid、C源码引用、CodeGraph。

## Global Constraints

- 文档必须使用中文，文件格式为Markdown。
- 文档保存在 `docs/review/`，编号使用 `09`。
- 区分演示代码、产品机制和评审建议，不把推断写成已实现能力。
- 所有关键结论必须关联源码文件、函数或宏。

---

### Task 1: 编写Debug Auth深度文档

**Files:**
- Create: `docs/review/09_ehsm_debug_auth_challenge_response_deep_dive.md`

**Interfaces:**
- Consumes: Host demo/API、`mb.h`、BL `dbgauth.c`、FW `dbgauth_srv.c`、OTP和密码封装源码。
- Produces: Debug Auth调用链、状态机、算法数据流、信任边界和安全发现。

- [x] **Step 1: 固化调用链和数据结构**

记录 `ehsm_get_challenge -> mailbox -> dbgauth_get_challenge_handle -> ehsm_debug_auth -> dbgauth_ehsm_debug_auth`，并列出48字节challenge、公钥、签名和调试位图格式。

- [x] **Step 2: 固化算法和OTP信任锚**

分别记录SM2、ECDSA、RSA、AES-CMAC和SM4-CMAC的输入检查、公钥Hash验证、challenge摘要及签名/MAC校验。

- [x] **Step 3: 固化状态机与安全分析**

说明challenge生成、按类型保存、覆盖、消费和清除；区分demo本地签名与量产外部授权，并记录位图未纳入签名、无会话绑定等代码观察。

- [x] **Step 4: 检查文档结构**

Run: `rg -n '^#|^```mermaid|安全发现|量产' docs/review/09_ehsm_debug_auth_challenge_response_deep_dive.md`

Expected: 章节、Mermaid图、安全发现和量产建议均存在。

### Task 2: 接入Bootloader Review主文档

**Files:**
- Modify: `docs/review/03_bootloader_deep_review.md`

**Interfaces:**
- Consumes: Task 1生成的独立深度文档。
- Produces: 从BL主review文档到Debug Auth深度文档的稳定入口。

- [x] **Step 1: 添加文档索引**

在Debug/Auth相关章节附近添加 `09_ehsm_debug_auth_challenge_response_deep_dive.md` 链接，不改动已有评审结论。

- [x] **Step 2: 校验引用和Markdown**

Run: `rg -n '09_ehsm_debug_auth_challenge_response_deep_dive|debug_auth' docs/review/03_bootloader_deep_review.md docs/review/09_ehsm_debug_auth_challenge_response_deep_dive.md`

Expected: 主文档存在入口，深度文档存在关键函数引用。

- [x] **Step 3: 扫描占位符和英文主叙述**

Run: `rg -n 'TBD|TODO|implement later|fill in' docs/review/09_ehsm_debug_auth_challenge_response_deep_dive.md`

Expected: 无匹配。
