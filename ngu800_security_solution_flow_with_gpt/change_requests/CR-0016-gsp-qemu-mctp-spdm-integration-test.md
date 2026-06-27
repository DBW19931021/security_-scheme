# CR-0016 GSP QEMU MCTP/SPDM Integration Test

> 2026-06-11 ownership update: CR-0017 partially supersedes the rule that all
> responder runtime belongs to `tests/qemu`. Requester/test fixtures remain
> QEMU-only; responder endpoint, adapter, and persistent task now belong to the
> production service.

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0016` |
| Title | GSP QEMU 双 task MCTP/SPDM 设备认证集成测试 |
| Status | `applied` |
| Mode | `implementation-only` |
| Owner | security owner / project owner |
| Reviewer | Codex follow-up review |
| Created Date | 2026-06-10 |
| Source / Context Pack | 用户确认的完整 QEMU 集成设计 |
| Related Decision ID | `DEC-0021` |

## 2. 背景

现有 MCTP/SPDM endpoint 已通过 host 测试和 GSP target 编译，但公共入口没有
启动 runtime，也没有 QEMU 串口 PASS/FAIL 证据。当前 runtime 还是测试性质，
却位于生产 `src/mctp`，与“所有测试代码放在 tests”约束不一致。

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| Runtime ownership | 双 ring、双 endpoint、SPDM responder、measurement、requester 和 task 全部迁移到 `components/security/tests/qemu` | 测试实现与生产协议组件分离 | `[CONFIRMED]` |
| Public entry | GSP `main.c` 仅在测试宏下调用一个 test 接口 | 保持公共入口轻量且可审查 | `[CONFIRMED]` |
| Task lifecycle | requester 完成后自删除，responder 保持运行 | requester 是一次性 test driver，responder 是服务 task | `[CONFIRMED]` |
| Build scope | 只在 `TARGET=gsp` 且 `board=qemu` 时编译和启用 | 不污染真实板和其他 target | `[CONFIRMED]` |
| Result | 串口使用唯一 PASS/FAIL marker，外部脚本解析并返回状态 | 建立自动化 target evidence | `[CONFIRMED]` |

## 4. 影响范围

| 范围 | 影响 |
|---|---|
| `components/security/tests/qemu` | 新增 QEMU runtime、header 和 runner |
| `components/security/src/mctp` | 删除测试 runtime |
| `components/security/sub.mk` | 增加 GSP+QEMU 条件构建 |
| `solutions/gsp/app/src/main.c` | 增加宏保护下的单一测试调用 |
| OpenSpec / guide / evidence | 记录替代约束和执行证据 |

## 5. 不允许改变的内容

- MCTP/SPDM wire format、固定 EID、Tag 和 shared packet ring 契约。
- Root of Trust、Host trust boundary、证书、签名和 measurement 安全语义。
- 非 GSP/QEMU 构建行为。
- `pre_main.c` 的 scheduler 启动和 app task wrapper。

## 6. 验收标准

- [x] 测试 runtime 不再位于生产 `src/mctp`。
- [x] GSP QEMU 镜像包含两个真实 static task。
- [x] requester 实现 connection、GSP measurement 和 attestation report 流程。
- [x] requester 输出唯一 marker 后自删除，responder 继续运行。
- [x] 外部脚本在 PASS 时返回零，FAIL/timeout/早退时返回非零。
- [ ] 真实 QEMU 串口 PASS。当前本地 machine 未映射 GSP 高地址 SRAM/UART，
      固件在进入 `main()` 前被阻塞。
- [x] host、OpenSpec、GSP build 和回归验证通过。

## 7. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-06-10 |
| 修改文件 | `components/security/tests/qemu/*`; `components/security/sub.mk`; `solutions/gsp/app/src/main.c`; component OpenSpec/evidence/code guide |
| 未完成项 | QEMU 平台需实现 GSP `0x101008080000` SRAM、`0x101008208000` UART 及对应 timer/interrupt map |
| 执行说明 | 代码、runner、host 回归和 GSP 链接已完成；真实 QEMU 运行由平台地址模型阻塞，未误报 PASS |
