# CR-0017 SPDM Responder Production Service

## 1. 基本信息

| 字段 | 内容 |
|---|---|
| CR ID | `CR-0017` |
| Title | 将 SPDM responder 长期运行职责抽取为生产 service |
| Status | `applied` |
| Mode | `implementation-only` |
| Owner | security owner / project owner |
| Reviewer | Codex follow-up review |
| Created Date | 2026-06-11 |
| Source / Context Pack | 用户确认的 `spdm_responder_service` 模块划分与本轮“开始调整”指令 |
| Related Decision ID | `DEC-0022` |

## 2. 背景

CR-0016 为保证测试代码不污染生产目录，将当时全部 QEMU runtime 迁入
`tests/qemu`。后续设计复核确认：requester、测试 measurement 和结果标记属于
测试，但 responder endpoint、libspdm responder 和长期 task 是正式 Device
职责，应在接入真实 mailbox/shared SRAM 前形成稳定生产接口。

## 3. 设计裁决

| Topic | Decision | Reason | Status |
|---|---|---|---|
| Production service | 新增 `spdm_responder_service`，拥有 responder endpoint、adapter 和 static task | 避免真实硬件接入时再次搬迁核心服务 | `[CONFIRMED]` |
| Test ownership | QEMU 测试继续拥有双 ring、requester、测试 measurement 和 PASS/FAIL | 保持测试夹具与生产服务分离 | `[CONFIRMED]` |
| Public entry | `main.c` 仍只调用 `mctp_spdm_qemu_test_start()` | 公共入口不感知内部初始化细节 | `[CONFIRMED]` |
| Protocol contract | 不改变 EID、ring、MCTP/SPDM wire format 和 attestation 语义 | 本 CR 仅调整代码所有权 | `[CONFIRMED]` |

## 4. 受影响文件

| 范围 | 影响 |
|---|---|
| `components/security/src/spdm` | 新增 production service |
| `components/security/include/security/spdm` | 新增 public service API |
| `components/security/tests/qemu` | 删除 test-owned responder task/state |
| `components/security/tests/spdm` | 增加 service lifecycle/processing contract |
| component OpenSpec/docs | 记录部分替代 CR-0016 runtime ownership |

## 5. 需要替换的旧口径

CR-0016 中“双 ring、双 endpoint、SPDM responder、measurement、requester 和
task 全部迁移到 tests/qemu”替换为：

- 双 ring、requester endpoint、measurement、requester task 留在 tests/qemu；
- responder endpoint、SPDM responder 和 responder task 归生产 service。

CR-0016 的 QEMU-only public entry、requester self-delete、serial marker 和
runner 规则继续有效。

## 6. 不允许改变的内容

- Root of Trust、Host trust boundary、证书、签名和 measurement 安全语义。
- MCTP/SPDM wire format、固定 EID、Tag 和 shared ring 契约。
- 非 GSP/QEMU 构建行为。
- GSP `main.c` 的单一测试接口调用规则。

## 7. 验收标准

- [x] 生产目录存在 responder service API 和实现。
- [x] service 使用 static FreeRTOS task，重复 start 返回 `-EALREADY`。
- [x] QEMU 测试不再直接拥有 responder endpoint、adapter、TCB/stack/task。
- [x] host 测试与 GSP QEMU clean build 通过。
- [x] OpenSpec、代码指南、decision log 和 changelog 同步。

## 8. Codex 执行记录

| 项目 | 内容 |
|---|---|
| 执行时间 | 2026-06-11 |
| 修改文件 | production responder service；QEMU requester test；host contracts；component OpenSpec/guide；workflow traceability |
| 未完成项 | 真实 QEMU 串口 PASS 仍由既有 GSP address-map blocker 阻塞 |
| 执行说明 | 两轮 TDD RED/GREEN、完整 host 回归、GSP QEMU clean build、ELF symbol 和 strict OpenSpec validation 均完成 |
